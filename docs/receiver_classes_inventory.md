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
| #2 | Decode `ChangeActorSubStatStatusReceiver` (last 5-slot) | 🔲 pending |
| #3 | Decode the 6-slot `JobQuestCompleteTripleReceiver` | 🔲 pending |
| #4 | Decode the 6-slot `UserDataReceiver` | 🔲 pending |
| #5 | Cross-reference each receiver to its opcode (the engine wires opcode → receiver at script load; need to find that registration) | 🔲 pending |
| #6 | Walk the 37 2-slot receivers' Receive bodies — most are simple `actor[+offset] = value` updaters | 🔲 pending |
| #7 | Build a cheat-sheet of "what gate does each opcode's receiver check" so garlemald can reason about silent-drop symptoms | 🔲 pending |
| #8 | Cross-reference the SEQ_005 cinematic body packets (0x012F Kick, 0x0130 Run, 0x0136 SetEventStatus, 0x016B SetNoticeEventCondition) against their receivers' gates to identify the *exact* gate currently failing in garlemald | 🟡 partial — cross-ref doc at `docs/seq005_receiver_gate_audit.md`; identifies Branch B1's `receiver[+0x80]` flag as the prime suspect for the silent kick drop, but Phase 7 didn't decode which packet byte maps to it. Three follow-ups (#8a/#8b/#8c) below. |
| #8a | Map KickEvent packet body → receiver instance offsets (especially what byte sets `receiver[+0x80]`) | 🟡 partial — `receiver[+0x80]` is mapped to `(LuaParamsContainer at +0x6c)[+0x14]` (see `docs/kick_receiver_offset_map.md`). The byte's PACKET source still requires tracing the IpcChannel parser (slot 1 is the heap copy ctor, not the original parser). The byte-by-byte KickEvent diff vs pmeteor showed body bytes are identical, suggesting the gate value comes from receiver STATE (not the kick packet itself) — possibly primed by an earlier opcode garlemald doesn't send. |
| #8b | Decode SetEventStatusReceiver slot 1 + SetNoticeEventConditionReceiver slot 1 (both 2-slot receivers in the SEQ_005 path) | 🔲 pending — verifies whether condition-enablement has its own gates |
| #8c | Look for pre-kick "receiver state init" packets that prime `context_root[+0x128]` (would shift Branch B1 → Branch B2) | 🔲 pending — pmeteor may send a packet we don't |

## Cross-references

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
