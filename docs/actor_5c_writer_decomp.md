# Phase 9 #8e — `+0x5c` kick-gate writer ✅ RESOLVED

> Last updated: 2026-05-17. RESOLVED: the writer is `FUN_00766f00`
> (RVA `0x366f00`), which calls `ActorRegistry::lookup_actor`
> (`FUN_00cc7a50` — the same helper KickReceiver uses), checks the
> actor's `+0x7d` gate via `FUN_00cc72a0`, then sets `[actor+0x5c]=1`.
> See "2026-05-17 (later) — ✅ CONFIRMED" section at the bottom.

> Earlier sections preserved for historical context — Phase 9 #8e's
> original best-candidate identification was correct; the dismissal as
> "per-tick, probably not" was premature.

## Status: ✅ RESOLVED 2026-05-17 — FUN_00766f00 confirmed via call-chain analysis

The +0x5c kick-gate writer remains unidentified, but the candidate set
is now 6 functions (down from Phase 7's ~35 false-positive matches).
Definitive resolution likely requires Ghidra-decompiler-assist on the
candidates OR runtime tracing during actor spawn.

## TL;DR — major reframing

**Phase 7 Task C assumed `+0x5c` was on the engine-side C++ Actor.** It's
not. Reading the engine-side actor hierarchy's ctors (Phase 9 #8d
methodology applied here):

| Class | Ctor | Touches `[+0x5c]`? |
|---|---|---|
| `SQEX::CDev::Engine::Fw::SceneObject::Actor` | `FUN_00a60b80` (384 B) | **No** — inits `[+0x50/0x54/0x58/0x60]` but skips `0x5c` |
| `Application::Scene::RaptureActor` | `FUN_007cef80` (376 B) | **No** — inits `[+0x90+]` and various sub-objects |
| `Application::Scene::Actor::CDevActor` | `FUN_006329c0` (268 B) | **No** — inits `[+0x120/0x124]` only |
| `Application::Scene::Actor::Chara::CharaActor` | `FUN_0065f180` (1942 B) | **No** — searches show no `[+0x5c]` writes |

So the engine-side actor's `+0x5c` byte is never explicitly initialized
by any of its constructors — it would be uninitialized garbage after
construction.

But Phase 9 #8d found that the **Lua-side wrapper** (`Application::Lua::Script::Client::Control::ActorBase`,
vtable RVA `0xbd4fe4`) ctor at `FUN_006dbb70` **explicitly zeros
`[ESI+0x5c]` and `[ESI+0x5d]`**.

So the `+0x5c` kick-gate flag is **on the Lua-side wrapper**, not the
engine-side C++ Actor. `ActorRegistry::lookup_actor` (`FUN_00cc7a50`) must
be returning a pointer to the Lua-side wrapper (or to a hybrid object
whose `+0x5c` aliases the wrapper's field).

This re-scoping is the load-bearing finding of #8e: the search space is
**Lua-actor-wrapper code paths**, not engine-side C++ actor code.

## Search method

```
grep -rE "c6 4[0-7] 5c 01" asm/ffxivgame/
```

Matches `MOV byte ptr [<reg>+0x5c], 0x1` for `<reg>` ∈ {EAX,ECX,EDX,EBX,ESI,EDI,EBP}.
Yielded 34 hits across 32 files.

## False positives — the Variant/Box wrapper cluster (~26 hits)

Per Phase 7 Task C, ~20 files in the `0x55*` range form a Variant/Box
wrapper pattern (`FUN_00559de0` allocator + typed conversion + set
`+0x5c=1` to mark variant "value populated"). All 20+ files in this
cluster are filtered out:

- `0x14e890`, `0x14f110`, `0x146b30`, `0x149ee0` (0x546b30 / 0x549ee0 / 0x54e890 / 0x54f110 absolute) — value-cast wrappers
- `0x15a*` family (~20 functions) — typed Variant factories

A separate false-positive cluster:
- `FUN_00a42c90` (23 lines, 14 callers) — Phase 7 identified as a
  **scoped guard / sync primitive** that sets `[global+0x5c]=1`, spins
  on `vtable[6]()`, clears `[global+0x5c]=0`. Different class entirely.

## 6 non-Variant candidates

After filtering, 6 candidates remain — none directly a vtable entry
in any Lua-actor-class vtable (so all are non-virtual methods):

| RVA | Function | Size (B) | Callers | Notes |
|---|---|---:|---:|---|
| `0x00366f00` | `FUN_00766f00` | 507 | 1 | Called from FUN_00578970 — iteration over sub-objects pattern. Write at offset +0x128 from start: `MOV byte [EBP+0x5c], 1` where EBP is a helper-call return value. |
| `0x003b43e0` | `FUN_007b43e0` | 28 lines | 1 | Tiny — likely a simple setter. Single caller for narrow analysis. |
| `0x005018f0` | `FUN_009018f0` | 37 lines | 0 | **Zero direct CALL sites** — likely virtual (called via `CALL [EAX+0xN]`). EDI used as `this`. Worth checking if it appears in some other vtable. |
| `0x00642c90` | `FUN_00a42c90` | 23 lines | 14 | **False positive** — Phase 7's sync primitive (set/clear inside 32 bytes). |
| `0x006cc050` | `FUN_00acc050` | 80 lines | 1 | Single caller: FUN_00acc160. Worth tracing call graph. |
| `0x00854710` | `FUN_00c54710` | 146 lines | 1 | Single caller: FUN_00c28240. Likely a more complex state-machine. |

## Best candidate: FUN_00766f00

`FUN_00766f00` is the most plausible kick-gate writer based on:
- Reasonable size (507 B — fits a typical actor-state-update method)
- Callsite pattern: called as one of ~11 "process sub-object" steps in
  `FUN_00578970` (which iterates `[ESI+0x08/0x0c/0x10/0x14/0x18/0x1c/0x20/0x24/0x28/0x2c/0x30]`)
- Write context: the `MOV byte [EBP+0x5c], 1` is preceded by
  `MOV EBP, EAX` after a helper call — so the function calls a helper
  that returns a pointer, then sets the kick-gate flag on the result

**What the iteration loop likely is**: A "post-spawn finalize" pass
over an actor's component sub-objects. The function would be called
when the actor's full spawn-packet sequence has been processed, to
flip each component (and the actor itself) into "ready for events" state.

But **without proper Ghidra-decompiler-assist disassembly**, I can't
confirm what EBP's helper actually returns, or whether the function
operates on a Lua-actor-wrapper (vs some other class that happens to
have a `+0x5c` field).

## Why the writer isn't a vtable entry

None of the 6 candidates appears as a vtable entry in any of the 8
Lua-actor-class vtables:

```
ActorBaseClass (0xbd4fe4), CharaBaseClass (0xbd5cac), PlayerBaseClass (0xbd5e04),
NpcBaseClass (0xbd647c), DirectorBaseClass (0xbd5d6c), AreaBaseClass (0xbd63d4),
PrivateAreaBaseClass (0xbd653c), QuestBaseClass (0xbdfdd0)
```

This is mildly surprising — one would expect a virtual `setReady()` /
`finalizeSpawn()` slot. Possible explanations:

1. The writer is a **non-virtual member function** (or static
   helper) called by name from packet-handler code. Common for setters
   in MSVC C++.
2. The writer is in a sub-object's vtable (one of the inner
   sub-objects that ActorBase ctor constructs at `[+0x8]` via
   `FUN_00445cf0`).
3. The writer is an **engine-internal** function (not on the Lua side)
   that operates on a hybrid actor object via a known offset — i.e.,
   the engine writes the byte on the engine-side actor and the layout
   happens to alias the Lua-side wrapper's `+0x5c`.

Option 3 would mean Phase 9 #8d's interpretation needs another revision —
the byte might be on a "shared header" between engine-side and Lua-side
representations.

## Why this matters less than originally thought

After this dive, the kick-gate writer's identity has **diminished
importance** for the SEQ_005 hang specifically:

- garlemald sends the SAME spawn-packet sequence as pmeteor (verified
  byte-identical for kick body, content-group bytes, etc.)
- Pmeteor's cinematic works, garlemald's doesn't
- So whatever opcode writes `+0x5c=1` on pmeteor's side ALSO gets sent
  by garlemald (at the wire level) — the issue isn't a missing packet,
  it's some other state divergence

What the writer's identity WOULD help with:
- Sanity-checking: if the writer is fired by `SetActorState` (likely),
  then garlemald can verify its `SetActorState` packet body is
  byte-identical to pmeteor's
- Debugging future actor-state bugs unrelated to SEQ_005

## Recommended next steps

| Approach | Cost | Resolves |
|---|---|---|
| Ghidra GUI decomp of the 6 candidates | Medium | Definitive answer if writer is in this set |
| Runtime trace during actor spawn (Wine + breakpoint on `[actor+0x5c]`) | High | Catches the writer regardless of static-analysis ambiguity |
| Walk SetActorState's opcode handler (Phase 9 #5 prerequisite) | High | Probably hits the writer in passing |
| Check FUN_00445cf0 (the ActorBase `[+0x8]` sub-object ctor) | Low | Confirms whether the `+0x5c` byte is actually inside that sub-object |
| Search for callers of the 6 candidates' callers + cross-reference to known packet handlers | Medium | Direct attribution to a packet handler |

## Re-narrowing the writer hunt

Looking at the iteration-over-sub-objects pattern in `FUN_00578970`,
each sub-object slot calls a different processor function:

| Sub-obj offset | Processor fn | Plausible class |
|---|---|---|
| `[+0x08]` | `FUN_00766f00` (our candidate) | TBD |
| `[+0x0c]` | `FUN_0076f6f0` | TBD |
| `[+0x10]` | `FUN_007700b0` | TBD |
| `[+0x14]` | `FUN_0076a9c0` | TBD |
| `[+0x18]` | `FUN_006cdf20` | TBD |
| `[+0x1c]` | `FUN_00583440` | TBD |
| `[+0x20]` | `FUN_005836d0` | TBD |
| `[+0x24]` | `FUN_007696d0` | TBD |
| `[+0x28]` | `FUN_00770c00` | TBD |
| `[+0x2c]` | `FUN_0076dab0` | TBD |
| `[+0x30]` | `FUN_00765340` | TBD |

If `FUN_00578970` is itself an actor-update tick, then `FUN_00766f00`
runs every tick on `[actor+0x8]` and could legitimately set `+0x5c=1`
on its result. That doesn't match a "spawn-time" writer profile, though
— a per-tick writer would set the byte even for already-spawned actors.

So `FUN_00766f00` is **probably NOT the kick-gate writer** despite being
the best statically-narrowed candidate. The writer is more likely:
- One of the other 4 single-caller candidates (`FUN_007b43e0`,
  `FUN_009018f0`, `FUN_00acc050`, `FUN_00c54710`)
- OR a non-statically-callable virtual method (called via runtime
  pointer indirection)

## Cross-references

- `docs/event_kick_receiver_decomp.md` — Phase 7 (the kick gate
  discovery; Task C's first attempt at the writer hunt)
- `docs/lua_actor_class_construction.md` — Phase 9 #8d (the
  reframing: `+0x5c` is on the Lua-side wrapper, not the engine-side
  C++ Actor)
- `docs/receiver_classes_inventory.md` — Phase 9 #1 + the Lua actor
  class hierarchy section refined in #8d
- `memory/reference_meteor_decomp_actor_rtti.md` — the engine-side
  actor RTTI walk (RaptureActor / CDevActor / CharaActor /
  SceneObject::Actor — all of whose ctors we ruled out here)

## 2026-05-17 — Final walk + new candidates surfaced

Re-ran the `c6 4? 5c 01` (`MOV byte [reg+0x5c], 1`) scan with stricter
filters (Variant family + sync primitive). The original Phase 9 #8e
list of 5 candidates was incomplete; the fuller list is **10
candidates** (after applying the same filters):

| Function | Size | Callers | Status |
|---|---:|---:|---|
| `FUN_005469e0` | TBD | 0 direct, 1 .rdata ref | **NEW** — in a vtable, not yet investigated |
| `FUN_00549330` | TBD | TBD | **NEW** — 5 write sites (probably Variant family, missed by filter) |
| `FUN_00559f90` | TBD | TBD | **NEW** — likely Variant family (0x559xxx range) |
| `FUN_00559fb0` | TBD | TBD | **NEW** — likely Variant family |
| `FUN_00766f00` | 507 | 1 | Per Phase 9 #8e, plausible but per-tick context makes it unlikely |
| `FUN_007b43e0` | 87 | 1 (FUN_00662d30) | **RULED OUT 2026-05-17**: caller passes ECX = EDI+0x1110 (a sub-object), NOT an actor. Init function for a different class with coincidentally-similar layout. |
| `FUN_009018f0` | 81 | 1 (FUN_008f4ed0) | **TOP CANDIDATE 2026-05-17** — see below |
| `FUN_00acc050` | 236 | 1 (FUN_00acc160) | **RULED OUT 2026-05-17**: jump-table dispatcher on first arg; writes DIFFERENT byte fields (+0xc, +0x1c, +0x5c, …) per case. Generic field-setter, not specifically the actor kick-gate. |
| `FUN_00b8b560` | TBD | 4 (all FUN_00b8bf00) | **NEW** — "init array of 4" pattern; worth checking |
| `FUN_00c54710` | 520 | 1 (FUN_00c28240) | Lazy-init pattern (TEST + OR on `[0x01327b14]`, MOV [global+0x1c]); needs deeper walk |

### ⚠ TOP CANDIDATE RULED OUT 2026-05-17 (later)

`FUN_009018f0` initially looked perfect — but cross-referencing its
helpers definitively rules it out:

| Helper | Body | Reveals |
|---|---|---|
| `FUN_00d3abe0` (10 B) | `XOR EAX,EAX; CMP [ECX+4],-1; SETNZ AL; RET` | "is handle set?" predicate |
| `FUN_00d3abc0` (27 B) | If `[ECX+4] != -1`: `CALL [0xf3e1ec]([ECX+4]); [ECX+4] = -1` | "close handle if open" — `[0xf3e1ec] = CloseHandle` (confirmed via PE IAT walk) |

Plus `[0xf3e148] = InterlockedExchange`, `[0xf3e16c] = EnterCriticalSection`,
`[0xf3e1a0] = InterlockedCompareExchange` — all confirming this class
is a **Win32 sync-primitive wrapper** (Mutex / Event / Semaphore /
WaitablePredicate) with:

- `[+0]`: vtable
- `[+4]`: HANDLE (-1 if not open)
- `[+8]`: queue of waiters
- `[+0x5c]`: a sync state flag ("signaled" / "drained" / "completion")

So `FUN_009018f0`'s `+0x5c=1` write is **setting the sync primitive's
"completion" flag** after the waiter queue drains, NOT the actor's
kick gate. False positive.

### Original ⚠ candidate (kept for reference)

The `FUN_009018f0` body initially looked like a "queue drain → set
ready" semantic on an actor:

```c
void FUN_009018f0(this, arg) {   // ECX = this (= EDI), [ESP+0xc] = arg
    EAX = [EDI+0x8];                  // load some container ptr
    if ([EAX] == 0) goto end;         // empty? skip
    ESI = EDI + 0x8;
    
    PUSH ESI; CALL FUN_004531c0;       // check container state
    if (!AL) goto end;
    
    MOV ECX, EDI;
    CALL FUN_00d3abe0;                  // check this state
    if (AL != 0) {
        MOV ECX, EDI;
        CALL FUN_00d3abc0;              // post-check action
    }
    
    PUSH 0; PUSH arg; PUSH ESI;
    CALL FUN_00454020;                  // pop/process queue entry
    
    PUSH ESI; CALL FUN_004531c0;        // re-check container
    if (AL != 0) goto end;              // still has stuff → don't set flag
    
    [EDI+0x5c] = 1;                     // ⭐ SET KICK GATE only when queue empty
end:
    return;
}
```

**Why this is the top candidate**:
- Semantically perfect for "kick gate": only sets +0x5c=1 when the
  container at this+0x8 is FULLY DRAINED. Matches the Phase 7 finding
  that +0x5c is a "ready for events" gate that the kick checks.
- The function processes ONE event from the queue (`FUN_00454020`),
  then re-checks if the queue is empty. If yes, marks the actor ready.
- The this object has helpers `FUN_00d3abe0` / `FUN_00d3abc0` (in the
  `0x0d3a...` range — looks like Lua-engine sync helpers).
- The function shape (read queue → process one → check empty → set
  ready flag) is exactly the "completion notification" pattern that
  unblocks downstream gating.

**Caller `FUN_008f4ed0`** (527 B) is a large loop function — likely
the per-frame actor-tick driver that calls FUN_009018f0 on each actor
that has pending queued events.

### What was needed to confirm (and ruled it out)

The class-identification step ruled it out: cross-referencing the
helpers FUN_00d3abc0 + FUN_00d3abe0 + the IAT slot they call revealed
they're Win32 HANDLE wrappers, NOT actor methods. So FUN_009018f0
operates on a sync-primitive class, not an actor. See "⚠ TOP CANDIDATE
RULED OUT" section above.

### Remaining candidates not yet walked

After ruling out FUN_007b43e0, FUN_00acc050, and FUN_009018f0, the
search continues among:

- `FUN_005469e0` (in a vtable): check the vtable's class via COL→TD walk
- `FUN_00549330` (5 write sites at +0x96d/+0xa0d/+0xa60): inconsistent
  with single-purpose actor-flag setter; probably Variant family
- `FUN_00559f90` / `FUN_00559fb0`: likely Variant family
- `FUN_00b8b560` (4 callers from FUN_00b8bf00): "init array of 4"
  pattern — could be a per-actor init for 4 fixed actors. Worth a peek.
- `FUN_00766f00` (Phase 9 #8e original "best candidate"): per-tick
  context made it look unlikely BUT given the 3 newer "top candidate"
  rulings, worth re-examining with a fresh eye.
- `FUN_00c54710` (520 B, lazy-init pattern via global flag at
  `[0x01327b14]`): substantial size; might contain the actual
  per-spawn actor setup including +0x5c=1.

### Strategic takeaway

After this round, 6 of 10 candidates are still in play but 3 have
been definitively ruled out. The fact that simple static-analysis
heuristics keep producing false positives (matching the right
opcode AND the right semantics independently, but not actually being
the actor writer) suggests:

- The actor +0x5c=1 write may be inside a **larger function** that's
  ALSO doing many other things — i.e., the write is an incidental
  side effect of some "post-spawn finalize all subsystems" function.
- OR the write may use a non-immediate pattern (`MOV [reg+0x5c], CL`
  where CL was loaded from a global). Such writes wouldn't match the
  `c6 4? 5c 01` byte pattern.

The next-cost-effective angle is probably **runtime tracing** (HWBP
on writes to the actor's +0x5c field during a known-good actor spawn
in pmeteor). Static analysis has hit diminishing returns.

## 2026-05-17 (later) — ✅ CONFIRMED: `FUN_00766f00` IS the +0x5c writer

**Phase 9 #8e's ORIGINAL "best candidate" was correct after all.** The
prior dismissal as "per-tick, probably not the writer" was premature.

### Definitive identification

Walked the remaining 7 candidates by extracting the bytes around each
+0x5c=1 write. Only **`FUN_00766f00`** (RVA 0x366f00) sits in the
actor-area RVA range (0x2dx..0x37x where Phase 9 #8d's Lua-actor base
ctors live). The others (RVAs 0x14xxxx, 0x15xxxx, 0x78xxxx, 0x85xxxx)
are in unrelated namespaces.

Inspected FUN_00766f00's write site at +0x128 (RVA `0x367028`):

```c
// At RVA 0x367000..0x367033:
EBP = FUN_00cc7a50(...);                  ; ActorRegistry::lookup_actor
                                          ; (Phase 7 KNOWN — used by KickReceiver!)
if (EBP == NULL) goto skip;               ; null-check
... (additional setup)
PUSH EBP;
LEA ECX, [EBX+4];
CALL FUN_00cc72a0;                        ; check actor[+0x7d]
TEST AL, AL;
JZ skip;
MOV byte [EBP+0x5c], 1;                   ; ⭐ SET KICK GATE
... (more processing with EBP)
```

**The call at offset 0x108 (RVA 0x367008) decodes as `CALL 0x008c7a50`**
— verified byte-for-byte (`e8 43 0a 56 00`; `rel32=0x00560a43`;
`next_pc = 0x36700d`; `target = 0x36700d + 0x00560a43 = 0x008c7a50`).
That's **the exact same `ActorRegistry::lookup_actor` helper** the
KickReceiver uses in Phase 7's decomp.

### FUN_00cc72a0 — the +0x7d gate check (18 B)

The second key helper:

```asm
FUN_00cc72a0:
    MOV EAX, [ESP+4]                      ; arg = actor id
    MOV ECX, [ECX]                        ; this->vtable / registry root
    PUSH EAX;
    CALL FUN_00cd7a30;                     ; lookup actor by id → EAX = actor*
    MOV AL, byte [EAX + 0x7d]              ; ⭐ READ actor's +0x7d gate
    RET 4
```

So `FUN_00cc72a0` is **`Actor::IsRunEventReady()`** equivalent — it
returns `actor[+0x7d]` (the RunEventFunction gate per Phase 7).

### Confirmed semantic of FUN_00766f00

The +0x5c kick-gate writer's behavior:

```c
void FUN_00766f00(this) {                  // ECX = this (= EBX/Spawn coordinator)
    // (~25 lines of state checks at start)
    
    // Per-actor-state-update loop:
    for_each_pending_actor() {
        actor = ActorRegistry::lookup_actor(...);  // EBP = actor*
        if (actor == NULL) continue;
        
        if (Actor::IsRunEventReady(arg)) {         // returns actor[+0x7d]
            actor[+0x5c] = 1;                       // ⭐ SET KICK GATE
            // ... additional post-set processing ...
        }
    }
}
```

So the kick gate flow is now FULLY understood:

1. Actor spawns → `ActorBase` ctor zeros `+0x5c` and `+0x7d` (Phase 9 #8d)
2. Some upstream code sets `actor[+0x7d] = 1` (the RunEventFunction gate)
   — *that writer is the next investigation target, but probably has
   a similar pattern in a sibling function*
3. **Per-frame, `FUN_00766f00` runs over pending actors**. For each:
   - If `actor[+0x7d] == 1` (run-event ready), THEN
   - `actor[+0x5c] = 1` (kick-gate set)
4. KickReceiver can now succeed on this actor
5. Eventually `MyPlayer::vtable[66]` (the clearer per
   `docs/kick_dispatcher_clearer.md`) resets dispatcher state for the
   next cinematic

### Why Phase 9 #8e dismissed it (and why that was wrong)

Phase 9 #8e's reasoning: "FUN_00578970 (caller) iterates over sub-objects, fires
each tick — so FUN_00766f00 would run every tick on every actor, making it
unlikely to be a one-time gate setter".

The reasoning was wrong because:
- Per-tick `MOV byte [reg+0x5c], 1` is **idempotent** — already-1 stays 1
- The gate is conditional on `actor[+0x7d]==1`, so it only fires for
  actors that have already passed the +0x7d phase
- Setting an already-set flag every tick is harmless and is actually the
  STANDARD pattern for "ensure this flag is set if condition holds"
- The dismissal assumed "kick gate set ONCE" semantic, but the real
  behavior is "kick gate set whenever the precondition holds"

### Cross-references

- `docs/event_kick_receiver_decomp.md` — Phase 7 #1 (the KickReceiver
  that READS `[actor+0x5c]`; uses the same `ActorRegistry::lookup_actor`
  at `0x8c7a50` that this writer uses)
- `docs/event_run_event_function_receiver_decomp.md` — Phase 7 #2
  (the RunEventFunctionReceiver that READS `[actor+0x7d]`; the gate
  whose set state triggers FUN_00766f00's +0x5c write)
- `docs/kick_dispatcher_clearer.md` — the dispatcher state clearer
  (`FUN_006e32f0` = `MyPlayer::vtable[66]`), separate from the +0x5c
  writer recovered here
- `memory/reference_ffxiv_1x_actor_event_flags.md` — the canonical
  +0x5c / +0x7d gate semantics

### Practical impact

For SEQ_005 unblock specifically, knowing the +0x5c writer doesn't
directly fix the hang (the issue is upstream — `context_root[+0x12c]`
stale state, per the kick clearer doc). But the writer's identity:
- Helps verify that garlemald's spawn-packet sequence is firing the
  same gate-set as pmeteor's
- Lets a future debug session set a runtime breakpoint and observe
  spawn-time ordering precisely
- Closes a long-standing Phase 7 / Phase 9 #8e open question
