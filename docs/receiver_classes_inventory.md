# Phase 9 — `Application::Lua::Script::Client::Command::*::Receiver` inventory

> Last updated: 2026-05-15 — kickoff inventory of the 43 Receiver
> classes that handle inbound game-logic packets in the FFXIV 1.x
> client. Phase 7 decoded 3 of them (KickClientOrderEvent /
> StartServerOrderEventFunction / EndClientOrderEvent); the other
> 40 are still to-walk.

## Why this phase

Per `docs/network_dispatch_dual_paths.md`, the FFXIV 1.x client
uses **two parallel packet-handling paths**:

1. The `ZoneProtoChannel` → `DummyCallback` dispatch path (no-op
   stubs for game logic — used only as routing scaffolding for
   group-related opcodes that the work-table system consumes).
2. The `Application::Lua::Script::Client::Command::*::Receiver`
   class system — 43 dedicated classes, each handling one or a
   small family of opcodes via a 2-, 5-, or 6-slot vtable.

Phase 9 walks the Receiver classes systematically. Each receiver's
`Receive` slot contains the actual gate-and-dispatch logic for the
opcode it handles — including any actor-state checks like the
`+0x5c` flag gate that Phase 7 surfaced for KickClientOrderEvent.

Knowing what each receiver gates on directly informs garlemald's
wire emission: a "silent drop" symptom in garlemald usually means
the receiver gate isn't satisfied at the moment of the packet's
arrival.

## Inventory — all 43 Receivers

Sorted by RTTI vtable RVA. `slot1 fn` is the `Receive` entry
(slot 1 for 2-slot variants, slot 2 for 5/6-slot variants —
slot 0 is the destructor).

### `Application::Lua::Script::Client::Command::System::*` (11 receivers)

| RTTI rva | Slots | slot1 fn | Receiver leaf | Phase 7 | Best-guess opcode |
|---|---:|---|---|---|---|
| `0xbdfaf8` | 2 | `FUN_008a4270` | ExecutePushOnEnterTriggerBoxReceiver | | trigger-box enter |
| `0xbdfb04` | 2 | `FUN_008a4340` | ExecutePushOnLeaveTriggerBoxReceiver | | trigger-box leave |
| `0xbdfb10` | 2 | `FUN_008a3de0` | AttributeTypeEventEnterReceiver | | attr-type enter |
| `0xbdfb1c` | 2 | `FUN_008a3e20` | AttributeTypeEventLeaveReceiver | | attr-type leave |
| `0xc57598` | 2 | `FUN_008a2f30` | ChocoboReceiver | | mount: chocobo |
| `0xc575a4` | 2 | `FUN_008a3020` | ChocoboGradeReceiver | | mount: chocobo grade |
| `0xc575b0` | 2 | `FUN_008a3100` | GoobbueReceiver | | mount: goobbue |
| `0xc575bc` | 2 | `FUN_008a31e0` | VehicleGradeReceiver | | mount: vehicle grade |
| `0xc575c8` | 5 | `FUN_008a34d0` | **ChangeActorSubStatStatusReceiver** | | actor sub-stat status |
| `0xc575e0` | 2 | `FUN_008a32c0` | ChangeActorSubStatModeBorderReceiver | | actor sub-stat mode/border |
| `0xc575ec` | 2 | `FUN_008a4880` | ExecuteDebugCommandReceiver | | GM debug command |

### `Application::Lua::Script::Client::Command::Network::*` (32 receivers)

| RTTI rva | Slots | slot1 fn | Receiver leaf | Phase 7 | Best-guess opcode |
|---|---:|---|---|---|---|
| `0xc572ac` | 2 | `FUN_0089c510` | AchievementPointReceiver | | achievement: point |
| `0xc572b8` | 2 | `FUN_0089c5f0` | AchievementTitleReceiver | | achievement: title |
| `0xc572c4` | 2 | `FUN_0089c6d0` | AchievementIdReceiver | | achievement: id |
| `0xc572d0` | 2 | `FUN_0089c7c0` | AchievementAchievedCountReceiver | | achievement: count |
| `0xc572dc` | 2 | `FUN_0089c8b0` | AddictLoginTimeKindReceiver | | playtime warning |
| `0xc572e8` | 2 | `FUN_0089c990` | ChangeActorExtraStatReceiver | | actor: extra stat |
| `0xc572f4` | 2 | `FUN_0089ca80` | ChangeSystemStatReceiver | | system stat |
| `0xc57300` | 2 | `FUN_0089cb60` | JobChangeReceiver | | actor: job change |
| `0xc5730c` | 2 | `FUN_0089cc70` | ChangeShadowActorFlagReceiver | | actor: shadow flag |
| `0xc57318` | 2 | `FUN_0089cd60` | GrandCompanyReceiver | | actor: grand company |
| `0xc57324` | 2 | `FUN_0089ce70` | HamletSupplyRankingReceiver | | hamlet: supply rank |
| `0xc57330` | 2 | `FUN_0089e420` | HamletDefenseScoreReceiver | | hamlet: defense |
| `0xc5733c` | 2 | `FUN_0089d030` | HateStatusReceiver | | combat: hate status |
| `0xc57348` | 5 | `FUN_0089d180` | **EndClientOrderEventReceiver** | ✅ Phase 7 | `0x0131 EndEvent` |
| `0xc57360` | 6 | `FUN_0089d350` | JobQuestCompleteTripleReceiver | | quest: job complete |
| `0xc5737c` | 2 | `FUN_0089d4f0` | SetCommandEventConditionReceiver | | event: command cond |
| `0xc57388` | 2 | `FUN_0089d610` | SetDisplayNameReceiver | | actor: display name |
| `0xc57394` | 2 | `FUN_0089d750` | SetEmoteEventConditionReceiver | | event: emote cond |
| `0xc573a0` | 2 | `FUN_0089d860` | SetEventStatusReceiver | | `0x0136 SetEventStatus` |
| `0xc573ac` | 2 | `FUN_0089d980` | SetNoticeEventConditionReceiver | | `0x016B SetNoticeEventCondition` |
| `0xc573b8` | 2 | `FUN_0089db00` | SetPushEventConditionWithCircleReceiver | | event: push circle cond |
| `0xc573c4` | 2 | `FUN_0089dc90` | SetPushEventConditionWithFanReceiver | | event: push fan cond |
| `0xc573d0` | 2 | `FUN_0089de20` | SetPushEventConditionWithTriggerBoxReceiver | | event: push triggerbox cond |
| `0xc573dc` | 2 | `FUN_0089df60` | SetTalkEventConditionReceiver | | event: talk cond |
| `0xc573f4` | 2 | `FUN_008a04b0` | SetTargetTimeReceiver | | target: time |
| `0xc57470` | 2 | `FUN_0089cb90` | EntrustItemReceiver | | item: entrust |
| `0xc5747c` | 2 | `FUN_0089e550` | SyncMemoryReceiver | | sync: memory |
| `0xc57488` | 6 | `FUN_008a2a20` | UserDataReceiver | | user data (2 vtables, same fn) |
| `0xc574a4` | 2 | `FUN_008a2a20` | UserDataReceiver | | user data (sibling) |
| `0xc574b0` | 5 | `FUN_0089f530` | **KickClientOrderEventReceiver** | ✅ Phase 7 | `0x012F KickEvent` |
| `0xc574c8` | 5 | `FUN_0089f430` | **StartServerOrderEventFunctionReceiver** | ✅ Phase 7 | `0x0130 RunEventFunction` |
| `0xc574e0` | 2 | `FUN_0089fbf0` | SendLogReceiver | | system: log message |

## Distribution

- **2-slot variants** (37): destructor + Receive. Simplest pattern.
- **5-slot variants** (4): destructor + intermediate slots + Receive. Used for the actor-bound event lifecycle (Kick / RunEventFunction / EndEvent / ChangeActorSubStatStatus).
- **6-slot variants** (2): JobQuestCompleteTripleReceiver + UserDataReceiver. Richest — likely have both an in-place Receive AND additional state-mutation slots.

The 4 5-slot receivers are the most architecturally significant.
3 of them are decoded in Phase 7. The 4th —
**ChangeActorSubStatStatusReceiver** (`0xc575c8`) — is the next
priority Phase 9 target.

## Why "ChangeActorSubStatStatus" matters

Looking at the namespace (`System::*`, sibling to
`ChangeActorSubStatModeBorderReceiver`), this receiver handles the
client-side update of an actor's "sub-stat status" — likely the
buff/debuff/condition tray on the nameplate (e.g. poisoned, stoned,
sleep). If the receiver gates on actor flags similar to Kick's
`+0x5c`, the gate would dictate when status icons can land
client-side. Wrong gate → stuck status icons or invisible buffs.

## Phase 9 work pool

| Item | Description | Status |
|---|---|---|
| #1 | Inventory the 43 Receiver classes | ✅ done (this doc) |
| #2 | Decode `ChangeActorSubStatStatusReceiver` (last 5-slot) | ✅ done 2026-05-16 — `docs/event_change_actor_substat_status_decomp.md`. Most-gated receiver in the inventory: checks BOTH `+0x7d` on primary StatusBase AND `+0x5c` on secondary CharaBase, with per-instance done-flag at `[+0x15]`. Surfaced 3 new RTTI types and the architectural finding that System-ns receivers use SrcType `Component::Lua::GameEngine::LuaControl` (a deeper engine base) — distinct from Network-ns receivers' `ActorBase`. StatusBase is a SIBLING of ActorBase under LuaControl, not a subclass. |
| #3 | Decode the 6-slot `JobQuestCompleteTripleReceiver` | ✅ done 2026-05-16 — `docs/event_job_quest_complete_triple_decomp.md`. Introduces **dispatch Pattern C** (stack-built, dispatched via 2-step success-gated `FUN_00785bf0` instead of the standard `Receive` slot). Slot 5 (real handler) navigates to `MyPlayer[+0x110]` and swaps in a fresh 36-byte JobQuestObject (3 × 12-byte triple). Gates use two distinct global success-byte sentinels (`[0x012c41af]` = Phase 7's "default kick result byte" + `[0x012c3120]` = NEW). UserDataReceiver (#4, also 6-slot) is the natural next target — likely the same Pattern C. |
| #4 | Decode the 6-slot `UserDataReceiver` | 🔲 pending |
| #5 | Cross-reference each receiver to its opcode (the engine wires opcode → receiver at script load; need to find that registration) | 🟡 partial 2026-05-16 — `docs/receiver_dispatch_via_actorimpl.md`. **35 of 42 Receivers mapped to specific vtable slots on `Component::Lua::GameEngine::{LuaActorImpl, NullActorImpl}` (two parallel 90-slot vtables at `0xbdfb2c` / `0xbe02ac`).** Two dispatch patterns: stack-temporary (Pattern A — 2-slot Receivers, 28 found) and heap-allocated long-lived (Pattern B — 5/6-slot Receivers in slots 56/57/58 = Kick/Start/End event lifecycle, 78 = JobQuestComplete, 88 = ChangeActorSubStatStatus, 59 = UserData). 7 unmapped Receivers are all `Set*EventCondition` variants — owned by event-handler instances, not LuaActorImpl. Still pending: the per-opcode dispatcher that picks the slot index — direct `CALL [reg+disp32]` searches return no hits, suggesting computed-index dispatch via Lua VM or via `FUN_004e20a0` (the real router downstream of `FUN_00dae520` per Phase 8 #9). |
| #6 | Walk the 37 2-slot receivers' Receive bodies — most are simple `actor[+offset] = value` updaters | 🟡 partial — class-hierarchy sweep done (see §"Lua actor class hierarchy" below): for 25 of the 32 Network/System 2-slot variants, the receive body is just `dynamic_cast<TargetSubclass>(ctx); subclass->doIt(...)`. Remaining 7 use inline patterns (no `__RTDynamicCast` helper) — TBD if they cast via a different idiom. |
| #7 | Build a cheat-sheet of "what gate does each opcode's receiver check" so garlemald can reason about silent-drop symptoms | 🟡 partial — for the 25 dynamic_cast receivers, the gate is "target actor must be of the right derived Lua-class". For KickReceiver / RunEventFunctionReceiver, the gate is the `+0x5c` / `+0x7d` actor flag respectively (Phase 7). Comprehensive cheat-sheet still pending. |
| #8 | Cross-reference the SEQ_005 cinematic body packets (0x012F Kick, 0x0130 Run, 0x0136 SetEventStatus, 0x016B SetNoticeEventCondition) against their receivers' gates to identify the *exact* gate currently failing in garlemald | 🟡 partial — cross-ref doc at `docs/seq005_receiver_gate_audit.md`; identifies Branch B1's `receiver[+0x80]` flag as the prime suspect for the silent kick drop, but Phase 7 didn't decode which packet byte maps to it. Three follow-ups (#8a/#8b/#8c) below. |
| #8a | Map KickEvent packet body → receiver instance offsets (especially what byte sets `receiver[+0x80]`) | 🟡 partial — `receiver[+0x80]` is mapped to `(LuaParamsContainer at +0x6c)[+0x14]` (see `docs/kick_receiver_offset_map.md`). The byte's PACKET source still requires tracing the IpcChannel parser (slot 1 is the heap copy ctor, not the original parser). The byte-by-byte KickEvent diff vs pmeteor showed body bytes are identical, suggesting the gate value comes from receiver STATE (not the kick packet itself) — possibly primed by an earlier opcode garlemald doesn't send. |
| #8b | Decode SetEventStatusReceiver slot 1 + SetNoticeEventConditionReceiver slot 1 (both 2-slot receivers in the SEQ_005 path) | ✅ done 2026-05-15 — `docs/event_status_condition_receivers_decomp.md`. **Both are `__RTDynamicCast` + dispatch.** SetEventStatus casts to NpcBase, no null-check (unguarded). SetNoticeEventCondition casts to DirectorBase, with FALLBACK to ActorBase[+0x118] if cast fails. **Neither has a `+0x5c`-style actor flag gate** — eliminated as silent-drop suspect. New suspect surfaced: if `ScriptBind` (step 8 in spawn sequence) is what promotes the actor to DirectorBase, then SetNoticeEventCondition packets sent at step 2 (BEFORE ScriptBind) would silently land in `ActorBase[+0x118]` instead of `DirectorBase[+0x60]`. This is **the orphaned-conditions hypothesis** — needs verification via Phase 7's StartServerOrderEventFunctionReceiver path. |
| #8c | Look for pre-kick "receiver state init" packets that prime `context_root[+0x128]` (would shift Branch B1 → Branch B2) | 🔲 pending — pmeteor may send a packet we don't |

## Lua actor class hierarchy (recovered via Phase 9 #8b sweep)

By parsing all 32+ Network and System namespace receivers'
`Receive` bodies for the `PUSH SrcType / PUSH TargetType / CALL
__RTDynamicCast` pattern (2026-05-15), the complete `dynamic_cast`
target-type set was recovered. Every cast's SrcType is the same —
`Application::Lua::Script::Client::Control::ActorBase` (RTTI Type
Descriptor at `0x01270964`). The TargetTypes form the **Lua-side
actor class hierarchy** that the engine wires receivers against:

All RTTI addresses **concretely recovered via Phase 9 ext (`__RTDynamicCast`
callsite sweep)** — see `docs/dynamic_cast_callsite_sweep.md`:

| Subclass | RTTI addr | # Receivers | Receivers |
|---|---|---:|---|
| `Component::Lua::GameEngine::LuaControl` | `0x01270b4c` | — (System-ns source) | System namespace receivers cast FROM this; 24 LuaControl-derived classes total |
| `ActorBase` | `0x01270964` | — (Network-ns source) | Network namespace receivers cast FROM this |
| `MyPlayer` | `0x012c19a4` | 12 | AchievementPoint/Id/AchievedCount, AddictLoginTimeKind, AttributeTypeEventEnter/Leave, ChocoboReceiver, ChocoboGrade, GoobbueReceiver, VehicleGrade, EntrustItem, SetCommandEventCondition |
| `NpcBase` | `0x012709e4` | 5 | ExecutePushOnEnter/LeaveTriggerBox, HateStatus, SetEventStatus, SetTalkEventCondition |
| `CharaBase` | `0x012709a4` | 4+1 | ChangeActorExtraStat, ChangeActorSubStatModeBorder, ChangeSystemStat, SetDisplayName, *+ ChangeActorSubStatStatus secondary cast (Phase 9 #2)* |
| `PlayerBase` | `0x012bfa48` | 3 | AchievementTitle, GrandCompany, JobChange |
| `DirectorBase` | `0x012bf9c8` | 1 | SetNoticeEventCondition |
| `AreaBase` | `0x012c2a6c` | 1 | HamletSupplyRanking |
| `StatusBase` | `0x012c31f8` | 0+1 | *ChangeActorSubStatStatus primary cast (Phase 9 #2; sibling of ActorBase under LuaControl)* |
| `WorldMaster` | `0x012c1328` | 1 | SendLog |

Inferred class diagram (**refined via Phase 9 #2** — adds the deeper
`LuaControl` base and the `StatusBase` sibling under it; see
`docs/event_change_actor_substat_status_decomp.md`):

```
Component::Lua::GameEngine::LuaControl        (deepest engine-Lua base; System-ns SrcType)
├── Application::Lua::Script::Client::Control::ActorBase  (Network-ns SrcType)
│     ├── CharaBase               (anything with character stats — players + NPCs)
│     │     ├── NpcBase           (5 receivers — non-player NPCs / mobs)
│     │     └── PlayerBase        (3 receivers — local + remote players)
│     │           └── MyPlayer    (12 receivers — local player ONLY)
│     ├── DirectorBase            (1 receiver — directors, content groups, etc.)
│     ├── AreaBase                (1 receiver — zones/private-areas/hamlets)
│     │     └── PrivateAreaBase   (no receiver — slot 0 differs from AreaBase)
│     ├── QuestBase               (no receiver — vtable diverges significantly)
│     └── WorldMaster             (1 receiver — engine-global broadcasts)
│
└── Application::Lua::Script::Client::Control::StatusBase  (status-effect wrapper; sibling of ActorBase)
```

**Inheritance edges confirmed by Phase 9 #8d:**
- `DirectorBase` ctor calls `ActorBase` ctor → `DirectorBase` IS-A `ActorBase` (direct, not via `CharaBase`)
- `CharaBase` overrides slot 4 to `0x6f3000`; `NpcBase` and `PlayerBase` both inherit this override → both extend `CharaBase`
- `ActorBase` ctor explicitly zeros `[+0x5c]` (the kick gate flag from Phase 7)
- `DirectorBase` ctor initializes `[+0x60]` as an empty `std::vector` (First/Last/End all NULL)

### 7 receivers that don't use `__RTDynamicCast`

These 2-slot receivers' Receive bodies pack their fields and forward
to a downstream method without going through `FUN_009da6cc`:

- `ChangeShadowActorFlagReceiver` (`FUN_0089cc70`)
- `HamletDefenseScoreReceiver` (`FUN_0089e420`)
- `EndClientOrderEventReceiver` (`FUN_0089d180` — 5-slot, already
  Phase 7-decoded)
- `JobQuestCompleteTripleReceiver` (`FUN_0089d350` — 6-slot)
- `SetEmoteEventConditionReceiver` (`FUN_0089d750`)
- `SetPushEventConditionWithCircleReceiver` (`FUN_0089db00`)
- `SetPushEventConditionWithFanReceiver` (`FUN_0089dc90`)
- `SetPushEventConditionWithTriggerBoxReceiver` (`FUN_0089de20`)
- `SetTargetTimeReceiver` (`FUN_008a04b0`)
- `SyncMemoryReceiver` (`FUN_0089e550`)
- `UserDataReceiver` (`FUN_008a2a20` — 6-slot)
- `KickClientOrderEventReceiver` (`FUN_0089f530` — 5-slot, but slot 1
  here is the New() factory; the actual Receive is slot 2 at
  `FUN_0089e450` and DOES gate on `+0x5c` — Phase 7)
- `StartServerOrderEventFunctionReceiver` (`FUN_0089f430` — 5-slot,
  factory-vs-Receive same caveat; Receive at slot 2 — Phase 7)
- `ChangeActorSubStatStatusReceiver` (`FUN_008a34d0` — 5-slot)
- `ExecuteDebugCommandReceiver` (`FUN_008a4880`)

(Note: `SendLogReceiver` (`FUN_0089fbf0`) DID register as casting to
`WorldMaster` in the sweep — the sweep found 3 separate
`__RTDynamicCast` call sites in its body, suggesting it has multiple
target-type branches rather than a single one. Worth its own walk
later.)

The non-casting pattern (e.g. `SetPushEventConditionWithCircleReceiver`,
`FUN_0089db00`, 67 bytes) packs ALL receiver fields (the inline
`+0x58/+0x59/+0x5c/+0x60/+0x64/+0x65/+0x66/+0x67/+0x68` block of
mixed bytes + floats) and forwards them to a downstream function
with `dispatch_ctx` directly as `this`. The engine's script-load
wiring is presumed to enforce the type contract by construction.

## Cross-references

- `docs/dynamic_cast_callsite_sweep.md` — **Phase 9 ext (2026-05-16):
  engine-wide __RTDynamicCast sweep recovering 129 RTTI addresses,
  closing all open Lua-actor-class RTTI gaps and mapping 6 major
  class hierarchies (LuaControl, Sqwt UI framework, engine-side
  Actor, Work-table info, Debug binders, Network channels)**
- `docs/network_dispatch_dual_paths.md` — Phase 8 #9 finding that
  receivers are the real dispatch (vs the no-op
  ZoneProtoChannel/DummyCallback path)
- `docs/event_kick_receiver_decomp.md` — Phase 7 #1 (Kick Receive
  body decomp; identified `actor[+0x5c]` gate)
- `docs/event_run_event_function_receiver_decomp.md` — Phase 7 #2
- `docs/event_end_receiver_decomp.md` — Phase 7 #3 (102-case
  end-event sub-dispatcher)
- `docs/group_system_decomp.md` — Phase 8 (the no-receiver
  channel-bound queue path used for Group/SharedWork opcodes)
- `garlemald-server/docs/post_warp_respawn_fix_analysis.md` — the
  garlemald-side application of Phase 7's `+0x5c` gate finding
  (still being applied as of 2026-05-15 SEQ_005 work)
