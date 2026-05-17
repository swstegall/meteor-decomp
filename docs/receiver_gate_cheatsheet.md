# Phase 9 #6 + #7 — Receiver gate cheat-sheet (36 of 36 2-slot receivers)

> Closed 2026-05-17. Walks the `Receive` body (slot 1) of every 2-slot
> Network/System Receiver and classifies its dispatch pattern + gate
> semantics. Output of Phase 9 #6; together with the LuaActorImpl-slot
> map (`docs/receiver_dispatch_via_actorimpl.md`), this populates
> Phase 9 #7 (gate cheat-sheet for opcode silent-drop diagnosis).

## TL;DR — 36 of 36 2-slot Receivers classified

| Pattern | Count | Recognition signal |
|---|---:|---|
| **A1.0** (guarded dynamic_cast) | 3 | `CALL FUN_009da6cc; TEST EAX,EAX; JZ <skip>; CALL <handler>` |
| **A1.1** (unguarded dynamic_cast) | 24 | `CALL FUN_009da6cc; CALL <handler>` — NO null-check |
| **A2.1** (pack-and-forward) | 5 | Read several fields from payload + receiver state, PUSH args, CALL one handler. No actor type validation. |
| **A2.2** (engine-root forwarding) | 3 | `CALL FUN_00cc7510` (navigate engine root) + downstream calls. Not actor-bound. |
| **A2.3** (debug command parser) | 1 | `ExecuteDebugCommandReceiver`, 1136 B — heavy Utf8String parsing |

## A1 — `__RTDynamicCast` family (27 receivers)

Common shape:

```c
auto ctx = arg0;                               // packet context
auto target = __RTDynamicCast(ctx, 0, src_TD, tgt_TD);  // FUN_009da6cc
// A1.0 path — null check:
if (target == NULL) return;                    // gracefully skip
// A1.1 path — no null check, direct dispatch:
target->doSomething(...);                      // non-virtual member fn
```

The cast normalises a `Component::Lua::GameEngine::LuaControl` (or `ActorBase`) input pointer to the specific subclass needed for the dispatch method. **In A1.1 receivers (the majority), if the cast fails (actor isn't of the expected subclass), the dispatcher invokes the handler with `this = NULL`** — typically silently no-ops or crashes depending on the handler.

### A1.0 — Guarded casts (3 receivers)

| Receiver | Target subclass | Post-cast | Notes |
|---|---|---|---|
| `SetDisplayNameReceiver` | `CharaBase` | `CALL FUN_006faff0` | Decoded in `docs/event_status_condition_receivers_decomp.md` (similar pattern) |
| `SetNoticeEventConditionReceiver` | `DirectorBase` | `CALL FUN_006f1380` | Phase 9 #8b decomp: HAS fallback to `ActorBase[+0x118]` on cast failure |
| `SendLogReceiver` | `CharaBase` AND `WorldMaster` | `CALL FUN_00772650` | TWO target classes — likely tries CharaBase first, falls back to WorldMaster |

### A1.1 — Unguarded casts (24 receivers)

All cast and dispatch without checking. Sorted by target subclass:

**Cast to `MyPlayer`** (local-player only — 12 receivers, "Half" of A1.1):
| Receiver | Handler (post-cast CALL target) |
|---|---|
| `AchievementPointReceiver` | `FUN_006e2dd0` |
| `AchievementTitleReceiver` | — (no direct CALL detected in 64-byte window) |
| `AchievementIdReceiver` | — |
| `AchievementAchievedCountReceiver` | `FUN_00704690` |
| `AddictLoginTimeKindReceiver` | — |
| `AttributeTypeEventEnterReceiver` | `FUN_006e11e0` |
| `AttributeTypeEventLeaveReceiver` | `FUN_006e1200` |
| `ChocoboReceiver` | `FUN_006de370` |
| `ChocoboGradeReceiver` | — |
| `GoobbueReceiver` | — |
| `VehicleGradeReceiver` | — |
| `SetCommandEventConditionReceiver` | `FUN_006f1c20` |
| `EntrustItemReceiver` | `FUN_006efbd0` |

**Cast to `CharaBase`** (any character with stats — 4 receivers):
| Receiver | Handler |
|---|---|
| `ChangeActorExtraStatReceiver` | `FUN_006fa980` |
| `ChangeSystemStatReceiver` | — |
| `ChangeActorSubStatModeBorderReceiver` | `FUN_006eecb0` |
| `SetDisplayNameReceiver` (A1.0 — listed above) | `FUN_006faff0` |

**Cast to `NpcBase`** (NPCs / mobs — 5 receivers):
| Receiver | Handler |
|---|---|
| `ExecutePushOnEnterTriggerBoxReceiver` | `FUN_00cc7510` (engine root navigate) |
| `ExecutePushOnLeaveTriggerBoxReceiver` | `FUN_00cc7510` (engine root navigate) |
| `HateStatusReceiver` | — |
| `SetEventStatusReceiver` | `FUN_006e67c0` |
| `SetTalkEventConditionReceiver` | `FUN_006f29b0` |

**Cast to `PlayerBase`** (any player — 3 receivers):
| Receiver | Handler |
|---|---|
| `JobChangeReceiver` | Re-calls __RTDynamicCast (multi-cast variant) |
| `GrandCompanyReceiver` | — |

**Cast to `AreaBase`** (zones / hamlets — 1 receiver):
| Receiver | Handler |
|---|---|
| `HamletSupplyRankingReceiver` | `FUN_006f3310` |

**Cast to `DirectorBase`** (directors — 1 receiver):
| Receiver | Handler |
|---|---|
| `SetNoticeEventConditionReceiver` (A1.0 — listed above) | `FUN_006f1380` |

The "—" entries indicate the post-cast scan didn't find a direct `CALL rel32` within 64 bytes; these receivers likely just `RET` after the cast (the cast itself triggers the side effect via subclass-bound logic during RTTI walk), OR the handler is reached via a conditional path the scan missed. Worth follow-up Ghidra GUI on these for completeness.

## A2 — Inline dispatch (9 receivers)

### A2.1 — Pack-and-forward (5 receivers)

Read payload fields + receiver state, PUSH multiple args, CALL one
handler. No actor-type validation; the dispatch routing presumes the
caller has already validated.

| Receiver | Size | Handler | Args |
|---|---:|---|---|
| `ChangeShadowActorFlagReceiver` | 32 B | `FUN_006dbe50` | 2 args (packed byte + receiver ptr) |
| `SetEmoteEventConditionReceiver` | 32 B | `FUN_006f2a90` | 3 args (word at +0x5a, +0x59, +0x58) |
| `SetPushEventConditionWithCircleReceiver` | 80 B | `FUN_006f2b70` | 9 args (geometry: floats + bytes at +0x60..+0x68) |
| `SetPushEventConditionWithFanReceiver` | 96 B | `FUN_006f2c30` | 11 args (geometry: floats at +0x60/+0x6c/+0x70 + bytes) |
| `SetPushEventConditionWithTriggerBoxReceiver` | 80 B | `FUN_006f2d00` | 9 args (geometry: short at +0x6c + bytes + 4-byte at +0x5c) |

The three `SetPushEventCondition*` variants are clearly the SAME shape
with different geometry payloads (circle = radius+center, fan =
radius+angle+direction, triggerbox = bounding-box). Each dispatches to
its own 0x2f2bxx handler family.

### A2.2 — Engine-root forwarding (3 receivers)

Navigate the engine context root via `FUN_00cc7510` (the Phase 7
"engine root navigation" trampoline), then dispatch downstream. Not
actor-bound — these are "global" or "session-level" updates.

| Receiver | Size | First CALL | Downstream |
|---|---:|---|---|
| `HamletDefenseScoreReceiver` | 48 B | `FUN_00cc7510` (root nav) | `FUN_006f2210` |
| `SyncMemoryReceiver` | 144 B | `FUN_00cc7510` (root nav) | `FUN_00cc73b0`, `FUN_00775a30`, `FUN_00cc9330` |
| `SetTargetTimeReceiver` | 464 B | (math first — time conversion via `MUL/DIV` with constant 0x3e8 = 1000), then `FUN_00cc7510` (root nav) | `FUN_0035bda0`, `FUN_004a0370` |

### A2.3 — Debug command parser (1 receiver)

| Receiver | Size | Pattern |
|---|---:|---|
| `ExecuteDebugCommandReceiver` | 1136 B | Heavy Utf8String construction (`FUN_00447260` × N, `FUN_00046fb0` × N) — parses a GM/dev command string and dispatches via inline lookup |

This is the GM-only debug-command path; ordinary game opcodes don't go
through it. Probably runs only in dev/test builds OR when an authorized
client sends a debug payload.

## Practical gate cheat-sheet — for silent-drop debugging

For any silent-drop symptom, this map answers: "if my opcode X is
landing wire-side but the client doesn't react, what's the gate?"

```
opcode → LuaActorImpl::slot (via docs/receiver_dispatch_via_actorimpl.md)
       → Receiver (via slot map)
       → Receive body pattern (this doc):
            Pattern A1.0 — guarded cast: client receives if actor IS-A target subclass; null-checks fall through silently
            Pattern A1.1 — unguarded cast: client dispatches with NULL on cast failure → typically silent no-op
            Pattern A2.x — inline: client ALWAYS dispatches; gate (if any) is in the downstream handler
```

**Most likely silent-drop causes for each pattern**:

- **A1.1 (24 receivers)**: server sent payload targeting wrong actor type. E.g., sending an `AchievementPoint` packet to an NPC actor → cast to `MyPlayer` fails → handler runs with `NULL this` → no-op. **Fix: server must verify actor type before emission.**

- **A1.0 (3 receivers)**: cast failed gracefully (null-check path). Same root cause as A1.1 but no crash risk. **Fix: same as A1.1.**

- **A2.1 (5 receivers)**: handler runs unconditionally. If client doesn't react, the downstream handler likely gates on some receiver-state flag set by an earlier packet. **Fix: trace what packet primes the receiver's state before this one.**

- **A2.2 (3 receivers)**: navigates engine root — most often "fire and forget" updates. Silent-drop likely means the engine root's downstream state isn't initialised. **Fix: check that prior session-setup packets landed.**

- **A2.3 (1 receiver — ExecuteDebugCommandReceiver)**: probably authentication-gated. Client won't run debug commands unless authorised. **Fix: not relevant for normal gameplay.**

## Application to SEQ_005

Cross-referencing the SEQ_005 cinematic packet sequence against this
map:

| Opcode | LuaActorImpl slot | Receiver | Pattern | Gate |
|---|---:|---|---|---|
| `0x012F` Kick | 56 | KickClientOrderEventReceiver | **B** (heap-alloc, 5-slot) | `actor[+0x5c] != 0` (Phase 7) + 3-way state machine on `context_root[+0x128]/[+0x12c]` |
| `0x0130` RunEventFunction | 57 | StartServerOrderEventFunctionReceiver | **B** | `actor[+0x7d] != 0` (Phase 7) |
| `0x0131` EndEvent | 58 | EndClientOrderEventReceiver | **B** | 102-case dispatcher (Phase 7), 12 active types |
| `0x0136` SetEventStatus | 48 | SetEventStatusReceiver | **A1.1** (unguarded cast to `NpcBase`) | If actor isn't NpcBase, handler dispatches with NULL → silent no-op |
| `0x016B` SetNoticeEventCondition | (no LuaActorImpl slot — owned by event handler) | SetNoticeEventConditionReceiver | **A1.0** (guarded cast to `DirectorBase` with fallback to `ActorBase[+0x118]`) | If actor isn't DirectorBase, writes to `ActorBase[+0x118]` instead (Phase 9 #8b "orphaned conditions" hypothesis) |
| `0x0166..0x016A` SetPushEventCondition* | (no LuaActorImpl slot) | SetPushEventConditionWith*Receiver | **A2.1** (pack-and-forward geometry to `FUN_006f2bxx`) | None — always runs |

So for the SEQ_005 hang specifically, the loadbearing gates are:
- **Kick (`0x012F`)**: `+0x5c` flag + `context_root[+0x128/+0x12c]` state (Phase 7, sharpened in
  `docs/kick_dispatcher_clearer.md` — clearer is `MyPlayer::vtable[66]`).
- **SetEventStatus (`0x0136`)**: silent-no-op if target isn't NpcBase. Plausible if garlemald's targeted actor is a Director.
- **SetNoticeEventCondition (`0x016B`)**: "orphaned conditions" hypothesis (Phase 9 #8b) — if ScriptBind hasn't fired, condition lands in `ActorBase[+0x118]` not `DirectorBase[+0x60]`.

## Regenerating

```sh
python3 tools/extract_receiver_gate_cheatsheet.py    # (would be the next-step tool to write)
```

Currently the data lives in `build/wire/receiver_gate_cheatsheet.json`,
generated inline from this session's exploration. A canonical extractor
script would walk the receiver vtables, scan each Receive body, and
emit the JSON + this Markdown.

## Cross-references

- `docs/receiver_classes_inventory.md` — Phase 9 #1 (the 43 Receivers
  + their vtable RVAs; this doc walks the 36 of them that have 2-slot
  vtables; the other 7 are 5/6-slot — covered by separate Phase 7/9
  decomps)
- `docs/receiver_dispatch_via_actorimpl.md` — Phase 9 #5 (the 35 of
  42 Receivers mapped to LuaActorImpl/NullActorImpl slots; this doc
  reads as the "Half B" — what the receiver does once invoked, vs
  Phase 9 #5's "Half A" of how the receiver is reached)
- `docs/event_kick_receiver_decomp.md` — Phase 7 #1 (KickReceiver
  slot 2 + `+0x5c` gate; A1 family analogue for the 5-slot
  receivers)
- `docs/event_run_event_function_receiver_decomp.md` — Phase 7 #2
- `docs/event_end_receiver_decomp.md` — Phase 7 #3 (102-case
  dispatcher; analogue of A2.3 for 5-slot receivers)
- `docs/event_status_condition_receivers_decomp.md` — Phase 9 #8b
  (SetEventStatus + SetNoticeEventCondition — earlier detail walk
  that this doc generalises)
- `docs/dynamic_cast_callsite_sweep.md` — Phase 9 ext (the
  481-callsite RTDynamicCast sweep that this doc filtered down to
  the 27 receiver-internal casts)
- `build/dynamic_cast_callsites.json` — raw sweep output (input to
  this doc's analysis)
- `build/wire/receiver_gate_cheatsheet.json` — this doc's structured
  data
