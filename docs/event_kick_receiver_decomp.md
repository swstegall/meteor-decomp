# Phase 7 — `KickClientOrderEventReceiver` decomp + garlemald implications

> Last updated: 2026-05-04 — slot 2 (Receive) static-decoded from
> asm dump; Task A confirmed `0x0130c778` is the `0xE0000000`
> `NO_ACTOR` sentinel; Task B confirmed `FUN_00cc7a50` is
> `ActorRegistry::lookup_actor` with a two-collection split; Task
> B.1 confirmed `FUN_00cd80f0` is a 7-byte navigation thunk into a
> nested id-classifier sub-object; Task B.2 confirmed the partition
> predicate at `FUN_00d035d0` is a TYPE-TAG-based metadata lookup
> (collection B = "actor type tag == `0x0F`", everything else =
> collection A) — the partition is runtime-assigned by spawn-side
> opcodes, not derived from the id namespace. Triggered by
> 2026-05-03 garlemald smoke-test debugging where the SimpleContent
> man0g0 cinematic hung at "Now Loading" after talking to Yda the
> second time, even after fixing 4 server-side director-flow bugs.

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

> 📐 **Addressing convention** (clarified 2026-05-04 after a
> Ghidra GUI mix-up): meteor-decomp's asm dumps and the
> `ffxivgame.net_handlers` index use TWO addressing schemes that
> are easy to confuse:
>
> - **`fn_rva`** = offset from image base 0 (e.g. `0x0049e450`).
> - **`fn_name`** = absolute address with image base `0x00400000`
>   applied (e.g. `FUN_0089e450` → absolute `0x0089e450`).
>
> Ghidra loads the binary at its actual image base of `0x00400000`,
> so it always shows **absolute addresses**. To navigate from a
> meteor-decomp `fn_rva` to the right Ghidra location, ADD
> `0x00400000`. The table below now lists both columns so the
> mapping is unambiguous.
>
> The asm dump in §"Slot 2 (`Receive`) — annotated" uses absolute
> addresses everywhere (the JZ targets like `0x0089e4b4` are
> absolute) — those numbers ARE the right Ghidra navigation targets.

| Slot | rva (meteor-decomp) | absolute (Ghidra) | Size | Role (inferred) |
|---|---|---|---|---|
| 0 | `0x004a1b90` | `0x008a1b90` | 30 B | Scalar deleting destructor (standard MSVC pattern: `CALL <body>; TEST [esp+8],1; JZ skip; PUSH ESI; CALL _free; skip:`) |
| 1 | `0x0049f530` | `0x0089f530` | 125 B | `New()` factory — `PUSH 0x84` (size=132) → `operator new` → 6-arg ctor at `0x0089f2b0` constructing from member offsets `(ESI+8, ESI+0xc, ESI+0x68, ESI+0x10, ESI+0x14, ESI+0x6c)` |
| 2 | `0x0049e450` | **`0x0089e450`** | **207 B** | **`Receive()` — the actor-lookup + flag-check entry. See per-slot analysis below.** |
| 3 | `0x0049d230` | `0x0089d230` | 48 B | Auxiliary dispatch — `CALL 0x00cc7a50` (actor lookup) + `CALL 0x006ee680` (Director-base entry per Phase 6 doc — likely `_dispatchEvent` or similar) |
| 4 | `0x0049d260` | `0x0089d260` | 15 B | Predicate — compares `[ECX+8]` against `[0x0130c778]` (the `0xE0000000` `NO_ACTOR` sentinel — see §2 below), returns `SETNZ AL`. So slot 4 = "is this kick targeting a real (non-null) actor id?" — used by callers that want to skip processing of zero-target kicks. |

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

    MOV EAX, [0x0130c778]       ; load NO_ACTOR sentinel (0xE0000000) —
                                ; CONFIRMED 2026-05-04 via Ghidra GUI
                                ; (Task A): the constant at this RVA is
                                ; `undefined4 E0000000h`, matches
                                ; garlemald's NO_ENMITY_TARGET / "null
                                ; actor id" sentinel used everywhere
                                ; an actor-id slot can be empty.
    CMP [EDI+0x12c], EAX        ; is the dispatcher's [+0x12c] target
                                ; field == NO_ACTOR? (i.e. "no target
                                ; currently set?")
    JZ <0x0089e4b4>             ; if target unset → branch to the init /
                                ; setup path that establishes a target;
                                ; if target already set → fall through to
                                ; the "kick on existing target" path.

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

### 2. The `[0x0130c778]` global — `NO_ACTOR` sentinel (`0xE0000000`)

**CONFIRMED 2026-05-04 via Ghidra GUI (Task A).** The 4-byte value at
RVA `0x0130c778` is `undefined4 E0000000h` — the engine's universal
"null actor id" sentinel. (Same value appears in garlemald as
`NO_ENMITY_TARGET = 0xE0000000` in the 0x0195 enmity-indicator
packet builder; per memory `project_garlemald_enmity_indicator.md`.)

This corrects an earlier (incorrect) speculation that `0x0130c778`
was the "local player actor id". It is NOT — it's a static constant
that the engine compares actor-id slots against to detect "no actor
set". The 20+ xrefs Ghidra reports for this address are every
actor-id-bearing dispatcher / receiver that branches on "is this
slot empty?".

**Re-interpreting slot 2's branch:**

```
CMP [EDI+0x12c], NO_ACTOR
JZ <init path>                  ; [+0x12c] == NO_ACTOR → no target
                                ;   currently set, take init path
                                ;   that establishes a target
; else                          ; [+0x12c] != NO_ACTOR → target is
                                ;   already set, take the dispatch
                                ;   path that kicks on it
```

So `[EDI+0x12c]` is the dispatcher's "current target actor id" slot.
The two slot-2 branches are NOT "self-targeted vs other-targeted" —
they are **"first-time init for this kick" vs "kick on an already-set
target"**. The init branch (Branch B in the asm dump) writes
`[EDI+0x128]` and `[EDI+0x12c]` to set up the target; subsequent
calls hit the established-target branch (Branch A).

This shifts the porting story: garlemald doesn't need to think about
"is the kick for the local player" — both branches do the SAME
`+0x5c` flag check on the target actor either way. The kick gate is
**universal**: any kick to any actor needs the actor's `+0x5c` flag
set, which means the actor must have completed its spawn-packet
sequence on the client.

### 3. `0x00cc7a50` — `ActorRegistry::lookup_actor` (with two-collection split)

**CONFIRMED 2026-05-04 via Ghidra GUI (Task B).** Decompiles to:

```c
Actor* FUN_00cc7a50(Registry* this /* in ECX */, ActorId id) {
    if (FUN_00cd80f0(id) == 0)
        entry = FUN_00cd8160(id);   // search collection A
    else
        entry = FUN_00cd81d0(id);   // search collection B
    if (entry == NULL) return NULL;
    return *(Actor**)entry;         // unwrap hashmap-entry → actor ptr
}
```

20+ xrefs (every receiver that needs to look up the kick / event /
target actor by id).

**The new architectural finding** is that the registry is NOT a
single flat map — it's split across two backing collections, with
`FUN_00cd80f0(actor_id)` choosing which collection to search based
purely on the actor id itself. The predicate is small (presumably a
single comparison or bit test on the id).

Likely split criterion (to be confirmed by Task B.1 below):

1. **id-range partition**: dynamic / world-server actor ids
   (`< 0x80000000`?) vs map-server-allocated ids (`≥ 0x80000000`?).
   Directors live at `0x66080000+` per garlemald memory
   `project_garlemald_director_id_offset.md`; if collection B is
   the "directors / instance actors" partition, the kick-receiver
   slot 2 is implicitly looking up the kick target in BOTH paths.
2. **flag bit on the id**: the FFXIV id-space has dedicated bits
   for actor type (PC / NPC / mob / director / system); the
   predicate may just be `(id >> 28) & 0x7 == X`.

Whichever the split is, it has direct consequences for the
garlemald porting target:

- The kick gate finding (`+0x5c` flag check on the looked-up actor)
  applies regardless of which collection the actor lives in, since
  slot 2 calls `FUN_00cc7a50` once and that wrapper does the split
  internally. So the "spawn must precede kick" rule is universal.
- BUT the spawn-side opcode that flips `+0x5c` may differ between
  collections (e.g. `AddActor` for collection A vs `CreateDirector`
  for collection B). Task C (the `+0x5c` setter) needs to identify
  whether one or two opcodes flip the flag.

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

### Task A — ✅ DONE 2026-05-04

**Result:** `0x0130c778` is the `0xE0000000` `NO_ACTOR` sentinel
constant, NOT the "local player actor id" as initially speculated.
Sits immediately after the `Sqex::CDev::CDevMedia` RTTI Type
Descriptor in the data section, has 20+ xrefs (every dispatcher /
receiver that branches on "is this actor-id slot empty?"). See §2
of "Critical findings" above for the corrected slot-2 interpretation.

**Follow-up (Task A.1, optional):** Apply a `dword NO_ACTOR_ID`
label + comment to `0x0130c778` in Ghidra so the 20+ xref-bearing
functions decompile with `if (foo == NO_ACTOR_ID)` instead of
`if (foo == DAT_0130c778)`. This will make every other receiver
that uses the same gate trivially recognizable in subsequent passes.

### Task B — ✅ DONE 2026-05-04

**Result:** Confirmed `ActorRegistry::lookup_actor(this, id) →
Actor*`, AND surfaced new architectural finding that the registry
splits across two backing collections via predicate `FUN_00cd80f0`.
See §3 of "Critical findings" above for full analysis.

**Follow-up (Task B.1, ✅ DONE 2026-05-04):** `FUN_00cd80f0` is a
7-byte navigation thunk, NOT the predicate body itself:

```asm
MOV ECX, [ECX + 0x1c4]    ; navigate: this = (*ECX)->[+0x1c4]
JMP FUN_00d035d0          ; tail-call the real predicate
```

This surfaces TWO additional architectural findings:

**Registry has a nested object hierarchy.** Combined with the
caller's `MOV ECX, [ESI]; CALL FUN_00cd80f0`, the layout is:

```
Registry              (ECX from caller)
  [+0x0]   → subobject_1 ptr     (loaded by caller's MOV ECX,[ESI])
    [+0x1c4] → id_classifier ptr (loaded inside the thunk)
```

Two levels of sub-object before we reach the predicate. Canonical
C++ composition; the `[+0x1c4]` sub-object is probably an "id
classifier" / "namespace policy" component shared across registries.

**Thunk shared across 6 registry-like functions** — direct evidence
of multiple parallel registries with the same id-classification
policy:

- `FUN_00cc70b0`, `FUN_00cc7190`, `FUN_00cc7a50` — the `0x00cc7`
  cluster (our `lookup_actor` + 2 neighbors, probably `add_actor`
  / `remove_actor` siblings on the same registry).
- `FUN_00d2fe00`, `FUN_00d30160`, `FUN_00d303c0` — the `0x00d2/d30`
  cluster, distant from the `0x00cc7` cluster — likely a SIBLING
  registry with the same sub-object layout (candidate: a "directors
  registry" or "instance-actors registry" built from the same base
  class).

This supports the theory that **directors live in a different
registry than world actors**, and the kick gate's `+0x5c` flag
might be set by a different opcode pipeline depending on which
registry the target was registered into.

**Follow-up (Task B.2, ✅ DONE 2026-05-04):** `FUN_00d035d0`
decompiles to:

```c
bool FUN_00d035d0(u32 *id_ptr) {        // id passed by REFERENCE
  if (FUN_00d03540(id_ptr) == 0)        // gate: "is id known to classifier?"
    return false;                       //   no → false (use collection A)
  
  char *meta = FUN_00d03340(&id_ptr, *id_ptr);   // lookup metadata
  return *meta == 0x0F;                 // tag == 0x0F → true (use collection B)
}
```

**The partition is NOT id-range or bit-tag based — it's
type-tag-based via a runtime metadata lookup.** The classifier
sub-object maintains a `Map<ActorId, Metadata>` where Metadata's
first byte is a type tag. Tag `0x0F` (constant `DAT_0130d426`)
puts the actor into collection B; everything else (including
unknown ids) defaults to collection A.

This means the spawn-side opcode doesn't just write to a registry
— it also assigns the type tag that determines which collection
the actor ends up in. The director's `CreateDirector` packet
likely writes tag `0x0F` (or whatever specific tag it uses) AND
sets the `+0x5c` flag; a regular `AddActor` packet writes a
different tag AND sets `+0x5c`. Both pipelines must land before
any kick targeting the actor.

**Tag `0x0F` semantic** — best guess pending further decomp:
- An actor "kind" enum value (Director / ZoneInstance / SystemActor)
- A synthetic-actor flag in the engine's actor-kind taxonomy
- Confirming requires decoding `FUN_00d03340` (the metadata-set
  side, not just the lookup) — i.e. find the WRITES that set
  `*meta = 0x0F`.

**The 8 xrefs to FUN_00d035d0** include `FUN_00cd81d0` itself
(collection B lookup), which re-calls the predicate to gate its
own search. The `FUN_00d2ab60`, `FUN_00d2ae60`, `FUN_00d2b2c0`
cluster matches the `FUN_00d2*` cluster from `FUN_00cd80f0`'s
xref list — strong confirmation of two parallel registries with
identical id-classification policy.

**Follow-up (Task B.3, deferrable):** Decompile `FUN_00d03340`
both for the LOOKUP side AND find xrefs that WRITE to the
metadata's first byte to identify what spawn-side code path
assigns tag `0x0F`. This is the single highest-leverage next
finding since it directly identifies the spawn-side opcode that
inhabits collection B (probably the same opcode that flips
`+0x5c`).

Also worth applying labels in Ghidra:
- `FUN_00cc7a50` → `ActorRegistry::lookup_actor`
- `FUN_00cd80f0` → `ActorRegistry::id_partition_predicate_thunk`
- `FUN_00d035d0` → `ActorRegistry::id_partition_predicate` (real
  body — rename once decoded)
- `FUN_00cd8160` → `ActorRegistry::lookup_collection_a`
- `FUN_00cd81d0` → `ActorRegistry::lookup_collection_b`

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
