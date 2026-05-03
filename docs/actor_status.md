# Phase 5 work-pool item #2 — Status controllers

> Last updated: 2026-05-02 — architecture mapped; active-state
> pointer location TBD.

## Inventory

The `App::Scene::Actor::Chara::Status::*` namespace contains
**10 status controller types** plus a main dispatcher. Each
represents a distinct character state that can be active.

| Controller | Delegate-vtable count | Likely role |
|---|---:|---|
| `CharaMainStatusController` | 1 | Main dispatcher / state-machine root |
| `CharaStatusBattle` | 7 | Combat state (rich event surface) |
| `CharaStatusBattleChocobo` | 7 | Combat while mounted on chocobo |
| `CharaStatusField` | 7 | Field exploration / movement |
| `CharaStatusFieldChocobo` | 7 | Field movement on chocobo |
| `CharaStatusFieldRidden` | 7 | Being a chocobo carrying a player |
| `CharaStatusCraft` | 3 | Crafting (synthesis loop) |
| `CharaStatusGround` | 3 | On the ground (downed?) |
| `CharaStatusPic` | 3 | "Pic" — possibly portrait / posing |
| `CharaStatusSit` | 3 | Sitting |
| `CharaActionMotionController` | (4 slots, standalone RTTI) | Motion playback for the active action |

Plus 1 standalone-RTTI controller:
- **`CharaActionMotionController`** (vtable RVA `0xbe7fb4`, 4 slots)
  — drives motion playback; not a "state" but always-on. Slot map:
  - slot 0: `FUN_007a0bd0` — destructor
  - slot 1: `FUN_007ac9c0`
  - slot 2: `FUN_007a0be0`
  - slot 3: `FUN_007c5940`

## Delegate-richness pattern

The 1/3/7 delegate-vtable counts reveal each state's complexity:

- **1 delegate** (`CharaMainStatusController`): just the basic
  "state changed" notification — appropriate for the dispatcher
  that routes events to the active sub-state.
- **3 delegates** (Craft, Ground, Pic, Sit): minimal event surface —
  these are passive/locked states. A character that's sitting only
  needs hooks for "stand up," "interrupt," and one more.
- **7 delegates** (Battle, BattleChocobo, Field, FieldChocobo,
  FieldRidden): rich event surface for the active states. Combat
  needs hit / damage / death / target-switch / cast-start / etc.;
  field movement needs zone-cross / jump / fall-damage / etc.

This is consistent with a state-pattern architecture where each
state has its own event-handler set.

## Delegate construction pattern

Each delegate is a small 12-byte object: `{ vtable, fn_ptr,
bound_this }`. The construction function (e.g. `FUN_007c3c00` for
CharaStatusBattle delegates, 99 B) sets the fields in 2 stages:

```
new_delegate->vtable = 0xfe7f10;       // generic delegate base
new_delegate->vtable = 0xfe7f70;       // CharaStatusBattle-specific
new_delegate->fn_ptr = arg1;           // callback function
new_delegate->bound_this = arg2;       // bound `this` for the callback
```

The 2-stage vtable assignment is the standard MSVC base-then-derived
construction order; the binary preserves it because each stage has
distinct behaviour (the base vtable's destructor is registered for
SEH unwind between the two stages).

## RTTI quirk: status controllers only appear as template args

The 10 `CharaStatus*` controller classes do NOT have standalone
RTTI entries with their own vtables. Instead, they appear ONLY as
template arguments to `Delegate0X<...>::DelegateHolderDynamic`
specialisations. This means:

- The Status controllers are CONCRETE classes used in delegates.
- MSVC didn't emit standalone RTTI for them, presumably because
  they're never `dynamic_cast`-targeted.
- We can identify them BY NAME (via the delegate template arg) but
  can't directly find their constructors via the standard
  vtable-write-pattern grep — we'd need to find the delegate
  constructors and trace back to the controllers they embed.

## Active-state-pointer location — TBD

A character must have ONE active status controller at a time.
Where is the "active controller" pointer/index stored in CharaActor?

What we know:
- It's NOT `array_1690[10]` (initially hypothesised). Those are
  10 SSE-aligned 4×4 matrix transforms at `+0x16d0..+0x18b0`,
  populated by `FUN_00664890` calling vtable slot 0x60/4=24 on each
  array element to fetch a transform matrix. **`array_1690[10]`
  is bone/attachment-point transforms, NOT status pointers.**

What's left to investigate:
- Find callers of each delegate-constructor (e.g. `FUN_007c3c00`
  for CharaStatusBattle). The caller sets up `ECX` = the address
  of an inline delegate slot in CharaActor. That address is
  `ESI + offsetX` for some `offsetX` — and that `offsetX` IS the
  field where the CharaStatusBattle delegate lives in CharaActor.
- Once each controller's offset is found, look for a separate
  `current_state` enum/pointer field that the dispatcher uses to
  pick the active controller.

This is a follow-up task. The architecture / inventory is
documented; the active-state plumbing is one focused investigation
away.

## Status-controller layout in CharaActor (partial)

From scans so far, several CharaStatus-related fields are
known to be inline sub-objects in CharaActor:

- `subobj_0fc0`, `subobj_1030`, `subobj_1070`, `subobj_1110` —
  CharaActor's ctor calls sub-init functions on these (visible in
  the ctor's `LEA ECX, [ESI+0xN]; CALL <sub_ctor>` chain). These
  are CANDIDATE locations for the inline status-controller delegate
  storage, but mapping each one to a specific controller still
  needs one more grep pass: the delegate-ctor address invoked
  inside each CharaActor sub-init tells us which controller it is.

## Practical impact for garlemald

This finding doesn't change garlemald's wire layer (the server
doesn't directly poke status-controller pointers). It does mean:

1. **State transitions are CLIENT-driven** in 1.x — the server
   sends events / packets, the client's state machine routes them
   to the active controller, and the controller decides what
   visual/audio response to render. So garlemald sending an
   "enter combat" packet is enough; the client's state machine
   handles the "switch from CharaStatusField to CharaStatusBattle"
   transition internally.
2. The 10-state inventory is the **complete** set of major
   character states in 1.x. No "Swimming" / "Flying" / "Jumping"
   states — those didn't exist in 1.x. Useful for garlemald's
   map-server when it decides what state-changes to broadcast.

## Cross-references

- `docs/actor.md` — high-level Phase 5 plan (this is item #2)
- `include/actor/chara_actor.h` — CharaActor field-offset catalog
  (the sub-objects at +0xfc0/+0x1030/+0x1070/+0x1110 are candidate
  status-controller storage)
- `land-sand-boat-server/xi-private-server.md` — XI's character
  states are similarly state-pattern (the Idle / Mounted / Engaged
  / Resting / Crafting / Dead / etc. enum is a direct cousin)
