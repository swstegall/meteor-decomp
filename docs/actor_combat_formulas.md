# Actor combat formulas — Lua-side ground truth (potencial / level adjust / stat tables / 4-param scaling / battle WorkSync)

> Sourced from **ffxivDecomp** (github.com/Yokimitsuro/ffxivDecomp), an independent
> docs-only RE of FFXIV 1.23b `ffxivgame.exe`, used with permission (see `NOTICE.md`).
> **Cross-referenced, not byte-verified** by meteor-decomp's own decompilation —
> confirm against the asm before relying on offsets for a code change.
> Captured 2026-05-30 from the ffxivDecomp 2026-05-27/28 session.

This is the companion to `docs/actor_damage.md` (the damage *display*
path). That file maps the client-side clip/plate render chain but
carries **no formula text**; this file holds the numeric combat values
ffxivDecomp recovered from `chara/charabaseclass_battle.lua` (2027
lines) and its companion `chara/charabaseclass_ffxivbattle.lua` (636
lines, decoded from `charabaseclass_44m1o89qqy5.lua`).

These are **client-side computed** values. The 1.x client predicts /
displays damage with them; the server is authoritative and **validates
against the same math**. That makes this file directly relevant to
garlemald's stat-recalc TODO (`map-server`): garlemald must reproduce
these curves to land the same numbers the client expects, or the client
will show a number the server then "corrects", producing visible
desync.

## 1. `calcPotencial` — the sqrt level-difference adjust

`CharaBaseClass:calcPotencial(targetLevel, casterLevel)`
(`charabaseclass_battle.lua` line 1316; `casterLevel` defaults to
`self:getStateMainSkillLevel()`). This is the **base** damage
multiplier for level mismatch — applied *before* the per-command
4-param scaling in §3, not instead of it.

```text
diff      = targetLevel - casterLevel
|diff| <= 10   →  factor = 1                         (no adjustment; "graceful zone")
|diff| >  10   →  factor = sqrt(1 + (|diff| - 10) * 0.4)

  caster OVER target  (diff < 0)  →  return 1 / factor   (damage REDUCED, < 1)
  caster UNDER target (diff > 0)  →  return     factor   (damage INCREASED, > 1)
```

The over-level side carries the **asymmetric `1/factor`**; the
under-level side returns the raw `factor`. So the curve is *not*
symmetric in effect — it penalises over-levelled casters and boosts
under-levelled ones (anti-twink / assist-the-underdog).

Sample values (from the source finding):

| `|diff|` | excess | `factor` | over-level (`/factor`) | under-level (`*factor`) |
|---:|---:|---|---:|---:|
| 0–10 | 0 | 1.000 | 100% | 100% |
| 15 | 5 | √3 ≈ 1.732 | 58% | 173% |
| 20 | 10 | √5 ≈ 2.236 | 45% | 224% |
| 25 | 15 | √7 ≈ 2.646 | 38% | 265% |
| 30 | 20 | √9 = 3.000 | 33% | 300% |
| 40 | 30 | √13 ≈ 3.606 | 28% | 361% |
| 50 | 40 | √17 ≈ 4.123 | 24% | 412% |

The `0.7` constants in `GameCommandBaseClass` (§3) are a **separate**
per-param fudge applied *on top* of this base multiplier — they are not
the level-adjust formula.

## 2. `getPotencial` — the 21-step monster danger curve + NM tiers

`CharaBaseClass:getPotencial()` (`charabaseclass_battle.lua` line 1149)
overloads one float field for two jobs: an NM-tier flag (negative) or a
danger multiplier (positive).

### NM tier overrides (`MonsterBaseData[132]` = NM tier code)

| NM tier code | `getPotencial` returns | Meaning |
|---:|---:|---|
| 1 | −1 | Regular NM |
| 2 | −2 | Undead NM (confirmed via `isUndead`) |
| 3 | −3 | HNM (high NM) |
| 4 | −4 | World Boss |
| else | normal | falls through to the level curve below |

`isNotoriousMonster()` re-reads the sign: `−1 → (true, 11)`,
`−2 → (true, 12)`, `−3 → (true, 13)`, `−4 → (true, 14)` (the 11–14 are
nameplate-variant codes), positive → `(false, 0)`. One field carries
both "is this an NM" and "which tier" — no separate packet needed.

### Normal monster: 21-step level curve

For non-NM monsters, potencial is **linearly interpolated** from a
21-step table indexed by the monster's state-main-skill level
(`MonsterBaseData[87]`), then multiplied by the monster's base level
(`MonsterBaseData[87]`). Player potencial is `1` (players get no NM
bonus).

| skill level | potencial | | skill level | potencial |
|---:|---:|---|---:|---:|
| 1 | 1 | | 58 | 13 |
| 5 | 2 | | 63 | 14 |
| 9 | 3 | | 68 | 15 |
| 13 | 4 | | 73 | 16 |
| 18 | 5 | | 78 | 17 |
| 23 | 6 | | 83 | 18 |
| 28 | 7 | | 88 | 19 |
| 33 | 8 | | 93 | 20 |
| 38 | 9 | | 98+ | 21 |
| 43 | 10 | | | |
| 48 | 11 | | | |
| 53 | 12 | | | |

So a level-50 monster lands between the 48 (=11) and 53 (=12) rows.
This is the **core monster difficulty curve** for 1.x.

## 3. 4-parameter command scaling (`GameCommandBaseClass`)

Every battle command exposes 4 generic params (Param1–4). Each has a
high-level-use and a low-level-use multiplier plus a sheet-driven growth
column. The Lua carries this **configuration**; the actual roll /
defense / crit math is native C++.

### Default scaling constants

| | Param 1 | Param 2 | Param 3 | Param 4 |
|---|---:|---:|---:|---:|
| `ForHighLevelUse` (caster > target) | **0.7** | **0.7** | **0.7** | **1.0** |
| `ForLowLevelUse` (caster < target) | 1.0 | 1.0 | 1.0 | 1.0 |

High-level use downscales Params 1/2/3 to 70%; Param 4 (probably
proc-rate / hit-chance) stays 100% because probability should not scale
with level. Low-level use applies no penalty. This is **additional** to
the §1 sqrt level adjust.

### Growth-curve columns (`gameCommandSheet`)

| Param | grow column |
|---:|---:|
| 1 | 42 |
| 2 | 47 |
| 3 | 52 |
| 4 | 57 |

`judgeGrowColumn(skill, sheetValue)` interpolates the actual value from
the caster's skill level; a sheet value `< 0` means the param does not
scale (use base directly). Likely param semantics (speculative in the
source): Param 1 = base damage/power, Param 2 = secondary effect power
(status/heal), Param 3 = tertiary (AoE radius / duration), Param 4 =
probability.

The 5 battle command types gating this: `1=Attack`, `2=MagicMissile`,
`3=Ability`, `4=Magic`, `5=WeaponSkill`. `canFire` runs an 8-gate
validation chain (actor stat / target stat / skill / HP / MP / TP /
recast / internal cmd-work) before any of the above applies.

## 4. Party-size multiplier

Damage / difficulty scaling by party member count (from the combat
pipeline finding):

| party size | multiplier |
|---:|---:|
| 1 (solo) | 1.0 |
| 2 | 1.5 |
| 3 | 1.4 |
| 4 | 1.3 |
| 5 | 1.2 |
| 6 | 1.1 |

(Note the curve peaks at 2 then decays toward 1.1 at a full party — a
per-member efficiency taper rather than a flat scale.)

## 5. `getMagicAttack` — 50-entry hardcoded magic-attack table

`CharaBaseClass:getMagicAttack(skillLevel)`
(`charabaseclass_battle.lua` line 1892) is a **hardcoded lookup** (not
sheet-driven) of magic-attack stat by skill level 1–50, capping at
`99,999,999` for level 50+.

| Lvl | MAttack | Lvl | MAttack | Lvl | MAttack | Lvl | MAttack | Lvl | MAttack |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 570 | 11 | 5900 | 21 | 20000 | 31 | 45000 | 41 | 74000 |
| 2 | 700 | 12 | 6800 | 22 | 22000 | 32 | 47000 | 42 | 78000 |
| 3 | 880 | 13 | 7700 | 23 | 23000 | 33 | 50000 | 43 | 81000 |
| 4 | 1100 | 14 | 8700 | 24 | 25000 | 34 | 53000 | 44 | 85000 |
| 5 | 1500 | 15 | 9700 | 25 | 27000 | 35 | 56000 | 45 | 89000 |
| 6 | 1800 | 16 | 11000 | 26 | 29000 | 36 | 59000 | 46 | 92000 |
| 7 | 2300 | 17 | 12000 | 27 | 31000 | 37 | 62000 | 47 | 96000 |
| 8 | 3200 | 18 | 13000 | 28 | 33000 | 38 | 65000 | 48 | 100000 |
| 9 | 4300 | 19 | 15000 | 29 | 35000 | 39 | 68000 | 49 | 100000 ¹ |
| 10 | 5000 | 20 | 16000 | 30 | 38000 | 40 | 71000 | 50 | 110000 |

¹ Lvl 49 = `100000`, **same as Lvl 48** — preserved as-is from the
decompiled source (likely a retail typo/plateau; can't disambiguate
without retail comparison). Lvl 50+ caps at `99,999,999`.

Per-decade growth ratio decreases (≈9× over 1→10, falling toward ≈1.5×
at 40→50) — diminishing returns.

### Physical counterpart — `getPhysicalParameter`

The physical side is **formula-based**, not table-based
(`charabaseclass_ffxivbattle.lua` line 41):
`ceil(base * multiplier * 0.001)` where `multiplier` is a per-mille
factor (1000 = ×1.0) and `base` is a 7-bracket piecewise-linear curve on
skill level:

| skill level | base = |
|---|---|
| ≤ 10 | 100 + lvl·10 |
| ≤ 20 | 200 + (lvl−10)·20 |
| ≤ 30 | 400 + (lvl−20)·40 |
| ≤ 40 | 800 + (lvl−30)·70 |
| ≤ 50 | 1500 + (lvl−40)·130 |
| ≤ 60 | 2800 + (lvl−50)·200 |
| ≤ 70 | 4800 + (lvl−60)·320 |
| 71+ | 8000 + (lvl−70)·500 |

Physical and magic scales are *not* comparable directly (magic numeric
scale is ~25× higher); downstream normalisation differs per side.

## 6. Battle WorkSync schema (`initBattleSync`)

The state a server must replicate, in two tiers. From
`charabaseclass_battle.lua` (`initBattleSync` ~line 1784) +
`charabaseclass_ffxivbattle.lua`.

### `battleSave` — persistent (survives logout, server-authoritative)

| Field | Type | Notes |
|---|---|---|
| `potencial` | float | NM strength multiplier, sign-coded (§2) |
| `physicalLevel` | int16 | character physical level |
| `physicalExp` | int32 | physical experience |
| `skillLevel` | array[52] int16 | per-skill level — full 1.x roster |
| `skillLevelCap` | array[52] int16 | per-skill level cap |
| `skillPoint` | array[52] int32 | per-skill experience |
| `negotiationFlag` | array[2] bool | negotiation/trade state |

The **52-skill arrays** bracket as: 1–20 battle (DoW/DoM), 21–28
mid-tier (reserved/unclear), 29–38 crafter (DoH), 39–52 gatherer (DoL).

### `battleTemp` — transient (recomputed, not persisted)

| Field | Type | Notes |
|---|---|---|
| `castGauge_speed` | array[2] float | cast-bar speed (main/off hand?) |
| `timingCommandFlag` | array[4] bool | reactive combo availability (§7) |
| `generalParameter` | array[35] int16 | the 35 battle stats |

### `generalParameter[35]` sync map

Indices are individually registered. The split (cross-referenced
between the two findings — the 28-index list and the 31-index
`initBattleSync` count agree on the **synced 16–19 + 24–35** body; the
two findings differ only on whether the primary-attribute block 4–15 is
counted as synced):

```text
SYNCED   (broadcast / self-sync)  : 4–19, 24–35
LOCAL    (client derives, not pushed): 1–3, 20–23
```

Notable per-index facts recovered: index **18 = active job's soul
crystal item ID** (`getJobItemId`, not a Parry/Block stat); 16=Attack,
17=NormalDefence, 19=AttackRate, 24=Evasion, 25=AttackMagic,
26=HealMagic, 27=ReinforceMagic, 28=WeekMagic (sic — debuff power),
29=MagicRate, 30=MagicEvasion, 31–33=craft stats, 34–35=harvest stats.
Indices 0–3 (HP/MP/TP/pool) sync via the **separate**
`parameterSave.hp[idx]` / `hpMax[idx]` path (they update far more often
than the once-per-state-change parameters). Indices 13/14 are
registered in **swapped order** (entry-13 carries index 14, entry-14
carries index 13) — deliberate serialization ordering of the paired
cap stats.

Three WorkSync groups carry these: `battleStateForSelf` (self-only:
potencial, skill arrays, castGauge_speed, skillPoint, physicalExp,
negotiationFlag), `timingCommand` (the 4 combo flags), `battleParameter`
(the broadcast generalParameter indices). See
`docs/actor.md` for the action-result wire opcodes (0x148–0x156) these
ride on and `docs/lua_actor_impl.md` for the WorkSync/0x12F binding.

## 7. Timing-command combo system (the "Morrow" reactive commands)

`battleTemp.timingCommandFlag[1..4]` are 4 booleans that, when set,
enable specific reactive command IDs (combo prompts). The server sets a
flag when a combo opportunity arises, the client shows the prompt, the
player fires one of the enabled IDs, the server clears the flag.

| flag | enabled command IDs | role |
|---:|---|---|
| flag[1] | **27278, 27279** | combo step A (2 options) |
| flag[2] | **27119** | combo step B (1 option) |
| flag[3] | **27198, 27199** | combo step C (2 options) |
| flag[4] | **27158, 27157** | combo step D (2 options) |

(IDs verified against `finding_charabaseclass_battle_schema_and_timing_commands.md`;
flag[4] lists **27158 then 27157** in that order in the source.) The
command IDs are `gameCommand.csv` references in the 27xxx range.
`getEnableTimingCommands()` returns the currently-available list;
`isMorrowTimingCommand(idx)` checks one flag.

Separately, `convertSkillId` returns **36 cross-class command IDs** (35
in the 27xxx range organised as 7 groups of 5 — one per job — plus the
outlier `29742`, likely the universal Limit-Break/special).

## Server implications (garlemald)

Relevant to the `map-server` stat-recalc TODO:

1. **Reproduce both base multipliers.** §1 sqrt level-adjust (`1/factor`
   over-level, `factor` under-level) **and** §3 per-command 4-param
   scaling (high-level 0.7 on Params 1/2/3, 1.0 on Param 4) are
   *multiplicative and stacked*, not alternatives. Miss either and the
   client's predicted number won't match the server's authoritative one.
2. **Port the stat tables verbatim**, including the Lvl-49 plateau in
   `getMagicAttack` and the 7-bracket `getPhysicalParameter` slopes —
   these are the per-skill-level base power the rest of the chain
   multiplies.
3. **Push only the synced `generalParameter` indices** (16–19, 24–35,
   plus the primary block 4–15) via the `battleParameter` /
   `battleStateForSelf` WorkSync groups; let the client derive 1–3 and
   20–23. Route HP/MP/TP through the separate `parameterSave.hp[]` path.
4. **Drive combos by setting `timingCommandFlag[N]`** and accepting only
   the matching 27xxx IDs from §7 as the reaction; clear on
   use/timeout.
5. **Sign-code `potencial`** for NM tiers (−1..−4) rather than adding a
   separate "is NM" field.

## Cross-references

- `docs/actor_damage.md` — the damage *display* path (clip/plate render
  chain); this file is its formula companion.
- `docs/actor.md` — Phase 5 plan; action-result wire opcodes 0x148–0x156.
- `docs/actor_battle_regimen.md` — the Battle Regimen / chain UI side;
  the §7 timing combos are 1.x's reactive-command layer that feeds it.
- `docs/lua_actor_impl.md` — LuaActorImpl bindings + WorkSync (0x12F).
- `ffxiv_1x_battle_commands_context.md` — per-command damage type +
  element + ClassJob + WS-id table; the sheet rows behind §3.
- `land-sand-boat-server/xi-private-server.md` — XI's structurally
  analogous level-correction (`pDIF` / level-difference penalty),
  TP→WS, and per-skill stat curves; the *grammar* matches even though
  1.x's sqrt curve + Stamina-era numbers diverge.

## Confidence (per ffxivDecomp source findings)

```text
Confirmed (in the cited Lua source):
  - calcPotencial sqrt(1 + (|diff|-10)*0.4); plateau |diff|<=10;
    over-level = 1/factor, under-level = factor.
  - 21-step potencial curve (exact pairs) + NM tiers -1..-4.
  - getMagicAttack 50-entry hardcoded table (incl. Lvl-49 plateau, 99M cap).
  - getPhysicalParameter 7-bracket piecewise-linear formula.
  - 4-param defaults: high 0.7/0.7/0.7/1.0, low 1.0×4; grow cols 42/47/52/57.
  - party multiplier 1.0/1.5/1.4/1.3/1.2/1.1.
  - battleSave 52-skill arrays + battleTemp generalParameter[35] +
    timingCommandFlag[4] + castGauge_speed[2].
  - timing combo IDs flag1→27278/27279, flag2→27119, flag3→27198/27199,
    flag4→27158/27157; convertSkillId 36 IDs (29742 outlier).
  - generalParameter[18] = soul crystal item ID.

Caveat (cross-referenced, not byte-verified here):
  - The synced-index *count* differs between the two findings (28 vs 31)
    because they disagree on counting the primary block 4–15; the synced
    BODY (16–19, 24–35) and LOCAL set (1–3, 20–23) agree. Confirm against
    the WorkSync 0x12F serializer before treating any single index as
    authoritative for a wire change.
  - All offsets/IDs are from ffxivDecomp's Lua decompilation, not
    meteor-decomp's own asm pass — verify before a code change.
```
