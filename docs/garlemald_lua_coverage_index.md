# Garlemald Lua-binding coverage report — index

> Last updated: 2026-05-03 — auto-generated coverage report at
> `build/wire/garlemald_lua_coverage.md`. Re-run via
> `make garlemald-lua-coverage`.

## What this captures

Cross-references methods bound in garlemald's
`map-server/src/lua/userdata.rs` against methods CALLED by garlemald's
own server-side Lua scripts under `scripts/lua/`. Each missing
binding is a probable `attempt to call a nil value (method 'X')` at
runtime when the relevant script path executes.

This is the **actionable companion** to `docs/cpp_bindings_index.md`.
That doc tells us what the CLIENT engine exposes (the 429-method
inventory). This doc tells us what GARLEMALD's own scripts need
its userdata.rs to expose so they don't crash.

## How it's built

`tools/garlemald_lua_coverage.py`:

1. Parses garlemald's `userdata.rs` looking for `impl UserData for
   <T> { ... }` blocks and the per-block `add_method("name", ...)` /
   `add_async_method("name", ...)` declarations.
2. Walks garlemald's `scripts/lua/` for all `<var>:<method>(` calls.
3. Uses a hand-curated CONVENTIONS table to map common variable
   names to UserData types (`player → LuaPlayer`, `quest →
   LuaQuestHandle`, etc.).
4. Categorizes every observed call into:
   - **OK** — bound on the variable's apparent type
   - **TRUE gap** — not bound anywhere in `userdata.rs` (runtime error)
   - **Cross-type** — bound on a different UserData type than expected
     (works via composition / metamethod forwarding)
5. Reports per-class summaries + the prioritized gap list.

## How to regenerate

```bash
make garlemald-lua-coverage
# → build/wire/garlemald_lua_coverage.{json,md}
```

## Headline numbers (2026-05-03 snapshot)

- 19 UserData types observed
- 251 bindings (`add_method` calls)
- 200 distinct method calls in scripts
- **87 TRUE gaps** — called but bound nowhere (high-priority fixes)
- 4 cross-type calls (bound elsewhere, reaches via composition)
- 142 dead bindings (bound but never called — cleanup candidates)
- 37 unmapped variable names

## Top gaps by class

| Class | True gaps | Notes |
|---|---:|---|
| `LuaPlayer` | **40** | Core player operations — `AcceptQuest`, `DoEmote`, `EquipAbility`, `PlayAnimation`, `SendPacket`, `SetCurrentJob`, `SetHP`, `SetTP`, etc. |
| `LuaWorldManager` | **27** | 87% gap rate (27 of 31 distinct calls). The least-bound type relative to use. |
| `LuaQuestHandle` | **15** | Includes obvious typos like `setENpc` (lowercase, while `SetENpc` IS bound) |
| `LuaActor` | 2 | `PlayMapObjAnimation`, `SendAppearance` |
| `LuaDirectorHandle` | 2 | `GetPlayerMembers`, `StartContentGroup` |
| `LuaZone` | 1 | One missing method |

## Most actionable findings

### LuaWorldManager has an 87% gap rate

Only 4 of 31 distinct `GetWorldManager():method()` calls are bound:

```
GetWorldManager():DoZoneChangeContent(...)    ← bound
GetWorldManager():GetActorInWorldByUniqueId(...)  ← bound
GetWorldManager():DoPlayerMoveInZone(...)     ← bound
GetWorldManager():GetZoneByID(...)            ← bound
```

The other 27 distinct calls (e.g. `SpawnNpc`, `SpawnRandomMonster`,
`GetActorByCustomId`, `WarpToBindStone`, etc.) are unbound. These
scripts will fail the moment a path through them runs.

This is the **highest-priority single-type fix** — 27 methods to
add to `impl UserData for LuaWorldManager`.

### LuaPlayer typo / case-mismatch evidence

The gap list includes lowercase variants of bound methods:

| Called (wrong case) | Bound (correct case) | Cause |
|---|---|---|
| `doEmote` | (no `DoEmote` either; both missing) | Both case variants used; neither bound |
| `endEvent` | `EndEvent` ✓ | Lowercase typo — bound version is `EndEvent` |
| `getInventory` | (no binding either) | Genuine gap |
| `getItemPackage` | (no binding either) | Genuine gap |
| `kickEvent` | `KickEvent` ✓ | Lowercase typo |
| `hpstuff` | (none) | Clearly a stub/placeholder that escaped review |

**Recommended garlemald fix:** add Lua-side aliases or normalize the
script-side call sites. The lowercase form is the FFXIV 1.x convention
(per the corpus survey — `getQuestId`, `getQuestData`, etc. are
lowercase), so the bound names in `userdata.rs` may need to be
lowercased for parity. `hpstuff` should be cleaned up.

### LuaQuestHandle: 15 missing including N→S casing

`setENpc` is called but `SetENpc` is the bound name — a Pascal-case
↔ camelCase mismatch. The other 14 `LuaQuestHandle` gaps are mixed
between true missing methods (`GetPhase`, `NewNpcLsMsg`, `craftSuccess`,
`getCurrentCrafted`, `hasMaterials`) and additional case variants.

### LuaPlayer 40 gaps — sample by domain

Quest control:
- `AcceptQuest`, `RemoveGuildleve`, `AddGuildleve`,
  `AddNpcLs`, `SetNpcLS`

Combat:
- `DoBattleAction`, `EquipAbility`, `EquipAbilityInFirstOpenSlot`,
  `UnequipAbility`, `SwapAbilities`, `FindFirstCommandSlotById`,
  `SetCurrentJob`, `DoClassChange`, `PrepareClassChange`,
  `SetHP`, `SetTP`

Trade / inventory:
- `AcceptTrade`

Snpc (Soul-Sync NPCs, FFXIV 1.x mechanic):
- `GetSNpcCoordinate`, `GetSNpcNickname`, `GetSNpcPersonality`,
  `GetSNpcSkin`, `SetSNpc`

Misc:
- `GraphicChange`, `PlayAnimation`, `SendAppearance`,
  `SendPacket`, `SendGameMessageLocalizedDisplayName`,
  `ResetMusic`, `SetHomePointInn`, `SavePlayTime`,
  `SetWorkValue`, `SetProc`, `examinePlayer`,
  `GetActorInInstance`

These are mostly methods that quest scripts assume exist for basic
player-state manipulation. Each one is a potential script-flow
break point.

## What this enables

1. **Prioritized binding work** — garlemald can start with the
   LuaWorldManager (27 missing) and LuaPlayer (40 missing) gaps,
   binding methods in priority order.
2. **Typo / case audit** — surface-able typos like `hpstuff`,
   `endEvent` vs `EndEvent` etc. should be cleaned up.
3. **Dead code identification** — 142 bound methods are never
   called by any script. Could be intentional forward-API surface
   or stale code; review at leisure.
4. **CONVENTIONS table extension** — the 37 unmapped variables
   include some legitimate UserData accessors that could be added
   to the `CONVENTIONS` dict for future runs to catch more calls.

## Caveats

- **Type inference is convention-based** — relies on variable names
  matching the `CONVENTIONS` table. Scripts that use non-conventional
  variable names (e.g., `myPlayer` instead of `player`) will be
  undercounted.
- **Cross-type composition** — methods bound on a base type
  (LuaActor) and accessed via a derived type (LuaPlayer) need
  garlemald's UserData metamethod forwarding to be wired. The tool
  flags these as "cross-type" rather than gaps; verify forwarding
  is actually in place.
- **Comments are stripped** — but multi-line strings and other Lua
  edge cases may produce false positive method calls.
- **Static analysis only** — methods reached via dynamic dispatch
  (e.g. `obj[method_name]()`) won't show up in the call list.

## Cross-references

- `build/wire/garlemald_lua_coverage.md` — auto-generated full report
- `build/wire/garlemald_lua_coverage.json` — machine-readable
- `tools/garlemald_lua_coverage.py` — the tool
- `docs/cpp_bindings_index.md` — companion: client-engine API surface
- `docs/lpb_corpus_survey.md` — corpus-wide patterns + pipeline
- garlemald-server's `map-server/src/lua/userdata.rs` — the
  binding source under audit
- garlemald-server's `scripts/lua/` — the calling-side script tree
