# Phase 5 — Actor + Battle architecture

> Last updated: 2026-05-02 — kickoff inventory.

This document captures the Actor RTTI inventory and the planned
Phase 5 work. It supersedes the speculative "battle math in client"
framing in `PLAN.md` — combat math is **server-side** in 1.x (as in
all FFXIV-style MMOs). What the client has is **Actor display +
status state machines + animation dispatch**, all of which are
useful for garlemald's correctness but answer different questions
than "what's the damage formula."

## Key reframing — the damage formula is NOT in the client

All authoritative combat math (damage rolls, hit/crit chance, status
durations, stat curves) is computed on the **server**. The client
receives result packets (e.g. `ApplyDamage`, `BattleAction`,
`SetActorState`) and renders them via `RaptureActionDamageCallClip`
animation, the `DamagePlate` UI element, and various status
controllers.

So Phase 5's value for garlemald is **not** "extract the damage
formula from the binary" — it's:

1. **Actor field schema**: what does the client expect each Actor
   to look like in memory? Garlemald sends `SetActorProperty`
   packets to populate these fields; if the client expects a u8
   where garlemald sends a u16, the field renders incorrectly.
2. **Status effect display logic**: how does the client decide
   which icon / duration / overlay to show for a given status
   packet?
3. **Motion-pack ID dispatch**: which animation plays for which
   skill? (Cross-reference: `ffxiv_1x_battle_commands_context.md`
   plus the per-skill motion ID column.)
4. **Battle Regimen (combo) display**: how does the chain UI
   render the next-skill prompt?
5. **Combat-related opcodes**: which opcodes does the client treat
   as "battle event"? What's the packet structure?

For damage **calibration** in garlemald, the cross-references are:
- `land-sand-boat-server/xi-private-server.md` — XI cousins of FFXIV
  damage formulas (closest structural analogue we have)
- `ffxiv_1x_battle_commands_context.md` — per-skill metadata
- `ffxiv_youtube_atlas_context.md` — empirical damage roll ranges
  (12+ canonical 1.x abilities with min/median/max samples)
- `ffxiv_mozk_tabetai_context.md` — gear stat contributions

The *client* decomp surfaces field offsets and packet shapes; the
*server* re-derivation (in garlemald) plus the cross-references
above produce the actual numbers.

## Actor RTTI inventory (what's been mapped)

The Actor namespace is `Application::Scene::Actor::*`. Top classes
by vtable slot count (= roughly "amount of behavior"):

| Class | Vtable RVA | Slots | Role |
|---|---|---:|---|
| **`CharaActor`** | `0xbc0d34` | **188** | Main player/NPC actor — the prize |
| **`WeaponActor`** | `0xc57ee4` | **165** | Held weapon as a separate actor |
| `CharaVisual` | `0xbd3ed4` | 29 | Character mesh/material display |
| `CharaCutVisual` | `0xc444a4` | 26 | Cutscene-only visual variant |
| `WeaponVisual` | `0xc64ed4` | 26 | Weapon mesh display |
| `CharaActionVisualBase` | `0xbe4434` | 25 | Action-time visual base |
| `CharaActionVisual` | `0xbe4544` | 25 | Concrete action visual |
| `CharaVisualBase` | `0xbbbc64` | 24 | Base visual class |
| `CharaActionQueBase` | `0xc3e37c` | 14 | Action queue base |
| `CharaActionPreLoadQue` | `0xc3e3b8` | 14 | Pre-load queue |
| `CharaActionQue` | `0xc3e428` | 14 | Concrete action queue |
| `CharaActorClipListener` | `0xbc0b44` | 12 | Cutscene-clip listener for an actor |
| `CharaWeaponController` | `0xc3ee4c` | 6 | Manages weapon swap / draw |
| `CharaActionController` | `0xc3e468` | 5 | Drives action playback |
| `CharaActionMotionController` | `0xbe7fb4` | 4 | Drives action motion (per-state) |
| `CharaSoundController` | `0xc400f4` | 3 | Audio dispatch |
| `WeaponSoundController` | `0xc625ec` | 3 | Weapon-specific audio |

Plus a large family of `App::Scene::Actor::Chara::Status::*` status
controllers (CharaStatusBattle, CharaStatusField, CharaStatusCraft,
CharaStatusGround, CharaStatusPic, CharaStatusSit, plus
CharaStatusFieldChocobo, CharaStatusFieldRidden, etc.) — these are
state-machine objects representing what the character is currently
doing (in battle, gathering, sitting, etc.). Each is wired up to
delegates (`Delegate00..Delegate07<...>::DelegateHolderDynamic`)
that fire on state transitions.

`CharaActor` is structurally the prize: a 188-slot class extending
some unknown parent. Slot 0 is a 34-byte scalar deleting destructor
that calls a 968-byte parent destructor at `FUN_00666130` (file
0x266130), suggesting a substantial inheritance chain. The parent
class isn't directly identifiable via slot-0 cross-reference in the
RTTI dump, so further inheritance walking will need Ghidra-GUI
typeinfo lookups.

## Phase 5 work pool (priority order)

1. **CharaActor base class identification + field layout**.
   Map what's at each `[ESI+offset]` access in the destructor + a few
   commonly-called slots. Exit criterion: `include/actor/chara_actor.h`
   with field-by-field struct definition that garlemald-server can
   `#include` to validate its actor-state packets against.

2. **Status controller identification**. The `CharaStatus*` family
   represents per-state behaviors; each controller's slots tell us
   what packets the client expects in each state. Exit criterion:
   `docs/actor_status.md` mapping each state to the relevant opcodes
   it consumes.

3. **Action queue + motion dispatch**. `CharaActionQue` +
   `CharaActionMotionController` drive the visible "play action N
   on this actor" pipeline. Exit criterion: a function from
   `(BattleCommand id, motion_pack_id)` to "what animation plays."

4. **Damage display path**. The client receives `ApplyDamage`-style
   packets and dispatches them to `App::Main::Element::Chara::
   DamagePlate` + `RaptureActionDamageCallClip`. Decompiling this
   path tells garlemald exactly what shape its damage packet must
   take for the popup to render correctly.

5. **Status effect tick / display**. Per-status-effect rendering
   (icon, duration bar, color). Probably driven by the
   `CharaStatusBattle` controller + a separate effects list on the
   actor.

6. **Battle Regimen (combo) UI**. How does the client display the
   "next skill in chain" prompt?

## Approach (not byte-matching)

Phase 5 is **functional decomp** per `PLAN.md` — re-derive into
clean C++ with whatever helpers we want; verify against behavioural
fixtures (saved packet captures from `captures/`, save states from
`data-backups/`, OCR damage samples in
`ffxiv_youtube_atlas_context.md`).

Byte-matching individual Actor methods is low priority for Phase 5
— the value is in the **field layouts and control flow**, not the
exact byte sequences. Compare to Phase 4 where matching mattered
because we needed to swap garlemald's hand-rolled wire encoders for
generated ones; here, garlemald's combat logic is already
re-derived (via LSB cross-reference), and what it needs from the
client is the *schema* the client expects.

## Progress (work-pool item #1)

**`include/actor/chara_actor.h` — initial field-offset catalog
landed 2026-05-02.** Recovered 139 distinct field offsets across
the constructor and destructor. Highlights:

- **vtable** = 0xbc0d34 (188 slots)
- **ctor**: `FUN_0065f180` (1942 B at file 0x25f180) — sets the
  vtable + initialises 47 distinct field offsets
- **dtor**: `FUN_00666130` (968 B at file 0x266130) — touches 48
  distinct fields with cleanup writes; wrapped by slot 0
  (`FUN_00669e20`, 34 B scalar deleting destructor)
- **class size**: ≥ 0x2ba4 (= 11,172 bytes) from highest offset

Important course-correction from the kickoff sketch: slot 1 is NOT
the constructor (constructors aren't virtual so they're not in the
vtable). Slot 1 is `FUN_006207d0` — a "ReferenceResource access
wrapper" with a Shift-JIS Japanese debug message
(`"ReferenceResourceが初期化されていません [%s]\n"`). The actual
constructor was found by scanning for `MOV [reg], 0xfc0d34`
(vtable-write pattern) — only 2 sites in the binary: the dtor and
`FUN_0065f180`.

Also course-corrected: the dtor at `FUN_00666130` is **CharaActor's
own** dtor, not the parent's. The vtable swap to 0xfc0d34 at the
top is the standard MSVC "set vtable to this class's own table
during destruction" pattern. The parent class is still unidentified
— it would show up as a different vtable address being set
somewhere later in the dtor's body (via a chained parent-dtor call),
which is a follow-up task.

**Interesting literal initializers** (these are the most
promising semantic-meaning anchors):
- `+0x0169` = 1 (byte flag)
- `+0x1170` = 0xED (237 dword)
- `+0x1178` = 0xC9 (201 dword)
- `+0x1958` = 0x10 (16 dword)
- `+0x1690..+0x16b8` = 10-dword array of pointers, all 0

Cross-referencing these literals against game-data tables (race
ids, class ids, motion-pack ids, etc.) is the next investigation.

## Inheritance chain (recovered 2026-05-02)

By chasing the chained parent-dtor calls (`MOV [ESI], <vtable>`
swap then `CALL <parent_dtor>` near the end of each dtor) and
cross-referencing each surfaced vtable VA against
`config/ffxivgame.rtti.json`:

```
SQEX::CDev::Engine::Fw::SceneObject::Actor (vtable 0xc9ca94, 89 slots)
    └── App::Scene::RaptureActor             (vtable 0xbea50c, 160 slots) [+71]
        └── App::Scene::Actor::CDevActor     (vtable 0xbbc03c, 164 slots) [+4]
            └── App::Scene::Actor::Chara::CharaActor (vtable 0xbc0d34, 188 slots) [+24]
```

Layer interpretation:

- **`SceneObject::Actor`** is the CDev engine's base scene object —
  89 slots of generic engine behaviour (lifecycle, transform, draw,
  etc.). This is the root of the actor hierarchy in the underlying
  engine.
- **`RaptureActor`** is the game-application "Rapture" layer that
  adds 71 game-specific virtual hooks — the bulk of the additions.
  This is where the game-specific behaviour lives.
- **`CDevActor`** adds only 4 slots, all related to Excel-driven
  resource loading. Sibling RTTI classes
  (`CDevActorResourceEvent`, `CDevActorSetResourceEvent`,
  `CDevActorSetResourceWithExcelEvent`, `CDevActorExcelWaiter`)
  confirm this. So `CDevActor` is essentially "RaptureActor + Excel
  hooks" — every actor type that reads game data (characters,
  weapons, BG models, etc.) extends here.
- **`CharaActor`** adds 24 character-specific slots. These are the
  slots that govern character-only behaviour (chara visual,
  motion, action queue dispatch, status controllers, etc.).

The 16 sibling `CDevActor` subclasses include `WeaponActor` (165),
`BgModelActor`/`BgObjActor`/`BgPlateActor` (167 each — they all
share the same +3-slot extension over CDevActor), `MapLayoutActor`,
several `System::*Actor` types, `LightActor`, `EffectActor`,
`WindowActor`, etc.

For garlemald, this means an actor's behaviour is layered:
- Generic engine ops (slots 0..88) — `SceneObject::Actor`
- Rapture game hooks (slots 89..159) — `RaptureActor`
- Excel resource loader hooks (slots 160..163) — `CDevActor`
- Per-type slots (164..188 for CharaActor) — class-specific

When sending an `SetActorProperty` packet, the field offset hits a
specific layer's storage; the responsible vtable slot lives in the
matching parent. Future field-naming work will benefit from
knowing which layer adds each field.

## Next concrete step

The remaining items in the work pool:

1. **Identify what the literal initializers MEAN** — search for
   functions that read each of `+0x0169`, `+0x1170`, `+0x1178`,
   `+0x1958`, looking for context (cross-references to game-data
   tables, comparison constants, etc.).
2. **Map RaptureActor's field layout** — same recipe as
   CharaActor (extract from ctor + dtor at the surfaced RVAs), and
   merge the resulting offset catalog into the same header.
   Together with CharaActor's offsets, this gives a near-complete
   field schema for any character actor.
3. Move on to work-pool items #2..#6 (Status controllers,
   Action queue, Damage display, Status effect tick, Battle Regimen UI).

## Cross-references in this workspace

Lean on these alongside the binary:

- `land-sand-boat-server/xi-private-server.md` — XI-side cousins
  of every Actor / status / damage subsystem
- `ffxiv_1x_battle_commands_context.md` — per-skill metadata
  (1,237 commands with damage attribute / element / cast type /
  AoE / ClassJob / WS id)
- `ffxiv_youtube_atlas_context.md` — damage samples for calibration
- `mirke-menagerie-context.md` — quest dialogue (some quests
  reference combat mechanics in flavour text)
- `project_meteor_discord_context.md` — first-hand notes from
  Ioncannon / Tiam / etc. on combat packet layouts
