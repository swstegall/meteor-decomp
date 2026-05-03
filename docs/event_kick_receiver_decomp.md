# Phase 7 — `KickClientOrderEventReceiver` decomp + garlemald implications

> Last updated: 2026-05-04 (early morning) — slot 2 (Receive)
> static-decoded from asm dump. Triggered by 2026-05-03 garlemald
> smoke-test debugging where the SimpleContent man0g0 cinematic
> hung at "Now Loading" after talking to Yda the second time, even
> after fixing 4 server-side director-flow bugs.

## TL;DR for garlemald porters

The client's `KickClientOrderEventReceiver` (vtable `0xc574b0`,
5 slots) processes the `0x0131 KickEvent` packet. Slot 2 — the
actual receive entry, 207 bytes at RVA `0x0049e450` — does an
**actor lookup followed by a `+0x5c` flag check** on the kick
target. If the actor isn't in the client's actor list, OR if its
`+0x5c` byte is zero, **the kick is silently dropped**.

This means: when the server sends a KickEvent for an actor (e.g.
a director), the client MUST have already received + processed
the spawn packets for that actor. If the spawn packets were
nullified (e.g. by a subsequent `DeleteAllActors` packet wiping
the client's actor list), the kick fails silently and the
cinematic never starts → permanent "Now Loading" hang.

**garlemald's bug shape matches this** as of commit `ff74a0e`: in
`apply_do_zone_change_content`, the BattleNpc spawn packets fire
BEFORE the zone-in bundle dispatches `DeleteAllActors` (the
canonical content-area wipe). Pmeteor's
`SendInstanceUpdate(true)` re-broadcasts the actor list AFTER
the wipe; garlemald doesn't, so the post-wipe client has an
empty actor list when the KickEventPacket lands at the bundle's
end, and the kick silently drops.

## Vtable map (5 slots)

| Slot | RVA | Size | Role (inferred) |
|---|---|---|---|
| 0 | `0x004a1b90` | 30 B | Scalar deleting destructor (standard MSVC pattern: `CALL <body>; TEST [esp+8],1; JZ skip; PUSH ESI; CALL _free; skip:`) |
| 1 | `0x0049f530` | 125 B | `New()` factory — `PUSH 0x84` (size=132) → `operator new` → 6-arg ctor at `0x0089f2b0` constructing from member offsets `(ESI+8, ESI+0xc, ESI+0x68, ESI+0x10, ESI+0x14, ESI+0x6c)` |
| 2 | `0x0049e450` | **207 B** | **`Receive()` — the actor-lookup + flag-check entry. See per-slot analysis below.** |
| 3 | `0x0049d230` | 48 B | Auxiliary dispatch — `CALL 0x00cc7a50` (actor lookup) + `CALL 0x006ee680` (Director-base entry per Phase 6 doc — likely `_dispatchEvent` or similar) |
| 4 | `0x0049d260` | 15 B | Predicate — compares `[ECX+8]` against `[0x0130c778]` (a global), returns `SETNZ AL`. Likely a "is this kick targeting me?" check used in some dispatch loop. |

The 132-byte `sizeof` of the receiver class (slot 1 factory)
encodes the kick payload's per-field offsets:
- `+0x08` → ECX-receiver (the receiver instance pointer in the
  ctor call's first arg)
- `+0x0c` → some field (referenced by `LEA ECX,[ESI+0xc]`)
- `+0x10` → some field
- `+0x14` → some field
- `+0x68` → some field (referenced by both ctor + slot-2 receive)
- `+0x6c` → "trigger" or similar (referenced by slot 1 ctor's last arg AND by slot 2 Receive's lookup target)

The exact field shapes need a Ghidra GUI pass to confirm — see
"Open Ghidra GUI tasks" below.

## Slot 2 (`Receive`) — annotated

```asm
0x0049e450:
    MOV AL, [0x012c41af]        ; load global "default kick result" byte
    PUSH EBX
    MOV EBX, [esp+0xc]          ; arg0: out-result pointer (the byte the
                                ; receiver writes the dispatch result into)
    PUSH EBP
    MOV EBP, [esp+0xc]          ; arg1: lookup-context (per-server-tick
                                ; ActorRegistry-like)
    PUSH ESI
    MOV ESI, ECX                ; this = receiver instance
    PUSH EDI
    MOV ECX, EBX
    MOV [EBP], AL               ; pre-init out byte to default
    CALL 0x00cc7510             ; vtable trampoline (MOV ECX,[ECX]; JMP) —
                                ; resolves the receiver's parent dispatcher
    MOV ECX, [EAX]              ; load EAX→[EAX] = vtable
    MOV EAX, [ECX+0x4]          ; vtable[1] = some method
    MOV EDI, [EAX+0xc]          ; +0xc field of the dispatcher = "context
                                ; root", probably the LuaEngine instance
                                ; or DirectorRegistry root

    MOV EAX, [0x0130c778]       ; global "current player actor id" (or
                                ; similar — the player whose kick we're
                                ; about to dispatch)
    CMP [EDI+0x12c], EAX        ; check "is this kick for us?" — compare
                                ; the dispatcher's current-player slot
                                ; against the global player id
    JZ <0x0089e4b4>             ; branch if matches → the "self-targeted"
                                ; path (likely the cinematic-init path)

    ; Branch A: kick is NOT for the current player — secondary dispatch
    ADD ESI, 0xc                ; advance into receiver state past header
    PUSH ESI
    MOV ECX, EBX
    CALL 0x00cc7a50             ; ActorRegistry::lookup_actor(ECX, ESI)
                                ; — looks up the kick-target actor by id
    TEST EAX, EAX
    JZ <0x0089e4a2>             ; actor not found → write error byte, return
    CMP byte [EAX+0x5c], 0x0    ; check actor's "+0x5c" flag — likely an
                                ; "is_event_ready" / "spawned" / "active"
                                ; flag the spawn packet sets.
    JZ <0x0089e4a2>             ; flag clear → write error byte, return
    MOV ECX, EDI                ; ECX = dispatcher
    CALL 0x006e11d0             ; <unknown — likely "can-kick" gate>
    TEST AL, AL
    JZ <0x0089e4ab>             ; gate fail → return without writing result
    ; (fall through — kick succeeds)

  <0x0089e4a2:>                 ; failure label
    MOV DL, [0x0134c560]        ; load global "kick failure result code"
    MOV [EBP], DL               ; write it to the out-result byte
  <0x0089e4ab:>
    POP EDI
    POP ESI
    MOV EAX, EBP                ; return the out-result pointer
    POP EBP
    POP EBX
    RET 8                       ; cleanup 2 args (cdecl-call-clean? or
                                ; thiscall with 2 stack args)

    ; Branch B: kick IS for the current player — initialization path
  <0x0089e4b4:>
    CMP [EDI+0x128], EAX
    LEA EBP, [EDI+0x128]
    JZ <0x0089e4ef>
    LEA ECX, [ESI+0x6c]         ; load the "trigger" field
    CALL 0x0089e200             ; <internal> — inits the trigger handle
    PUSH EBP
    MOV ECX, EBX
    CALL 0x00cc7a50             ; ActorRegistry::lookup_actor again
    TEST EAX, EAX
    JZ <0x0089e4dc>
    CMP byte [EAX+0x5c], 0x0    ; SAME +0x5c flag check on this branch
    JNZ <0x0089e514>            ; flag set → success path
    ; ... (rest of the function, ~70 more bytes, similar structure)
```

## Critical findings

### 1. The `+0x5c` flag — actor "ready for events"

**Both branches** of slot 2 do the **same `CMP byte [EAX+0x5c], 0x0`
check** on the looked-up actor. This is THE gate. If the actor
either doesn't exist OR has `+0x5c == 0`, the kick is silently
dropped (the function returns with the failure-code byte written
to the out-result pointer; the caller — presumably the IpcChannel
dispatcher — has no way to surface "kick not delivered" to the
user).

**`+0x5c` is set by the actor spawn packets.** Specifically, the
spawn packet sequence (`AddActor` 0x00CA + the trailing
properties + the player-state-finalize) ends with whatever
SetActorState / SetActorIsZoning / etc. flips the actor into
"ready for events" mode. The flag corresponds (in C# terminology)
to `Actor.spawned` or `Actor.isInitialized`.

For garlemald: any KickEvent sent to a target actor that
**hasn't received its full spawn packet sequence on the client
yet** will silently drop. Particularly damning when the spawn
packets were nullified by a subsequent `DeleteAllActors`.

### 2. The `[0x0130c778]` global — current player actor id

The slot-2 entry compares `[EDI+0x12c]` against `[0x0130c778]` to
decide between "self-targeted" and "other-targeted" kick paths.
The `0x0130c778` global is very likely **the local player's actor
id** (the engine's `MyPlayer.actor_id`). Worth confirming with a
Ghidra GUI pass — if so, every kick path that checks "is this
for me" can be traced through this global.

### 3. `0x00cc7a50` — `ActorRegistry::lookup_actor`

Called twice in slot 2 + once in slot 3. Almost certainly the
client-side actor registry's "find by id" method. Worth naming
in Ghidra so future analysis of any actor-targeting receiver can
recognize it immediately.

### 4. `0x00cc7510` — vtable trampoline

`MOV ECX,[ECX]; JMP [vtable[X]]` pattern. Used to resolve the
receiver's parent-dispatcher vtable. Standard MSVC indirect-call
trampoline; not a real method.

## Implications for garlemald

**Tonight's "Now Loading hang" diagnosis** (post-`DoZoneChangeContent`,
client receives KickEvent for the new director but never starts
the cinematic):

The server-side flow IS correct as of commit `ff74a0e`:
1. `CreateContentArea` creates the director (registry-side).
2. `SpawnBattleNpcById` for yda + papalymo + 3 wolves spawns
   them in the area's actor pool (registry-side).
3. The zone-in bundle's `login director spawn packets prepended`
   step pushes director-spawn packets into the bundle.
4. The bundle dispatches: `DeleteAllActors` → `0x00E2(0x10)` →
   send_zone_in_bundle (player self-spawn + props) → `KickEventPacket`
   targeting the new director.

**Missing**: the BattleNpcs (and the director itself, if its
spawn packet was prepended BEFORE the wipe) are never re-spawned
to the client AFTER the `DeleteAllActors` wipes the actor list.
When the trailing `KickEventPacket` arrives, the client looks
up the director by id — finds nothing — silently drops the
kick. The cinematic never starts. The client sits at "Now
Loading" because the director's `noticeEvent` cinematic is the
trigger that would have cleared the load-screen overlay.

**Pmeteor's fix**: after `DeleteAllActors` in `DoZoneChangeContent`,
it calls `playerSession.UpdateInstance(aroundMe, true)` which
iterates the area's actors and re-broadcasts spawn packets for
each. This re-establishes the client's actor list before the
KickEvent arrives.

**garlemald port path**: in `apply_do_zone_change_content`, after
the zone-in bundle dispatches but BEFORE the trailing
`KickEventPacket` (or as part of the bundle's own final steps),
re-broadcast spawn packets for every actor in the destination
content area. The actor list is reachable via the
`PrivateAreaContent.area.core.actors` collection that
SpawnBattleNpcById populated.

## Open Ghidra GUI tasks

When the user is back at the Ghidra GUI, the following
investigations would close the remaining ambiguity in this
finding:

### Task A — Confirm `0x0130c778` is the local player actor id

Navigate to `0x0130c778` in the Ghidra data view. Check:
- Has it been auto-typed as an int / actor_id type?
- What other functions reference it? (Right-click → References →
  Find references to address.) The expected pattern is "anywhere
  in the code that does a 'is this me?' branch on actor id".
- Does the import path (Phase 1's RTTI dump or the `MyGameLoginCallback`
  state machine) name it explicitly?

### Task B — Confirm `0x00cc7a50` is `ActorRegistry::lookup_actor`

Navigate to RVA `0x00cc7a50`. The function should:
- Take `(ActorRegistry*, ActorId)` in `(ECX, ESI/stack)`
- Iterate or hash-lookup an actor map
- Return either NULL or a pointer to the actor object

If yes, name it accordingly in Ghidra and we can recognize this
pattern in 30+ other receivers that do the same lookup.

### Task C — Decode the `+0x5c` actor flag

The `EAX+0x5c` byte test is the kick-gate. Navigate to the
ActorBase (or whatever class the looked-up object is) — RVA
near the `0x00cc7a50` lookup. Look for:
- The struct definition's field at offset `+0x5c`
- The setter that flips this byte (likely called from the
  actor-spawn pipeline)
- The corresponding `0x...` opcode whose receiver flips the byte

If we can identify the spawn-side opcode that flips `+0x5c`,
garlemald can ensure that opcode's packet is sent for every
post-warp actor BEFORE the trailing KickEvent.

### Task D — Decompile slot 3 (48 B) to confirm it's the dispatch entry

Slot 3's pattern looks like the actual "dispatch this kick" entry
that wraps slot 2's gate check. Its `CALL 0x006ee680` likely
trampolines into `DirectorBase::OnEventStarted` (or whichever
DirectorBase slot is the "got a kick, run the noticeEvent body"
hook). Confirm via Ghidra's call-graph view.

## Cross-references

- `docs/director_base_hooks.md` — Phase 6 #8 doc for DirectorBase's
  34-slot vtable. Slot 3 of the receiver here probably trampolines
  into one of those director slots.
- `docs/director_quest.md` — Phase 6 architectural reframing.
- `garlemald-server/map-server/src/processor.rs::apply_do_zone_change_content`
  — the porting target for the spawn-rebroadcast fix.
