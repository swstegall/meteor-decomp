# `PlayerBaseClass` decomp — the player-side engine surface

> Last updated: 2026-05-03 — focused decomp of `PlayerBaseClass.lpb`,
> the largest single class in the shipped script corpus (~73 KB across
> 7 files). 94 C++-bound methods + 77 Lua-defined methods on the main
> file alone, plus split-file work/cliprog/etc.

## Headline correction to `docs/cpp_bindings_index.md`

That doc said:
> PlayerBase needs 94 + 76 (CharaBase inherited) ≈ 170 distinct
> C++-bound method signatures. Garlemald's userdata.rs currently
> exposes ~50 → coverage gap of ~120 methods.

**That framing was wrong.** The decomp here makes it clear:

- The engine's PlayerBaseClass API surface (94 C++ bindings + the
  inherited CharaBase methods) is what the **CLIENT-side shipped
  scripts** call. They live in the binary's Lua VM, not in
  garlemald's mlua VM.
- Garlemald's `LuaPlayer` userdata is the **SERVER-side binding**
  for garlemald's own scripts. It uses garlemald's own Pascal-case
  conventions (`AcceptQuest`, `AddGuildleve`, etc.) that DON'T
  overlap with the engine's `_executeTalk_cpp` / `_cancelCommand_cpp`
  family.

Cross-reference confirms this: 0 of garlemald's 40 LuaPlayer gaps
(from `docs/garlemald_lua_coverage_index.md`) match any engine C++
binding even case-normalized. They're two independent API layers in
two independent VMs.

What this decomp DOES surface for garlemald is the **state model**
the engine maintains client-side — every method here reads or
writes some player-state field that garlemald has to populate via
SetActorProperty packets for the CLIENT's PlayerBaseClass scripts
to behave correctly.

## File inventory

| File | LOC | Purpose |
|---|---:|---|
| `PlayerBaseClass.lpb` (main) | **3,020** | The largest single base-class main in the corpus |
| `PlayerBaseClass_u.lpb` | 941 | 94 C++ binding declarations |
| `PlayerBaseClass_work.lpb` | 369 | Work-table accessors (`playerWork.guildleveId[N]`, etc.) |
| `PlayerBaseClass_cliprog.lpb` | 180 | Client-prog command-variation accessors |
| `PlayerBaseClass_craft.lpb` | 1 | Empty stub |
| `PlayerBaseClass_harvest.lpb` | 1 | Empty stub |
| `PlayerBaseClass_negotiation.lpb` | 1 | Empty stub |

**Total: ~73 KB of Lua source** — about 2× CharaBaseClass and
~10× WorldMaster. This is THE class.

`require()` ordering at the top of main:
```lua
require("/Chara/Player/PlayerBaseClass_craft")
require("/Chara/Player/PlayerBaseClass_harvest")
require("/Chara/Player/PlayerBaseClass_negotiation")
require("/Chara/Player/PlayerBaseClass_cliprog")
-- (then defines PlayerBaseClass methods directly in main)
```

The 4 split files compose into PlayerBaseClass via `require()`. The
3 empty stubs were probably aspect-files at one point that got
absorbed back into main.

## C++-bound API (94 methods, from `_u`)

Per `docs/cpp_bindings_index.md`, organized by domain:

### Talk / emote / command lifecycle (15)

The paired `_callServer*` / `_doServer*` / `_execute*` /
`_cancel*` / `_break*` / `_can*` / `_count*` / `_is*Playing` /
`_is*PushingOut` family for player actions:

| Method | Role |
|---|---|
| `_executeTalk` / `_canExecuteTalk` / `_cancelTalk` | Talk action |
| `_executeEmote` / `_canExecuteEmote` / `_cancelEmote` | Emote action |
| `_executeCommand` / `_canExecuteCommand` / `_cancelCommand` / `_breakCommand` | Combat / system command |
| `_callServerOnCommand` / `_doServerOnCommand` | Server-RPC pair (per `docs/scenario_monster_decomp.md`) |
| `_isCommandPlaying` / `_countCommandPlaying` | Active-command predicates |
| `_isPushingOut` | Player-being-pushed-out predicate |
| `_cancelPush` / `_cancelNotice` | Push/notice cancellation |

### Camera / lock-on / player control locks (10)

Locks that prevent normal input while a cutscene/tutorial runs:

| Method | Role |
|---|---|
| `_lockCameraControl` / `_unlockCameraControl` / `_isCameraControlEnabled` | Camera lock |
| `_lockLockonControl` / `_unlockLockonControl` / `_isLockonControlEnabled` | Lock-on lock |
| `_lockPlayerControl` / `_unlockPlayerControl` / `_isPlayerControlEnabled` | Movement lock |
| `_forceCameraTPSMode` | Force third-person camera |
| `_getLockonTarget` / `_setLockonTarget` | Lock-on target |

### Fade / cinematic primitives (9)

Mirrors QuestBaseClass_common's fade primitives — accessible from
PlayerBase too:

| Method | Role |
|---|---|
| `_fadeOut` / `_fadeIn` / `_fadeInAfterWarp` | Fade transitions |
| `_fadeInNowLoadingForNoticeEventJustInArea` | Fade with loading screen, area-context |
| `_cancelFading` / `_isFading` / `_resetFade` | Fade state |
| `_waitForFading` / `_waitForMapLoaded` | Block-on-fade primitives |

### Achievements / trophies (16)

Largest single domain in PlayerBase. Achievement system is a major
1.x sub-system:

| Method | Role |
|---|---|
| `_achieveTrophy` / `_canGetTrophy` / `_isAchievedTrophy` | Trophy completion |
| `_isDoneAchievement` / `_isDoneAchievementRateList` | Status predicates |
| `_clearAchievementRateCache` | Cache invalidation |
| `_countAchievementCategory` / `_countAchievementItem` / `_countAchievementRateList` | Counts |
| `_countEnableAchievementTitle` / `_getEnableAchievementTitle` | Title visibility |
| `_getAchievementCategoryId` / `_getAchievementItemId` / `_getAchievementPoint` | ID/point queries |
| `_getAchievementRate` / `_getAchievementRateList` | Progress rate |
| `_getAchievementSheetDataIcon/Item/Point/Title` | Spreadsheet data |
| `_getAchievementTitle` / `_setAchievementTitle` | Active title |
| `_hasAchievementItem` / `_hasAchievementTitle` | Predicates |

### Inventory storage (4)

| Method | Role |
|---|---|
| `_canStoreItem` | Predicate: can the player accept this item? |
| `_countStoredItem` / `_getStoredItem` | Stored-item queries |
| `_haveEnmityCharacters` | Has enmity-relationship characters |

### Cutscene replay (5)

Replay system for completed quests:

| Method | Role |
|---|---|
| `_isCompletedCutSceneReplayQuest` | Quest-completion predicate |
| `_getCutSceneReplaySnpcCoordinate/Nickname/Personality/Skin` | Snpc spawn data for replay |

### Hamlet defense / behest content (7)

Content-instance state queries:

| Method | Role |
|---|---|
| `_countHamletDefenseScore` / `_getHamletDefenseScore` / `_getHamletDefenseScoreAll` | Hamlet defense scoring |
| `_getNMRushUpdateTime` | NM (Notorious Monster) rush event timer |
| `_getCompanyBehestTime` / `_getNormalBehestTime` | Behest timers |
| `_getOccupancyContentsTime` | Occupancy content timer |

### Touch / movement (3)

| Method | Role |
|---|---|
| `_isTouching` / `_setTouchAttribute` | Touch-event state |
| `_turn` | Programmatic turn |

### Inn / homepoint (2)

| Method | Role |
|---|---|
| `_readyInnBed` | Trigger inn-bed UI |
| `_setPosDirInn` | Set inn position + facing |

### GC / chocobo / GM (4)

| Method | Role |
|---|---|
| `_getBelongGrandCompany` / `_getGrandCompanyRank` | GC state |
| `_getChocoboGrade` / `_getChocoboRidingGrade` | Chocobo training/riding levels |
| `_getGMRank` | GM rank |
| `_isEnabledGoobbue` | Goobbue mount predicate |

### Misc (5)

| Method | Role |
|---|---|
| `_chat` | Send chat message |
| `_setMusic` | Change BGM |
| `_setWeather` | Change weather |
| `_getWarpRecastTime` | Aetheryte warp cooldown |
| `_isEventPlaying` | Event-mode predicate |

## Lua-side API (PlayerBaseClass main, 77 methods)

The main file's 77 Lua-defined methods compose the C++ primitives
into higher-level operations:

### Identity / demographics (10)

`isPlayer`, `isMyPlayer`, `isValidName`, `isFemale`, `isMale`,
`getTribe`, `getNation`, `getGuardian`, `getBirthday`,
`getInitialTown`

### Quest state (7)

| Method | Purpose |
|---|---|
| `getScenarioQuest(self, idx)` / `getScenarioQuestLength(self)` | Scenario quest table |
| `getGuildleveQuest(self, idx)` / `getGuildleveQuestLength(self)` | Guildleve quest table |
| `isQuestComplete(self, quest_id)` | Per-quest completion |
| `updateQuestComplete(self, quest_id)` | Mark complete |
| `processUpdateQuestComplete(self, ...)` | Engine-side update handler |

### Grand Company state (6)

| Method | Purpose |
|---|---|
| `getGrandCompanyRank(self)` / `getGrandCompanyRankLinear(self)` | Rank queries |
| `getGrandCompanySealCount(self)` / `getGrandCompanySealMax(self)` | Seal economy |
| `getGrandCompanyNeedSealNextRank(self)` | Next-rank threshold |
| `isPrebelongGrandCompany(self)` | "Was previously a member" predicate |

### NPC linkshell (3)

| Method | Purpose |
|---|---|
| `hasNpcLinkshell(self)` | Has any NPC LS membership |
| `isNpcLinkshellChatCalling(self)` | Pending chat call |
| `getNpcLinkshellChatLinkshellLength(self)` | Active LS count |

### Command system — the biggest group (15)

| Method | Purpose |
|---|---|
| `command(self, ...)` | Top-level command dispatch |
| `delegateCommand(self, ...)` | Delegate to sub-handler |
| `canCommand(self, ...)` | Predicate |
| `_onCommandRequest(self, ...)` / `_onCommandEvent(self, ...)` / `_onCommandCancel(self, ...)` / `_onCommandRejected(self, ...)` | Server-RPC pairs |
| `_onPreCommand(self, ...)` / `_onPostCommand(self, ...)` | Pre/post hooks |
| `getCastCommand(self)` / `getCastEndTime(self)` | Active cast info |
| `getComboInformation(self)` | Battle Regimen state |
| `getOtherClassAbilityCountInformation(self)` | Cross-class ability counts |
| `setEmoteSitCommandVariation(self, ...)` | Emote-sit variation |

### Content command / content widget (8)

| Method | Purpose |
|---|---|
| `getQuestContentsCommandPermitFlag(self)` / `setQuestContentsCommandPermitFlag(self, flag)` | The combat-command gate (per `docs/director_quest_decomp.md`) |
| `setContentCommandVariation(self, ...)` | Set the active content-command set |
| `setPlaceDrivenCommandVariation(self, ...)` / `resetPlaceDrivenCommandVariation(self)` | Place-driven (e.g. inside an inn) command set |
| `commandAboutWidget(self, ...)` / `cancelCommandAboutWidget(self, ...)` / `processCancelCommandAboutWidget(self, ...)` | About-widget commands |
| `isCommandAboutWidgetPlaying(self)` | Predicate |
| `commandAboutDebug(self, ...)` | Debug command |

### Information requests (4)

| Method | Purpose |
|---|---|
| `recordRequestInformation(self, ...)` / `canRequestInformation(self)` | Record + gate info-request actions |
| `getGiftCountInformation(self)` | Gift-count display |
| `getRestBonusExpRate(self)` | Rest-bonus xp multiplier |

### Achievements / bonus (2)

| Method | Purpose |
|---|---|
| `isAcquiredAdditionalCommand(self, command_id)` | Predicate |
| `isRemainBonusPoint(self)` | Has unspent bonus points |

### Touch / movement event hooks (5)

| Method | Purpose |
|---|---|
| `_onTouch(self, ...)` | Touch event |
| `_onMoveAtSit(self, ...)` | Movement-while-sitting |
| `_onChocoboRentalRide(self, ...)` / `_onChocoboWarpRide(self, ...)` | Chocobo-mounting events |
| `_onGetGoobbue(self, ...)` | Goobbue-pet acquisition |

### Login / event lifecycle (10)

| Method | Purpose |
|---|---|
| `_onInit(self, ...)` | Init |
| `_onLoginEvent(self, ...)` | Fired at login |
| `_onPreEvent(self, ...)` / `_onPostEvent(self, ...)` | Around event-mode entry/exit |
| `_onUpdateWork(self, ...)` | Work-table update hook |
| `_onReceiveDataPacket(self, ...)` | Generic data-packet receiver |
| `_onReceiveTimingPacket(self, ...)` | Timing-packet receiver |
| `_onReceiveAchievementId(self, ...)` / `_onReceiveAchievementRate(self, ...)` | Achievement updates |
| `_onReceiveLimitAddicted(self, ...)` | "Limit reached" notification |

### Misc (5)

| Method | Purpose |
|---|---|
| `getSystemCommand(self, ...)` | System-command lookup |
| `postMapOpen(self, ...)` | Post-map-open hook |
| `checkSameItemCatalogId(self, a, b)` | Item-id equality check |
| `getWarpRecastTime(self)` | Aetheryte warp cooldown (Lua wrapper for the C++ `_getWarpRecastTime`) |
| `decodeTimingPacketInformation(self, packet)` | Parse timing packet |
| `isEventPlaying(self)` | Event-mode predicate (Lua wrapper) |

## `_work` file — playerWork accessors (369 LOC)

Implements typed accessors for the `playerWork` work-table fields,
mostly for guildleve state:

```lua
function PlayerBaseClass:getGuildleveID(idx)
    return self.playerWork.guildleveId[idx]
end

function PlayerBaseClass:getGuildleveIndexMax()
    -- linear scan of playerWork.guildleveId[] for first 0
    for i = 1, #self.playerWork.guildleveId do
        if self.playerWork.guildleveId[i] == 0 then
            return i - 1
        end
    end
    return #self.playerWork.guildleveId
end

function PlayerBaseClass:isHavingGuildleveById(id)
    -- linear scan for matching ID
end
```

Other observed methods: `isHavingGuildleveCompletedById`,
`getGuildleveDoneCount`, etc. — all wrappers around
`self.playerWork.guildleveId[]` and `self.playerWork.guildleveDone[]`.

The `playerWork` is the **per-player Lua-side state mirror** — the
client's view of player state. Server-driven property updates feed
into this via the SyncWriter mechanism (per `docs/sync_writer.md`).

## `_cliprog` file — command variation accessors (180 LOC)

7 methods, all `getConfirm*CommandVariation` accessors:

| Method | Purpose |
|---|---|
| `getConfirmGroupCommandVariation(self)` | "Confirm group?" command set |
| `getConfirmRaiseCommandVariation(self)` | "Confirm raise?" command set |
| `getConfirmTradeCommandVariation(self)` | "Confirm trade?" command set |
| `getConfirmWarpCommandVariation(self)` | "Confirm warp?" command set |
| `getContentCommandVariation(self)` | Active content command set |
| `getEmoteSitCommandVariation(self)` | Emote-sit command set |
| `getPlaceDrivenCommandVariation(self)` | Place-driven command set |

These return a numeric ID identifying which command variation is
active for the given UI context (confirmation dialogs, content
instances, etc.).

## Implications for garlemald

### Correction: garlemald's LuaPlayer is NOT meant to mirror this API

The earlier framing in `cpp_bindings_index.md` suggested garlemald's
`userdata.rs` should converge toward the engine's PlayerBaseClass
API surface. The decomp here makes clear that's wrong — the two
APIs serve different VMs:

- **Engine PlayerBaseClass** (94 C++ methods + 77 Lua wrappers) →
  consumed by **client-side** shipped `.lpb` scripts via the
  binary's Lua VM
- **Garlemald LuaPlayer** (whatever methods garlemald defines) →
  consumed by **server-side** garlemald scripts via mlua

Garlemald's LuaPlayer should be designed for what garlemald's OWN
scripts need, not for what client scripts call. The 40 LuaPlayer
gaps from `docs/garlemald_lua_coverage_index.md` are the actionable
list — garlemald should bind the methods its own scripts call,
which are NOT the same as the engine's C++ bindings.

### What this decomp DOES reveal for garlemald

1. **State fields the client expects to read** via PlayerBaseClass
   methods — every `playerWork.<field>` access here is a field
   garlemald MUST populate via SetActorProperty packets:
   - `playerWork.guildleveId[]` — array of active guildleve IDs
   - `playerWork.guildleveDone[]` — paired completion flags
   - `playerWork.questComplete[]` — quest-completion bit table
   - `playerWork.scenarioQuest[]`, `playerWork.guildleveQuest[]`
   - GC state: `playerWork.companyRank`, `playerWork.companySealCount`,
     etc.
   - NPC LS state: `playerWork.npcLinkshell*`
   - Achievement state: `playerWork.achievementTitle`, etc.

2. **The combat-command gate is `getQuestContentsCommandPermitFlag`**
   (already documented in `docs/director_quest_decomp.md`). Setting
   `playerWork.questContentsCommandPermitFlag` on the player work
   table enables / disables combat commands during content instances
   like the man0g0 SEQ_005 tutorial.

3. **The achievement system is a major sub-system** (16 of 94 C++
   bindings). Garlemald can defer this until achievement-related
   gameplay matters, but the wire format for achievement state
   broadcasts will eventually need to mirror the field layout
   referenced by the achievement methods (sheet IDs, points, rates,
   titles).

4. **Server-RPC pairs follow the same pattern as NpcBaseClass**
   (per `docs/scenario_monster_decomp.md`):
   `_callServerOn{Talk,Emote,Command}` → `_doServerOn{Talk,Emote,Command}`
   → `_on{X}{Request,Event,Cancel,Rejected}`. Garlemald's existing
   `onTalk`/`onEmote`/`onCommand` server-side dispatch maps to these.

5. **Snpc support requires 4 Lua-callable methods + 1 predicate**:
   `_getCutSceneReplaySnpcCoordinate/Nickname/Personality/Skin` +
   `_isCompletedCutSceneReplayQuest`. These power the cutscene-replay
   feature in 1.x where players could re-watch completed
   storyline cutscenes with their custom Snpc actors. Likely
   deferrable for garlemald.

6. **Touch / movement event hooks** (`_onTouch`, `_onMoveAtSit`,
   `_onChocoboRentalRide`, `_onChocoboWarpRide`, `_onGetGoobbue`)
   are CLIENT-side hooks that garlemald should be aware of as the
   client may emit corresponding wire events. Most are mount-related
   (chocobo-mounting + Goobbue-pet acquisition).

## Cross-references

- `docs/cpp_bindings_index.md` — the corrected understanding
  acknowledged here (engine vs garlemald API independence)
- `docs/garlemald_lua_coverage_index.md` — the 40 LuaPlayer gaps
  are the actionable list (NOT the 94 engine C++ bindings)
- `docs/scenario_monster_decomp.md` — companion (CharaBaseClass +
  NpcBaseClass which PlayerBaseClass extends)
- `docs/director_quest_decomp.md` —
  `getQuestContentsCommandPermitFlag` flow
- `docs/world_master_decomp.md` — sibling class (the other
  most-referenced engine global)
- `docs/desktop_widget_decomp.md` — closes the major-Lua-base
  trilogy plus PlayerBase
- `docs/sync_writer.md` — Phase 6 item #4 (the wire mechanism that
  populates the playerWork fields PlayerBaseClass methods read)
- `docs/lpb_corpus_survey.md` — corpus pipeline
