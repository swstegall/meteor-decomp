# Phase 9 #8c — `context_root[+0x128]` priming-packet hunt

> Recovered 2026-05-17. Phase 9 #8c sub-task of #8 (SEQ_005
> receiver-gate audit): find the pre-kick "receiver state init"
> packet/path that primes `context_root[+0x128]` (would shift
> KickReceiver's Branch B1 → Branch B2). Background: pmeteor seems
> to send a packet that garlemald doesn't; the result is garlemald
> hits Branch B1 and falls through to a silent no-op kick.

## TL;DR

**Substantially advanced** — `context_root` class definitively
identified, candidate writer set narrowed from 98 → 57 → 26, and
SetEventStatusReceiver flagged as the most likely indirect primer
via a 2+ hop dispatch chain. **Direct 1-hop primer not found via
static analysis** — receivers don't directly call [reg+0x128]
writers; priming happens either through vtable dispatch (computed,
not direct CALL) or through a multi-level chain.

## What's now confirmed

### `context_root` class identified

`Application::Lua::Script::Client::Control::NpcBase` (vtable
`0xfd647c`, RVA `0xbd647c`, **41 slots**). NpcBase has two ctors:
- `FUN_006f3650` (329 B) — writes initial state
- `FUN_006f37a0` (alternate ctor, 44 fields)

### NpcBase `[+0x128]`/`[+0x12c]` field semantics

Per the kick-receiver decomp (`docs/event_kick_receiver_decomp.md`):

| `[+0x128]` | `[+0x12c]` | State | Kick path |
|---|---|---|---|
| 0 | 0 | (Ctor default) | Treated as "Branch A: target exists" — needs further checks |
| `NO_ACTOR` (`0xE0000000`) | `NO_ACTOR` | Cleared (idle) | Branch B1: store target if `receiver[+0x80]` set, else no-op |
| `NO_ACTOR` | set | Primary kick in progress on `[+0x12c]` | Branch A: gate on `+0x5c` |
| set | `NO_ACTOR` | **Previous target stored, init pending** ⭐ | **Branch B2: look up `[+0x128]`, gate on `+0x5c`** |

The Phase 9 #8c question is: **what writes a real actor id to
`[+0x128]` while leaving `[+0x12c]` as `NO_ACTOR`?** That sets up
Branch B2 — which is what SEQ_005 needs garlemald to trigger.

### Writer-set narrowing

Static analysis over all `asm/ffxivgame/*.s`:

| Filter | Hits |
|---|---:|
| All `MOV [reg+0x128], ?` writes | 98 |
| Excluding `[ESP+0x128]` (local stack frame writes) | 57 |
| Functions writing to BOTH `[+0x128]` AND `[+0x12c]` | 26 |
| Receivers (1-hop direct CALL to a writer) | **0** |

### Known special-case writers (already identified)

| Function | RVA | Pattern | Role |
|---|---|---|---|
| `FUN_006e32f0` | `0x002e32f0` | `[+0x128] = NO_ACTOR; [+0x12c] = NO_ACTOR` | **Clearer** — `MyPlayer::vtable[66]`, sole writer of NO_ACTOR to both. See `docs/kick_dispatcher_clearer.md`. |
| `FUN_006f3650` | `0x002f3650` | Sets all fields to 0 incl. `[+0x128]/+0x12c` | **NpcBase ctor** — initial state is 0, not NO_ACTOR |
| `FUN_008e5ff0` | `0x004e5ff0` | Resets EDI to 0x12 fields incl. both | NpcBase reset/reinit |

### 26-candidate breakdown (write to BOTH +0x128 and +0x12c)

The 26 hits break into 4 categories:
- **Clearer (1)**: `FUN_006e32f0`
- **Ctor / reset (3)**: `FUN_006f3650`, `FUN_008e5ff0`, `FUN_00773270`
- **Bulk-copy / load-state (~10)**: e.g. `FUN_008f0a70` (copies many
  fields from source struct to NpcBase under critical-section lock —
  this is likely a "load NpcBase from serialized blob" path)
- **Misc (~12)**: vtable callbacks, internal state updates

The bulk-copy functions are the **most suspicious category for
priming**. `FUN_008f0a70` specifically (under `EnterCriticalSection`,
copies ~20 fields from `[ESI+0x8..0x428]` to `[EDI+0x118..0x15c]`)
fits the shape of "load saved NpcBase state from network or disk".

## SetEventStatusReceiver — the most-likely indirect primer

`SetEventStatusReceiver::Receive` (`FUN_0089d860`, 58 B) does:
```c
NpcBase *npc = __RTDynamicCast(actor, NpcBase);   // SrcType ActorBase
// ECX = npc; PUSH receiver[+0x59], receiver[+0x58], &receiver[+0x4], packet
FUN_006e67c0(npc, packet, receiver_internal, receiver_byte_a, receiver_byte_b);
```

`FUN_006e67c0` (113 B) is a 3-way switch on a packet byte:

```c
char tag = *(char*)packet;
if (tag == byte_at_012c3f7a) {
    handler = vector_find(npc + 0xe8, packet);    // first 16-byte slot
} else if (tag == byte_at_012c3f7c) {
    handler = vector_find(npc + 0xf8, packet);    // second 16-byte slot
} else if (tag == byte_at_012c3f7b) {
    handler = vector_find(npc + 0x108, packet);   // third 16-byte slot
}
if (handler) {
    handler->vtable[9](handler, packet, npc, packet_byte);  // process
}
```

NpcBase has **4 inline 16-byte event-handler vector slots** at
`+0xe8`, `+0xf8`, `+0x108`, `+0x118`. Each slot is a vector of
handler instances. `FUN_0071ca50` (the `vector_find` helper) does a
linear walk through the vector comparing each entry against the
packet payload via `FUN_00445d20`.

If the handler's `vtable[9]` writes to `npc[+0x128]`, that's the
priming path. Confirming this requires:
1. Identifying the handler class (the vector-element type)
2. Walking its vtable[9] for `[reg+0x128]` writes

The 4 inline slots at `+0xe8..+0x127` (= 4 × 0x10 bytes) end
**immediately before** `+0x128` — strongly suggesting the handlers
own/manage the state-machine field that follows them.

## What's still pending

To definitively close Phase 9 #8c, two paths:

| Path | Cost | Yield |
|---|---|---|
| Walk SetEventStatusReceiver handler chain to vtable[9] → look for `[reg+0x128]` writes | Medium | Likely definitive |
| Runtime trace (HWBP on `npc[+0x128]` during pmeteor SEQ_005 cinematic via Wine debugger) | Higher (Wine debugger setup) | Definitively conclusive |

The static-analysis path is closer — needs identifying the
event-handler class then its vtable[9].

## Action items for the parallel SEQ_005 garlemald session

Even without the definitive primer identified, the structural picture
points to two practical garlemald implications:

1. **The pmeteor "5 extra SetEventStatus pre-kick" packets** (per
   `project_garlemald_seq005_8a_findings.md`) likely ARE the
   primers. Each SetEventStatus could install/promote an
   event-handler in npc[+0xe8/+0xf8/+0x108] whose `vtable[9]`
   primes `npc[+0x128]`.

2. **Branch B2 trigger**: garlemald could SKIP the Lua-level handler
   chain entirely and emit a direct write to `npc[+0x128]` via a
   different opcode (or potentially via an `Init`-style opcode that
   pre-fills the event-handler vector). Decoding the 5 missing pmeteor
   packets' opcodes would identify which path is taken.

## Cross-references

- `docs/event_kick_receiver_decomp.md` — kick receiver 3-way branch
  + state machine spec
- `docs/kick_dispatcher_clearer.md` — `FUN_006e32f0` clearer
- `docs/event_status_condition_receivers_decomp.md` — SetEventStatus
  + SetNoticeEventCondition receivers (#8b)
- `docs/receiver_gate_cheatsheet.md` — Phase 9 #6/#7 (38 receivers)
- `docs/receiver_classes_inventory.md` — Phase 9 #1, #5, #8 task list
