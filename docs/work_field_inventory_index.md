# Work-table state-field inventory — index

> Last updated: 2026-05-03 — auto-generated full inventory at
> `build/wire/work_field_inventory.md`. Re-run via
> `make extract-work-fields`.

## What this captures

Per `docs/player_base_decomp.md`, every C++-bound method on the
engine's PlayerBaseClass / CharaBaseClass / etc. implicitly
references some work-table field (`playerWork.guildleveId`,
`charaWork.parameterSave`, etc.). This tool walks the
**entire decompiled corpus** (2671 scripts) and produces the
canonical inventory: per work-table, what fields are accessed.

For garlemald, this is the **state-field requirement spec** —
every field listed must be populated correctly via SetActorProperty
or work-sync packets for the client's scripts to behave.

## How it's built

`tools/extract_work_fields.py`:

1. Walks `build/lua/**/*.lua` (the decompiled corpus from
   `make decompile-lpb`).
2. Tracks unluac's two-line access pattern:
   ```
   L1_2 = A0_2.playerWork      -- load work-table into local
   L1_2 = L1_2.guildleveId      -- field access on the local
   ```
3. Captures the field name from each `L? = L?.<field>` line where
   the source local was previously bound to a known work-table.
4. Filters method-pointer loads (verb-prefixed names, calls on
   the next line) into a separate "ambiguous" report so they can
   be audited.

## How to regenerate

```bash
make decode-lpb decompile-lpb extract-work-fields
# OR
make lpb-corpus extract-work-fields
```

## Headline numbers (2026-05-03 snapshot)

- 2671 files scanned
- 11 work tables observed
- **130 distinct fields** identified
- 462 total field-access sites

## Per-work-table field counts

| Work table | Fields | Accesses | Domain |
|---|---:|---:|---|
| `playerWork` | **42** | 128 | Player persistent + transient state |
| `charaWork` | 16 | 111 | Shared chara state (wraps battleSave/Temp/parameterSave/Temp etc.) |
| `guildleveWork` | 17 | 78 | Active guildleve UI state |
| `aetheryteWork` | 14 | 46 | Aetheryte / leve reward UI state |
| `widgetWork` | 6 | 40 | Widget lifecycle state |
| `instanceRaidWork` | 9 | 14 | Instance-raid state |
| `normalItemWork` | 11 | 14 | Per-item normal-item attributes (subQuality, life, fitness, etc.) |
| `npcWork` | 6 | 12 | Npc-specific state (battleCommon, hateType, push command) |
| `directorWork` | 2 | 8 | `contentCommand` + `contentCommandSub` (per `docs/director_quest_decomp.md`) |
| `areaWork` | 4 | 6 | Area state (actorNumber, isInstanceRaid, isEntranceDesion, floor) |
| `askWork` | 3 | 5 | Ask-dialog state (inputControlFlag, askResult) |

## Caveats

This is a **lower bound**. The tool only catches the unluac-specific
two-line `L = X.workTable; L = L.field` pattern. Many additional
field accesses likely use inline forms like:
```
if A0_2.playerWork.someFlag then ...
```
which the current regex doesn't match. So expect the true field
count to be 1.5×-2× the 130 reported here.

The "ambiguous" section in the auto-generated full report
(`build/wire/work_field_inventory.md`) lists method-like names
that match the access pattern; these are mostly tool false-positives
(method-pointer loads followed by calls) but a few engine fields
legitimately have method-like names (e.g., `_temp`, `_sync`, `_tag`
sub-fields per `docs/director_quest_decomp.md`).

## Top fields (cross-table, by access count)

The most-accessed fields across the corpus tell us what state
garlemald should prioritize getting right:

### `charaWork` sub-tables (the major nested groups)

| Sub-table | Accesses |
|---|---:|
| `charaWork.battleTemp` | 25 |
| `charaWork.parameterSave` | 25 |
| `charaWork.parameterTemp` | 15 |
| `charaWork.eventTemp` | 10 |
| `charaWork.battleSave` | 7 |
| `charaWork.command` | 5 |
| `charaWork.commandBorder` | 5 |
| `charaWork.eventSave` | 4 |
| `charaWork.commandCategory` | 3 |
| `charaWork.currentContentGroup` | 3 |
| `charaWork.statusShownTime` | 3 |

These sub-tables (`battleTemp`, `parameterSave`, etc.) are themselves
record-like — each contains many sub-fields read at deeper paths
(e.g., `charaWork.parameterSave.hp`). The current tool stops at the
first sub-field; deeper paths are NOT captured.

**For garlemald:** the SetActorPropertyPacket builders mostly target
these sub-tables. Garlemald's wire-property paths like
`charaWork/parameterSave/hp` set values that the client's scripts
read via `self.charaWork.parameterSave.hp` (a 3-level access).

### `playerWork` top fields (variable-command system dominates)

| Field | Accesses |
|---|---:|
| `variableCommandPlaceDriven` | 18 |
| `guildleveId` | 16 |
| `variableCommandPlaceDrivenPriority` | 10 |
| `variableCommandPlaceDrivenSub` | 9 |
| `variableCommandPlaceDrivenTarget` | 9 |
| `npcLinkshellChatCalling` | 6 |

The "variable command" system is the per-context command bar
override (different command set in inn / content instance / combat
tutorial / etc.). Garlemald should populate these to drive the
correct UI command set per context.

## Implications for garlemald

### `playerWork` field-population priority list

The 42 `playerWork` fields directly map to garlemald's
SetActorProperty packet targets. Garlemald's `playerWork` writers
should cover:

**Demographics** (5 fields — sent at character creation / login):
- `tribe`, `guardian`, `birthdayMonth`, `birthdayDay`, `initialTown`

**Quest tables** (4 fields — sent at login + on quest changes):
- `questScenario`, `questScenarioComplete`,
  `questGuildleve`, `questGuildleveComplete`

**Guildleve active state** (3 fields):
- `guildleveId`, `guildleveDone`, `guildleveChecked`

**NPC linkshell** (3 fields):
- `npcLinkshellChatCalling`, `npcLinkshellChatExtra`,
  + 1 more (linkshell length is a method, not a field)

**Variable command system** (16+ fields, 5 categories):
- Place-driven: `variableCommandPlaceDriven*` (Priority, Sub, Target)
- Confirm-raise: `variableCommandConfirmRaise*` (Sender, SenderByID,
  SenderSex)
- Confirm-warp: `variableCommandConfirmWarp*` (Sender, SenderByID,
  SenderSex, Place)
- Content: `variableCommandContent`, `variableCommandContentSub`
- Emote-sit: `variableCommandEmoteSit`

**Combat / cast state** (5):
- `restBonusExpRate`, `comboNextCommandId`, `comboCostBonusRate`,
  `requestBurstBlocker`, `commandBurstBlocker`,
  `widgetCommandBurstBlocker`

**Cast state** (2):
- `castCommandClient`, `castEndClient`

**Misc** (5):
- `isContentsCommand`, `isRemainBonusPoint`, `weatherNow`,
  `event_achieve_aetheryte`

### `directorWork` is small — `contentCommand` + `contentCommandSub`

Per `docs/director_quest_decomp.md`, the DirectorBase work-table
has two main fields used by scripts: `contentCommand` and
`contentCommandSub`. These drive the active content-command set
during a director's lifetime. Garlemald already manages these via
its director state-sync flow.

### `npcWork.hateType` is a single field garlemald should populate

`npcWork.hateType` controls the nameplate colour for hostile NPCs
(hostile / neutral / friendly). Garlemald's hate-broadcast packets
(per `project_garlemald_set_occupancy_group.md`) feed into this
field on the client.

### `aetheryteWork` populates the leve-reward UI

The 14 aetheryteWork fields (`glRewardItem`, `glRewardSubItem`,
`difficulty`, `factionNumber`, `iconGil`, `clearTime`, etc.) drive
the per-leve reward summary UI. Garlemald should populate these
when a leve completes.

## Cross-references

- `build/wire/work_field_inventory.md` — auto-generated full report
- `build/wire/work_field_inventory.json` — machine-readable
- `tools/extract_work_fields.py` — the tool
- `docs/cpp_bindings_index.md` — companion: client-engine API surface
  (the C++ methods that READ these fields)
- `docs/player_base_decomp.md` — the source of the "every C++ method
  references a `playerWork.<field>`" insight
- `docs/scenario_monster_decomp.md` — Chara/Npc base hierarchy
- `docs/director_quest_decomp.md` — DirectorBase + QuestDirectorBase
  (where `directorWork` + `questDirectorWork` live)
- `docs/sync_writer.md` — Phase 6 item #4 (the wire mechanism that
  feeds these fields via SyncWriter typed serializers)
- garlemald-server's per-property packet builders that populate
  these fields
