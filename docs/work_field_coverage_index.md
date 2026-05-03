# Work-field coverage report — index

> Last updated: 2026-05-03 — auto-generated full report at
> `build/wire/work_field_coverage.md`. Re-run via
> `make work-field-coverage`.

## What this completes

Closes the meteor-decomp **spec triangle into a 4-way cross-reference**:

```
client engine API         →  cpp_bindings.json           (419 methods)
        ↓                       ↓
state-fields the API reads →  work_field_inventory.json   (130 fields)
        ↓                       ↓
garlemald property writers →  work_field_coverage.json    (THIS)
        ↓                       ↓
garlemald scripts          →  garlemald_lua_coverage.json (87 gaps)
```

This tool reports — for each of the 130 client-side state fields
the inventory captured — whether garlemald has any
`SetActorProperty` writer that targets it.

## How it's built

`tools/work_field_coverage.py`:

1. Walks garlemald's Rust source (`common/src/`, `map-server/src/`,
   `world-server/src/`, `lobby-server/src/`) for property-path
   string literals in two styles:
   - **Dot-separated**: `"playerWork.tribe"`, `"charaWork.parameterSave.hp[0]"`,
     `"playerWork.questScenario[i]"`
   - **Slash-separated** (murmur2-hashed wire IDs per
     `docs/murmur2.md`): `"playerWork/journal"`, `"charaWork/exp"`
2. Loads the `work_field_inventory.json` produced by
   `extract_work_fields.py`.
3. Cross-references: a garlemald path like
   `charaWork.parameterSave.hp[0]` SATISFIES the inventory's
   `charaWork.parameterSave` requirement (the parent-prefix
   match).
4. Reports per-table covered / uncovered counts + the prioritized
   uncovered list.

## How to regenerate

```bash
make work-field-coverage
# (auto-runs after lpb-corpus pipeline produces the inventory)
```

## Headline numbers (2026-05-03 snapshot)

- **130 inventory fields** total (from `work_field_inventory.md`)
- **17 covered** by a garlemald writer (**13.1%**)
- **113 uncovered** (**86.9%**) — actionable garlemald work
- 46 garlemald property paths total (across both dot + slash styles)
- 7 orphan garlemald paths (write but no matching inventory entry —
  deeper sub-paths or inline-access patterns the inventory missed)

## Per-table coverage

| Work table | Inventory fields | Covered | Uncovered |
|---|---:|---:|---:|
| `charaWork` | 16 | 9 | **7** (44% gap) |
| `npcWork` | 6 | 1 | **5** (83% gap) |
| `playerWork` | 42 | 7 | **35** (83% gap) |
| `guildleveWork` | 17 | 0 | **17** (100% gap) |
| `aetheryteWork` | 14 | 0 | **14** (100% gap) |
| `normalItemWork` | 11 | 0 | **11** (100% gap) |
| `instanceRaidWork` | 9 | 0 | **9** (100% gap) |
| `widgetWork` | 6 | 0 | **6** (100% gap) |
| `areaWork` | 4 | 0 | **4** (100% gap) |
| `askWork` | 3 | 0 | **3** (100% gap) |
| `directorWork` | 2 | 0 | **2** (100% gap) |

`charaWork` is the strongest area (56% coverage) because garlemald
has many `charaWork.parameterSave.X` and `charaWork.battleTemp.X`
writers for combat-state broadcasts. The other tables are largely
unwired.

## Top-priority gap list (highest access counts first)

### `playerWork` — 35 uncovered, top by access count

| Field | Access count | Why it matters |
|---|---:|---|
| `variableCommandPlaceDriven` | 18 | Per-context command-bar override (most-touched UI state) |
| `guildleveId` | 16 | Active guildleve tracking |
| `variableCommandPlaceDrivenPriority` | 10 | Command-bar variation priority |
| `variableCommandPlaceDrivenSub` | 9 | Sub-variation |
| `variableCommandPlaceDrivenTarget` | 9 | Variation target |
| `npcLinkshellChatCalling` | 6 | "NPC LS chat calling" indicator |
| `questScenarioComplete` | 4 | Scenario quest completion bitfield |
| `questGuildleveComplete` | 4 | Guildleve completion bitfield |
| `guildleveDone` | 3 | Per-active-leve done flag |
| `guildleveChecked` | 3 | Per-active-leve checked flag |

The variableCommand* family (16+ fields) is one big gap to close —
it controls the per-context command bar (which commands are
displayed in inn / content instance / combat tutorial / etc.).

### `guildleveWork` — 17 uncovered (100% gap)

UI state for the active guildleve. Top fields: `uiState`, `markerX`,
`markerY`, `markerZ`, `signal`, `aimNum`. **All UI-driving fields
needed to display an active leve.**

### `aetheryteWork` — 14 uncovered (100% gap)

Reward UI state shown when a leve completes: `glRewardItem`,
`glRewardSubItem`, `difficulty`, `factionNumber`, `iconGil`,
`clearTime`, etc. Defer until leve-reward UI matters.

### `npcWork` — 5 uncovered

Most-relevant: `pushCommand`, `pushCommandSub`, `pushCommandPriority`
(the push-event command set), and `actorClassId` (NPC's actor class).

### `directorWork` — 2 uncovered

Both `contentCommand` and `contentCommandSub` are documented as
the director's content-command-variation tuple in
`docs/director_quest_decomp.md`. Garlemald MAY already manage these
via its director state-sync flow at a different layer (not via
SetActorProperty), so the "uncovered" here might be a false-positive
gap. Worth verifying.

## Orphan garlemald paths

7 garlemald property paths target fields the inventory tool didn't
capture. Most likely cause: the inventory only captures
**top-level** field access; garlemald's deeper sub-paths
(`charaWork.parameterSave.hp[0]` writes a child of `parameterSave`,
which IS captured at the parent level — so this isn't actually
"orphan"). The 7 cases here are paths that don't even have a
parent-level match.

## Caveats

1. **String-literal scan only** — paths constructed at runtime via
   `format!("playerWork.{}", suffix)` won't be detected. Likely
   present in garlemald but undercounted.
2. **Inventory is a lower bound** — per `extract_work_fields.py`
   docstring, inline access patterns aren't captured. So the true
   gap count is likely higher than 113.
3. **Coverage = "writer exists somewhere"** — doesn't mean the
   writer fires at the right time, with the right value, or in
   the right packet. Surfaces structural absence; correctness is
   a deeper audit.
4. **Slash-style paths get murmur2-hashed** — garlemald's runtime
   sees them as 32-bit IDs. The inventory tool can match them by
   prefix but the runtime correctness depends on the hash matching
   what the client expects (validated in `docs/murmur2.md`).

## What to do with this

1. **Pick a table with high gap count + high access counts** —
   `playerWork` (35 gaps, 128 accesses) or `guildleveWork` (17
   gaps, 78 accesses) are top targets.
2. **For each uncovered field**, add a garlemald `SetActorProperty`
   writer that emits the dot or slash-form path with a real value.
3. **Re-run** `make work-field-coverage` to see coverage improve.
4. **Eventually**, target 100% coverage on the most-accessed tables
   so the client has every state field it expects to read populated.

## Cross-references

- `build/wire/work_field_coverage.md` — auto-generated full report
- `build/wire/work_field_coverage.json` — machine-readable
- `tools/work_field_coverage.py` — the tool
- `docs/work_field_inventory_index.md` — inventory companion (the
  fields this report cross-references against)
- `docs/cpp_bindings_index.md` — engine API surface
- `docs/garlemald_lua_coverage_index.md` — garlemald script
  coverage
- `docs/sync_writer.md` — Phase 6 item #4 (the wire mechanism that
  delivers SetActorProperty writes to the client)
- `docs/murmur2.md` — Phase 3 (the slash-path hash function)
- garlemald-server's per-property packet builders (the source of
  the 46 paths analyzed here)
