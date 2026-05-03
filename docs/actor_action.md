# Phase 5 work-pool item #3 — Action queue + motion dispatch

> Last updated: 2026-05-02 — class hierarchy mapped; per-class
> storage location in CharaActor TBD.

## The action subsystem (7 classes)

The "action" subsystem in `App::Scene::Actor::Chara::*` handles the
entire pipeline of "character executes a battle command":
queueing → controller orchestration → motion playback → visual
rendering.

| Class | Vtable RVA | Slots | Role |
|---|---|---:|---|
| **`CharaActionQueBase`** | `0xc3e37c` | 14 | Abstract base for action queues |
| **`CharaActionQue`** | `0xc3e428` | 14×2 = 26 (multi-inh) | Concrete action queue |
| **`CharaActionPreLoadQue`** | `0xc3e3b8` | 14 | Pre-load queue (caches resources for upcoming actions) |
| **`CharaActionController`** | `0xc3e468` | 5 | Orchestrator (drives execution) |
| **`Status::CharaActionMotionController`** | `0xbe7fb4` | 4 | Motion playback driver |
| **`CharaActionVisualBase`** | `0xbe4434` | 32 | Abstract base for action visuals |
| **`CharaActionVisual`** | `0xbe4544` | 32 | Concrete visual (mesh/material/effects during action) |

Each class has its OWN vtable (set at `[this]` in its constructor),
so they're allocated as **separate heap-or-inline objects** that
CharaActor holds pointers to — NOT inline-embedded at known offsets
in CharaActor's body.

## Multi-inheritance on `CharaActionQue` (26 slots)

`CharaActionQue` shows 26 slots in the slot dump because it has
**two vtables** (multiple inheritance). Pattern:

- Each slot index 0..13 has TWO entries (one from primary base
  `CharaActionQueBase`, one from a secondary base — likely an
  `IActionQueueListener` interface).
- 13 of the slots have CharaActionQue-specific overrides (the ★
  marker in the slot dump). That's a substantial amount of
  queue-specific behaviour: enqueue, dequeue, peek, clear, validate,
  serialize, etc.

## Slot maps (class-specific overrides only)

`CharaActionController` (5/5 slots all unique to this class):
- `slot[0]` = FUN_008462f0 — destructor
- `slot[1]` = FUN_008450b0 — likely `Init` or `Update`
- `slot[2]` = FUN_00844080 — small (just past CharaActionQue::slot9)
- `slot[3]` = FUN_00845430
- `slot[4]` = FUN_00844090 — small (16 B sibling of slot[2])

`CharaActionMotionController` (4/4 slots all unique):
- `slot[0]` = FUN_007a0bd0 — destructor
- `slot[1]` = FUN_007ac9c0
- `slot[2]` = FUN_007a0be0 — small (16 B after slot 0)
- `slot[3]` = FUN_007c5940

`CharaActionPreLoadQue` (9 unique + 5 inherited):
- 9 unique overrides at slots 0, 2, 3, 4, 5, 6, 7, 8, 9
- Slots 1, 10, 11, 12, 13 inherited from QueBase

The **5-slot Controller + 4-slot MotionController** pair is the
"narrow waist" — small interfaces driving the larger Que / Visual
machinery.

## Pipeline (inferred)

```
Battle Command arrives  →  PreLoadQue resolves resources
                               (BattleCommand metadata,
                                animation pack ID, VFX,
                                sound bank, etc.)
                                       │
                                       ▼
                          ActionQue enqueues the action
                                       │
                                       ▼
                          ActionController dispatches
                          (drives the state machine)
                                       │
                          ┌───────────┴────────────┐
                          ▼                          ▼
              MotionController                   ActionVisual
              (skeletal animation)               (mesh + VFX render)
```

The PreLoad step matches FFXIV's well-known "pre-cast resource
download" behaviour (visible in client log files when entering a
new zone — the client pre-loads action animations for nearby
characters).

## Per-class storage location in CharaActor — TBD

CharaActor must hold pointers to these subsystems. None of the
class vtable VAs are written into a `CharaActor + offset` slot in
CharaActor's ctor — confirmed via grep for
`MOV [ESI+disp32], <action_vtable>`. So CharaActor stores
**pointers** (set via separately-called `new`-style allocators),
not inline-embedded objects.

To pin down the exact storage offsets:
1. Find callers of each class's ctor (e.g. `FUN_008462b0` for
   CharaActionQue's primary ctor).
2. Walk back to the allocation site:
   `CALL operator_new; PUSH ...; CALL <class_ctor>; MOV [ESI+offsetX], EAX`.
3. The `[ESI+offsetX]` IS the field where the pointer lives.

This is a focused follow-up pass (one grep + one decompile per
class). Deferred — the architectural map is the immediate value.

## Practical impact for garlemald

When the server sends a `BattleAction` or equivalent packet:
1. The client's `CharaActionQue` enqueues the action.
2. The `PreLoadQue` ensures the relevant motion + VFX resources
   are loaded.
3. `ActionController` dispatches the queued action.
4. `ActionMotionController` plays the skeletal animation
   (mapped from the action's motion-pack ID).
5. `ActionVisual` renders mesh / VFX overlays.

For garlemald to drive a smooth animation, the `BattleAction`
packet must arrive **before** the cast time elapses (so PreLoad
has time to fetch resources) and the motion-pack ID in the packet
must match a valid pack the client knows about. Garlemald
already has the per-skill motion-pack IDs in
`ffxiv_1x_battle_commands_context.md`.

## Cross-references

- `docs/actor.md` — Phase 5 high-level plan
- `docs/actor_status.md` — work-pool item #2 (status controllers)
- `include/actor/chara_actor.h` — CharaActor field-offset catalog
- `ffxiv_1x_battle_commands_context.md` — per-skill motion-pack
  IDs (the data the client expects in the BattleAction packet)
- `project_meteor_discord_context.md` — Ioncannon's notes on
  the 1.x action / motion / VFX dispatch pipeline
