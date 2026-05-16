# Phase 9 #8d — Lua actor class ctor/dtor inspection

> Last updated: 2026-05-15. Started as "validate the orphaned-conditions
> hypothesis from #8b by walking `StartServerOrderEventFunctionReceiver`",
> pivoted when Phase 7 turned out to have already-decoded that
> receiver's slot 2 as a pending-queue drainer (NOT the ScriptBind
> handler). Refocused on the underlying question: **what code
> constructs the Lua-side `ActorBase` / `DirectorBase` / `NpcBase`
> wrapper objects, and what fields do they initialize?**

## TL;DR

Walking the ctor + dtor of every Lua-side script-binding base class
(via `lua_class_registry.md`'s vtable RVAs + a PE-binary byte-pattern
search for vtable-write sites) confirms:

1. **`DirectorBase` IS-A `ActorBase`** — `DirectorBase` ctor
   (`FUN_006ecf90`) chains to `ActorBase` ctor (`FUN_006dbb70`)
   before installing its own vtable.
2. **`DirectorBase[+0x60]` is a std::vector**, initialized to empty
   (First/Last/End all NULL) in `DirectorBase` ctor.
3. **`ActorBase[+0x5c] = 0`** at construction time — confirms Phase
   7's kick gate. The kick gate flag is **explicitly cleared by the
   constructor** and must be flipped on by a later opcode.
4. **`ActorBase` ctor does NOT initialize `+0x118`** (the fallback
   condition vector). Either lazy-initialized on first push, or
   initialized by an unidentified parent / sibling sub-object.

The original "orphaned-conditions hypothesis" from #8b **remains
unverifiable from static analysis**. It depends on what type of
Lua-side object the SetNoticeEventConditionReceiver's `dispatch_ctx`
points to AT PACKET-HANDLING TIME — and that's a wiring question that
requires Phase 9 #5 (opcode → receiver context wiring discovery).

## Construction sites (ctor + dtor) for every Lua actor class

By searching the binary for each vtable's absolute address (RVA +
`0x400000`) as a 4-byte LE pattern, we get exactly the 2 occurrences
per class that the MSVC vtable-install pattern produces (one in
ctor, one in dtor).

| Class | Vtable RVA | Ctor (size) | Dtor (size) |
|---|---|---|---|
| `ActorBaseClass` | `0xbd4fe4` | `FUN_006dbb70` (107 B) | `FUN_006dbbe0` |
| `CharaBaseClass` | `0xbd5cac` | `FUN_006ecd80` (CharaBase ctor) | `FUN_006ece20` |
| `PlayerBaseClass` | `0xbd5e04` | `FUN_006ed720` (ctor) | `FUN_006ed7a0` (dtor) + 2 more sites |
| `NpcBaseClass` | `0xbd647c` | `FUN_006f3650` (ctor) | `FUN_006f37a0` (dtor) |
| `DirectorBaseClass` | `0xbd5d6c` | `FUN_006f1310` (106 B) | `FUN_006ecf90` (94 B) |
| `AreaBaseClass` | `0xbd63d4` | `FUN_006f3210` (ctor) | `FUN_006f32a0` (dtor) |
| `PrivateAreaBaseClass` | `0xbd653c` | `FUN_006f3d90` (ctor) | `FUN_006f3e00` (dtor) |
| `QuestBaseClass` | `0xbdfdd0` | `FUN_00776f50` (ctor) | `FUN_00776fc0` (dtor) |

(For each class, the larger function is the ctor and the smaller
one is the dtor — the dtor just reinstalls its own vtable and
chains to base. PlayerBaseClass has 4 sites total — 2 extra in
some PlayerBaseClass-specific code; needs follow-up.)

## DirectorBase ctor (`FUN_006f1310`, 106 B)

```c
DirectorBase::DirectorBase(DirectorBase *this) {
  // SEH frame setup ...
  ActorBase::ActorBase(this);         // CALL 0x006dbb70 — chain to base
                                       // (after this, this->vtable == ActorBaseClass)
  
  // Now upgrade vtable to derived class
  this->vtable = (void**)0xfd5d6c;    // DirectorBaseClass vtable
  
  // Init [+0x60] as empty std::vector<T> (T size = ?)
  // (No vptr at [+0x60][+0] write — std::vector isn't polymorphic)
  this->[+0x60]._First = NULL;        // EAX+0x4
  this->[+0x60]._Last = NULL;         // EAX+0x8
  this->[+0x60]._End = NULL;          // EAX+0xc
  
  // SEH unwind, RET
  return this;
}
```

So `DirectorBase[+0x60]` is a 16-byte sub-object whose +4/+8/+c are
the standard MSVC `std::vector<T>::{First, Last, End}` triple. The
fact that `+0x0` isn't written means this sub-object either uses
the parent's vtable or has no vtable (std::vector isn't polymorphic
so the latter is likely — bytes 0..3 are unused / pad).

## ActorBase ctor (`FUN_006dbb70`, 107 B)

```c
ActorBase::ActorBase(ActorBase *this) {
  // SEH frame setup ...
  FUN_00cccb70(this);                  // CALL parent ctor (tiny — see below)
  
  this->vtable = (void**)0xfd4fe4;    // ActorBaseClass vtable
  FUN_00445cf0(&this->[+0x8]);         // ctor for sub-object at +0x8 (TBD)
  
  this->[+0x5c] = 0;                   // KICK GATE FLAG — explicitly zero
  this->[+0x5d] = 0;                   // sibling byte (also init to 0)
  
  // SEH unwind, RET
  return this;
}
```

**Key finding**: `[+0x5c] = 0` at construction. Phase 7's kick gate
flag is **explicitly cleared by the ActorBase ctor**. So an actor
freshly created from `AddActor`'s C++ side starts with `+0x5c == 0`
— kick gate disabled. **Some other opcode must flip it to 1** before
KickEvent will succeed (presumably `SetActorState` or the
post-spawn ActorInstantiate / ScriptBind sequence).

**No `[+0x118]` write in ActorBase ctor.** The fallback condition
vector's storage isn't initialized here. Possibilities:
- (a) It's lazy-initialized (first `push_back` call constructs the
  vector body)
- (b) It's initialized by the parent ctor's chain (next section
  rules this out)
- (c) It's part of the `[+0x8]` sub-object (which extends past 0x118)
- (d) It belongs to a derived class (DirectorBase et al.) that
  happens to put a vector at the offset that ActorBase reaches via
  `dispatch_ctx + 0x118`

Possibility (d) is the most interesting — it would mean the
"fallback" path on a NON-DirectorBase actor is actually writing to
garbage memory (or to a different derived class's field at the same
offset). That would be a SEMANTIC BUG in the engine, not just an
ordering issue.

## The parent ctor `FUN_00cccb70` is trivial (16 B)

```asm
MOV EAX, ECX
MOV [EAX],   0x110e30c    ; some Sqex/Component base vtable
MOV [EAX+4], 0           ; init [+4] field to 0
RET
```

Two writes only. **Does not initialize `[+0x118]`** either. So the
"+0x118 is lazily initialized" theory (possibility a) is the most
plausible — `push_back` to an uninitialized vector with all-NULL
pointers IS the standard MSVC convention (the first push allocates
the buffer).

## Inheritance verification — vtable comparison

The first few vtable slots of `ActorBaseClass`, `DirectorBaseClass`,
`NpcBaseClass`, `CharaBaseClass`, `PlayerBaseClass` show many
shared entries — confirming a true derived-class hierarchy:

| slot | ActorBase | DirectorBase | NpcBase | CharaBase | PlayerBase |
|---:|---|---|---|---|---|
| 4  | `0x712b40` | **`0x712b40`** (=) | `0x6f3110` | `0x6f3000` | `0x6f3000` |
| 5  | `0x6f6900` | `0x6f6d30` | `0x6fa7e0` | `0x6fa7e0` | `0x6fa7e0` |
| 7  | `0x5b8d90` | `0x5b8d90` | `0x5b8d90` | `0x5b8d90` | `0x5b8d90` |
| 8  | `0x5b8d90` | `0x5b8d90` | `0x5b8d90` | `0x5b8d90` | `0x5b8d90` |
| 9  | `0x5c5c80` | `0x5c5c80` | `0x5c5c80` | `0x5c5c80` | `0x5c5c80` |
| 10 | `0x776340` | `0x776340` | `0x776340` | `0x776340` | `0x776340` |
| 11 | `0x776340` | `0x6e1f70` | `0x6e17e0` | `0x6e17e0` | `0x6e17e0` |
| 13 | `0x6dc7620` | `0x6dc7620` | `0x6dc7620` | `0x6dc7620` | `0x6dc7620` |

**Inferred edges** (refined from #8b):

- DirectorBase matches ActorBase at slot 4 (overrides slot 5/11),
  and DirectorBase ctor chains to ActorBase ctor → **DirectorBase
  extends ActorBase directly** (NOT through CharaBase)
- NpcBase, CharaBase, PlayerBase all share slot 4 = `0x6f3000`
  (different from ActorBase's `0x712b40`) → **CharaBase overrides
  slot 4; NpcBase + PlayerBase inherit that override** → confirms
  NpcBase + PlayerBase extend CharaBase, NOT ActorBase directly
- This refines the #8b inheritance tree to:

```
ActorBase
├── CharaBase                            (slot 4 → 0x6f3000)
│     ├── NpcBase                        (5 receivers cast to this)
│     └── PlayerBase                     (3 receivers cast to this)
│           └── MyPlayer                 (12 receivers cast to this)
├── DirectorBase                         (1 receiver — SetNoticeEventCondition)
├── AreaBase
│     └── PrivateAreaBase                (slot 0 differs — proper override)
└── QuestBase                            (very different — possibly extends a different intermediate base)
└── WorldMaster
```

## Implications for the orphaned-conditions hypothesis

The hypothesis from #8b was:
> If `ScriptBind` is what allocates the Lua-side `DirectorBase`
> instance, then for 6 ticks the conditions land in the wrong field,
> and a post-`ScriptBind` `DirectorBase` would have empty `[+0x60]`.

After this static-analysis sweep, **the hypothesis is partially
plausible but cannot be confirmed**:

- **Pro** (still plausible): If pre-ScriptBind, dispatch_ctx is a
  plain `ActorBase` (not yet promoted to derived class), the
  dynamic_cast in the receiver would fail, and the fallback path
  writes to `ActorBase[+0x118]`. A post-ScriptBind `DirectorBase`
  would have an empty `[+0x60]`, and the cinematic notice-evaluator
  (which reads from `[+0x60]`) would never trigger.

- **Con** (refuting the hypothesis): C++ inheritance doesn't allow
  in-place type promotion — once an object is constructed as
  `ActorBase`, you can't "upgrade" it to `DirectorBase` (you'd have
  to destroy + reconstruct). So either:
  - (a) `AddActor` already creates the actor with its correct
    derived type (based on actor-kind in the packet) — in which
    case the orphaned-conditions can't happen
  - (b) The dispatch_ctx for SetNoticeEventCondition isn't the
    actor itself but a SEPARATE lookup that happens at handler
    invocation time — in which case the timing/order doesn't
    matter for the C++ object's lifetime, only for whether the
    dispatch_ctx's lookup returns the right type

Resolving requires Phase 9 #5 — finding the script-load-time
wiring that connects opcode → receiver → dispatch_ctx. Without
that, the dispatch_ctx type at packet-handling time is unknown.

## What this DOES confirm for the SEQ_005 hang

Even though the orphaned-conditions hypothesis can't be confirmed,
the construction sweep surfaced something **useful**:

- **`+0x5c` is explicitly zeroed by `ActorBase` ctor.** This means
  the Phase 7 kick gate IS a real gate that needs explicit flipping.
  Garlemald's `AddActor` packet alone won't flip it — the C++ ctor
  runs as part of packet handling, then sets `+0x5c = 0`. Something
  AFTER must set it to 1.
- For SEQ_005's content director, the kick fires AFTER the full
  spawn sequence. If the spawn sequence's last opcode (e.g.
  ScriptBind / SetActorState / SetActorProperty(/_init)) doesn't
  flip `+0x5c`, the kick silently drops.
- Per Phase 9 #1's open follow-up "Task C — Decode the `+0x5c`
  actor flag setter" (still partial), the writer for `+0x5c` is
  identified-by-elimination only. This sweep STRENGTHENS the need
  for that task — knowing exactly which opcode writes `+0x5c = 1`
  would let garlemald verify it's emitting that opcode.

## Recommended Phase 9 follow-ups (revised)

| # | Task | Why |
|---|---|---|
| #8e (NEW) | Find the `+0x5c` setter — search for `MOV byte [reg+0x5c], 1` in actor-related code paths (use the inheritance tree to scope: only `ActorBase`-derived class code matters) | Definitively identifies which spawn-sequence opcode is responsible for enabling kicks. Stronger than the prior asm-pattern search (Task C of Phase 7) because we can scope to ActorBase-derived ctors and member functions |
| #5 (existing) | Find the script-load-time wiring that connects opcode → receiver → dispatch_ctx | The dispatch_ctx mystery underlies the orphaned-conditions question and many others. Without it we can't fully reason about which class the receiver's `__RTDynamicCast` is operating on |
| #8f (NEW) | Walk the actor-side `AddActor` opcode (0xCA) handler from its dispatch table entry to confirm which Lua-class wrapper gets constructed for different actor kinds | Direct answer to "does AddActor construct a `DirectorBase` for directors, or a plain `ActorBase` that gets upgraded later?" |

## Cross-references

- `docs/lua_class_registry.md` — Phase 6 item #3 (the registration
  function that runs at engine startup; source of the vtable RVAs)
- `docs/lua_actor_impl.md` — Phase 6 item #6 (the engine-side
  `LuaActorImpl` companion, distinct from these script-binding base
  classes)
- `docs/event_status_condition_receivers_decomp.md` — Phase 9 #8b
  (the cast-success vs fallback paths that motivated this dive)
- `docs/event_kick_receiver_decomp.md` — Phase 7 (the `+0x5c` flag
  gate; reinforced by this finding that the ctor explicitly zeroes it)
- `docs/seq005_receiver_gate_audit.md` — Phase 9 #8 (the
  SEQ_005-specific cross-reference)
- `docs/receiver_classes_inventory.md` — Phase 9 #1 (the receiver
  inventory + #8b's "Lua actor class hierarchy" section refined
  here with confirmed inheritance edges)
