# Phase 9 #8e — `+0x5c` kick-gate writer hunt (partial)

> Last updated: 2026-05-16. Re-attempts Phase 7 Task C with two
> advantages: (1) Phase 9 #8d's confirmation that the `+0x5c` byte is
> on the **Lua-side wrapper class** (ActorBase ctor zeros it), not on
> the engine-side `CDev::SceneObject::Actor`/`RaptureActor`/`CDevActor`/`CharaActor`
> hierarchy; and (2) a scoped grep that excludes the
> already-identified Variant/Box wrapper false positives.

## Status: PARTIAL — search narrowed to 6 candidates, none definitively identified

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
