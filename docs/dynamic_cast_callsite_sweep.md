# Phase 9 ext — `__RTDynamicCast` callsite sweep (engine-wide class hierarchy)

> Last updated: 2026-05-16. Walked all 481 `__RTDynamicCast`
> (`FUN_009da6cc`) callsites in `ffxivgame.exe`, extracted the
> `(SrcType, TargetType)` RTTI Type Descriptor pair from each, and
> resolved every pair to its demangled class name via
> PE-data-section lookup.

## TL;DR

**129 distinct RTTI Type Descriptor addresses** recovered in a
single sweep — closing all RTTI gaps from Phase 9 #8b/#8d/#2 and
revealing 6 major class hierarchies the engine uses for polymorphic
dispatch.

- **Total callsites parsed**: 481 (of 483 grep matches — 2 didn't have
  clean PUSH-literal pairs and were skipped)
- **Distinct SrcType classes**: 32 (the polymorphic-dispatch source bases)
- **Distinct TargetType classes**: 103
- **Distinct (SrcType → TargetType) edges**: 123
- **Total classes seen**: 129

Top SrcTypes by reach:

| SrcType | # Targets | # Casts | Hierarchy |
|---|---:|---:|---|
| `Component::Lua::GameEngine::LuaControl` | 24 | 105 | **Lua-script-side class system** (every BaseClass from `lua_class_registry.md` + 3 new) |
| `Sqwt::FrameworkElement` | 21 | 29 | **Sqwt UI framework** (WPF-like — Border, Button, CheckBox, Grid, Label, ListBox, Panel, TabItem, TextBlock, TextBox, Window, etc.) |
| `Application::Lua::Script::Client::Control::ActorBase` | 10 | 53 | Lua-side actor sub-hierarchy (already mapped in #8d; here with concrete RTTIs) |
| `Application::Main::Element::Window::Debug::DebugBinder` | 9 | 22 | Debug-window data-binding system (template instantiations) |
| `Component::Lua::GameEngine::Work::InitializeInformation` | 8 | 13 | Work-table typed information subclasses |
| `Application::Main::Element::Window::Widget::WidgetBase` | 5 | 20 | UI widget templates (party manager, linkshell, etc.) |
| `Application::Scene::Actor::CDevActor` | 4 | 11 | Engine-side actor hierarchy (RaptureActor → CDevActor → CharaActor, etc.) |
| (24 more SrcTypes with 1-3 targets each) | — | — | Network channels, XML DOM, Patch, Cut Scheduler, etc. |

## Hierarchy 1: Lua-script-side class system (rooted at `LuaControl`)

The `Component::Lua::GameEngine::LuaControl` base has **24 known
subclasses** — the complete Lua-bindable class registry. This closes
all the open RTTI gaps in `docs/lua_class_registry.md`:

```
Component::Lua::GameEngine::LuaControl       (RTTI 0x01270b4c)
├── Application::Lua::Script::Client::Control::
│   ├── ActorBase                            (RTTI 0x01270964) [9 casts; Network-ns receiver SrcType]
│   │   ├── CharaBase                        (RTTI 0x012709a4) [17 ActorBase casts target this]
│   │   │   ├── NpcBase                      (RTTI 0x012709e4) [5 ActorBase casts]
│   │   │   └── PlayerBase                   (RTTI 0x012bfa48) [3 ActorBase casts]
│   │   │       └── MyPlayer                 (RTTI 0x012c19a4) [13 ActorBase casts]
│   │   ├── DirectorBase                     (RTTI 0x012bf9c8) [3 ActorBase casts]
│   │   ├── AreaBase                         (RTTI 0x012c2a6c) [1 ActorBase cast]
│   │   │   └── PrivateAreaBase              (RTTI 0x012c2d90)
│   │   ├── CutScene                         (RTTI 0x012c17ac) [7 ActorBase casts]
│   │   ├── DesktopWidget                    (RTTI 0x012c2108) [1 ActorBase cast]
│   │   ├── GroupBase                        (RTTI 0x012c07e8) [1 ActorBase cast]
│   │   ├── WorldMaster                      (RTTI 0x012c1328) [2 ActorBase casts]
│   │   └── QuestBase                        (RTTI 0x012c09a8)
│   │
│   ├── StatusBase                           (RTTI 0x012c31f8) [Phase 9 #2 — sibling of ActorBase]
│   ├── CommandEventRelationControlBase      (RTTI 0x012bfd88) [NEW]
│   ├── WidgetBase                           (RTTI 0x012c1964) [NEW]
│   ├── ItemBase                             (RTTI 0x012c0864) [NEW]
│   ├── SpreadSheet                          (RTTI 0x012c2308) [NEW]
│   ├── Debug                                (RTTI 0x012c0324) [NEW]
│   ├── Math                                 (RTTI 0x012c09e8) [NEW]
│   └── Global                               (RTTI 0x012c27c0) [NEW]
│
└── Component::Lua::GameEngine::             (engine-internal LuaControl variants)
    ├── LuaTentativeControl                  (RTTI 0x012c0828) [7 casts]
    ├── LuaGlobalTentativeControl            (RTTI 0x012c0a28) [1 cast]
    └── LuaManyTentativeControlCreator       (RTTI 0x012bfc80) [1 cast]
```

Key observations:

- **`StatusBase`, `ItemBase`, `WidgetBase`, etc. extend `LuaControl`
  directly** — NOT `ActorBase`. The "actor-like" appearance was a
  naming convention, not a true inheritance edge.
- **`MyPlayer` is at the bottom** of the player chain
  (`ActorBase → CharaBase → PlayerBase → MyPlayer`) — confirms #8d's
  inferred hierarchy.
- **`CutScene`, `DesktopWidget`, `Global`, `Math`, `SpreadSheet`,
  `Debug`** are siblings of the actor classes — they're the
  engine-utility Lua-bindables (per `lua_class_registry.md`).
- **3 engine-internal `LuaControl` variants** (`LuaTentativeControl`
  family) aren't in `lua_class_registry.md` — they're engine-only
  intermediate classes, possibly used for class-instantiation
  bookkeeping.

### Cross-reference to receiver inventory

Of the 24 LuaControl-derived classes:
- **5 are cast targets in System-namespace receivers** (StatusBase via
  ChangeActorSubStatStatus #2; CharaBase via the same; others TBD)
- **8 are cast targets in Network-namespace receivers via the
  ActorBase intermediate path** (#8b sweep)
- **11 have no known direct receiver** (CutScene, DesktopWidget,
  Global, Math, SpreadSheet, Debug, ItemBase, WidgetBase, GroupBase,
  CommandEventRelationControlBase, QuestBase) — handlers for these
  live elsewhere (likely in dedicated subsystems, not the receiver
  inventory)

## Hierarchy 2: Sqwt UI framework (WPF-like)

`Sqwt` (the **S**qe**x** **W**indows **T**oolkit) is a C++ port of
WPF / Silverlight UI primitives. The hierarchy:

```
Sqwt::Object                                 (RTTI 0x01269cfc)
└── Sqwt::DependencyObject                   (RTTI 0x01269ef0)
    ├── Sqwt::Media::Visual                  (RTTI 0x01269ed0)
    │   └── Sqwt::Controls::RadioButton      (RTTI 0x012e49f8)
    └── Sqwt::UIElement                      (RTTI 0x01269eb0)
        └── Sqwt::FrameworkElement           (RTTI 0x01269e8c)  [21 cast targets]
            ├── Sqwt::Window                 (RTTI 0x01269e20)
            ├── Sqwt::Controls::
            │   ├── Border                   (RTTI 0x0126fb84)
            │   ├── Button                   (RTTI 0x0126d244)
            │   │   └── (extends Primitives::ButtonBase, RTTI 0x0126d28c)
            │   ├── CheckBox                 (RTTI 0x0126d2e4)
            │   ├── ContentControl           (RTTI 0x01269e3c)
            │   ├── ContentPresenter         (RTTI 0x012e0c74)
            │   ├── Grid                     (RTTI 0x0126d268)
            │   ├── IconControl              (RTTI 0x0126d21c)
            │   ├── ItemsControl             (RTTI 0x0126d6ec)
            │   ├── Label                    (RTTI 0x0126d1f8)
            │   ├── ListBox                  (RTTI 0x0126d30c)
            │   ├── ListBoxItem              (RTTI 0x0126ebf4)
            │   ├── Panel                    (RTTI 0x0126d330)
            │   ├── ProgressBar              (RTTI 0x012bd800)
            │   ├── SparkleControl           (RTTI 0x012bada8)
            │   ├── TabItem                  (RTTI 0x012e8bc0)
            │   ├── TextBlock                (RTTI 0x012bd188)
            │   ├── TextBox                  (RTTI 0x012d6c20… see TBD)
            │   ├── Primitives::
            │   │   ├── ButtonBase           (RTTI 0x0126d28c)
            │   │   └── ToggleButton         (RTTI 0x012d6e44)
            │   └── Primitive::
            │       └── ContentLabel        (RTTI 0x012bd09c)
            └── (extends UIElement / IInputElement)
                Sqwt::InputElement           (RTTI 0x01269d3c)
                Sqwt::IInputElement          (RTTI 0x01269d5c)
                Sqwt::DesktopWindow          (RTTI 0x01269dfc)
```

And the XML / Animation / Resource branches:

```
Sqwt::Xml::XmlElemenBase                     (RTTI 0x012e11ec)
└── Sqwt::Xml::XmlElement                    (RTTI 0x012bad84)
    (parented to Component::Xml::DOM_Node, RTTI 0x012bad5c
     → DOM_Element 0x012e1214)

Sqwt::Media::Animation::BeginStoryboard      (RTTI 0x012e159c)
Sqwt::Media::DrawingContext                  (RTTI 0x0126dce8)
Sqwt::ResourceDictionary                     (RTTI 0x012bad08)
Sqwt::FontAttribute                          (RTTI 0x0126e5dc)
```

Sqwt is **structurally a C++ port of WPF**: same `Object →
DependencyObject → Visual / UIElement → FrameworkElement → Controls.*`
chain, same `BeginStoryboard` animation primitive, same
`ResourceDictionary` pattern, same `DependencyObject` reactive
property system. Given FFXIV 1.x ships in 2010 and WPF was
established by then, this looks like Squenix licensed (or built
from scratch following) WPF's design language.

The bridge between game logic and UI is `Application::Main::*`:

```
Application::Main::RaptureElement            (RTTI 0x01269c58)
└── Application::Main::Element::
    ├── RelativeElement                      (RTTI 0x01269c88)
    └── Chara::CharaElement                  (RTTI 0x01269cc0)

Application::Main::RaptureElementContainer   (RTTI 0x01269dc4)
  (extends Sqwt::DesktopWindow per 10 casts)

Application::Main::Element::Window::Widget::WidgetBase  (RTTI 0x0126b800)
└── LinkshellListWidget                      (RTTI 0x0126b8d8)
    LinkshellMembersListWidget              (RTTI 0x0126b928)
    PartyManagerWidget                      (RTTI 0x0126b978)
    PartyRootWidget                          (RTTI 0x0126b840)  [14 casts — most-used widget]
    PcMatchingViewWidget                     (RTTI 0x0126b888)
```

## Hierarchy 3: Engine-side Actor hierarchy (confirmed)

The previously-documented engine-side actor hierarchy (from
`memory/reference_meteor_decomp_actor_rtti.md`) is now CONFIRMED via
the sweep's cast edges:

```
SQEX::CDev::Engine::Common::ISceneObject::IActor  (RTTI 0x012b7c48) [80 casts target this!]
├── Application::Scene::RaptureActor            (RTTI 0x012b77e0)
│   └── (cast targets: CharaActor, CutManagerActor, IActor)
│       Application::Scene::Actor::CDevActor    (RTTI 0x012b7ab4)
│       └── Application::Scene::Actor::Chara::CharaActor  (RTTI 0x012b7810)
│           Application::Scene::Actor::Chara::WeaponActor (RTTI 0x012b77a8)
│           Application::Scene::Actor::System::CutManagerActor (RTTI 0x012b7894)
│           Application::Scene::Actor::System::GameManagerActor (RTTI 0x012b7ae4)

SQEX::CDev::Engine::Common::ISceneObject::IWorld (RTTI 0x012b6784)
└── Application::Scene::RaptureWorld           (RTTI 0x012b671c)
```

**The 80 casts from `ISceneObject::IActor` make it the second-most-cast
type after `LuaControl`.** This is the engine's "universal scene-object
actor" base — every code path that touches an actor at the engine
level starts from this type.

## Hierarchy 4: Work-table typed information (LuaControl::Work::*)

The work-table system uses a tagged union of typed information
subclasses. All extend `InitializeInformation`:

```
Component::Lua::GameEngine::Work::InitializeInformation  (RTTI 0x0130d608)
├── ActorInformation                                     (RTTI 0x0130d9ec)
├── ArrayInformation                                     (RTTI 0x0130d68c)
├── AssignInformation                                    (RTTI 0x0130d7bc)
├── FloatInformation                                     (RTTI 0x0130d920)
├── Integer16Information                                 (RTTI 0x0130d9a8)
├── Integer32Information                                 (RTTI 0x0130d960)
├── NestingInformation                                   (RTTI 0x0130d7fc)
└── ReserveInformation                                   (RTTI 0x0130d64c)
```

**These match the 8 work-field-kind enums** documented in
`docs/work_field_inventory_index.md`. The runtime uses dynamic_cast
to dispatch on field type when reading work-table memory.

## Hierarchy 5: Debug-window data binding (template instantiations)

```
Application::Main::Element::Window::Debug::DebugBinder   (RTTI 0x012bb490)
├── DebugLabelSpinBinder                                  (RTTI 0x012bb800)
├── DebugLabelSpinRefBinder                               (RTTI 0x012bb7b0)
└── (template instantiations — H/I/M = 8/16/32-bit kinds)
    H::?$DebugBinderT                                     (RTTI 0x012bb6c0)
    H::?$DebugBinderRefT                                  (RTTI 0x012bb708)
    I::?$DebugBinderT                                     (RTTI 0x012bb630)
    I::?$DebugBinderRefT                                  (RTTI 0x012bb678)
    M::?$DebugBinderT                                     (RTTI 0x012bb5a0)
    M::?$DebugBinderRefT                                  (RTTI 0x012bb5e8)
    Sqex::Misc::VUtf8String::?$DebugBinderT               (RTTI 0x012bb750)
```

9 template instantiations of a single template class. Standard MSVC
template-RTTI pattern (mangled names include the `?$` template
marker). These are likely "data binders" for the debug HUD's
inspectable fields — H/I/M correspond to half/integer/maybe-money or
similar field types.

## Hierarchy 6: Network channel template instantiations

```
Application::Network::ZoneProtoChannel::TZoneProtoUp::
    ?$ConnectionManagerTmpl                              (RTTI 0x0131b468)
└── Application::Network::ZoneProtoChannel::
    ServiceConsumerConnectionManager                     (RTTI 0x0131b4f8)

Application::Network::ChatProtoChannel::TChatProtoUp::
    ?$ConnectionManagerTmpl                              (RTTI 0x0131bfa8)
└── Application::Network::ChatProtoChannel::
    ServiceConsumerConnectionManager                     (RTTI 0x0131c038)
```

The ConnectionManager is a template parameterized on the protocol
(`TZoneProtoUp` / `TChatProtoUp`). Both up-channels have a single
`ServiceConsumerConnectionManager` concrete impl, suggesting only
one client-side connection manager per channel type.

## Other recovered hierarchies

### Cut-scene scheduler
```
SQEX::CDev::Engine::Cut::Scheduler::IClip               (RTTI 0x012b7a10)
├── Application::Scene::Cut::Clip::RaptureBindCameraClip (RTTI 0x012b7998)
├── SQEX::CDev::Engine::Cut::Plugins::CameraClip        (RTTI 0x012b79d8)
└── SQEX::CDev::Engine::Cut::Plugins::LocalCameraClip   (RTTI 0x012b7a44)

SQEX::CDev::Engine::Lay::Stella::External::Cut::Scheduler::
    ILaySchedulerManipulator                            (RTTI 0x012d0c90)
└── SQEX::CDev::Engine::Lay::Default::External::Cut::Scheduler::
    LaySchedulerManipulator                             (RTTI 0x012d0cf0)
```

### Patch system (unexpected finding — torrent-based downloads)
```
Component::Patch::PatchDownloadInterface                (RTTI 0x012d614c)
└── Component::Patch::PatchTorrentDownload              (RTTI 0x012d6184)
```

The patch system uses **BitTorrent** (or torrent-like protocol) for
downloads. Only one concrete `PatchDownloadInterface` impl.

### Lua actor impl pair
```
Application::Lua::Script::Client::LuaActorImplInterface (RTTI 0x01270a90)
├── Application::Lua::Script::Client::LuaActorImpl      (RTTI 0x01270b10)
└── Application::Lua::Script::Client::NullActorImpl     (RTTI 0x01270ad4)
```

Confirms `docs/lua_actor_impl.md`'s 90-slot interface + impl pair.
**Surprise**: there's also a `NullActorImpl` (null-object pattern)
used 6× — when a script needs an actor reference but none is bound.

### Lua-Group builders (from Phase 8)
```
Application::Lua::Script::Client::Group::PacketRequestBase (RTTI 0x012bf788)
├── BreakupBuilder                                          (RTTI 0x012bf7d0)
└── EntryBuilderBase                                        (RTTI 0x012bf818)
```

These were mentioned in `docs/group_system_decomp.md` (Phase 8) but
RTTI addresses weren't recovered until this sweep.

### XML DOM
```
Component::Xml::DOM_Node                                (RTTI 0x012bad5c) [27 casts]
├── Component::Xml::DOM_Element                         (RTTI 0x012e1214)
├── Sqwt::Xml::XmlElemenBase                            (RTTI 0x012e11ec)
└── Sqwt::Xml::XmlElement                               (RTTI 0x012bad84)
```

XML DOM with both a Component-level base and a Sqwt-level extension.

## How to use this map

For future Phase 9 / matching-decomp work:

1. **RTTI address quick reference** — when reading a function that has
   `PUSH 0x????` before a `CALL 0x009da6cc`, look up the address in
   `build/dynamic_cast_callsites.json` to know what class is being
   cast.
2. **Class-hierarchy resolution** — for a class you discover by name,
   find its RTTI address here, then search the binary for
   xrefs (4-byte LE pattern) to identify its vtable, ctor, dtor sites.
3. **Receiver expansion** — there are 481 polymorphic-dispatch sites
   in the engine; the receiver inventory only covers 32 of them. The
   remaining ~449 are other engine subsystems with their own cast
   patterns. If you need to find what handles opcode X, the sweep
   data tells you what classes are being cast in the area.

## Method

`tools/sweep_dynamic_cast.py`:
1. For each `.s` file in `asm/ffxivgame/`, scan line-by-line
2. Track the most recent 10 `PUSH 0x<literal>` instructions
3. On `CALL 0x009da6cc`, extract the 2 most recent literal PUSHes
   that fall in the data-section range (`0x1000000..0x2000000`)
4. The 5 args in file order are: `isReference, TargetType, SrcType,
   vfDelta, inptr` (right-to-left pushed). So the **last 2 literal
   PUSHes are TargetType (earlier in file) + SrcType (closer to CALL)**.
5. Resolve each RTTI address by:
   - Locating its `.data` file offset via PE section table
   - Reading the mangled name string at offset+8 (TypeDescriptor
     skips vtable_ptr + spare = 8 bytes)
   - Demangling `.?AV<Name>@<NS>@<NS>...@@` → `NS::...::Name`
6. Dump structured JSON to `build/dynamic_cast_callsites.json` for
   downstream processing

Run time: ~5 seconds on a 12 MB binary with 94 000 asm files.

## Cross-references

- `docs/receiver_classes_inventory.md` — Phase 9 #1 receiver inventory
  (32 of the 481 callsites are here)
- `docs/event_status_condition_receivers_decomp.md` — Phase 9 #8b
  (first __RTDynamicCast pair recovered, surfacing the technique)
- `docs/event_change_actor_substat_status_decomp.md` — Phase 9 #2
  (discovered `LuaControl` as the System-namespace SrcType)
- `docs/lua_class_registry.md` — Phase 6 #3 (Lua class registry —
  every `*BaseClass` entry now has a concrete C++ RTTI address)
- `docs/lua_actor_impl.md` — Phase 6 #6 (LuaActorImpl pair — RTTI
  addresses now confirmed)
- `docs/group_system_decomp.md` — Phase 8 (Group builders — RTTI
  addresses now recovered)
- `docs/work_field_inventory_index.md` — Phase 6 (work-field kinds —
  the 8 InitializeInformation subclasses match the registered kinds)
- `memory/reference_meteor_decomp_actor_rtti.md` — engine-side actor
  RTTI walk (confirmed by this sweep's cast edges)
- `build/dynamic_cast_callsites.json` — raw structured data (all 481
  callsites + RTTI address map)
- `tools/sweep_dynamic_cast.py` — the extractor
