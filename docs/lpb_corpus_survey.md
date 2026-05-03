# Bulk-decompiled `.lpb` corpus survey

> Last updated: 2026-05-03 — all 2671 shipped `.le.lpb` files decoded
> via `tools/decode_lpb.py`, then `unluac`-decompiled to readable Lua.
> Total corpus: **647,660 lines of Lua across 2672 files** (the +1 is
> from a sample run). This page surfaces the cross-script patterns
> garlemald should validate against.

## Class hierarchy (top base classes by subclass count)

From scanning all `_defineClass(name, parent)` calls across the
corpus (2,436 instantiations recovered):

| Parent base class | # subclasses | `require()` path |
|---|---:|---|
| `ScenarioBaseClass` | **619** | `/Quest/Scenario/ScenarioBaseClass` |
| `NpcBaseClass` | 171 | `/Chara/Npc/NpcBaseClass` |
| `StatusBaseClass` | 157 | `/Status/StatusBaseClass` |
| `WidgetBaseClass` | 137 | `/Widget/WidgetBaseClass` |
| `QuestDirectorBaseClass` | **115** | `/Director/Quest/QuestDirectorBaseClass` |
| `MonsterBaseClass` | 89 | `/Chara/Npc/Monster/MonsterBaseClass` |
| `FighterBaseClass` | 75 | `/Chara/Npc/Monster/Fighter/FighterBaseClass` |
| `SimpleQuestBattleBaseClass` | **60** | `/Director/Quest/SimpleQuestBattle/SimpleQuestBattleBaseClass` |
| `EmpireBaseClass` | 59 | `/Chara/Npc/Monster/Empire/EmpireBaseClass` |
| `SystemCommandBaseClass` | 55 | `/Command/System/SystemCommandBaseClass` |
| `AskBaseClass` | 54 | `/Widget/Ask/AskBaseClass` |
| `ZoneBaseClass` | 41 | `/Area/Zone/ZoneBaseClass` |
| `DirectorBaseClass` | 28 | `/Director/DirectorBaseClass` |
| `GuildleveBaseClass` | 32 | `/Director/Guildleve/GuildleveBaseClass` |
| `GameCommandBaseClass` | 28 | `/Command/Game/GameCommandBaseClass` |
| `MagicBaseClass` | 26 | `/Command/Game/Magic/MagicBaseClass` |
| `BattleCommandBaseClass` | 16 | `/Command/Game/BattleCommandBaseClass` |
| `NormalItemBaseClass` | 16 | `/Item/Normal/NormalItemBaseClass` |
| `GimmickNpcBaseClass` | 15 | `/Chara/Npc/Gimmick/GimmickNpcBaseClass` |
| `AbilityBaseClass` | 14 | `/Command/Game/Ability/AbilityBaseClass` |
| `JudgeBaseClass` | 12 | `/Judge/JudgeBaseClass` |
| `InstanceRaidBaseClass` | 12 | `/Director/InstanceRaid/InstanceRaidBaseClass` |

**Mob-family bases** (under `/Chara/Npc/Monster/<Family>/...`):
Birdman (30), Lizardman (24), Rebel (20), Elemental (18), Gnole (16),
Sprite (13), Termite (13), BirdmanConjurer (11), and ~30 more
families with smaller counts.

This means the engine recognizes ~30+ distinct **mob taxonomies**,
each with its own AI behaviour scripts. Garlemald's monster-spawn
work (per `project_garlemald_server_monster_spawn.md`) targets
behaviours that derive from these mob-family bases.

### Refinement to Phase 6 item #3 (`docs/lua_class_registry.md`)

The Phase 6 class-registry doc listed the 24 *root* `*BaseClass`
entries registered via `FUN_0078e3a0`. The corpus survey reveals
**a deeper hierarchy** — `Scenario`, `QuestDirector`,
`SimpleQuestBattle`, `Monster`, `Fighter`, `Empire`,
`<MobFamily>BaseClass`, etc. are intermediate bases above the
per-instance scripts. The registration function only registers the
top-level set; intermediate bases like `ScenarioBaseClass` are loaded
via `require("/Quest/Scenario/ScenarioBaseClass")` from per-quest
scripts.

## Top method-name vocabulary

Across all 2672 files, the most-recurring method-name patterns
(method definitions, not calls):

| Method | Count | Purpose (inferred) |
|---|---:|---|
| `initText` | **591** | Load text data permanently — every quest/widget |
| `init` | 214 | Generic init entry |
| `_temp` | 192 | Work-table temp field declaration |
| `initForEvent` | 150 | Per-event init |
| `processEvent010` | 113 | Sequence 10 event handler |
| `processUICommandOperate` | 105 | UI button "OK" handler |
| `processEvent020` | 88 | Sequence 20 event handler |
| `processUICommandCancel` | 75 | UI button "Cancel" handler |
| `processEvent010_<N>` | 65 | Sub-step within sequence 10 |
| `processEvent030` | 64 | Sequence 30 event handler |
| `processUICommandDefault` | 63 | UI default-action handler |
| `isGoodStatus` | 60 | Status-effect predicate |
| `canFire` | 55 | Action availability check |
| `initAsk` | 55 | Dialog-choice setup |
| `processEvent000` | 55 | Sequence 0 (intro) event handler |
| `_onInit` | 45 | Engine-internal lifecycle hook |
| `processUICommandClose` | 45 | UI close handler |
| `processEventChuui` | 42 | "Chuui" = JP "warning" — quest warning popup |
| `processEventStart` | 41 | Quest-start event |
| `processEventClear` | 37 | Quest-clear event |
| `getAskResult` | 32 | Read player's dialog choice |
| `processOfferBeforeTalk` | 32 | Pre-talk hook |
| `processOfferAfterTalk` | 32 | Post-talk hook |

The numeric suffix pattern (`processEvent000`, `010`, `020`, ...)
matches the **sequence-number scheme** in garlemald's quest scripts
(`SEQ_000`, `SEQ_005`, `SEQ_010`, etc.). Each `processEvent<NNN>_<M>`
corresponds to step M within sequence NNN. Garlemald's
`callClientFunction(player, "delegateEvent", player, quest,
"processEvent000_3")` pattern dispatches to these methods.

Other frequent patterns:
- `processTtr*` (Tutorial) — used heavily in beginner quests like
  Man0g0 (`processTtrNomal001withHQ`, `processTtrBtl001`, etc.)
- `processUICommand*` — UI widget event handlers
- `process*BeforeShow` / `process*AfterShow` — UI lifecycle

## Engine-API globals (most-accessed)

Across all scripts, the most-accessed engine globals:

| Global | Hits | Role |
|---|---:|---|
| `desktopWidget` | **3,154** | Desktop UI manager (most-used global) |
| `worldMaster` | **3,100** | World-script entry point (party messages, asks, system events) |
| `_defineClass` | 2,436 | Class system — register a concrete class |
| `_defineBaseClass` | 139 | Class system — register an intermediate base |
| `_isInstanceOf` | 205 | RTTI check |
| `tostring` / `tonumber` / `select` / `type` / `unpack` | (Lua stdlib) | Standard Lua 5.1 builtins, all available |
| `string` / `_string` / `_math` / `_table` / `debug` / `_debug` | (stdlib) | Lua stdlib + `_`-prefixed engine variants |
| `require` | 2,606 | Module loader (per-class requires) |

The `_defineBaseClass` count (139) corresponds to the count of
**intermediate base classes** (above the registered roots from
Phase 6 item #3), e.g. `ScenarioBaseClass`, `QuestDirectorBaseClass`,
`SimpleQuestBattleBaseClass`, etc.

**Class-name globals** that scripts reference directly (most-hit):
`CharaBaseClass` (321), `WidgetBaseClass` (230), `ItemBaseClass`
(221), `PlayerBaseClass` (191), `GameCommandBaseClass` (121),
`NpcBaseClass` (82). Plus regional globals: `DftWil` (Default
Wilderness = 292), `DftSea` (Default Sea = 258), `DftFst` (Default
Forest = 221) — the three Grand Company territory namespaces.

**Sequel quest globals**: `Man0u1` (114), `Man0l1` (101), `Man0g1`
(94) — confirming the man*0g0/Man0g1, Man0l0/Man0l1, Man0u0/Man0u1
pairing structure (each opening quest has a "next" quest).

**Most-accessed widgets** (referenced by name): `RetainerItemListWidget`
(149), `EquipWidget` (146), `ItemListWidget` (89), `TutorialWidget`
(86), `StatusWidget` (86), `CraftEditWidget` (72).

## Garlemald-relevant findings

### `SimpleQuestBattleBaseClass` for SEQ_005 work

Garlemald's recent SEQ_005 (man0g0 combat tutorial) work
(per memory `project_garlemald_seq005_b6.md`,
`project_garlemald_seq005_b7.md`) targets exactly this base class.
Located at `build/lua/61s57qvs/tp5rq/r1xuy5tp5rq89qqy5/r1xuy5tp5rq89qqy589r57y9rr.lua`
(= `/Director/Quest/SimpleQuestBattle/SimpleQuestBattleBaseClass`).

First-line decompilation reveals:
- Inherits from `QuestDirectorBaseClass`
- Method `eventContentGiveUp(self, A1, target)` calls
  `worldMaster:ask(self, worldMaster, 25230, 2, target)` — the
  "give up combat tutorial?" dialog
- Method `getOwnClientQuestIdAsSimple()` returns the per-instance
  client quest id

60 subclasses of SimpleQuestBattleBaseClass exist (e.g.
`QuestDirectorGcu30101`, `QuestDirectorGcl30101` etc.) — these are
the per-quest combat-tutorial directors. Garlemald's
`man0g0_combat_tutorial` plumbing should mirror this hierarchy.

### `ScenarioBaseClass` is the canonical Quest base

Phase 6 item #3 listed `QuestBaseClass` as a registered root, but
the corpus shows **619 quest scripts inherit from `ScenarioBaseClass`**
(the intermediate that lives above `QuestBase` in the chain). The
`ScenarioBaseClass` is what every main-scenario quest extends —
including Man0g0 — NOT `QuestBaseClass` directly.

So garlemald's quest scripts should use the `Scenario` shape,
not the bare `Quest` shape. The path is
`/Quest/Scenario/ScenarioBaseClass.prog` (also 619 hits in the
require survey).

### `_defineBaseClass` — intermediate bases

Phase 6 item #3 documented `_defineClass` (concrete class registration)
but missed `_defineBaseClass` (intermediate base registration). The
139 occurrences of `_defineBaseClass` define abstract intermediates
like `ScenarioBaseClass`, `MonsterBaseClass`, `FighterBaseClass`,
`EmpireBaseClass`, `SimpleQuestBattleBaseClass`, etc. — none of which
are registered via `FUN_0078e3a0`.

If garlemald's Lua VM doesn't expose `_defineBaseClass`, scripts that
try to load these intermediate bases will fail. Garlemald's class
system needs both functions.

### Mob-family AI hierarchy

Each of ~30 mob families (`Empire`, `Birdman`, `Lizardman`,
`Elemental`, `Gnole`, `Sprite`, `Termite`, `Rebel`, etc.) has its own
base class with family-specific AI behaviour. When garlemald's
`map-server` spawns a monster of a given family, it should serve the
matching family's `BaseClass.lpb` script for the client to bind. The
30+ family files are at:

```
/Chara/Npc/Monster/<Family>/<Family>BaseClass.lpb
/Chara/Npc/Monster/<Family>/<Family>ConjurerBaseClass.lpb  (mage variant)
... (per-family specializations)
```

## How to query the corpus

```bash
# One-command corpus pipeline (decode + decompile, ~40s total).
# Set FFXIV_INSTALL=<path-to-FINAL_FANTASY_XIV> if not at the
# default ../ffxiv-install-environment/.../FINAL_FANTASY_XIV.
# UNLUAC_JAR defaults to /tmp/unluac/unluac.jar — download from
# https://sourceforge.net/projects/unluac/files/latest/download
make lpb-corpus

# Or run the two passes separately:
make decode-lpb           # → build/lpb/*.luac (~0.5s, decodes the
                          #   rle\x0c XOR-0x73 wrapper + filename cipher)
make decompile-lpb        # → build/lua/*.lua (~40s with 8 workers)

# Single-script lookup by source name:
python3 tools/decode_lpb.py '$FFXIV_INSTALL' Man0g0
java -jar /tmp/unluac/unluac.jar build/lpb/Man0g0.luac

# Then grep across the corpus for any pattern:
grep -r "processEvent020_3" build/lua --include="*.lua" -l    # quests with this method
grep -r "_defineBaseClass" build/lua --include="*.lua"        # intermediate bases
grep -r "/Director/Quest/Simple" build/lua --include="*.lua"  # SEQ_005-relevant
```

## What this corpus is good for

For each thing garlemald sends to the client, there's a script in
this corpus that tells you what the client expects:

- **Quest**: per-quest `Man*g*.lpb`, `Man*l*.lpb`, `Man*u*.lpb`,
  `Bsm*.lpb` (Black Shroud Side-quests), `Cls*.lpb` (Class quests),
  `Job*.lpb` (Job quests). Scan for `processEvent<NNN>_<M>` to see
  the per-sequence flow.
- **NPC**: per-NPC `<NpcName>.lpb` under `/Chara/Npc/`. Look at
  `processOfferBeforeTalk` / `processOfferAfterTalk` for the
  per-NPC dialog flow.
- **Director**: per-director `*Director*.lpb` under `/Director/`.
  The `OpeningDirector.lpb` decompilation will confirm garlemald's
  cinematic dispatch shape.
- **Status**: per-status `<StatusName>.lpb` under `/Status/`. Look at
  `isGoodStatus` / `isBadStatus` predicates.
- **Mob AI**: `/Chara/Npc/Monster/<Family>/<Family>*.lpb`.

This is the largest single source of ground-truth for
garlemald's client-facing API expectations. Whenever garlemald
sends a packet whose effect on the client isn't obvious, find the
relevant `.lpb`, decode + decompile it, and read what the client
actually does with the bytes.

## Cross-references

- `docs/lpb_format.md` — wrapper format + filename cipher (the
  prerequisite for using this corpus)
- `docs/lua_class_registry.md` — Phase 6 item #3 (root class
  registry; this corpus reveals the intermediate bases above)
- `docs/director_quest.md` — Phase 6 architectural overview
- `docs/garlemald_validation.md` — Phase 6 item #9 (this closes
  Gap 2 and partially refines Gap 4: the 5-Director-subtype
  inference now expanded to ~28 + intermediates)
- `tools/decode_lpb.py` — decoder
- garlemald-server's `scripts/lua/quests/`, `scripts/lua/directors/`
  — the server-side quest/director scripts that drive these
  client-side `.lpb` consumers
- `project_garlemald_seq005_b6.md` / `_b7.md` (memory) — garlemald's
  SEQ_005 work, which targets `SimpleQuestBattleBaseClass`
