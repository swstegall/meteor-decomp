# Phase 9 #8 — SEQ_005 cinematic packets vs receiver gates

> Last updated: 2026-05-15 (revised after #8b decomp). Cross-reference of
> every packet garlemald sends in the SEQ_005 cinematic body against the
> Phase 7- and Phase 9-decoded receiver gates. Identifies which gates are
> known, which are presumed satisfied, and which remain
> unverified — the latter are the prime suspects for the
> still-open SEQ_005 "Now Loading" hang.

## TL;DR

The two known gates from Phase 7 (`+0x5c` for Kick, `+0x7d` for
RunEventFunction) are both **theoretically satisfied** by garlemald's
existing spawn-packet sequence in
`build_director_spawn_subpackets`. But the post-warp smoke test
still hangs, with the client never echoing `IN 0x012D EventStart`
for the content director.

After Phase 9 #8b (2026-05-15) closed SetEventStatusReceiver +
SetNoticeEventConditionReceiver, the suspect list is **narrowed** to:

1. **Branch B1's `receiver[+0x80]` flag** (KickReceiver) — could cause
   the kick to silently no-op even with the actor flags set. Phase 7
   identified the existence of this flag, #8a mapped it to
   `(LuaParamsContainer at +0x6c)[+0x14]`, but the packet-byte source
   is still unidentified.
2. **Actor-lookup-before-spawn** — pmeteor sends the kick PRE-warp
   (target actor doesn't exist client-side yet). The client must
   somehow defer/queue the kick until the actor is spawned. Whatever
   defers it on pmeteor's side might fail on garlemald's.
3. **NEW (post-#8b): orphaned-conditions hypothesis** —
   SetNoticeEventConditionReceiver always runs but has a fallback
   path: if the target actor isn't already `DirectorBase`, the notice
   conditions get registered into `ActorBase[+0x118]` instead of
   `DirectorBase[+0x60]`. Garlemald's spawn sequence has
   `SetNoticeEventCondition` at step 2 — BEFORE `ScriptBind` at step
   8. If `ScriptBind` is what allocates the Lua-side `DirectorBase`
   instance, then for 6 ticks the conditions land in the wrong field,
   and a post-`ScriptBind` `DirectorBase` would have empty `[+0x60]`.
   The cinematic's notice-event evaluator would never trigger.

**Eliminated** (post-#8b):
- SetEventStatus has no actor-state gate.
- SetNoticeEventCondition has no actor-state gate either; its only
  silent-divergence mode is the fallback container-routing above.

Resolving #1 and #3 requires either runtime analysis OR Phase 9 #5
(opcode → receiver wiring discovery) AND a Phase 7 follow-up walking
`StartServerOrderEventFunctionReceiver` (the `ScriptBind` handler) to
see if it migrates the orphaned conditions.

## SEQ_005 cinematic packet inventory

For the man0g0 SEQ_005 combat tutorial, the post-warp cinematic
body emits these opcodes (from pmeteor's reference capture
`captures/pmeteor-quest/20260426-160210-gridania-manual3/`):

| Opcode | Direction | Receiver | Phase 7 gate | Garlemald wire-fmt | Smoke status |
|---|---|---|---|---|---|
| `0x012F` | OUT | KickClientOrderEventReceiver | `target_actor[+0x5c] != 0` AND target in registry | ✅ matches pmeteor | drops silently |
| `0x012D` | IN | (echo from client) | (the EventStart echo we expect) | n/a | **never fires** |
| `0x0130` | OUT | StartServerOrderEventFunctionReceiver | `target_actor[+0x7d] != 0` | n/a (never reaches here) | n/a |
| `0x0131` | OUT | EndClientOrderEventReceiver | (no actor-flag gate per Phase 7) | wire-fmt match | n/a |
| `0x0133` | OUT | (no Receiver class — handled by Lua RPC) | n/a | wire-fmt match | n/a |
| `0x0136` | OUT | SetEventStatusReceiver | ✅ #8b: NO actor gate; `dynamic_cast<NpcBase>(ctx)` (unguarded) → sub-vector lookup by event name | wire-fmt match | enables conditions |
| `0x016B` | OUT | SetNoticeEventConditionReceiver | ✅ #8b: NO actor gate; `dynamic_cast<DirectorBase>(ctx)` with fallback to `ActorBase[+0x118]` if cast fails — see "orphaned-conditions hypothesis" | wire-fmt match | registers conditions |
| `0x017A` | OUT | (no Receiver — Group::PacketProcessor consumes) | n/a (work-table mutate) | wire-fmt match | wire-correct |
| `0x017C/D/E/F` | OUT | (Group::PacketProcessor) | n/a | wire-fmt match (commit `47041f4`) | wire-correct |
| `0x0183` | OUT | (Group::PacketProcessor) | n/a | wire-fmt match (commit `dbcc19a`) | wire-correct |

## Garlemald's spawn-packet sequence vs gate satisfaction

`map-server/src/world_manager.rs::build_director_spawn_subpackets`
emits this sequence for the content director:

```
1. AddActor(0)                                     ← sets +0x5c (presumed)
2. SetNoticeEventCondition x3                      ← registers
3. SetActorSpeed
4. SetActorPosition
5. SetActorName
6. SetActorState
7. SetActorIsZoning(false)
8. ActorInstantiate (ScriptBind)                   ← sets +0x7d (presumed)
9. SetActorProperty(/_init)
10. SetEventStatus("noticeEvent"|"noticeRequest"|"reqForChild") x3
```

**Both `+0x5c` and `+0x7d` should be set** after this sequence
completes client-side. The sequence is comprehensive — every
packet pmeteor sends in its parallel content-director spawn
sequence (15:54:31.110 in the reference capture) is mirrored.

## The unresolved gate — Branch B1's `receiver[+0x80]` flag

Re-reading `docs/event_kick_receiver_decomp.md` slot 2 logic:

```c
if (context_root[+0x12c] != NO_ACTOR) {
    // BRANCH A: target IS set — check existing target's gates
    actor = ActorRegistry_lookup_actor(receiver_this + 0xc);
    if (actor == NULL || actor[+0x5c] == 0 || FUN_006e11d0() != 0)
        return FAILURE;
    return SUCCESS;
}
// target NOT set — init path
if (context_root[+0x128] == NO_ACTOR) {
    // BRANCH B1: completely fresh, no previous target
    if (receiver[+0x80] != 0) {                ← THIS CHECK
        context_root[+0x12c] = receiver[+0xc]; ← store target id
        return FAILURE;
    }
    // (else fall through → return success, no-op kick)
}
```

The `receiver[+0x80]` field is "primary kick" flag, set from
some byte in the KickEvent packet body. If it's non-zero, the
kick STORES the target id at `[+0x12c]` for a later retry. If
it's zero, the kick **silently no-ops** (the function fall-through
returns success WITHOUT storing anything).

For SEQ_005's post-warp kick:
- `context_root[+0x128] == NO_ACTOR` — fresh session post-warp,
  no previous target stored
- `context_root[+0x12c] == NO_ACTOR` — also fresh
- → Branch B1 fires
- → If `receiver[+0x80] == 0`, the kick is a silent no-op

**Phase 7 didn't decode which packet byte maps to
`receiver[+0x80]`.** This is a Phase 9 follow-up — the
KickEventPacket struct's offset must be reverse-mapped against
the receiver class's stack-allocated copy.

If garlemald's KickEvent has `0x00` at the byte that maps to
`receiver[+0x80]`, that's the smoking gun: every post-warp kick
silently no-ops. A byte-diff against pmeteor's KickEvent at the
right offset would confirm.

## What we know is byte-identical (won't help)

The pmeteor pcap byte-diff (today) confirmed:
- KickEvent body: byte-identical (`smoke-test #2 line 7913` vs `pcap line 31928`)
- SetActorProperty(currentContentGroup): byte-identical
- ContentGroup GroupHeader: byte-identical (after the 7-member fix)

So the issue isn't a wire-format gap on the OUT side. The kick
arrives at the client with the right bytes. The client just
doesn't dispatch it.

## What the smoke test confirms is missing

After the kick:
- Client never sends `IN 0x012D EventStart` for the content
  director (would normally fire ~2.28s post-warp per pmeteor)
- Client never sends `IN 0x0007 ZoneInComplete` (would normally
  fire ~2.6s post-warp per pmeteor)

So the client's post-warp state machine isn't progressing. The
kick dispatch is the gate that should drive it forward.

## Recommended Phase 9 follow-ups

| # | Task | Why |
|---|---|---|
| #8a | Map KickEvent packet byte offsets to receiver instance offsets (especially what byte maps to `receiver[+0x80]`) | Resolve the Branch B1 `receiver[+0x80]` mystery (🟡 partial — instance layout done; packet-byte source TBD) |
| #8b | Decode SetEventStatusReceiver slot 1 + SetNoticeEventConditionReceiver slot 1 | Both are critical to the cinematic; if either has gates we don't satisfy, the conditions don't actually enable (✅ done — neither has an actor-state gate; orphaned-conditions hypothesis surfaced) |
| #8d | Walk `StartServerOrderEventFunctionReceiver` (the `ScriptBind` handler, Phase 7-decoded but slot 2's body wasn't fully unrolled) to see if it migrates pre-bind notice conditions from `ActorBase[+0x118]` → `DirectorBase[+0x60]` after instantiating the derived Lua class | 🟡 partial — Phase 7 already decoded slot 2 as a pending-queue drainer, NOT a ScriptBind handler. Pivoted to Lua-actor-class ctor inspection (`docs/lua_actor_class_construction.md`): confirmed DirectorBase IS-A ActorBase, confirmed inheritance edges, confirmed ActorBase ctor zeros `+0x5c` (the kick gate). Orphaned-conditions hypothesis still unresolved — depends on Phase 9 #5 (dispatch_ctx wiring). |
| #8e | Find the `+0x5c` setter — scoped to ActorBase-derived class members | 🟡 partial (2026-05-16) — `docs/actor_5c_writer_decomp.md`. Search narrowed to 6 candidates (down from Phase 7's ~35 hits). Major reframing: confirmed the +0x5c byte is on the LUA-SIDE WRAPPER, NOT the engine-side C++ Actor (none of RaptureActor/CDevActor/CharaActor/SceneObject::Actor ctors touch +0x5c). Best static candidate (FUN_00766f00) doesn't fit a spawn-time writer profile. Definitive resolution needs Ghidra GUI decomp or runtime tracing. |
| **#8f (NEW)** | Walk the `AddActor` opcode (0xCA) handler to confirm which Lua-class wrapper gets constructed for different actor kinds | Direct answer to "is AddActor what constructs a `DirectorBase`, or does something later promote a plain `ActorBase`?" — answers the orphaned-conditions hypothesis indirectly |
| #8c | Look for pre-kick "receiver state init" packets — anything that sets `context_root[+0x128]` to a non-NO_ACTOR value would change the branch from B1 to B2 | Maybe pmeteor sends a packet that primes the receiver state |
| #5 (broader) | Find the script-load-time wiring that connects opcodes to receivers | Without knowing how a receiver becomes the recipient of a given opcode's bytes, we can't fully reason about packet→receiver dispatch |

## Cross-references

- `docs/receiver_classes_inventory.md` — Phase 9 #1 (full
  receiver inventory) + recovered Lua actor class hierarchy
- `docs/event_kick_receiver_decomp.md` — Phase 7 KickReceiver
  decomp (the source of the `+0x5c` finding + Branch B1 logic)
- `docs/event_run_event_function_receiver_decomp.md` — Phase 7
  (the source of the `+0x7d` finding)
- `docs/event_status_condition_receivers_decomp.md` — Phase 9 #8b
  (2026-05-15): SetEventStatus + SetNoticeEventCondition
  Receive bodies decoded; sources the orphaned-conditions hypothesis
- `docs/lua_actor_class_construction.md` — Phase 9 #8d (2026-05-16):
  ctor/dtor walk of every Lua actor base class; confirms inheritance
  edges + that ActorBase ctor zeros `+0x5c`
- `docs/actor_5c_writer_decomp.md` — **Phase 9 #8e (NEW 2026-05-16):
  scoped +0x5c writer hunt; eliminates engine-side actor hierarchy
  as the holder of the kick-gate flag; 6 candidates narrowed but
  none definitively identified**
- `docs/group_system_decomp.md` — Phase 8 (Group system + the
  no-receiver opcodes)
- `docs/network_dispatch_dual_paths.md` — Phase 8 #9 (the
  dual-path dispatch architecture)
- `garlemald-server/docs/post_warp_respawn_fix_analysis.md` — the
  garlemald-side application of these findings (still open as of
  2026-05-15)
