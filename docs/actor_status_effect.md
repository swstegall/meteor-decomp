# Phase 5 work-pool item #5 — Status effect tick / display

> Last updated: 2026-05-03 — investigated; key finding is that 1.x's
> status-effect strip has **no dedicated C++ class hierarchy** (it's
> Sqwt UI + Lua). Empirical capture-driven validation is the right
> path for garlemald.

## Negative result: no `BuffIcon` / `StatusEffectStrip` class exists

A targeted RTTI search across the entire binary for status-effect-icon
display widgets came up empty. Search keywords (case-insensitive):
`BuffEffect`, `StatusEffect`, `EffectIcon`, `StatusIcon`, `StatusBar`,
`BuffBar`, `BuffList`, `EffectList`, `EffectPlate`, `StatusList`,
`EffectText`, `CharacterBuff`. None of these surface a dedicated
class in `App::Main::Element::*` or `App::Main::HUD::*`.

What the binary **does** have for status:

- `App::Main::Element::Window::Widget::Status` (1 slot) — leaf marker
- `App::Main::Element::Window::Widget::StatusWidget` (40 slots) — the
  player's own status-summary panel (the popup that opens with a
  hotkey showing your stats / buffs / debuffs in a list view, NOT
  the always-on icon strip)
- `App::Main::Element::System::TargetInfo` (10 slots) — sub-target
  info popup
- `App::Main::Element::Chara::NamePlate` (24 slots) — name + HP bar
  above each character

There's NO `BuffIconList` / `StatusEffectStrip` / `EffectIconStrip`
class. The always-on buff strip on each character (the modern
"grid of icons" UI) is **not implemented as a hardcoded C++ widget
hierarchy** in 1.x.

## How status effects are actually rendered in 1.x

Status effects flow through three layers:

1. **`Application::Main::SqwtInterface::*`** — the bridge between
   game data and the WPF-like Sqwt UI framework. The relevant
   `PacketData<...>` / `CommandPacket<...>` templates marshal
   status-effect updates from network packets into Sqwt-level
   property changes.
2. **Lua scripts** — the buff strip layout, icon binding, duration
   countdown, and stack-count rendering are driven by per-character
   Lua scripts (think the modern `addon.txt` / XAML+Lua stack but
   1.x-era). The `App::Lua::Script::Client::Control::CharaBase::*`
   namespace contains the relevant entry points.
3. **Sqwt UI primitives** — `Sqwt::Controls::Image`, `Sqwt::Controls::TextBlock`,
   `Sqwt::Controls::ProgressBar`, `Sqwt::Controls::StackPanel`, etc.
   (all 40-slot leaves of `Sqwt::FrameworkTemplate`) render the
   actual icons + countdown text + duration bars.

This matches the broader design pattern in 1.x: data-driven UI
through Lua + XAML / Sqwt, NOT hardcoded `Element::*` widgets per
visual element. The hardcoded `Element::*` widgets exist only
where game-state coupling is too tight for Lua to handle (NamePlate,
DamagePlate, etc.).

## Practical impact for garlemald

Because the rendering layer is Lua + Sqwt, not C++, **decompilation
won't surface a "expected packet shape" answer**. The right way to
validate garlemald's status-effect packets is **empirical**:

1. Capture a known-good status-effect packet sequence from
   project-meteor-server (the C# reference implementation) using
   the per-city packet-capture infrastructure under `captures/`.
2. Replay that sequence in garlemald and diff the wire bytes
   (using `packet-diff` from `server-workspace/packet-diff/`).
3. If the icons render correctly with garlemald's bytes, the
   packet shape is right. If not, the diff narrows where to look.

The relevant packet types from the wider workspace context:

- `SetActorIcon` / `SetActorEffect` — server tells client "actor X
  has buff/debuff Y"
- `RemoveActorEffect` — server tells client "remove buff Y from
  actor X"
- `SetActorEffectDuration` / status-effect tick packets — refresh
  the duration bar

These are documented in `ffxiv_classic_wiki_context.md` (Packet
Headers / Game Opcodes sections) and have C# reference
implementations in `project-meteor-server/`.

## Why this is a sufficient answer

For Phase 5's stated value ("what does the client expect from
garlemald's actor-state packets"), the answer here is:
**status-effect packets are forwarded into Lua + Sqwt, so as long
as the byte layout matches what the C# reference server sends,
the icon strip renders correctly.** The decomp-derived field
schema for status effects would only matter if the client did
something exotic with them in C++ (validation, rounding, special
display logic) — and the absence of a dedicated C++ class
strongly suggests it doesn't.

This is one of the cases where the C++ binary is a **thin pipe**
to the UI framework, and the UI framework is what owns the
actual rendering decisions. Garlemald's correctness validation
should target the framework's expected inputs (via packet
captures), not the framework's internals.

## Cross-references

- `docs/actor.md` — high-level Phase 5 plan (this is item #5)
- `docs/actor_damage.md` — work-pool item #4 (the popup family,
  including the lesson that `Element::Chara::*` has hardcoded
  popups but NO hardcoded buff-strip)
- `ffxiv_classic_wiki_context.md` — Packet Headers + Game Opcodes
  for the SetActorIcon family
- `project_meteor_discord_context.md` — Ioncannon / Tiam notes
  on Lua-driven UI (the buff-strip is mentioned as a Lua-callable
  refresh in `lua-scripting` channel)
- `captures/` — per-city packet captures for empirical validation
- `packet-diff/` — workspace tool for byte-level diffing
- `server-workspace/project-meteor-server/` — C# reference impl
