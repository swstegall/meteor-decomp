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

## 2026-05-17 follow-up — Handler-vtable walk (deeper)

### Handler installation chain (decoded)

`SetPushEventConditionWithCircleReceiver` (Phase 9 #6 group A2.1) →
`FUN_006f2b70` (downstream pack-forward) → **`FUN_006f2310` (the
inserter)** → allocates **either 0x68 (104 B) or 0x64 (100 B)
bytes** based on a packet byte tag, then:

```c
if (packet[0] == byte_at_0x0134c3fe) {
    handler = (Handler68 *)operator_new(0x68);
    FUN_00892980(handler, ...);  // init handler68 — vtable = 0x1056edc
} else {
    handler = (Handler64 *)operator_new(0x64);
    FUN_00892b40(handler, ...);  // init handler64 — vtable = 0x1056f10
}
push_back(&npc[+0x108], handler);  // FUN_00725ed0 → std::vector::push_back
```

So **two handler classes coexist** — `Handler68` (104 B, vtable
`0x1056edc`) and `Handler64` (100 B, vtable `0x1056f10`). Both
classes are NOT in `class_metadata.json` (RTTI extractor didn't
pick them up — likely missing the COL signature).

### Handler vtable layouts (read from `.rdata`)

| Slot | Handler68 (`0x1056edc`) | Handler64 (`0x1056f10`) | Same? |
|---:|---|---|:---:|
| 0 | `0x00899910` (dtor68) | `0x00899970` (dtor64) | no |
| 1 | `0x00897570` | `0x00897570` | **yes** |
| 2 | `0x00712b40` | `0x00712b40` | **yes** |
| 3 | `0x00897660` | `0x00897660` | **yes** |
| 4 | `0x008977b0` | `0x008977b0` | **yes** |
| 5 | `0x008998b0` | `0x008998b0` | **yes** |
| 6 | `0x008998c0` | `0x008998c0` | **yes** |
| 7 | `0x00b73290` | `0x005c5c80` | no |
| 8 | `0x00899900` | `0x008993c0` | no |
| **9** | **`0x00894d30`** | **`0x00894d30`** | **YES ⭐** |
| 10 | `0x00892a80` | `0x00892c30` | no |
| 11 | `0x00898760` | `0x008988d0` | no |

**Slot 9 is identical** across both handler variants — `FUN_00894d30`
(421 B, "process push trigger"). This is **the same function listed
in `receiver_actorimpl_map.md` as the dispatcher path for
`ExecutePushOn{Enter,Leave}TriggerBoxReceiver`**.

### vtable[9] = `FUN_00894d30` does NOT write to `npc[+0x128]` directly

Decompiled the 421-byte function:

```c
void Handler::ProcessPushTrigger(this, packet, EBP_local, EDI_arg) {
    char prev_state = handler[+0x58];           // last frame's state
    char now = *(char*)packet;                  // this frame's state
    char global_idle = byte_at_0x012c3f77;      // "idle" sentinel
    
    bool entering = false, leaving = false;
    
    // 3-way state-transition matrix:
    if (prev_state == global_idle && now != global_idle 
        && handler[+0x62] != 0 && handler[+0x61] != 0) {
        entering = true;                        // off → on
    } else if (prev_state != global_idle && now == global_idle 
               && handler[+0x62] != 0 && handler[+0x61] != 0) {
        leaving = true;                         // on → off
    }
    
    if (entering) {
        // Fire enter trigger:
        FUN_008a3cf0(&local);
        FUN_008a41e0(&local, EDI, EBP, &handler[+0x4]);
        handler[+0x62] = 1;
    }
    if (some_other_flag) {
        // Calls engine context root (TWICE here and below):
        EAX = FUN_00cc7510(EDI);                // engine context root
        ECX = EAX.vtable[+4];                    // (loaded but seemingly unused)
        FUN_0057ab60(&local);                    // some context-root method
        FUN_008a3d70(...);                       // sub-helper
        FUN_008a4340(EBP, EDI);                  // sub-helper
        handler[+0x62] = 1;
    }
    handler[+0x58] = now;                        // save current state
    if (leaving) {
        // Symmetric "leave trigger" path
        FUN_008a3ce0(&local2);
        FUN_008a4150(&local2, EDI, EBP, &handler[+0x4]);
    }
    if (some_other_flag2) {
        // SECOND engine-context call (mirrors entering path):
        EAX = FUN_00cc7510(EDI);
        ECX = EAX.vtable[+4];
        FUN_0057ab60(&local3);
        FUN_008a3d00(...);
        FUN_008a4270(EBP, EDI);
    }
}
```

**Crucial finding**: `FUN_00894d30` does NOT write to `npc[+0x128]`
anywhere in its 421 bytes. The priming-write must happen DEEPER —
inside one of the sub-helpers (likely `FUN_008a41e0`, `FUN_008a4340`,
`FUN_008a4150`, or `FUN_008a4270`) or inside `FUN_0057ab60`.

The 2× calls to `FUN_00cc7510` (the engine context root getter) +
the loaded-but-unused `vtable[+4]` slot 1 strongly suggest the
engine context manipulation happens via downstream calls, not via
direct vtable dispatch from this function.

### Remaining gap

To find the actual `npc[+0x128]` write, walk one more level of the
sub-helpers (FUN_0057ab60, FUN_008a41e0, FUN_008a4340, FUN_008a4150,
FUN_008a4270). The chain is **>= 2 hops deep** from the handler
vtable[9], which is why the 1-hop receiver→writer search found
nothing.

## 2026-05-17 follow-up #2 — Sub-helper walk + 2 newly-identified clearers

### Sub-helper walk: vtable[9]'s downstream doesn't touch +0x128 either

Walked the 7 sub-helpers called by `FUN_00894d30` (handler vtable[9]):

| Function | Size | +0x128 touches | +0x12c touches |
|---|---:|---:|---:|
| `FUN_0057ab60` | 78 B | 0 | 0 |
| `FUN_008a41e0` | 129 B | 0 | 0 |
| `FUN_008a4340` | 203 B | 0 | 0 |
| `FUN_008a4150` | 129 B | 0 | 0 |
| `FUN_008a4270` | 203 B | 0 | 0 |
| `FUN_008a3d70` | 108 B | 0 | 0 |
| `FUN_008a3d00` | 108 B | 0 | 0 |

**ZERO of the direct sub-helpers touch +0x128 or +0x12c.** The chain
is ≥ 3 hops deep from the Push EventCondition handler vtable[9],
or — more likely — **Push event conditions are simply NOT the
primer**. Push triggers are about position/area entry, not target
tracking — so the absence of `+0x128` writes makes architectural
sense.

### Two newly-identified clearers in the Lua-actor area

Re-scanning the 26 candidate writers, two more relevant clearers
surface in the Lua-actor RVA range (0x2dxxxx..0x37xxxx):

#### `FUN_00703970` (414 B) — **selective despawn-clearer** ⭐

```c
// EBX = this (NpcBase); EAX = arg (some packet)
if (npc[+0x128] == *(uint32*)packet) {
    npc[+0x128] = NO_ACTOR;   // from [0x0130c778] — confirmed NO_ACTOR sentinel
}
if (npc[+0x12c] == *(uint32*)packet) {
    npc[+0x12c] = NO_ACTOR;
}
```

**This is "actor died/despawned, clear from kick state if it was
the current/previous target".** `[0x0130c778]` = the NO_ACTOR
constant (verified — matches the session-memory record
`NO_ACTOR sentinel = 0xE0000000 at VA 0x0130c778`).

The argument is a packet/identifier — likely the RAW actor ID of
the despawning actor. If garlemald sends a DeleteActor or
DespawnActor packet that matches the prev-target id, this clears
it — which would put `[+0x128]` back to NO_ACTOR, NOT prime it.

#### `FUN_00706700` (250 B) — **another clearer** (NOT a setter)

After full decompile, this is **also a clearer**:

```c
// EDI = this (NpcBase)
if (npc[+0x161] == 0) goto early_exit;   // state branch
// ... navigation chain through some sub-objects ...
npc[+0x130] = result_of_FUN_00cc73b0_call;
if (npc[+0x161] >= 0x15) {
    EDX = [0x0130c778];                  // NO_ACTOR sentinel
    npc[+0x128] = EDX;                   // ⭐ clear, NOT a real id
    FUN_00748870(&local, ...);
} else {
    FUN_00748920(EAX, &local);
}
```

EDX is loaded from `[0x0130c778]` = NO_ACTOR. So this is **a
state-machine clearer**: when `npc[+0x161] >= 0x15` (some state
threshold), clear +0x128 to NO_ACTOR + call FUN_00748870 (some
post-clear hook).

### Final architectural conclusion

**Every identified writer to `npc[+0x128]` in the Lua-actor RVA
range (0x2dxxxx..0x37xxxx) writes NO_ACTOR — NONE writes a real
actor id.** Specifically:

| Function | What it writes | When |
|---|---|---|
| `FUN_006e32f0` (76 B) | `NO_ACTOR` to BOTH +0x128 and +0x12c | `MyPlayer::vtable[66]` clearer (sharp tool) |
| `FUN_006f3650` (329 B, ctor) | `0` (not NO_ACTOR) | Object construction |
| `FUN_00703970` (414 B) | `NO_ACTOR` conditionally | Despawn-clearer: matches actor id |
| `FUN_00706700` (250 B) | `NO_ACTOR` conditionally | State-machine clearer: state >= 0x15 |

**Implication**: `npc[+0x128]` (the "previous kick target" field)
**cannot be primed via a packet receiver path** — there is no
static C++ code that writes a real actor id to it. The priming
must happen via one of these non-packet-receiver paths:

1. **Lua-engine binding** — Lua scripts call an engine-side
   binding that writes the field. The shipped Lua scripts (the
   `.le.lpb` packed corpus) presumably contain script-level
   `npc:setKickTarget(actor)` calls or equivalent.
2. **Bulk-copy/load-state** — `FUN_008f0a70` (under
   `EnterCriticalSection`) copies many fields including the
   +0x128 layout zone from a source struct to an NpcBase
   destination. This could be the "load saved NpcBase from
   serialized network blob" path.
3. **vtable computed dispatch** — receivers might compute the
   target slot dynamically (via Lua VM or function-pointer
   table) rather than via direct CALL — which would evade my
   1-hop and 3-hop CALL-graph searches.

### Strongest hypothesis (after the full walk)

**The priming is a Lua-engine binding called from the shipped
quest/event scripts.** SEQ_005's pmeteor implementation likely
calls something like `npc:setKickTarget(prev_target)` from script,
which routes through a Lua-bound setter that writes
`npc[+0x128]`. Garlemald's SEQ_005 port likely doesn't replicate
that script-level call, leaving +0x128 at NO_ACTOR and forcing
KickReceiver into the silent Branch B1 fall-through.

To definitively close, the next-step targets are:
1. **Search the decompiled `.lpb` corpus** (Phase 6 work) for any
   Lua call that maps to a npc method writing +0x128
2. **Walk `FUN_008f0a70`'s caller chain** to see if it's reachable
   from a network packet handler (would explain a load-state opcode)
3. **Runtime HWBP** is still the cleanest definitive answer

### Updated next-step priorities

| Path | Cost | Yield |
|---|---|---|
| Grep `.lpb` decompiled corpus for Lua bindings that touch `npc:*KickTarget*` or `npc:*Pre*` | Low (~30 min) | **Likely definitive** |
| Walk `FUN_008f0a70` callers (the bulk-copy under critical section) | Medium | Possibly definitive |
| Runtime trace (HWBP on `npc[+0x128]` during pmeteor SEQ_005 cinematic via Wine debugger) | Higher (Wine debugger setup) | Definitively conclusive |

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
