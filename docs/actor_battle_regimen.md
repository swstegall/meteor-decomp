# Phase 5 work-pool item #6 — Battle Regimen (chain) UI

> Last updated: 2026-05-03 — popup component identified
> (`LinkPopup`); skill-selection menu is Lua + Sqwt.

## The Battle Regimen "chain link" popup family

The full `App::Main::Element::Chara::*` per-character HUD popup
inventory (extending the 72-slot `CharaElement` base class, each
overriding only their destructor):

| Popup | Vtable RVA | Slots | Triggered by |
|---|---|---:|---|
| `DamagePlate` | `0xbe1e34` | 1 | Incoming damage / heal event |
| `LevelupPlate` | `0xbe22c8` | 1 | Class / job level-up |
| **`LinkPopup`** | `0xbe280c` | 1 | **Battle Regimen chain link succeeds** |
| `ExpPopup` | `0xbe2c78` | 1 | XP gain |
| `CountdownPopup` | `0xbe3100` | 1 | Countdown timer (e.g. cast time, leve start) |

**`LinkPopup`** is the Battle Regimen visual indicator. In FFXIV 1.x
combat, when a player executes a skill that completes a chain link
(extending an active Battle Regimen), this popup floats above the
character with a "Link!" notification. The rendering is identical in
shape to DamagePlate / LevelupPlate / ExpPopup — fly-up, alpha fade,
optional colour tint — because they all share the same
`CharaElement` parent (72 slots of common popup behaviour).

`LinkPopup`'s scalar destructor (slot 0) is at `FUN_007973f0`
(file `0x3973f0`); its first call goes to `FUN_00796ce0` — the
shared `CharaElement` popup base destructor for the LinkPopup
sub-family. The base class structure is the same as documented in
`docs/actor_damage.md`: 1-slot leaf overriding only `dtor`,
inheriting all rendering + animation behaviour from the popup base.

## What's NOT a hardcoded C++ class

The Battle Regimen system has TWO visible UI components in 1.x
combat:

1. **The "Link!" popup** (above) — fires on each successful chain
   hit. Hardcoded as `LinkPopup`.
2. **The chain prompt / next-skill picker** — the small UI element
   showing which skill in your bar will extend the current chain.

The second component (the prompt) does NOT show up as a dedicated
RTTI class. Searches for `Regimen`, `Combo`, `Chain` across the
entire RTTI surface returned only `Window::Debug::DebugComboBoxControl`
(unrelated — that's a debug dropdown widget). This means the chain
prompt is rendered through:

- Sqwt UI primitives (`Sqwt::Controls::Image` / `TextBlock`)
- Lua scripts (the `App::Lua::Script::Client::Control::*` family)
- Possibly an existing per-action-bar widget that overlays a
  highlight on the relevant skill icon

This matches item #5's finding (status-effect strip is also Lua +
Sqwt, not hardcoded). 1.x's combat HUD relies heavily on Lua for
data-driven decisions, with hardcoded C++ only where game-state
coupling is too tight.

## Architectural picture

```
Server: BattleRegimen state advances        (chain hit detected)
        │
        ▼
ApplyDamage / SetActorState packet         (with chain-flag in payload)
        │
        ▼
Client receives packet
        │
        ├──► Damage display path (item #4)
        │       → RaptureActionDamageCallClip → DamagePlate
        │
        ├──► Chain link popup (THIS item)
        │       → LinkPopup created via CharaElement factory
        │       → fly-up + fade-out animation
        │
        └──► Lua-side chain prompt update
                → updates the Sqwt-rendered "next skill" highlight
                → no dedicated C++ class
```

## Practical impact for garlemald

Garlemald already implements Battle Regimen state on the
server side (per `project-meteor-server` reference). For the chain
link popup to appear correctly, the damage / hit-result packet
needs:

1. **A chain-hit flag** in the packet payload — tells the client
   "this hit extended a chain, fire LinkPopup."
2. **The chain-step number** (or equivalent) — for the popup's
   text content and for any sound-effect variation per chain
   depth.
3. The packet target actor ID — the popup attaches to this
   actor's transform.

Validation, like item #5, is **empirical** rather than
decomp-driven for the chain prompt:

- Capture project-meteor-server's chain packets from the local
  Mac branch (`project_pmeteor_quest_system_mac_captures.md`).
- Diff against garlemald's chain packets using `packet-diff`.
- If the LinkPopup fires + the chain prompt highlights the right
  skill in garlemald, the wire shape is right.

The decomp's contribution here is the **confirmation that the
LinkPopup is a hardcoded C++ class with the same animation
shape as DamagePlate** (so once garlemald's chain packets work,
the visual will be right without any additional UI plumbing on
either side). The chain prompt highlighting is data-driven
through Lua and falls outside the binary surface.

## Phase 5 work pool — done

This closes Phase 5 work-pool item #6 and the entire Phase 5 plan
that started 2026-05-02:

| Item | Status | Doc |
|---|---|---|
| #1 — CharaActor field layout | ✅ done | `include/actor/chara_actor.h`, `docs/actor.md` |
| #2 — Status controllers | ✅ done | `docs/actor_status.md` |
| #3 — Action queue + motion | ✅ done | `docs/actor_action.md` |
| #4 — Damage display | ✅ done | `docs/actor_damage.md` |
| #5 — Status effect tick | ✅ done (negative result) | `docs/actor_status_effect.md` |
| #6 — Battle Regimen UI | ✅ done | `docs/actor_battle_regimen.md` |

## Cross-references

- `docs/actor.md` — Phase 5 plan + work-pool tracking
- `docs/actor_damage.md` — DamagePlate / cutscene-clip family
  (LinkPopup is a sibling popup, same parent class structure)
- `docs/actor_status_effect.md` — sibling negative-result item;
  the broader pattern of "1.x combat HUD is data-driven through
  Lua + Sqwt, not hardcoded C++"
- `ffxiv_1x_battle_commands_context.md` — Battle Regimen / chain
  metadata per-skill (which skills can chain, weaponskill IDs)
- `project_meteor_discord_context.md` — Ioncannon notes on
  Battle Regimen packet shape
- `land-sand-boat-server/xi-private-server.md` — XI's Skillchain
  system (the structural ancestor of FFXIV 1.x Battle Regimens;
  the SC `primary_sc` / `secondary_sc` / `tertiary_sc` shape is
  the same)
