# Phase 6 item #8 — `DirectorBase` Lua hook map

> Last updated: 2026-05-03 — Director-specific overrides
> identified via 4-way diff against AreaBase / ActorBase / QuestBase
> sibling slot maps. Slot semantics inferred from body
> structure + cross-referenced Discord context.

## Director-specific overrides

`DirectorBase` (vtable RVA `0xbd5d6c`, 34 slots) shares slots
0..19 from `Component::Lua::GameEngine::LuaControl` and slots
20..33 from a parent Lua-script-binding base (likely the
intermediate above ActorBase). After diffing all 34 slots
against the 3 sibling bases (`AreaBase`, `ActorBase`,
`QuestBase`) — full diff table in the build artifact — the
**Director-specific overrides** are slots 0, 1, 2, 3, 5, 6,
11, 12. The other 26 slots are inherited from common parents.

Slot 26 and 29 are **shared with ActorBase + QuestBase** but
**differ from AreaBase** — confirming Director inherits from
the actor branch (not the area branch) of the Lua-binding
hierarchy.

## Object layout (recovered from ctor + factory)

`DirectorBase` ctor body at `FUN_006ecf90` (94 B,
RVA `0x2ecf90`) and dtor body at `FUN_006f1310` (106 B). The
factory at slot 1 (`FUN_007288f0`, 94 B) shows:

```
6a 70                PUSH 0x70             ; sizeof(DirectorBase) = 112
e8 1d 92 2a 00       CALL operator new
... (null check, ctor invocation)
```

So **`sizeof(DirectorBase) = 0x70 (112 bytes)`** — slightly
larger than QuestBase (100 B), reflecting the additional
Director-specific state (event-listener block at +0x60-ish,
embedded sub-objects).

## Per-slot semantics (Director-specific)

### Slot 0 — Destructor (`FUN_00721370`, 27 B)

Standard MSVC scalar deleting destructor:

```
56 8b f1             PUSH ESI; MOV ESI, ECX
e8 18 bc fc ff       CALL FUN_006ecf90      ; the dtor body
f6 44 24 08 01       TEST byte [esp+8], 1   ; delete-flag check
74 09                JZ skip
56                   PUSH ESI
e8 92 07 2b 00       CALL _free
[skip:] 8b c6 5e c3
```

### Slot 1 — Factory `New()` (`FUN_007288f0`, 94 B)

`operator new(0x70) + null-check + CALL ctor`. Standard
LuaControl-binding-base factory pattern — same shape as
QuestBase slot 1 from item #7, just with a different size +
ctor.

### Slot 2 — Typed init (`FUN_00758260`, 127 B)

Has **5 CALL sites** to functions that are ALL 330 B each:

```
fn+ 28: → FUN_0073bfd0 (330 B)
fn+ 49: → FUN_0073fc50 (330 B)
fn+ 70: → FUN_007570b0 (330 B)
fn+ 91: → FUN_00757200 (330 B)
fn+116: → FUN_0073c120 (330 B)
```

Each call is gated by a `CMP byte [ESI+0xe]` test (the
"Director-type tag" or "init-phase" flag). This is a
**multi-stage type-dispatched init** — slot 2 picks one of
5 per-Director-type initializer chains based on the type tag.

The 5 sub-initializers being identical-sized (330 B each) is
the hallmark of a code-generated dispatch table — the engine
likely emits one initializer per Director subtype (e.g.
Opening / Zone / Weather / Quest-bound / Generic) all sharing
a template body.

### Slot 3 — Lifecycle hook A (`FUN_006e1eb0`, 38 B)

```
56 8b 74 24 08       PUSH ESI; MOV ESI, [esp+8]
8b ce                MOV ECX, ESI
e8 54 56 5e 00       CALL FUN_00cc7510       ; vtable trampoline (MOV ECX,[ECX]; JMP)
8b 4c 24 0c          MOV ECX, [esp+0xc]
8b 10                MOV EDX, [EAX]
8b 42 04             MOV EAX, [EDX+4]
51                   PUSH ECX
8b 88 ec 00 00 00    MOV ECX, [EAX+0xec]     ; sub-object pointer at +0xec
56                   PUSH ESI
e8 0e be 08 00       CALL FUN_0076dce0 (214 B)
5e c2 08 00          POP ESI; RET 8
```

Takes 1 arg (this + 1 stack arg). Loads a sub-object pointer
from `[some_object+0xec]` and dispatches through a 214-byte
handler. **Most likely candidate: `_onInit`** — the standard
LuaControl-binding lifecycle hook that fires once per Director
instantiation, hands off to the engine's Director-init helper.

### Slot 5 — Lifecycle hook B (`FUN_006f6d30`, 46 B)

```
56 8b 74 24 0c       PUSH ESI; MOV ESI, [esp+0xc]
57 8b 7c 24 0c       PUSH EDI; MOV EDI, [esp+0xc]
56 57                PUSH ESI; PUSH EDI
e8 bf fb ff ff       CALL FUN_006f6900 (383 B)  ; AreaBase::slot5 — call PARENT first!
8b cf                MOV ECX, EDI
e8 c8 07 5d 00       CALL FUN_00cc7510 (vtable trampoline)
8b 00 8b 48 04       MOV EAX, [EAX]; MOV ECX, [EAX+4]
8b 89 ec 00 00 00    MOV ECX, [ECX+0xec]
56                   PUSH ESI
e8 b7 82 07 00       CALL FUN_0076f010 (42 B)
5f 5e c2 08 00       POP EDI; POP ESI; RET 8
```

Calls `FUN_006f6900` first — that's the **`AreaBase::slot5`
implementation**. Director's slot 5 is "do parent's behavior,
then add Director-specific tail." This is the
**`_callSuperClassFunc`** idiom (from item #3) materialized as
a hardcoded super-call inside the C++ override — the standard
MSVC pattern for an overridden virtual that needs to extend
the parent.

**Most likely candidate: `_onActivate` or `_onSetup`** — fires
when the Director becomes active (after init).

### Slot 6 — Lifecycle hook C (`FUN_006dcb10`, 51 B)

Three vtable-indirect calls in sequence to slots 13, 14, 15
of its own vtable, each with a different constant pointer:

```
8b 06                MOV EAX, [ESI]               ; this->vtable
8b 50 38             MOV EDX, [EAX+0x38]          ; slot 14
68 30 bd 34 01       PUSH 0x0134bd30              ; data block A
8b ce ff d2          MOV ECX, ESI; CALL EDX

8b 06 8b 50 34       MOV EDX, [EAX+0x34]          ; slot 13
68 50 bd 34 01       PUSH 0x0134bd50              ; data block B (32 bytes after A)
8b ce ff d2          MOV ECX, ESI; CALL EDX

8b 06 8b 50 3c       MOV EDX, [EAX+0x3c]          ; slot 15
68 70 bd 34 01       PUSH 0x0134bd70              ; data block C (32 bytes after B)
8b ce ff d2          MOV ECX, ESI; CALL EDX

5e c2 08 00          POP ESI; RET 8
```

The three contiguous 32-byte data blocks at
`0x0134bd30..0x0134bd90` are **listener registration
descriptors** (probably {event_id, callback_fn_ptr,
bound_this} triples). Slots 13/14/15 are the LuaControl-base
"register listener" virtuals.

So slot 6 is "register 3 specific event listeners with the
LuaControl base." **Most likely candidate: `_onSetupEvents`**
or part of `_onInit` — fires during Director setup to wire up
the standard 3 event handlers (e.g. timer-tick, scene-end,
player-event).

### Slot 11 — Director-only Lua hook A (`FUN_006e1f70`, 129 B)

This is one of two slots where Director has REAL behaviour but
all 3 sibling bases (Area, Actor, Quest) share the no-op
`FUN_00776340`. So slot 11 is **Director-only** in the
script-binding hierarchy.

Body sketch:

```
[SEH setup]
8b f9                MOV EDI, ECX (this)
8b 74 24 20          MOV ESI, [esp+0x20]
8b ce e8 70 55 5e 00 MOV ECX, ESI; CALL FUN_00cc7510  ; vtable trampoline
8b 00 8b 58 04       MOV EAX, [EAX]; MOV EBX, [EAX+4]
57                   PUSH EDI
8d 4c 24 24          LEA ECX, [esp+0x24]
51 8b ce             PUSH ECX; MOV ECX, ESI
e8 fe 53 5e 00       CALL FUN_00cc73b0 (41 B)
[zero local]
8b 8b ec 00 00 00    MOV ECX, [EBX+0xec]
... CALL FUN_00775890 (408 B)               ; main work
... CALL FUN_00cc9330 (1 B — empty stub)    ; intentional ICE marker
[SEH unwind, RET]
```

Heavy work via `FUN_00775890` (408 B). The 1-byte stub at the
end (`FUN_00cc9330`) is a deliberate empty function — possibly
a debug-build hook or a placeholder for an event-end callback.

**Most likely candidate: `playScene` / `playCutscene`** — the
standard Director-only Lua method that schedules a cutscene
clip (item #4 RaptureActionDamageCallClip family) for the
director's owning actor.

### Slot 12 — Director-only Lua hook B (`FUN_006e1ee0`, 136 B)

Same template as slot 11 but with different inner-call target:

```
[SEH setup, very similar to slot 11]
... CALL FUN_00773d90 (370 B)               ; main work
... CALL FUN_00cc9330 (1 B stub)
[unwind, RET]
```

Slot 12's main worker (`FUN_00773d90`, 370 B) is a sibling of
slot 11's (`FUN_00775890`, 408 B). The pair-pattern (similar
size, similar template, sibling sub-handlers) suggests
**slot 11 / slot 12 are a paired set/get or start/stop pair**.

**Most likely candidates:** `setSceneEnd` / `endScene` (start a
cutscene with end-trigger / signal cutscene end), or
`playScene` / `endScene`, or `addEventCondition` /
`removeEventCondition`. Without unluac output on a shipped
Director.prog file we can't disambiguate further.

### Slot 29 — Actor-shared hook (`FUN_006dbea0`, 13 B)

Tiny generic delegate trampoline — **identical shape to
QuestBase slot 28's `FUN_006dcfd0`** (item #7), just with a
different inner target:

```
51                   PUSH ECX
8b 4c 24 08          MOV ECX, [esp+8]
e8 c6 b4 5e 00       CALL FUN_00cc7370 (27 B)
c2 04 00             RET 4
```

Shared with `ActorBase` and `QuestBase` — same code in all
three. So slot 29 is an Actor-family generic hook: probably
**`getActor` / `getOwnerActor`** (return the controlling
actor handle as a Lua value).

## Confirmed parent: Director inherits from ActorBase

The diff confirms: DirectorBase shares slots 26 and 29 with
ActorBase and QuestBase but DIFFERS from AreaBase on those
same slots. So the inheritance chain is:

```
LuaControl                            (slots 0..19)
  └── (intermediate base, slots 20..27)
      └── ActorBase                   (slot 28: no-op)
          ├── DirectorBase            (Director-specific 11/12)
          ├── QuestBase               (slot 28: getQuestData hook)
          ├── CharaBase, NpcBase
          └── PlayerBase
```

`AreaBase` and `PrivateAreaBase` are siblings of `ActorBase`
on a different branch (their slot 26/29 differ from the actor
branch).

This confirms the script-binding hierarchy from the Lua
class-registry (item #3): `DirectorBaseClass` extends from
`ActorBaseClass` in the Lua-side classes, mirrored by the C++
DirectorBase extending ActorBase here.

## Summary table — DirectorBase Lua-callable surface

| Slot | Binding | Inferred Lua name | Confidence |
|---:|---|---|---|
| 0 | Destructor | (not Lua-callable) | — |
| 1 | Factory | `:new()` | High (standard pattern) |
| 2 | Typed init | (internal — type-dispatched ctor tail) | Medium |
| 3 | Lifecycle | `_onInit` | Medium (lifecycle position + sub-handler size) |
| 5 | Lifecycle | `_onActivate` / `_onSetup` | Medium (super-call + tail pattern) |
| 6 | Lifecycle | `_onSetupEvents` | Medium-high (3-listener-registration body) |
| 11 | Director-only | `playScene` / `playCutscene` | Low-medium (size + template) |
| 12 | Director-only | `setSceneEnd` / `endScene` | Low-medium (paired with slot 11) |
| 29 | Actor-shared | `getActor` / `getOwnerActor` | Low-medium (sibling-shared pattern) |

The other 26 slots inherited from LuaControl and the
intermediate base provide the standard event/work/timer/state
hooks documented in items #1-#7.

## How to disambiguate the inferred names

Two paths:

1. **Decompile a shipped `Director*BaseClass.prog` with
   `unluac`** — the script defines the per-Director-subtype
   methods. The set of methods called on `self` from a
   `DirectorBaseClass` script reveals the canonical Lua names
   that the engine routes through DirectorBase's vtable.
2. **Locate the metatable-build initializer** that wires Lua
   names → vtable slots. From item #3 it's known the Lua-class
   registry is built by `FUN_0078e3a0`; the metatable-build is
   a sibling function (probably nearby in `.text`) that
   consumes the same registry and emits the per-class
   metatables.

Both are focused follow-up passes. The structural map above is
the immediately-useful artifact for garlemald.

## Practical impact for garlemald

1. **Director object size = 112 bytes.** Garlemald's Rust-side
   Director representation is server-state-only (the C++
   instance lives client-side); but if garlemald ever wants to
   simulate a Director instance to validate against the binary,
   the 112-byte target tells it the C++ class is small.

2. **5 type-dispatched initializers.** The slot-2 multi-stage
   init suggests there are ~5 distinct Director subtypes the
   client recognizes natively. Garlemald's
   `OpeningDirector` / `ZoneDirector` / `WeatherDirector`
   trio hits 3 of those; the other 2 are likely
   `AfterQuestWarpDirector` and `GuildleveDirector` /
   `HarvestDirector` / `CraftDirector` (per Discord context).

3. **Slot 5 explicitly chains to AreaBase parent.** Confirms
   Director does NOT inherit from AreaBase (different vtable
   bodies on most slots) but DOES delegate to AreaBase's slot 5
   for shared activation logic. Garlemald's Director
   implementation should similarly call into the area-setup
   path during activation.

4. **Slot 6 registers 3 listeners with hardcoded data blocks
   at `0x0134bd30..0x0134bd90`.** These are data-driven
   listener descriptors — likely `{event_kind, fn_ptr,
   user_data}` triples. The 3 default listeners are part of
   the engine-baked Director setup; Lua scripts can add MORE
   listeners on top. Garlemald already drives this via
   `RegisterListener` packets it has been emitting.

5. **Slots 11 + 12 are paired Director-only methods.** The
   most likely identification is `playScene` + `endScene` (or
   `setSceneEnd`) — the methods OpeningDirector uses to
   trigger and close cutscenes. Garlemald already emits
   `KickEvent("pushDefault")` / `EndEvent` packets which
   correspond to these client-side hooks. The Director-only
   nature confirms these are NOT inherited from
   ActorBase / AreaBase / QuestBase — they're the engine's
   "this is a Director" capability.

## Phase 6 work pool — item #8 status

This closes Phase 6 item #8. Only #9 remains:

- #9 Functional `OpeningDirector` validation against
  garlemald's `man0g0.lua` — requires running the stack and
  comparing wire bytes. Highest practical payoff but the
  largest commit-of-time.

## Cross-references

- `docs/director_quest.md` — Phase 6 architecture
- `docs/lua_class_registry.md` — Phase 6 item #3 (the Lua
  class registry that names DirectorBaseClass + the source
  path `/Director/DirectorBaseClass.prog`)
- `docs/sync_writer.md` — Phase 6 item #4
- `docs/lua_bytecode_format.md` — Phase 6 item #5 (the
  `.prog` format — apply `unluac` to a shipped DirectorBase
  to disambiguate slot names)
- `docs/quest_dispatch.md` — Phase 6 item #7 (sibling
  diff-vs-base technique)
- `docs/lua_actor_impl.md` — Phase 6 item #6 (the engine-side
  90-slot actor methods that DirectorBase scripts query
  through)
- `project_garlemald_opening_director.md` (memory) —
  garlemald's OpeningDirector implementation that drives
  this DirectorBase via Lua-server scripts
- `project_garlemald_man0g0_seq000_complete.md` (memory) —
  end-to-end OpeningDirector + man0g0 success
- `project_meteor_discord_context.md` — Ioncannon notes on
  Director Lua method names (`playScene`, `endScene`,
  `setSceneEnd`, etc.)
