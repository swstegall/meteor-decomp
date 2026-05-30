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
| #4 | Decode the 6-slot `UserDataReceiver` | ✅ done 2026-05-16 — `docs/event_user_data_receiver_decomp.md`. **Pattern A, NOT C.** Despite being 6-slot, the 6-slot vtable is MI-thunk shim for the secondary base at `this+8`; the real Receive (592 B, FUN_008a0190) lives at slot 1 of a SECOND 2-slot vtable (0xc574a4, primary base at `this+0`). LuaActorImpl::slot59 stack-builds via FUN_0089eed0 and directly calls the primary's slot 1 — standard Pattern A. **Key taxonomy refinement: vtable size alone doesn't determine pattern** — the LuaActorImpl wrapper's dispatch path does. The two 6-slot receivers (JobQuestCompleteTriple → C, UserDataReceiver → A) confirm this. Probable semantic: per-character persistent KV blob (UI prefs, hotbar layouts, "don't show again" flags) — fits the 592-byte Receive's footprint (string compares, Utf8String ops, Lua-engine hooks). |
| #5 | Cross-reference each receiver to its opcode (the engine wires opcode → receiver at script load; need to find that registration) | 🟡 partial 2026-05-16 — `docs/receiver_dispatch_via_actorimpl.md`. **35 of 42 Receivers mapped to specific vtable slots on `Component::Lua::GameEngine::{LuaActorImpl, NullActorImpl}` (two parallel 90-slot vtables at `0xbdfb2c` / `0xbe02ac`).** Two dispatch patterns: stack-temporary (Pattern A — 2-slot Receivers, 28 found) and heap-allocated long-lived (Pattern B — 5/6-slot Receivers in slots 56/57/58 = Kick/Start/End event lifecycle, 78 = JobQuestComplete, 88 = ChangeActorSubStatStatus, 59 = UserData). 7 unmapped Receivers are all `Set*EventCondition` variants — owned by event-handler instances, not LuaActorImpl. Still pending: the per-opcode dispatcher that picks the slot index — direct `CALL [reg+disp32]` searches return no hits, suggesting computed-index dispatch via Lua VM or via `FUN_004e20a0` (the real router downstream of `FUN_00dae520` per Phase 8 #9). |
| #6 | Walk the 37 2-slot receivers' Receive bodies — most are simple `actor[+offset] = value` updaters | ✅ done 2026-05-17 — `docs/receiver_gate_cheatsheet.md`. All 36 of 36 walked (one was 5-slot, re-classified). 27 Pattern A1 (`__RTDynamicCast` to a subclass; 3 with null-check, 24 unguarded), 9 Pattern A2 (inline; sub-divides into A2.1 pack-and-forward, A2.2 engine-root forwarding, A2.3 debug-command parser). The 3 `SetPushEventCondition{Circle,Fan,TriggerBox}` variants are identical-shaped geometry packers calling sibling 0x2f2bxx handlers. |
| #7 | Build a cheat-sheet of "what gate does each opcode's receiver check" so garlemald can reason about silent-drop symptoms | ✅ done 2026-05-17 — `docs/receiver_gate_cheatsheet.md` (same doc as #6). Practical cheat-sheet section maps each pattern to its most-likely silent-drop cause (A1.1 = wrong actor type → null dispatch; A1.0 = same but graceful; A2.1 = handler always runs, gate is downstream; A2.2 = engine root init; A2.3 = auth-gated). Includes SEQ_005-specific application: maps the 5 cinematic opcodes (Kick / Run / End / SetEventStatus / SetNoticeEventCondition) to their patterns + gates. |
| #8 | Cross-reference the SEQ_005 cinematic body packets (0x012F Kick, 0x0130 Run, 0x0136 SetEventStatus, 0x016B SetNoticeEventCondition) against their receivers' gates to identify the *exact* gate currently failing in garlemald | ✅ resolved 2026-05-17 — all three sub-tasks (#8a/#8b/#8c) closed. The receiver-gate audit's two specific findings: (i) `receiver[+0x80]` is set IFF kick `event_type` byte == `0x05` (= "noticeEvent" tag), and (ii) `context_root[+0x128]` priming happens as a SIDE EFFECT of `_callServerOn{Talk,Push,Emote}_cpp` Lua-engine bindings — NOT via any packet receiver. If garlemald's KickEvent has `event_type == 0x05` AND silent drop persists, root cause is the client never fired the trigger event (missing `SetPushEventCondition*` packets that promote NPCs to talkable state). |
| #8a | Map KickEvent packet body → receiver instance offsets (especially what byte sets `receiver[+0x80]`) | ✅ resolved 2026-05-17 — `docs/kick_receiver_offset_map.md`. **`receiver[+0x80] = 1` IFF `event_type` byte (packet body offset 8 ≡ `receiver[+0x68]`) == `0x05`** ("noticeEvent" type tag at global `[0x012c3f7e]`). The KickEvent parser is `FUN_0089f180` (289 B), which conditionally calls `FUN_0089e200` (92 B) to set `LuaParamsContainer[+0x14] = 1` AND seed the first LuaParam byte to `0x01`. Branch B2 also calls `FUN_0089e200` defensively. Garlemald's silent-drop root cause: verify `event_type` byte in KickEvent body offset 8 is `0x05` — if it's any other value, Branch B1 silent fall-through. |
| #8b | Decode SetEventStatusReceiver slot 1 + SetNoticeEventConditionReceiver slot 1 (both 2-slot receivers in the SEQ_005 path) | ✅ done 2026-05-15 — `docs/event_status_condition_receivers_decomp.md`. **Both are `__RTDynamicCast` + dispatch.** SetEventStatus casts to NpcBase, no null-check (unguarded). SetNoticeEventCondition casts to DirectorBase, with FALLBACK to ActorBase[+0x118] if cast fails. **Neither has a `+0x5c`-style actor flag gate** — eliminated as silent-drop suspect. New suspect surfaced: if `ScriptBind` (step 8 in spawn sequence) is what promotes the actor to DirectorBase, then SetNoticeEventCondition packets sent at step 2 (BEFORE ScriptBind) would silently land in `ActorBase[+0x118]` instead of `DirectorBase[+0x60]`. This is **the orphaned-conditions hypothesis** — needs verification via Phase 7's StartServerOrderEventFunctionReceiver path. |
| #8c | Look for pre-kick "receiver state init" packets that prime `context_root[+0x128]` (would shift Branch B1 → Branch B2) | ✅ architecturally resolved 2026-05-17 — `docs/context_root_128_priming_hunt.md`. **The priming is NOT a packet receiver — it's a CLIENT-SIDE engine binding `_callServerOnTalk_cpp` / `_callServerOnPush_cpp` / `_callServerOnEmote_cpp`** (NpcBaseClass Lua-bound API, confirmed via `build/wire/cpp_bindings.md`). When the player initiates an interaction, the engine fires `npc:_onTalkEvent(player, packet)` (see `build/lua/729s9/wu7/wu789r57y9rr.lua:446`) which calls `_callServerOnTalk_cpp(player, packet)` — the C++ binding writes `npc[+0x128] = player_actor_id` as a SIDE EFFECT before sending the talk-request packet to the server. Root cause of silent Branch B1 in garlemald: client never fires the talk/push trigger event (most likely because garlemald isn't sending the right `SetPushEventCondition*` packets that promote NPCs to talkable/pushable state). Definitive close via runtime HWBP on `npc[+0x128]` would confirm the binding-side-effect timing. |

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

## `ResumeChecker` census — the engine's coroutine-yield queue (ffxivDecomp, 2026-05-30)

> Sourced from **ffxivDecomp** (github.com/Yokimitsuro/ffxivDecomp), an independent
> docs-only RE of FFXIV 1.23b `ffxivgame.exe`, used with permission (see `NOTICE.md`).
> **Cross-referenced, not byte-verified** by meteor-decomp's own decompilation —
> confirm against the asm before relying on offsets for a code change.
> Captured 2026-05-30 from the ffxivDecomp 2026-05-27/28 session.

The `Receiver` classes above are the **inbound** dispatch side (server→client
packet → actor-state mutation). Their **outbound** complement — how the client
*requests* a server-validated action and then *waits* for the reply — is the
`ResumeChecker` family. Each blocking Lua binding allocates a concrete
`ResumeChecker`, pushes it onto a per-coroutine queue, and yields; the engine's
pump dequeues it once its polymorphic `isReady()` returns true. This is the
**only** yield mechanism in 1.x Lua scripts (no other suspend path exists).

This matters for the Receiver inventory because the two halves are paired: the
event-lifecycle receivers (`Kick`/`Start`/`End`, slots 56/57/58 per Phase 9 #5)
deliver the inbound packets that flip a pending `ResumeChecker`'s readiness, and
the notice-event path specifically suspends on
`ClientOrderEventWaitingResumeChecker` (see
`docs/seq005_kick_gate_analysis.md`).

### Count correction: ~24 subclasses (was 11)

ffxivDecomp's inventory grew across three findings. The first two confirmed 10
then 11 subclasses by disassembling `_wait*` thunk ctors; the third corrected
the total to **~24** by enumerating `.?AV*ResumeChecker*` RTTI strings — roughly
doubling the known hierarchy. Treat the count as a floor (RTTI enumeration may
miss engine-internal checkers Ghidra didn't auto-detect, e.g. the
HamletDefenseScore candidate at `0x006dcb00` that resolved to a data label).

| Category | Subclasses |
|---|---|
| **Async I/O / loading** (6) | `LoadDataResumeChecker` (148 B), `TextDataReadResumeChecker`, `s_MapLoadResumeChecker`, `WaitLoadFormResumeChecker`, `s_PreloadResumeChecker`, `LpbLoader::ResumeChecker` (~120 B, engine-internal) |
| **Server RPC-backed** (3) | `CreateStaticActorResumeChecker`, `CreateClientItemResumeChecker`, `GetStringResumeChecker` |
| **Animation / visual** (3) | `PlayingResumeChecker` (cutscene), `s_FadeResumeChecker`, `s_WaitForTransformIntoChocoboResumeChecker` |
| **Scheduler** (3) | `WaitForCharaSchedulerFinishedResumeChecker`, `s_WaitForCharaSchedulerTutorialFinishedResumeChecker`, `BgSchedulerResumeChecker` |
| **Tutorial** (3) | `TargetTutorialResumeChecker`, `s_CameraTutorialResumeChecker`, `s_ItemSearchWidgetResumeChecker` |
| **Core / misc** (6) | `OnInitResumeChecker` (16 B), `WaitResumeChecker` (40 B, timer), `AppendMessageResumeChecker` (12 B), `WaitForTurningResumeChecker` (8 B), `ClientOrderEventWaitingResumeChecker`, `CancelResumeChecker` |

Concrete class names live across seven namespaces — most under
`Application::Lua::Script::Client::Control::{Global,CharaBase,DesktopWidget,SpreadSheet}`,
the three tutorial helpers under the anon TU `_anon_FD906835`, the generic
timer-based `WaitResumeChecker` under `_anon_1EEF0F3D`, and the two
engine-internal checkers (`LpbLoader::ResumeChecker`, base
`ResumeCheckerInterface`) under `Component::Lua::GameEngine`. Size tracks
readiness-check state complexity: 8 B (vtable + 1 ctx ptr) for simple "is X
done?" checks, up through 40 B (timer deadline) and 120–148 B for async-I/O
checkers that carry retry/handle state.

### SEQ-005-relevant subclasses

| Subclass | Role in the cinematic path |
|---|---|
| `ClientOrderEventWaitingResumeChecker` | **The notice-path suspend point.** The Lua notice/event flow yields on this while waiting for the server's event-lifecycle packets; its readiness flips when the inbound `Kick`/`Start`/`End` receivers (this doc's 5-slot trio) land. See `docs/seq005_kick_gate_analysis.md`. |
| `s_FadeResumeChecker` | Screen fade between cinematic beats / loading screens (e.g. the `_fadeInNowLoadingForNoticeEventJustInArea` flow noted in the kick-dispatcher work). |
| `s_MapLoadResumeChecker` | Map-load wait — the "Now Loading" suspend during a zone/area handoff (relevant to the SEQ_005 same-zone `DoZoneChangeContent` warp hang). |
| `PlayingResumeChecker` (`CutScenePlaying`) | Cutscene-playback wait — yields for the duration of an in-engine cutscene. |
| `LpbLoader::ResumeChecker` | Async `.lpb` bytecode-loader wait when the content/quest script is fetched. Engine-internal (not script-callable). |
| `LoadDataResumeChecker` | SpreadSheet/`.exd`-row async read (`_loadKeyTemporarily`); the canonical example of the 2-tier async pattern below. |

### The `FunctionEndCallbackInterface` 2-tier async pattern

Async operations that complete *off* the Lua tick (disk I/O, server RPC) use a
**two-object** scheme rather than a single `ResumeChecker`:

```
TIER 1 — FunctionEndCallback  (~40 B, e.g. SpreadSheet::LoadDataFunctionEndCallback)
  Component::Lua::GameEngine::FunctionEndCallbackInterface subclass.
  Fires when the underlying async op (disk load, RPC reply) actually finishes;
  flips a "done" flag the resume-checker polls.

TIER 2 — ResumeChecker        (~148 B, e.g. SpreadSheet::LoadDataResumeChecker)
  ResumeCheckerInterface subclass. Holds a reference to the Tier-1 callback.
  This is the object the Lua coroutine yields on; its isReady() just checks the
  linked callback's done flag.
```

Each pending coroutine therefore carries **two** queues in its
`CoroutineContext`, drained by distinct helpers:

| Queue | Push helper | Drain semantics |
|---|---|---|
| End-callbacks (Tier 1) | `CoroutineContext_pushEndCallback` (`FUN_00cd28c0`) | fired on async completion |
| Resume-checkers (Tier 2) | `CoroutineContext_pushResumeChecker` (`FUN_00cd2860`) | polled each tick via `isReady()` |

A dedup helper `CoroutineContext_findPendingCallback` (`FUN_00cd2630`) lets a
binding short-circuit when an identical request (e.g. the same SpreadSheet row)
is already in flight, avoiding a double load.

**Universal yield slot.** Every Lua-registered C++ class exposes its polymorphic
spawn/yield machinery through `vtable[0x6c]` (entry 27, `0x6c = 27 * 4`) — the
same slot `_createActor` invokes to allocate `OnInitResumeChecker`. The pump
calls each checker's readiness method through its vtable, so the engine stays
agnostic to wait type: it never blocks, scripts cooperate via the queue. (Note:
ffxivDecomp could not enumerate the concrete vtable addresses — Ghidra's
auto-analysis left most C++ class vtables as anonymous `.rdata` arrays, so the
per-class `vtable[0x6c]` targets remain unnamed; only the RTTI type descriptors
are symbolized. The 0x6c slot index is proven via the `_createActor` path, not
walked per-class.)

### Cross-references (ffxivDecomp section)

- `docs/seq005_kick_gate_analysis.md` — the SEQ_005 notice/kick gate; the
  notice path suspends on `ClientOrderEventWaitingResumeChecker` whose readiness
  the inbound `Kick`/`Start`/`End` receivers (above) flip.
- ffxivDecomp `finding_resumechecker_full_inventory_10_subclasses_confirmed.md`,
  `finding_resumechecker_11th_subclass_LpbLoader_plus_vtable_methodology.md`,
  `finding_outbound_rpc_0x12e_format_plus_resumechecker_count_correction.md`
  (source findings; the third carries the 11 → ~24 count correction and the
  outbound `0x12e` RPC pairing).
