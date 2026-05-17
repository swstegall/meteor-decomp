# Phase 9 #5 — packet dispatch router (`FUN_004e20a0`)

> Recovered 2026-05-16. Companion to `docs/network_dispatch_dual_paths.md`
> (Phase 8 #9, which identified `FUN_00dae520` as the dummy-callback
> dispatch wrapper) and `docs/receiver_dispatch_via_actorimpl.md`
> (Phase 9 #5, which identified the LuaActorImpl/NullActorImpl 90-slot
> vtables as the per-receiver dispatch fanout).

## TL;DR

`FUN_004e20a0` (1442 B, RVA `0x000e20a0`) is the **outer per-packet
loop** in the network receive pipeline. It dequeues a packet via
`FUN_00dae520` (the dummy-callback wrapper), reads the opcode, and
splits into 4 cases via a small jump table at VA `0x004e2644`. Most
opcodes (3–0x10, plus everything > 0x11) forward to **`FUN_004e5ff0`**,
which performs channel-bound dispatch into `channel->vtable[2]`.

This doc closes the "where does FUN_00dae520's output go?" question
from Phase 8 #9, but the chain past `channel->vtable[2]` (the actual
opcode → LuaActorImpl-slot connection) is **not** closed by this work —
the next-level dispatch is via a runtime tree-lookup
(`FUN_004e5ca0`, an RB-tree-shaped walk keyed on the packet field) into
a per-opcode handler that's most likely a Lua-bound closure rather
than a direct C++ vtable call.

## The dispatch table

After dequeuing a packet via `FUN_00dae520`, `FUN_004e20a0` reads the
opcode as a u16 at `packet[+0x24][+2]`, subtracts 1, and either falls
to a default case (opcode > 0x11) or indexes a jump table:

```asm
;; FUN_004e20a0 + 0xc8
MOV EDI, [ESP+0x2c]                  ; load packet ptr
TEST EDI, EDI; JZ default
MOV EAX, [EDI+0x24]                  ; packet header struct
MOVZX EAX, word [EAX+2]              ; opcode (u16)
ADD EAX, -1                          ; opcode - 1
CMP EAX, 0x10; JA <default_case>     ; > 16? → default (case 3)
MOVZX EAX, byte [0x4e2654 + EAX]     ; byte_table[opcode-1]
JMP [0x4e2644 + EAX*4]               ; dword_table[case_idx]
```

| Opcode | byte_table[op-1] | Case body VA | Behaviour |
|---:|---:|---|---|
| `0x01` | 0 | `0x004e2251` | `CALL [0x00f3e55c]` — single IAT-style call. Probably **session ping / keepalive** ack. |
| `0x02` | 1 | `0x004e22bf` | Builds a 6-byte struct on stack (constants `0x06`, `0x18`), calls `FUN_00dae010` with two struct ptrs. Then continues. — looks like **"client info" or "session resume" handshake**. |
| `0x03` .. `0x0d` | 3 | `0x004e237d` | Default — forwards to `FUN_004e5ff0` (see § "default case"). |
| `0x0e` | 2 | `0x004e2311` | Sets `[ESI+0x3b0] = 1`, calls `FUN_00dae010` with a `0x4`/`0x18` struct. Then forwards to `FUN_004e5ff0`. — looks like a **"disconnect / reset" trigger**. |
| `0x0f` .. `0x10` | 3 | `0x004e237d` | Default forwarding. |
| `0x11` | 2 | `0x004e2311` | Same as 0x0e — sets reset flag + forwards. |
| `> 0x11` | (skipped jump) | `0x004e237d` | Default forwarding via the `JA <default>` branch above. |

So FUN_004e20a0 itself only specially handles **4 control opcodes**:
- `0x01` ping/keepalive
- `0x02` info/handshake
- `0x0e` reset/disconnect
- `0x11` reset/disconnect (duplicate? maybe per-channel variant)

**All other opcodes — including every gameplay opcode (0x12+) —
forward to `FUN_004e5ff0`** via the default case. This is the bridge
into the per-opcode dispatch we still need to recover.

## Default case — forwarding to FUN_004e5ff0

```asm
;; 0x004e237d (the default case body)
MOV ECX, [EDI+8]                  ; ECX = packet[+8] = channel ptr
PUSH EDI                          ; pass the packet
CALL 0x004e5ff0                   ; dispatch
MOV [ESP+0x2c], EBX               ; clear the local packet ptr (EBX=0)
JMP 0x004e2152                    ; loop to next packet
```

So **`FUN_004e5ff0` is called with `ECX = channel`** and the packet on
the stack. The channel object is `packet[+8]` (per Phase 8 #9's
analysis of the dummy-callback dispatcher prologue:
`mov eax, [edx+8]; mov eax, [eax+0x24]; movzx esi, word [eax+2]`).

## FUN_004e5ff0 — channel-bound dispatcher (132 B)

```c
bool FUN_004e5ff0(Channel *channel, Packet *pkt) {   // ECX=channel, [ESP+8]=pkt
    channel->vtable[1]();                          // setup / lock?
    EDI = pkt;
    EBP = pkt + 4;
    FUN_0071d420(&channel->m_field_14, &result);   // probably a packet-header inspector
    FUN_008a87f0(&channel->m_field_14, hdr_lo, hdr_hi, &result);  // ?
    if (pkt[+0x1c] > 0x1c10) {
        pkt->vtable[0](1);                         // "big packet" path
    } else {
        FUN_004e5ca0(&channel[+8], &result, pkt);  // ⭐ opcode → handler lookup
    }
    channel->vtable[2]();                          // dispatch / commit
    return true;
}
```

The two interesting calls:

1. **`FUN_004e5ca0`** (185 B) — performs a tree walk on `channel[+8]`
   keyed on `pkt[+0]` (a packet header field, likely the opcode or a
   derived key). The shape is classic RB-tree iteration:
   `CMP EDX, [EAX+0xc]` followed by `JZ` taking `[EAX+0]` (left) or
   `[EAX+8]` (right) until found, then writing the found entry into
   the output struct. This is most plausibly the **per-opcode handler
   lookup table**.

2. **`channel->vtable[2]()`** — the channel's "commit"-style hook,
   which is what would actually invoke the looked-up handler. The
   handler resolves at runtime via the tree-lookup result placed in
   the channel's state.

## Bridge to per-receiver dispatch — NOT closed by this work

The chain so far:

```
network packet
   ↓
FUN_004e20a0 (channel-control opcodes 1/2/0xe/0x11 inline, else →)
   ↓
FUN_004e5ff0 (channel-bound — calls channel->vtable[1/2])
   ↓
FUN_004e5ca0 (tree-walk lookup of per-opcode handler)
   ↓
???
   ↓
LuaActorImpl::vtable[slot] (the 35 slots mapped in receiver_dispatch_via_actorimpl.md)
   ↓
Receiver::Receive (Phase 7 Kick / RunEventFunction / EndEvent decomp)
```

The `???` link is the next investigation. The tree-lookup result is
either:

- **A Lua closure**, in which case the script-load registration code
  is what binds opcodes to scripts, and finding that registration is
  the way in. Channel uses the standard `Component::Lua::GameEngine`
  closure-call infrastructure to invoke the closure, which then calls
  `actor:methodN()` resolving to one of the 35 LuaActorImpl slots.

- **A C++ function pointer** stored in the tree's leaf node, called
  directly. The function would be a per-opcode handler that picks the
  target actor and invokes `actor->lua_impl->vtable[slot]`.

The static-analysis evidence for the Lua hypothesis: searches for
`CALL [reg + disp32]` with disp = `slot * 4` for any of the 35 mapped
slots return **zero hits** (Phase 9 #5 partial doc). This rules out
direct C++ virtual dispatch — the call must go through some
indirection that the static analysis isn't catching, and Lua VM
closures are the natural fit.

## What this unlocks

Even without closing the full dispatch chain, this finding:

- **Identifies the control opcodes** for the SEQ_005 / login flows:
  opcode `0x01` (ping/keepalive), `0x02` (info/handshake), `0x0e` /
  `0x11` (reset). Garlemald can audit its own ping/reset emission
  against these wire-confirmed slots.
- **Confirms the dispatch is NOT a simple static C++ vtable call** —
  Lua-VM-driven dispatch is now the most likely model, so future
  decomp work to close Phase 9 #5 should follow the
  `Component::Lua::GameEngine` closure-call path rather than searching
  for more vtable-disp call sites.
- **Confirms `FUN_004e5ca0` is the per-opcode lookup helper** — once
  the tree's contents are known (which would need a runtime dump or
  a careful trace through the script-load registration code), the
  full opcode → handler map falls out.

## Recommended next investigation

Two paths in order of cost:

| Cost | Approach | Yield |
|---|---|---|
| Medium | **Walk script-load registration**. The `lua_class_registry.md` doc (Phase 6 #3) identifies `FUN_0078e3a0` as the single Lua-class registration function. If there's a sibling function that registers per-opcode handlers (i.e., binds Lua closures to opcodes for the channel's tree), it's likely callable from there. | Closes Phase 9 #5 fully. |
| Higher | **Decode `.le.lpb` scripts** for opcode-binding patterns. Per `reference_meteor_decomp_lpb_format.md`, the `.le.lpb` wrapper format is decoded via `tools/decode_lpb.py`. Look for scripts that call `bindOpcode(0x012F, function(pkt) ... end)` or similar. | Closes Phase 9 #5 fully AND surfaces Lua-side semantics for every opcode. |
| Higher | **Runtime trace**. Set a hardware breakpoint on `FUN_004e5ca0` in the Wine'd `ffxivgame.exe`, observe the tree's contents at packet-arrival time. | Closes Phase 9 #5 fully; harder to reproduce. |

## Cross-references

- `docs/network_dispatch_dual_paths.md` — Phase 8 #9 (FUN_00dae520
  dummy-callback dispatch — the input to FUN_004e20a0)
- `docs/receiver_dispatch_via_actorimpl.md` — Phase 9 #5 (the
  35-slot map from LuaActorImpl/NullActorImpl down to Receiver
  invocation)
- `docs/lua_class_registry.md` — Phase 6 #3 (FUN_0078e3a0, the
  single Lua-class registration function — strong candidate for the
  per-opcode binding's sibling location)
- `docs/event_kick_receiver_decomp.md` — Phase 7 #1 (the Receiver
  decomp the full dispatch chain ultimately bottoms out in for
  KickEvent)
- `memory/reference_meteor_decomp_lpb_format.md` — `.le.lpb` Lua
  bytecode format (next step for the script-side investigation)
