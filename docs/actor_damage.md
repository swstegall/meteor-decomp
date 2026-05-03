# Phase 5 work-pool item #4 — Damage display path

> Last updated: 2026-05-02 — class hierarchy mapped; clip-factory
> dispatch is virtual so the upstream packet → clip path is one
> indirection deeper than the others.

## The "floating popup" family in `App::Main::Element::Chara::*`

Three sibling UI elements live above each character in 1.x. All
three are leaf classes in the `Element::Chara::*` namespace and
share a parent UI base (the parent itself isn't 1-slot, so its name
isn't surfaced by the slot dump — its identity is in the
constructors of the leaves).

| Element | Vtable RVA | Slots | Role |
|---|---|---:|---|
| **`NamePlate`** + `NamePlateInterface` | `0xbcf98c` / `0xbcf924` | 24 each | Persistent name + HP bar above the character |
| **`DamagePlate`** | `0xbe1e34` | 1 | Floating damage number (popup → fly-up → fade) |
| **`LevelupPlate`** | `0xbe22c8` | 1 | "Level Up!" / job-change popup |

`DamagePlate` and `LevelupPlate` are **leaf classes with only the
destructor overridden**. All visible behaviour (text rendering, the
fly-up animation, the alpha fade-out) is inherited from the shared
`PlateBase`-style parent. This is consistent with how the same UI
behaviour is reused across all transient popups.

`NamePlate` has a much richer 24-slot vtable because it's a
persistent overlay (HP bar updates per tick, name re-aligns on zoom,
etc.) rather than a fire-and-forget popup.

## The "damage display" cutscene-clip family

The popups don't appear out of nowhere — they're triggered by a
**cutscene clip**, scheduled through the CDev cut/scheduler animation
system. The damage-display family in
`App::Scene::Cut::Clip::*`:

| Clip | Vtable RVA | Slots | Role |
|---|---|---:|---|
| `RaptureActionDamageCallClip` | `0xbf7aac` (+ `0xbf7aa0` MI) | 42 | Damage popup orchestrator |
| `RaptureCastResultClip` | `0xc12714` | 42 | Cast-result popup (resist / interrupt) |
| `RaptureActionSelectClip` | `0xc1316c` | 42 | Action-target / select indicator |
| `RaptureActionSelectAttackClip` | `0xc13bc4` | 42 | Attack-roll indicator (hit/miss/parry/etc.) |
| `RaptureActionSelectDamageMccClip` | `0xc1461c` | 42 | Multi-character cutscene damage variant |

Each clip class has a paired
`SQEX::CDev::Engine::Cut::Scheduler::BaseClipImpl<...>`
specialisation (also 42 slots, identical body except for slots 0/1
which are the destructor and one implementation slot). The clip
inherits multiply: the secondary vtable at `class+8` is the base
template's vtable. This shows up in the constructor as two
vtable-write instructions:

```
c706 ac7aff00      ; MOV [ESI+0],   0x00ff7aac   ; derived vt
c74608 a07aff00    ; MOV [ESI+8],   0x00ff7aa0   ; secondary (base template) vt
```

The 42 slots cover the standard CDev clip lifecycle (Begin /
Update / End / Reset / pause-resume / time-scale / etc.) plus
class-specific overrides on slots 17 and 24+.

## Flow (architectural)

```
ActorParam packet  (server tells client "actor X took N damage")
        │
        ▼
Damage-event handler  (decodes packet, picks the right clip type)
        │
        ▼
RaptureActionDamageCallClip  (scheduled via CDev cut/scheduler factory)
        │
        ▼
Clip Begin slot  (creates a DamagePlate instance attached to the actor)
        │
        ▼
Clip Update slot per frame
   (feeds animation curves into DamagePlate's text + transform fields:
    fly-up Y offset, alpha fade, optional crit colour)
        │
        ▼
DamagePlate's UI base renders the floating number
        │
        ▼
Clip End slot  (when curve finishes → destroy DamagePlate)
```

## What's confirmed vs inferred

**Confirmed from the binary:**
- `RaptureActionDamageCallClip` has its constructor at `FUN_00811690`
  (file `0x411690`, 107 B) and a single direct call site:
  `FUN_00638700` (file `0x238700`, 136 B) — a thin wrapper that
  takes a `this` pointer in `ECX`, two arguments on stack, and
  invokes the clip ctor.
- `FUN_00638700` has **zero direct `CALL` callers** — it's invoked
  through a vtable indirect call (likely a clip-factory dispatch).
  The CDev cut/scheduler creates clips via a factory pattern keyed
  on a clip-type ID, so the path from "damage packet arrives" →
  "clip allocated" goes through one vtable hop on the scheduler.
- `DamagePlate`'s destructor (slot 0, `FUN_00796800`) calls into
  `FUN_00794e80`, which is the **base UI element's destructor**.
  The same call shape appears in `LevelupPlate`'s destructor (slot
  0, `FUN_00796cc0`) — confirming the two share a base class.
- The clip's vtable is written at `[this]+0` AND `[this]+8`, so
  `RaptureActionDamageCallClip` extends two parents (multiple
  inheritance), one being the `BaseClipImpl<...>` template.

**Inferred (consistent with FFXIV's known architecture):**
- The "Begin → Update → End" lifecycle runs through specific slots
  of the 42-slot clip vtable (slots 17 and 24+ are the most likely
  candidates given they're in the class-specific override range).
- The animation curve that drives the fly-up + fade-out is loaded
  from a `.tmb` (Timeline Binary) resource referenced by the clip.
  The CDev cut/scheduler is the engine layer that owns this.
- Crit / weak-attack / resist variants probably switch the clip
  type (DamageCall vs SelectAttack vs SelectDamageMcc) rather than
  passing a flag — the 5 separate clip classes are the variants.

## Practical impact for garlemald

The damage display path is **rendered entirely client-side** from a
single damage-event packet. The server's job is to send the
damage value, the source/target actor IDs, and a damage-type tag
(physical / magical / heal / miss / etc.); the client's clip
scheduler picks the right `Rapture*Clip` variant and renders the
popup.

Concretely, what garlemald's damage packet must carry:

1. **Source actor ID** (who's dealing damage) — for the popup's
   anchoring.
2. **Target actor ID** (who's taking it) — the popup attaches to
   this actor's transform.
3. **Damage value** (signed int? or unsigned with a separate
   "is heal" flag?) — TBD; check whether `RaptureActionDamageCallClip`
   slots have a `setValue(int)` or `setValue(int, kind)` shape.
4. **Damage kind** — at least: hit / crit / miss / parry / evade /
   absorb / heal / over-time tick. The 5 clip-class variants give
   the upper bound on how many distinct *visual* kinds exist; the
   damage kind in the packet maps to one of them.
5. **Element / colour hint** (optional) — fire / ice / lightning
   damage should colour-tint the popup. May be encoded in the
   damage-kind field rather than a separate channel.

Garlemald doesn't need to set up clip lifecycles or animation
curves on its side — the client owns all of that. It just needs
to deliver the damage event, and the client's
`RaptureActionDamageCallClip` does the rest.

## What's NOT here (and where to look)

- The **upstream packet → clip-factory call** is one indirect-call
  hop deeper than this analysis. To pin it down, find xrefs to
  the CDev cut/scheduler's `CreateClip`-style entry point, then
  filter for ones whose first arg matches the
  `RaptureActionDamageCallClip` clip-type ID (likely a small
  integer enum). This would surface the exact server-packet handler
  that schedules the damage clip.
- The **specific 42-slot lifecycle map** for the clip (which slot
  is Begin, which is Update, etc.) would let us see exactly how
  the damage value flows from the clip to the DamagePlate. Slots
  17 and 24+ are the best-bet candidates; one slot is almost
  certainly a "set damage value" entry point that the packet
  handler invokes after creating the clip.

Both are focused follow-up passes (one cross-ref grep + one
slot-walk per pass). Deferred — the architectural map is the
immediate value.

## Cross-references

- `docs/actor.md` — high-level Phase 5 plan (this is item #4)
- `docs/actor_status.md` — work-pool item #2 (status controllers)
- `docs/actor_action.md` — work-pool item #3 (action queue +
  motion dispatch); the `RaptureActionDamageCallClip` is invoked
  *by* the action subsystem after damage resolution lands
- `include/actor/chara_actor.h` — CharaActor field-offset catalog
- `ffxiv_1x_battle_commands_context.md` — per-skill damage type
  + element table; the server uses these to populate the damage
  packet's "kind" field
- `land-sand-boat-server/xi-private-server.md` — XI's analogous
  damage display goes through `addchar (...)` packets driving the
  `damage` Lua hook + the engine's combat-log + floating-text
  layers; the *shape* is identical (server sends value + kind,
  client renders), even though XI's UI layer is different
