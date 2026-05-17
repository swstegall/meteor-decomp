# `FUN_0070ab40` — opcode-translation table

> Recovered 2026-05-16 while hunting Phase 9 #5's last rung
> (per-opcode → Lua-closure binder). NOT the dispatch table I was
> looking for — but a useful independent recovery worth documenting.

## TL;DR

`FUN_0070ab40` is a **740-case opcode translation table**, keyed on
`opcode - 100`, that maps an input opcode in the range [100, 2082] to
some output value (usually a smaller opcode, sometimes zero). Each
case body is exactly 6 bytes: `MOV EAX, <constant>; RET`. Opcodes
NOT in the case set return 0 (the default `XOR EAX, EAX; RET`).

**Critically, the SEQ_005 event opcodes (0x012F Kick, 0x0130
RunEventFunction, 0x0131 EndEvent, 0x0136 SetEventStatus, 0x016B
SetNoticeEventCondition) are NOT in this table** — they all map to
the default 0. So this is NOT the per-opcode → receiver dispatcher
that Phase 9 #5 still needs to find.

## Structure

```asm
;; FUN_0070ab40 — prologue (24 bytes)
MOV EAX, [ESP+4]                  ; load arg = opcode
ADD EAX, -0x64                    ; opcode - 100
CMP EAX, 0x7BE                    ; > 0x7BE (= 1982)?
JA <default>                      ; → default (XOR EAX,EAX; RET)
JMP [EAX*4 + 0x70bcb4]            ; index jump table

;; Case bodies, contiguous starting at 0x30ab59, 6 bytes each:
;;   B8 XX XX XX XX               ; MOV EAX, <constant>
;;   C3                           ; RET
```

Verified case bodies (sample):

| Input opcode | Case offset | Body | Returns |
|---:|:---|:---|---:|
| `0x65` (101) | `0x30ab59` | `b8 01 00 00 00 c3` | `1` |
| `0x66` (102) | `0x30ab5f` | `b8 02 00 00 00 c3` | `2` |
| `0x25B` (603) | `0x30af19` | `b8 2f 01 00 00 c3` | **`0x12F`** (Kick wire opcode!) |
| `0x6A3` (1699) | `0x30bcab` | `b8 92 03 00 00 c3` | `0x392` |
| `0x12F` (303) — not in table | `0x30bcb1` | `33 c0 c3` | `0` |

## What the table covers

The 740 cases group into contiguous opcode bands with notable gaps:

| Range | Count | Notes |
|---|---:|---|
| `0x65 .. 0x76` | 18 | Probably chat/system opcodes |
| `0xc5 .. 0x127` | 99 | Actor / data-update family |
| `0x186 .. 0x19c` | 23 | Includes garlemald's recent net-new builders (`0x190` Mass Item Modifier, `0x195` Set Enmity Indicator, etc.) |
| `0x1F2 .. 0x1FF` | 14 | |
| `0x250 .. 0x288` | 57 | Includes the cases that **emit 0x12F/0x130/0x131 as output** (input 0x25B/0x25C/0x25D → output Kick/Start/End) |
| `0x2AB .. 0x32B` | 129 | |
| `0x36F .. 0x392` | 36 | |
| `0x3E5 .. 0x452` | 110 | |
| `0x484 .. 0x4CA`, `0x495 .. 0x4A2`, etc. | 87 | |
| `0x501 .. 0x507`, `0x514 .. 0x51E` | 18 | |
| `0x578 .. 0x57E` | 7 | |
| `0x5DB .. 0x61D` | 67 | |
| `0x687 .. 0x6A3` | 29 | |
| `0x7D1 .. 0x822` | 82 | High-range (item-related?) |

**Notable gap**: opcodes `0x128 .. 0x185` are ALL absent — this is
exactly the **event-receiver opcode range** (Kick `0x012F`,
RunEventFunction `0x0130`, EndEvent `0x0131`, SetEventStatus `0x0136`,
SetNoticeEventCondition `0x016B`, the SetPushEventConditionWith*
family `0x0166..0x016A`, etc.). These opcodes don't appear in
FUN_0070ab40's input set — they have to be dispatched somewhere else.

The reverse direction is more interesting: input opcodes `0x25B..0x25D`
emit the event-receiver opcodes (`0x12F`/`0x130`/`0x131`) as output.
This is consistent with FUN_0070ab40 being a **protocol-version
translation table** — newer (or alternate) protocol IDs in the 0x250+
range translate down to the canonical 1.x event opcodes.

## Callers

`FUN_0070ab40` has 9 direct callers, all in the Lua-engine /
script-host neighborhood:

| Caller | RVA |
|---|---|
| `FUN_006edb70` | `0x2edb70` |
| `FUN_006edea0` | `0x2edea0` |
| `FUN_006edee0` | `0x2edee0` |
| `FUN_00705a70` | `0x305a70` |
| `FUN_00705bd0` | `0x305bd0` |
| `FUN_00705c70` | `0x305c70` |
| `FUN_00705eb0` | `0x305eb0` (×2) |
| `FUN_00706160` | `0x306160` |

These are all in the 0x2ed/0x305/0x306 range — sibling to LuaActorImpl
(0x35a-0x36c) and the Lua-engine helpers (0x38-0x39). Probably
script-side protocol-version negotiation: when a script wants to
trigger a wire emission, it can use either the canonical wire opcode
or a newer-protocol equivalent; FUN_0070ab40 normalizes between them.

## Why Phase 9 #5's last rung remains open

The per-opcode → LuaActorImpl-slot dispatch I was hunting would have
all 35 mapped receivers as inputs (0x12F Kick, 0x130 RunEvent, 0x131
End, 0x136 SetEventStatus, etc.) and emit slot indices (56, 57, 58,
48) or LuaActorImpl::vtable[slot] function pointers as output. This
table does neither — it doesn't even MENTION the receiver opcodes as
input keys, and its outputs are other opcodes (not slot indices).

Where else to look:

1. **Per-channel registration**. The channel's tree (walked by
   `FUN_004e5ca0` per `docs/packet_dispatch_router.md`) is the most
   likely site of per-opcode binding. Need to find what writes to
   `channel[+8]` (the tree root). The channel class itself isn't
   identified yet — finding its ctor + vtable is the next step.

2. **Lua-script `bindOpcode` style binders**. If receivers are wired
   via Lua-side bindings, the registration would be in `.le.lpb`
   bytecode (other session's territory) rather than in PE code.

3. **The OTHER 5 RECEIVER-OPCODE → FUN_0070ab40 output mappings**
   that I found (input `0x25B/0x25C/0x25D` → output `0x12F/0x130/0x131`)
   might point at an alternate dispatch pathway. Walking the callers
   of `FUN_0070ab40` for context would reveal whether the translation
   is consumed by a downstream dispatcher.

The negative result here is itself useful — Phase 9 #5's last rung is
NOT in this part of the binary; the hunt narrows to channel
construction code and/or Lua-script binders.

## Independent value

Even without closing #5, this finding has standalone value:

- **Opcode translation table is documented** — anyone porting wire
  opcodes can refer to the table to understand which inputs translate
  to canonical wire opcodes vs which are passthrough vs which return 0.
- **The 0x12F/0x130/0x131 alias mappings (input `0x25B/0x25C/0x25D`)**
  are recovered, useful if garlemald or pmeteor's clients ever observe
  these alternate-form opcodes.
- **`FUN_0070ab40`'s structure** (6-byte case bodies with consistent
  `MOV EAX, imm32; RET` shape) is now a known template — could feed
  into the Phase 2.5 template-derivation pipeline if many similar
  translation tables exist.

## Cross-references

- `docs/packet_dispatch_router.md` — Phase 9 #5 outer router decomp
  (`FUN_004e20a0` → `FUN_004e5ff0` → `FUN_004e5ca0`); identifies the
  channel-tree lookup as the next investigation target
- `docs/receiver_dispatch_via_actorimpl.md` — Phase 9 #5 partial (35
  of 42 Receivers mapped to LuaActorImpl slots; this doc was hunting
  the missing inverse — opcode → slot lookup — and ruled out
  FUN_0070ab40 as that function)
- `docs/lua_class_registry.md` — Phase 6 #3 (FUN_0078e3a0, the
  Lua-class registrar; FUN_0078fc90 is its caller and contains
  Lua-module alias registration in FUN_0078fad0)
