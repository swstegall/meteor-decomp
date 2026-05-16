# Phase 9 #8 — SEQ_005 cinematic packets vs receiver gates

> Last updated: 2026-05-15. Cross-reference of every packet
> garlemald sends in the SEQ_005 cinematic body against the
> Phase 7-decoded receiver gates. Identifies which gates are
> known, which are presumed satisfied, and which remain
> unverified — the latter are the prime suspects for the
> still-open SEQ_005 "Now Loading" hang.

## TL;DR

The two known gates from Phase 7 (`+0x5c` for Kick, `+0x7d` for
RunEventFunction) are both **theoretically satisfied** by garlemald's
existing spawn-packet sequence in
`build_director_spawn_subpackets`. But the post-warp smoke test
still hangs, with the client never echoing `IN 0x012D EventStart`
for the content director. Three unresolved suspect mechanisms:

1. **Branch B1's `receiver[+0x80]` flag** — could cause the kick
   to silently no-op even with the actor flags set. Phase 7
   identified the existence of this flag but not what packet byte
   maps to it.
2. **Actor-lookup-before-spawn** — pmeteor sends the kick
   PRE-warp (target actor doesn't exist client-side yet). The
   client must somehow defer/queue the kick until the actor is
   spawned. Whatever defers it on pmeteor's side might fail on
   garlemald's.
3. **Some receiver state initialization that's missing** —
   pmeteor may set up receiver-instance state via a packet we
   don't send (e.g. an early SetCommandEventCondition or similar
   that primes `context_root[+0x128]`).

Resolving these requires either runtime analysis OR Phase 9 #5
(opcode → receiver wiring discovery).

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
| `0x0136` | OUT | SetEventStatusReceiver | (2-slot — TBD; likely no actor gate) | wire-fmt match | enables conditions |
| `0x016B` | OUT | SetNoticeEventConditionReceiver | (2-slot — TBD) | wire-fmt match | registers conditions |
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
| #8a | Map KickEvent packet byte offsets to receiver instance offsets (especially what byte maps to `receiver[+0x80]`) | Resolve the Branch B1 `receiver[+0x80]` mystery |
| #8b | Decode SetEventStatusReceiver slot 1 + SetNoticeEventConditionReceiver slot 1 | Both are critical to the cinematic; if either has gates we don't satisfy, the conditions don't actually enable |
| #8c | Look for pre-kick "receiver state init" packets — anything that sets `context_root[+0x128]` to a non-NO_ACTOR value would change the branch from B1 to B2 | Maybe pmeteor sends a packet that primes the receiver state |
| #5 (broader) | Find the script-load-time wiring that connects opcodes to receivers | Without knowing how a receiver becomes the recipient of a given opcode's bytes, we can't fully reason about packet→receiver dispatch |

## Cross-references

- `docs/receiver_classes_inventory.md` — Phase 9 #1 (full
  receiver inventory)
- `docs/event_kick_receiver_decomp.md` — Phase 7 KickReceiver
  decomp (the source of the `+0x5c` finding + Branch B1 logic)
- `docs/event_run_event_function_receiver_decomp.md` — Phase 7
  (the source of the `+0x7d` finding)
- `docs/group_system_decomp.md` — Phase 8 (Group system + the
  no-receiver opcodes)
- `docs/network_dispatch_dual_paths.md` — Phase 8 #9 (the
  dual-path dispatch architecture)
- `garlemald-server/docs/post_warp_respawn_fix_analysis.md` — the
  garlemald-side application of these findings (still open as of
  2026-05-15)
