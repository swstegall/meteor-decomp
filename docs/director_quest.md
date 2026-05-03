# Phase 6 — Director / Quest framework architecture

> Last updated: 2026-05-03 — kickoff inventory + key reframing
> from "C++ director hierarchy" to "Lua scripts on a C++ Lua-binding
> base-class scaffolding."

## Key reframing — the directors / quests / NPCs are NOT C++ classes

Initial Phase 6 plan said "decompile OpeningDirector, ZoneDirector,
WeatherDirector." That framing is wrong. RTTI search across the
entire binary for `OpeningDirector`, `ZoneDirector`, `WeatherDirector`,
`AfterQuestWarpDirector`, `GuildleveDirector`, `HarvestDirector`,
`CraftDirector`, `SimpleContent` returned **zero** matches.

That's because in 1.x, the directors / private areas / quests / NPCs
are all **Lua scripts** sitting on top of a small set of **C++ base
classes** that provide the binding scaffolding. The C++ binary
exposes `DirectorBase`, `PrivateAreaBase`, `QuestBase`, `NpcBase`,
etc. as Lua-script-bindable bases; each concrete director (e.g.
"OpeningDirector for man0g0 Limsa intro") is a Lua script that
subclasses one of these bases and is loaded at runtime.

This matches what `project-meteor-server` and `garlemald-server` look
like in practice: the Lua scripts under
`scripts/quests/man/man_0_0/` and `scripts/directors/` ARE the
directors. The C++ side exposes the base classes (this binary) +
the Lua VM glue that binds them (also this binary).

## The C++ Lua-binding base classes

All under `Application::Lua::Script::Client::Control::*`. Three
size-buckets of base classes; the slot count tells you how rich
the per-Lua-method binding surface is:

### Tier 1 — ~34-slot bases (small to medium binding surface)

| Base class | Vtable RVA | Slots | What Lua subclasses it |
|---|---|---:|---|
| `ActorBase` | `0xbd4fe4` | 34 | Actor scripts (catch-all base) |
| `DirectorBase` | `0xbd5d6c` | 34 | OpeningDirector, ZoneDirector, WeatherDirector, AfterQuestWarpDirector, GuildleveDirector, HarvestDirector, CraftDirector, etc. |
| `AreaBase` | `0xbd63d4` | 35 | Public-area zone scripts |
| `PrivateAreaBase` | `0xbd653c` | 35 | Private-instance zone scripts |
| `QuestBase` | `0xbdfdd0` | 35 (counted as 1 in RTTI dump — leaf) | Per-quest scripts (`man0g0`, `man1l0`, etc.) |
| `DebugBase` | `0xbd5274` | 34 | Debug commands |
| `CommandDebuggerBase` | `0xbd510c` | 34 | Debug `!command` hooks |

Slots 0..19 are inherited from `Component::Lua::GameEngine::LuaControl`
(20 slots) — the Lua-bindable C++ object root. Confirmed by the
shared slot bodies across the 7 bases above:

- slots 7..8 = `FUN_005b8d90` (paired hook — likely Lua-table
  read / write or get/set context)
- slot 9 = `FUN_005c5c80`
- slot 10 = `FUN_00776340`
- slot 13 = `FUN_00c37620`
- slot 14 = `FUN_006d6f80`
- slots 15..19 = `FUN_00712b40` (all the same — universal
  "default-no-op" stub for unbound Lua methods)
- slot 20 = `FUN_00ab7340` (script-bound Lua-callable override)
- slots 21..23 = `FUN_0053c440` (default-no-op pattern)

So slots 0..19 are LuaControl plumbing; slots 20..33+ are the
script-base-specific Lua-callable hooks.

### Tier 2 — ~41-slot bases (richer Chara / NPC binding)

| Base class | Vtable RVA | Slots |
|---|---|---:|
| `CharaBase` | `0xbd5cac` | 41 |
| `NpcBase` | `0xbd647c` | 41 |
| `WidgetBase` | `0xbd5a74` | 38 |

`CharaBase` extends `ActorBase` with 7 more chara-specific slots
(slots 34..40 on top of the ActorBase 34). `NpcBase` is a sibling
of `CharaBase` with the same 41-slot count but different slot
bodies starting at slot 4 — different Lua-callable surface for
NPC-specific events (`onTalk`, `onPushDefault`, etc.).

### Tier 3 — `PlayerBase` (133 slots, the player-control surface)

`Application::Lua::Script::Client::Control::PlayerBase` (RVA
`0xbd5e04`, **133 slots**) is the big one. Player scripts get the
full kitchen sink: every UI event, every state transition, every
command hook, every menu callback.

Slots 41..132 are dominated by a long run of 16-byte trampoline
functions (`FUN_006de8a0`, `FUN_006de8b0`, …, `FUN_006de900`,
…) — these are the **Lua method bindings**, one per Lua-callable
method that PlayerBase exposes. Each slot is the dispatch point
for a specific Lua name (e.g. slot 41 = `getHp`, slot 42 =
`getMp`, etc. — the exact name-to-slot mapping requires a
follow-up Ghidra walk through the Lua method-registration
table).

This is why PlayerBase has so many slots vs CharaBase's 41:
PlayerBase exposes far more state to Lua because the player's
script needs to read inventory, stats, party state, all UI, etc.

### Tier 4 — leaf bases (1 slot, just a dtor)

| Base class | Vtable RVA | Slots |
|---|---|---:|
| `CommandBase` | `0xbdf834` | 1 |
| `StatusBase` | `0xbdf8d4` | 1 |
| `JudgeBase` | `0xbdfd38` | 1 |
| `QuestBase` (leaf wrap) | `0xbdfdd0` | 1 |

These are 1-slot leaves whose entire useful behaviour comes from
their parent class. They exist as RTTI markers for `dynamic_cast`
in the Lua VM glue.

## The Lua VM glue (`Component::Lua::GameEngine::*`)

The C++ side of the Lua bridge is huge — ~250 distinct RTTI
classes under `Component::Lua::GameEngine::*`. The most
load-bearing pieces:

### Core VM bridge

| Class | Vtable RVA | Slots | Role |
|---|---|---:|---|
| `LuaControl` | `0xd0e30c` | 20 | The Lua-bindable C++ object root (parent of all `*Base` classes above) |
| `LuaObject` | `0xd0e300` | 1 | Marker leaf for raw Lua values |
| `LuaTentativeControl` | `0xd0e360` | 7 | Pending Lua control (allocated but not bound to script) |
| `LuaGlobalTentativeControl` | `0xd0e380` | 1 | Tentative control in global scope |
| `ErrorHandler` | `0xd0e564` | 68 | The Lua-error-handling chain (68 slots = many error categories) |
| `LuaTimer` | `0xd0e678` | 2 | Lua-callable async timer |

### Type marshalling — `Parameter::StackOperator<T>`

Each C++ → Lua parameter type has a 9-slot
`StackOperator<T>` specialisation that knows how to push / pop
that type onto the Lua stack:

- `int`, `float`, `bool`, `Sqex::Misc::Utf8String`
- `LuaControl*` (any of the binding bases)
- `AutoReleaseTemp`, `Nil`, `Table`
- `IndividualIndex` (player or actor reference)
- `Variable`
- `LuaControlArray`, `LuaManyTentative`, `IntegerArray`,
  `VariantVector`
- `WhichStackOperator` (the dispatcher)

### Function binding — `Functor::MemberFunctionHolder<...>`

Each C++ method exposed to Lua has a 2-slot
`MemberFunctionHolder` template instantiation. The dump shows
~50+ visible instances at RVAs 0xbd5b10..0xbd77dc — these are
the Lua-callable `*Base` methods (`onTalk`, `onPush`, `onUpdate`,
`getHp`, etc.).

### Work memory + SharedWork

The "Work" subsystem manages **server-synchronised state** —
fields on Lua-controlled objects that the server can poke via
SetActorProperty / SetGroupSync packets:

| Class | Vtable RVA | Slots | Role |
|---|---|---:|---|
| `Work::Memory::SyncWriterBase` | `0xd101a8` | 8 | Base for type-typed sync writers |
| `Work::Memory::SyncWriterBoolean` | `0xd101cc` | 8 | bool field |
| `Work::Memory::SyncWriterInteger8/16/24/32` | `0xd101f0..0xd1025c` | 8 | int fields by width |
| `Work::Memory::SyncWriterFloat` | `0xd10280` | 8 | float field |
| `Work::Memory::SyncWriterString` | `0xd10358` | 8 | Utf8String field |
| `Work::Memory::SyncWriterActor` | `0xd10334` | 8 | actor reference |
| `Work::Memory::SyncWriterArray*` | `0xd102c8..0xd103c4` | 8 | array fields (incl. typed actor arrays) |
| `Work::*Information` (Boolean, Int8/16/24/32, Float, String, Actor, Timer, Index) | `0xd0fbc4..0xd0fffc` | 29 | Field metadata (type tag) |
| `WorkMemoryAllocator::Impl<size,count>` | `0xbdffe4..0xbe0164` | 7 | Slab allocators for Work objects (12 sizes hardcoded) |

This is the **`SharedWork` / `playerWork` / `groupWork` system**
that garlemald has been wrestling with: every Lua-script field
declared as part of a `*Work` table is backed by one of these
SyncWriter instances, which marshals updates onto the wire.

### Other VM pieces

- `WorkMemoryAllocator::Impl<size,count>` — 12 slab-allocator
  specialisations, sized for the most common Work-field sizes:
  `<18688,1>`, `<512,1>`, `<10496,1>`, `<256,16>`, `<512,4>`,
  `<256,64>`, `<384,64>`, `<1408,2>`, `<20480,8>`, `<1024,8>`,
  `<1280,8>`, `<512,8>`, `<64,800>`. The huge `<20480,8>` is for
  the largest Work tables (probably PlayerBase's full work block).
- `LuaManyTentativeControlCreator` — factory for batches of
  tentative controls (used during script init).
- `LpbLoader` — `.lpb` resource loader (compiled Lua bytecode).
- `Functor::MemberFunctionHolder<...>` — 50+ visible instances
  binding individual `*Base::method()` C++ methods into Lua.

## LuaActorImpl — the actor-side Lua entry point

`Application::Lua::Script::Client::LuaActorImpl` (RVA
`0xbdfb2c`, **90 slots**) is the runtime Lua-script-host attached
to each script-controlled actor. It pairs with
`LuaActorImplInterface` (90 slots, RVA `0xbdf98c`) — the
abstract interface side of the impl pattern.

90 slots is roughly 3× the size of DirectorBase, suggesting the
actor-side Lua hosting includes:
- Per-actor lifecycle hooks (`onCreate`, `onDestroy`,
  `onUpdate`, `onTalk`, `onPush`, etc.)
- Resource-loading hooks (icon, name, animations)
- Interaction hooks (target, untarget, click, hover)
- Combat hooks (engage, disengage, takeDamage, dealDamage)
- AI hooks (move, idle, pathfind)
- Scheduler hooks (spawn / despawn timers)

The exact slot-name mapping is a focused follow-up Ghidra walk
through the Lua method-registration tables.

## What this means for garlemald

Garlemald's existing Lua scaffolding is **architecturally
correct** — it already drives a Lua VM with the same set of
`*Base` classes (DirectorBase, PrivateAreaBase, QuestBase,
PlayerBase, NpcBase) bound from Rust. The decomp confirms:

1. **The class hierarchy is right.** Garlemald's
   `LuaPlayer` / `LuaActor` / `LuaQuestHandle` correspond
   directly to PlayerBase / ActorBase / QuestBase as the
   client expects them.
2. **The `SharedWork` / `playerWork` mechanism is the engine's
   primary state-sync layer** for Lua-scripted state. Every
   Lua-declared work field has a typed SyncWriter on the C++
   side and uses one of the 12 slab-allocator sizes.
3. **The Lua method registration uses
   `MemberFunctionHolder<&CharaBase::method>` template
   instances** — meaning every Lua-callable C++ method is
   visible in the binary as its own RTTI entry. We can recover
   every Lua method name by walking these.
4. **Quest scripts are NOT a special case.** `QuestBase` is
   one of ~10 sibling `*Base` classes. The Lua-loaded `man0g0`
   etc. scripts subclass QuestBase the same way an
   OpeningDirector subclasses DirectorBase.

## Phase 6 work pool

| Item | Description | Status |
|---|---|---|
| #1 | Lua VM glue inventory | ✅ done (this doc) |
| #2 | Director / Area / Quest base-class identification | ✅ done (this doc) |
| #3 | Slot-name recovery for `*Base` Lua hooks | 🔲 pending — walk `MemberFunctionHolder<...>` instantiations to map slot → Lua method name |
| #4 | `SyncWriter` wire format | 🔲 pending — decompile slot 1 of each `SyncWriter*` (the "write field to wire" path) and cross-reference with garlemald's SetActorPropertyPacket builder |
| #5 | `.lpb` (compiled Lua bytecode) format | 🔲 pending — decompile `LpbLoader::ResourceEvent` (slot 1?) to recover the bytecode header + chunk format |
| #6 | LuaActorImpl 90-slot map | 🔲 pending — walk slots 0..89 to identify lifecycle hooks |
| #7 | Quest dispatch path | 🔲 pending — decompile `QuestBase` slot 0 + walk callers to find the quest-script-load entry point |
| #8 | Director scheduling | 🔲 pending — decompile `DirectorBase` slots 20..33 (the Lua-callable hooks) |
| #9 | Functional `OpeningDirector` validation | 🔲 pending — drive a fresh-start-limsa session through the original .exe and validate garlemald's lua/directors/man0g0.lua against the binary's expectations |

## Cross-references

- `docs/wire-protocol.md` — Phase 3 wire architecture (the GAM
  parameter system that backs SyncWriter wire updates)
- `docs/actor.md` — Phase 5 (CharaActor field layout — the C++
  side that Lua scripts on top of CharaBase observe via
  SyncWriter Work fields)
- `project_garlemald_opening_director.md` (memory) — garlemald's
  current OpeningDirector + man0g0 Lua state
- `project_garlemald_run_event_function.md` (memory) — the
  RunEventFunction wire layout (what triggers Lua hooks
  client-side)
- `project_meteor_discord_context.md` — Ioncannon / Tiam / etc.
  on Lua scripting patterns: `processEvent`, `onTalk`, `Seq000`,
  `quest:GetData()`, `talkDefault`, `ElevatorStandard.lua`,
  `Shop.lua`, `MotionPack ID (1000-1109)`
- `land-sand-boat-server/xi-private-server.md` — XI's
  `interaction_framework.md` is the structural cousin (Lua
  scripts subclass C++ Trader / NPC / Region bases the same way)
- garlemald-server's `map-server/src/lua/*` — the Rust-side
  `LuaPlayer` / `LuaActor` / `LuaQuestHandle` bindings that
  mirror this client architecture
