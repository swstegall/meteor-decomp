# Scenario / Monster / Chara hierarchy — focused decomp

> Last updated: 2026-05-03 — decompiled inheritance chain + method
> inventories for ScenarioBaseClass (619 quest subclasses),
> MonsterBaseClass family (89 monsters across 30+ families), and
> CharaBaseClass (the shared base for Player + Npc). Grounds garlemald's
> mob-spawn + quest-lifecycle + NPC-interaction work.

## Inheritance chain (verified end-to-end)

```
QuestBaseClass.lpb                       (registered root, Phase 6 #3)
  └── ScenarioBaseClass.lpb              (8 LOC — empty marker base)
      └── Man0g0.lpb, Man0l0.lpb, ...    (619 main-scenario quests)

CharaBaseClass.lpb                       (registered root)
  ├── PlayerBaseClass.lpb                (registered root)
  │     ├── _craft (empty), _harvest (empty), _negotiation (empty)
  │     ├── _u (941 LOC), _work (369 LOC), _cliprog (180 LOC)
  │     └── (3020 LOC main)
  └── NpcBaseClass.lpb                   (registered root)
        ├── _u (237 LOC), _event (921 LOC), _battle (104 LOC)
        ├── _battletest (empty)
        ├── (623 LOC main)
        └── MonsterBaseClass.lpb         (8 LOC — empty marker base)
              ├── EmpireBaseClass.lpb    (8 LOC, 59 Garlean enemies)
              ├── BirdmanBaseClass.lpb   (30 yarzon-family mobs)
              ├── LizardmanBaseClass.lpb (24 mobs)
              ├── ElementalBaseClass.lpb (18)
              ├── GnoleBaseClass.lpb     (16 — Goobbues etc.)
              ├── SpriteBaseClass.lpb    (13)
              ├── TermiteBaseClass.lpb   (13)
              ├── RebelBaseClass.lpb     (20 — pirate / hostile humanoids)
              └── ... (~30 mob families total)
```

## File-split convention

Heavy base classes are split across multiple `.lpb` files by aspect:

| Suffix | Decoded | Purpose |
|---|---|---|
| (no suffix) | (main) | Class registration + entry |
| `_5o5wq` | `_event` | Event/talk-flow handlers |
| `_89qqy5` | `_battle` | Combat-oriented methods |
| `_89qqy5q5rq` | `_battletest` | Combat-tutorial overrides |
| `_u9s9x5q5s` | `_parameter` | Stat / parameter accessors |
| `_44m1o89qqy5` | `_ffxivbattle` | FFXIV-specific combat (legacy?) |
| `_7y1usv3` | `_cliprog` | Client-side prog (vs server-bound) |
| `_nvsz` | `_work` | Work-table declarations |
| `_29so5rq` | `_harvest` | Gathering-specific (Player only) |
| `_w53vq19q1vw` | `_negotiation` | Dialog / Aetheryte negotiation |
| `_7s94q` | `_craft` | Crafting-specific (Player only) |
| `_7vxxvw` | `_common` | Shared utility (Quest only) |
| `_p` | `_u` | Tail / "u"-suffix file (purpose unclear) |

The main file `require()`s the others to compose the full class. Some
suffix files are empty stubs (1-17 LOC) — present in the build but
without code, likely leftover from an earlier modular approach.

## ScenarioBaseClass (8 LOC — empty marker)

```lua
require("/Quest/QuestBaseClass")
_defineBaseClass("ScenarioBaseClass", "QuestBaseClass")
```

Adds NOTHING to QuestBaseClass. It exists purely as a typed marker so
quest-runtime code can `_isInstanceOf(quest, "ScenarioBaseClass")` to
distinguish main-scenario quests from job/class/guildleve/etc.

**For garlemald:** since ScenarioBaseClass is a no-op pass-through,
garlemald's per-quest scripts can effectively ignore it — the API
they consume is QuestBaseClass + QuestBaseClass_common.

## QuestBaseClass — core quest API (~30 methods)

`/Quest/QuestBaseClass.prog` (main + `_common`). Method inventory
(extracted from .luac string tables):

### Quest identity / data

| Method | Purpose |
|---|---|
| `getQuestId(self)` | Numeric quest ID |
| `getQuestData(self)` | Per-quest data table |
| `_getStaticActorID(self)` | Static actor ID for the quest's owning NPC |
| `_isExistActor(self, ...)` | Predicate: does an actor exist? |
| `_createActor(self, ...)` | Spawn an actor |
| `_runCharaScheduler(self, ...)` | Run a chara-scheduler step (animation/movement) |

### Text-data lifecycle

| Method | Purpose |
|---|---|
| `initText(self)` | Standard quest-start text load (every quest defines this) |
| `_loadKeySemipermanently(self, key)` | Load a text key permanently for the quest |
| `_unloadKey(self, key)` | Unload a previously loaded key |
| `_loadWord(self, word)` | Load a single text word |
| `_unloadWord(self, word)` | Unload a word |

### Player query helpers

| Method | Purpose |
|---|---|
| `_getMyPlayer()` | (worldMaster method) Get the local player handle |
| `getInitialTown(self)` | Player's starting town id (Limsa/Gridania/Ul'dah) |
| `getNation(self)` | Player's current Grand Company nation |
| `_getBelongGrandCompany(self)` | GC-membership state |

### Cutscene replay (job-quest replays)

| Method | Purpose |
|---|---|
| `_isCompletedCutSceneReplayQuest(self, quest_id)` | Has the player finished a replay-able quest? |
| `getCutSceneReplayData(self)` | Get replay metadata |
| `_getCutSceneReplaySnpcCoordinate(self, ...)` | Snpc spawn coords for replay |
| `_getCutSceneReplaySnpcNickname(self, ...)` | Snpc display name |
| `_getCutSceneReplaySnpcPersonality(self, ...)` | Snpc personality tag |
| `_getCutSceneReplaySnpcSkin(self, ...)` | Snpc skin id |
| `getSnpcSexualityToSkin(self, ...)` | Convert (sex, sexuality) → skin id |

### Job-quest completion hooks

| Method | Purpose |
|---|---|
| `_onJobQuestCompleteFirst(self)` | Engine internal; calls `onJobQuestCompleteFirst` |
| `onJobQuestCompleteFirst(self)` | Subclass override (first job quest done) |
| `_onJobQuestCompleteSecond(self)` | … |
| `onJobQuestCompleteSecond(self)` | … |
| `_onJobQuestCompleteThird(self)` | … |
| `onJobQuestCompleteThird(self)` | … |
| `_onCancelJobQuestComplete{First,Second,Third}(self)` | Cancel-path counterparts |

### Reward UI

| Method | Purpose |
|---|---|
| `openQuestRewardWidget(self, ...)` | Open the quest-complete reward popup |

### Lifecycle

| Method | Purpose |
|---|---|
| `_onInit(self, ...)` | Engine-internal init |
| `_onFinalize(self)` | Engine-internal cleanup |

## QuestBaseClass_common — cinematic + dialog primitives (~50 methods)

`/Quest/QuestBaseClass_common.prog` is the **cinematic / cutscene /
dialog** primitives library that all quests use. Notable methods:

### Camera / fade

| Method | Purpose |
|---|---|
| `_fadeOut(self, ...)` | Start a fade-to-black transition |
| `_fadeIn(self, ...)` | Start a fade-from-black transition |
| `_fadeInAfterWarp(self, ...)` | Fade-in after a zone change |
| `_fadeInNowLoadingForNoticeEventJustInArea(self, ...)` | Fade-in with loading screen, area-context |
| `_waitForFading(self, ...)` | Block script until fade completes |
| `_waitForMapLoaded(self, ...)` | Block until zone map is loaded |
| `_resetFade(self)` | Reset fade overlay state |

### Movement / orientation

| Method | Purpose |
|---|---|
| `_getPos(self, actor)` | Get actor's (x, y, z) |
| `_getDir(self, actor)` / `_getOrientation(self, actor)` | Get actor's facing |
| `_turnDir(self, actor, dir)` | Turn an actor to face a direction |
| `_waitForTurning(self, ...)` | Block until turn animation completes |
| `clientTrunDirForQuestNpc(self, ...)` | Client-side NPC turn (typo "Trun" preserved) |
| `_setMusic(self, ...)` | Change BGM track |
| `_wait(self, frames)` | Sleep N frames |

### Cutscene API

| Method | Purpose |
|---|---|
| `createCutScene(self, name, ...)` | Schedule a cutscene |
| `_getPendingCutSceneActor(self, ...)` | Get actor instance scheduled to appear in cutscene |

### Dialog widgets

| Method | Purpose |
|---|---|
| `askQuestDetailWidget(self, ...)` | "Quest details" popup |
| `askEventModeWidgetYield(self, ...)` | "Yield/give-up event mode" popup |
| `askRetainerNamingWidget(self, ...)` | Retainer-naming UI |
| `askSelectReleaseQuestWidget(self, ...)` | "Abandon quest" popup |
| `inputSnpcName(self, ...)` | Snpc-naming input UI |
| `contentsJoinAskInBasaClass(self, ...)` | "Join content?" popup |
| `instanceAreaJoinAskInBasaClass(self, ...)` | "Join instance area?" popup |

### Misc

| Method | Purpose |
|---|---|
| `_randomInteger(min, max)` | Quest-side RNG (deterministic per session) |
| `_getZoneName(self, zone_id)` | Zone display name |
| `_delete(self)` | Cleanup |
| `getScenarioQuest(self, idx)` / `getScenarioQuestLength(self)` | Scenario quest table accessors |
| `isCraftPassiveGuildleve(self, leve_id)` | Guildleve-type predicate |
| `isFemale/isMale/isPlayerFemale/isPlayerMale(self)` | Sex predicates |
| `jobTutorial(self, ...)` | Job-tutorial entry |
| `getJobQuestIcon(self, ...)` / `getJobQuestJobName(self, ...)` | Job-quest UI accessors |

### Embedded quest-name references

The `_common` script body literally references specific quest IDs:
`man20140`, `man20150`, `man20602`, `man20603`, `man20630`,
`man30020` — these are job-quest IDs the cutscene-replay system
needs hardcoded. Useful for garlemald's job-quest gating.

## MonsterBaseClass / EmpireBaseClass (8 LOC each — empty markers)

```lua
-- MonsterBaseClass.lua
require("/Chara/Npc/NpcBaseClass")
_defineBaseClass("MonsterBaseClass", "NpcBaseClass")

-- EmpireBaseClass.lua
require("/Chara/Npc/Monster/MonsterBaseClass")
_defineBaseClass("EmpireBaseClass", "MonsterBaseClass")
```

Both are empty marker bases. The mob-family hierarchy is purely
**typed-marker** (for `_isInstanceOf` checks) — all behaviour comes
from NpcBaseClass / NpcBaseClass_battle / NpcBaseClass_event.

This means **per-mob-family customization happens in
NpcBaseClass_battle**, NOT in the family bases. Garlemald's monster-
spawn work doesn't need to ship per-family scripts; the empty bases
are sufficient for `_isInstanceOf` queries.

## NpcBaseClass — talk / push / emote / aggro API

Per-aspect file inventory:
- `NpcBaseClass.lpb` (623 LOC) — main + lifecycle + work-table setup
- `NpcBaseClass_event.lpb` (921 LOC) — talk/dialog flow handlers
- `NpcBaseClass_battle.lpb` (104 LOC) — combat surface
- `NpcBaseClass_u.lpb` (237 LOC) — tail file

### Server-side RPC pairs (from main file)

The talk / push / emote flow uses **paired client-side and server-side
methods**:

| Client-side `_call` | Server-side `_do` | Event hook | Request hook | Rejected hook |
|---|---|---|---|---|
| `_callServerOnTalk` | `_doServerOnTalk` | `_onTalkEvent` | `_onTalkRequest` | `_onTalkRejected` |
| `_callServerOnPush` | `_doServerOnPush` | `_onPushEvent` | `_onPushRequest` | (no Rejected) |
| `_callServerOnEmote` | `_doServerOnEmote` | `_onEmoteEvent` | `_onEmoteRequest` | `_onEmoteRejected` |

The pattern: client calls `_callServerOn<X>` to request the action →
server validates and calls back `_doServerOn<X>` → client fires
`_on<X>Event` if accepted, `_on<X>Rejected` if denied. **`_onPush`
has no Rejected variant** — a push is always either accepted or
silently ignored.

This matches garlemald's existing `onTalk` / `onPush` / `onEmote`
server-side dispatch (per
`project_garlemald_proximity_push_kick.md`).

### Other NpcBaseClass methods

| Method | Purpose |
|---|---|
| `_setLockonTarget(self, target)` | Set the NPC's lock-on target |
| `cancelAllTarget(self)` | Clear all targeting |
| `_onReaction(self, ...)` | Reaction-event hook (e.g., emote response) |
| `_onTimer(self, ...)` | Periodic timer tick |
| `_getExtendedTemporaryGroup(self, ...)` | Get the NPC's temporary group |
| `delegateEvent(self, ...)` | RPC dispatch (same as DirectorBaseClass) |
| `actorClassId` | Field: numeric class ID |
| `commandContent` / `commandDefault` / `commandForced` / `commandJudgeMode` / `commandWeak` | Command-mode flags |
| `emoteDefault1` / `emoteDefault2` / `emoteDefault3` | Default emote IDs |

### NpcBaseClass_event (talk-flow primitives, 921 LOC)

The talk-flow API for dialogs:

| Method | Purpose |
|---|---|
| `_lookAtCharacter(self, target)` | Make NPC look at a character |
| `_lookAtPosition(self, x, y, z)` | Look at coordinate |
| `_cancelLookAt(self)` | Cancel look-at |
| `lookAtPosition(self, ...)` | Public variant |
| `_turnBack(self, ...)` | Turn 180° |
| `_setGroundOn(self, ...)` | Set "stand on ground" mode |
| `startCliantTalkTurn(self, ...)` | Start a client-side talk turn (typo "Cliant" preserved) |
| `startCliantTalkTurnNoWait(self, ...)` | Start without waiting for previous |
| `waitCliantTalkTurn(self, ...)` | Block until talk turn completes |
| `finishCliantTalkTurn(self, ...)` | End the talk turn |
| `normalTalkStep0(self, ...)` | Standard talk-step entry |
| `switchEvent(self, ...)` | Switch to another event |
| `showMessage(self, ...)` | Show a message bubble |
| `doSalute(self, ...)` | Perform a salute animation |
| `askExtendWidget(self, ...)` | Open an extend (sub) widget |
| `askForCustomizeOption(self, ...)` | Open customization options |
| `askRestrictChoices(self, ...)` | Restrict-choice dialog |
| `askForEventMode(self, ...)` | "Enter event mode?" prompt |
| `getMapMarkerTypeForTalkable(self)` | Map-marker type for this NPC |
| `isMapMarkerVisibleForTalkable(self, ...)` | Map-marker visibility |
| `getLimitedDistanceForTalk(self)` | Max distance for talk action |
| `isContentsInAsk(self, ...)` | "Are we currently asking?" predicate |
| `getGrandCompanyRank(self, ...)` / `_getGrandCompanyRank(self, ...)` | GC rank query |
| `_getGrandOnExtraStat(self, ...)` | GC "extra stat" |
| `isUpperRank(self, ...)` | Rank-comparison predicate |
| `initForEvent(self, ...)` / `initForEventCommon(self, ...)` | Event-init entries |

### NpcBaseClass_battle (combat surface, 104 LOC)

Compact combat-side API:

| Method | Purpose |
|---|---|
| `aggro` | Field: aggro level |
| `getAggro(self)` | Read aggro |
| `partsName` | Body-part name table |
| `getPartsName(self, ...)` | Read part name |
| `partsExists` | Boolean field per part |
| `isPartsExists(self, ...)` | Predicate |
| `npcWork` | NPC-specific work table |
| `battleCommon` | Shared combat work fields |
| `initForBattle(self, ...)` / `initForBattleCommon(self, ...)` | Combat-init entries |

The `partsExists` / `partsName` field-pair models per-mob **body
parts** — relevant for limb-targeting (per
`reference_ffxiv_1x_battle_commands.md` mentions of part-targeted
weaponskills).

## CharaBaseClass — the shared base (Player + Npc)

Large class split across 7 files:

| File | LOC | Purpose |
|---|---:|---|
| (main) | 1,734 | Lifecycle, group/inventory hooks, RTTI |
| `_battle` | 2,027 | Combat parameters + skill resolution |
| `_parameter` | 1,867 | Stat tables / command-slot tables |
| `_event` | 444 | Event-mode entry/exit |
| `_cliprog` | 454 | Client-side prog hooks |
| `_ffxivbattle` | 636 | FFXIV-specific combat (legacy? Pre-1.20?) |
| `_u` | 922 | Tail file |

### Main file — group/inventory/lifecycle (~1,734 LOC)

Top method categories:

**Identity / state predicates:**
- `_isAlive(self)`, `_isMember(self, char)`, `_isInstanceOf(self, type)`,
  `_isInn(self)`, `_isExistInAreaMember(self, char)`,
  `_isActorMainStatMode(self)`

**Stat accessors:**
- `_getActorMainStat(self)`, `_getSubStatStatus(self)`,
  `_onChangeActorMainStat(self, ...)`, `_onChangeSubStatStatus(self, ...)`,
  `_onChangeSubStatMode(self, ...)`

**Group / party:**
- `_getGroup(self)`, `_getAllGroup(self)`,
  `_getExtendedTemporaryGroup(self)`, `_getAllExtendedTemporaryGroup(self)`,
  `_getMember(self, ...)`, `_countMember(self)`, `_isMember(self, char)`,
  `_getMemberDisplayName(self, member)`, `_onUpdateDisplayName(self, ...)`,
  `_onUpdateGroupCurrent(self, ...)`

**Inventory:**
- `_getEquippingItem(self, slot)`, `_getItem(self, idx)`,
  `_getItemPackageCapacity(self)`, `_getItemPackageFreeSpace(self)`,
  `_getCatalogID(self, item)`, `_countStack(self, item_id)`,
  `_onUpdateItemPackage(self, ...)`, `_onUpdateTradingItem(self, ...)`

**State change hooks:**
- `_onChangeJob(self, new_job)`, `_onChangeAccessibleInServer(self, ...)`,
  `_onChangeNetStatSystem(self, ...)`, `_onChangeNetStatUser(self, ...)`,
  `_onChangeSystemFlag(self, ...)`, `_onReceiveDataPacket(self, ...)`

**Lifecycle:**
- `_onInit(self, ...)`, `_onUpdateWork(self, ...)`, `_bindWork(self, ...)`

### `_battle` file (~2,027 LOC)

Combat-specific accessors. Most relevant subset:

- **Battle work tables:** `battleParameter`, `battleSave`, `battleTemp`,
  `battleStateForSelf`, `charaWork`, `generalParameter`
- **Cast speed:** `getCastSpeed(self)`, `getCastSpeedAtEquip(self)`,
  `castGauge_speed`, `castGauge_speedAtEquip`
- **Lock-on:** `adjustLockOnTargetDirection(self, ...)`
- **Auto-guard:** `canAutoGuardWithAxe(self)`
- **Combination:** `canStartCombination(self, ...)` (Battle Regimen!)
- **Skill ID:** `convertSkillId(self, ...)`
- **Equipment query:** `getEquipmentEquipPointDetail(self, ...)`,
  `getEquipPointByAttackIndex(self, ...)`, `getEquipPointByHand(self, ...)`,
  `getEquipPointByParts(self, ...)`, `getHandByAttackIndex(self, ...)`
- **Attack work:** `getAttackUseAmmo(self, ...)`,
  `getAttackWorkIndexBy{EquipPoint,Hand,MyCommandIndex,Parts}(self, ...)`
- **Craft work:** `getCraftWorkIndexBy{EquipPoint,Hand}(self, ...)`
- **Negotiation:** `enableNegotiation(self, ...)` (NPC dialog/aetheryte trade)
- **Battalion:** `getBattalion(self)` (Grand Company battalion membership)
- **Misc:** `calcOverLevelAdjust(self, ...)`, `calcPotencial(self, ...)`,
  `getEnableTimingCommands(self)`

### `_parameter` file (~1,867 LOC)

The HUGE stat-table API. Selected method-name subset:

- **Action gauge:** `getActionGaugeMax(self)`
- **Bonus points:** `getBonusPointAtPhysicalParameter(self)`,
  `getBonusPointStockForElement(self)`, `getBonusPointStockForPhysical(self)`
- **Command slots:** `getCommandId(self, slot)`, `getCommandSlotCompatibility(self, slot)`,
  `commandSlot_compatibility`, `commandSlot_recastTime`,
  `getCommandRecastTime(self, slot)`, `getMaxCommandRecastTime(self)`,
  `getCustomCommand(self, slot)`, `getMyCombinationStackedNum(self)`
- **Constance commands:** `constanceCommandSlot_commandId`,
  `constanceCostPoint_max`, `constanceCostPoint_used`
- **Ability:** `abilityCostPoint_max`, `abilityCostPoint_used`
- **Force control (server-side overrides):**
  `forceControl_float_forClientSelf`, `forceControl_int16_forClientSelf`,
  `getForceCostMPForCaster(self, ...)`, `getForceCostTPForCaster(self, ...)`
- **HP/MP:** `getHpImpl(self)`, `getHPMax(self)`, `getHpMaxImpl(self)`,
  `getMPMax(self)`, `getMPMaxAtEquip(self)`
- **Sub-status:** `_getSubStatBreakage(self, ...)`
- **Aetheryte:** `getElapsedTimeAtCureMPFromAetheryte(self)`
- **Detail:** `commandDetailForSelf`, `commandResultTimeSave`
- **Equip:** `commandEquip`, `getEquipmentEquipPoint(self, ...)`
- **Misc:** `getContentGroup(self)`, `getCustomCommand(self, ...)`,
  `getExpBPCostSheetData(self, ...)`, `getItemData(self, ...)`,
  `getLooksPartyTarget(self)`, `getMainSkillLevel(self)`,
  `canSetOpenThinking(self)`, `canSetPartyTarget(self)`

## Implications for garlemald

### Mob spawning is simpler than the family count suggests

The 30+ mob-family base classes (`EmpireBaseClass`, `BirdmanBaseClass`,
`LizardmanBaseClass`, etc.) are all **empty markers**. Their behaviour
comes from `NpcBaseClass` + `NpcBaseClass_battle` + `NpcBaseClass_event`.

So when garlemald spawns a mob:
1. The client loads the mob's specific `.lpb` (which inherits from
   `<Family>BaseClass` → `MonsterBaseClass` → `NpcBaseClass`).
2. The empty intermediate bases let the engine `_isInstanceOf` query
   "is this an Empire mob?" / "is this a Birdman?" — used by AI / loot
   tables / etc.
3. All real behaviour (talk / aggro / parts / animations) comes from
   NpcBaseClass + its split files.

**Garlemald's monster spawn doesn't need per-family scripts** — the
hierarchy is purely typed-marker. Garlemald just needs to ensure the
mob's served class hierarchy lists the right family base for
`_isInstanceOf` queries.

### Server-side RPC API — talk / push / emote already mirrored

NpcBaseClass exposes `_callServerOn{Push,Talk,Emote}` (client requests
to server) paired with `_doServerOn{Push,Talk,Emote}` (server callback)
+ `_on{X}Event` / `_on{X}Request` / `_on{X}Rejected` (notification
hooks). Garlemald's existing `onTalk`/`onPush`/`onEmote` server-side
dispatch maps directly.

**Notable:** `_onPushRejected` doesn't exist — push is always accepted
or silently dropped. Garlemald's push-rejection logic should silently
no-op rather than send a rejection packet.

### QuestBaseClass cutscene API names align with garlemald

Garlemald's man0g0.lua (`startFadeOutCutSceneDefault` etc.) calls
methods that correspond to QuestBaseClass_common's `_fadeOut` /
`_fadeIn` / `_waitForFading` / `_waitForMapLoaded` /
`createCutScene` family. The "Default" suffix on garlemald's calls
(`startFadeOutCutSceneDefault`) suggests a shorthand wrapper that
isn't directly visible in the decomp — likely a worldMaster-level
wrapper that calls the underlying _fade* primitives with default
args. This is consistent with garlemald's existing flow.

### CharaBaseClass `_onChange*` hooks

The `_onChange{Job,NetStatSystem,NetStatUser,SystemFlag,SubStatMode,
SubStatStatus,ActorMainStat,AccessibleInServer}` hook family means
the client expects the server to TELL it when these fields change
(via SetActorProperty packets), and the client's `_onChange*` Lua
callback fires. Garlemald already drives most of these via its
property-update broadcasts.

### `_loadKeySemipermanently` / `_unloadKey` lifecycle

Quest text loading uses a "semipermanent" model — keys persist for
the quest's lifetime. Quest scripts call `_loadKeySemipermanently(391)`
(at quest-init) and `_unloadKey(391)` (at quest-finalize). Garlemald
doesn't need to track this — it's client-side resource management —
but the call shape matches what garlemald's quest-spawn packets
implicitly invoke.

## What's NOT decomped here (and why)

- **PlayerBaseClass.lpb (66 KB main)** — the player-side equivalent of
  CharaBaseClass + NpcBaseClass. Roughly 2× the size of CharaBaseClass.
  Worth a separate focused pass for garlemald's player-state work,
  but most player-side state is already mirrored by garlemald's
  per-property broadcasts.
- **WidgetBaseClass.lpb** — 137 widget subclasses. Relevant for UI
  decisions but garlemald's UI work is mostly Sqwt-rendered (per
  Phase 5 item #5 negative result for buff-icon strips).
- **Per-mob-family `_battle` overrides** — if any mob family
  customizes combat behaviour beyond the empty marker base, it's not
  visible from class-registration alone. Need per-mob inspection
  (e.g. an actual Empire-Conjurer mob script).

## Cross-references

- `docs/lpb_corpus_survey.md` — corpus-wide patterns + the pipeline
  (`make lpb-corpus`)
- `docs/lpb_format.md` — wrapper format + filename cipher
- `docs/director_quest_decomp.md` — companion doc for the
  Director side (DirectorBaseClass / QuestDirectorBaseClass /
  SimpleQuestBattleBaseClass)
- `docs/director_quest.md` — Phase 6 architectural overview
- `docs/lua_class_registry.md` — Phase 6 item #3 (root class
  registry; this doc grounds the corpus survey's intermediate
  bases in concrete decomp)
- garlemald-server's `scripts/lua/quests/`, `scripts/lua/mobs/`,
  `scripts/lua/npcs/` — server-side scripts that drive the
  client-side hierarchy decomped here
- `project_garlemald_server_monster_spawn.md` (memory) —
  garlemald's mob-spawn work (no per-family scripts needed per
  this doc's finding)
- `project_garlemald_proximity_push_kick.md` (memory) — the
  server-side push dispatch that pairs with NpcBaseClass's
  `_callServerOnPush` / `_doServerOnPush` / `_onPushEvent`
