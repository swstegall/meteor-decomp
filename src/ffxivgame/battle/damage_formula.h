// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Phase 5 — Damage formula public API.
//
// Re-derived from LSB's XI-cousin formula (`physical_utilities.lua`
// `calculateAttackDamage` / `calculateMeleeStatFactor`) and calibrated
// against YouTube atlas damage-sample medians for canonical 1.x
// abilities. The formula structure (fSTR piecewise linear + pDIF from
// cRatio + base = wpnDmg + fSTR + WSC) is structurally identical to
// FFXIV 1.x's XI-cousin damage model documented in NOTICE.md.

#ifndef FFXIV_BATTLE_DAMAGE_FORMULA_H
#define FFXIV_BATTLE_DAMAGE_FORMULA_H

#include <cstdint>

namespace ffxiv::battle {

// Weapon-skill enum aligned with the `xi.skill` set the formula
// branches on. Only the subset relevant to physical attacks is
// listed here; magic damage uses a separate path.
enum class WeaponSkill : int {
    None        = 0,
    HandToHand  = 1,
    Dagger      = 2,
    Sword       = 3,
    GreatSword  = 4,
    Axe         = 5,
    GreatAxe    = 6,
    Scythe      = 7,
    Polearm     = 8,
    Katana      = 9,
    GreatKatana = 10,
    Club        = 11,
    Staff       = 12,
    Archery     = 13,
    Marksmanship= 14,
    Throwing    = 15,
};

enum class CommandType : int {
    BasicAttack = 2,   // auto-attacks, basic swing
    Magic       = 3,
    Ability     = 5,
    WeaponSkill = 4,
    Trait       = 7,
};

// Physical attack actor stats. INT/MND/CHR are tracked for future
// magic-damage extension; for physical they're unused.
struct ActorStats {
    int str    = 0;
    int dex    = 0;
    int vit    = 0;
    int agi    = 0;
    int int_   = 0;
    int mnd    = 0;
    int chr    = 0;
    int atk    = 0;  // total attack rating
    int def    = 0;  // total defense rating
    int level  = 1;
    bool is_mob = false;
};

// Equipped weapon profile. `dmg` is the weapon's base DMG stat;
// `rank` is the FFXI-style "weapon rank" 0..15 that caps fSTR's
// range. For 1.x we map weapon-tier to rank via wpn_dmg_to_rank().
struct Weapon {
    WeaponSkill skill = WeaponSkill::HandToHand;
    int         dmg   = 3;   // minimal H2H natural damage default
    int         rank  = 0;
};

// Setup for one damage roll. Random component is exposed as
// `random_roll_pct` (0.0..1.0) so callers can pin a deterministic
// roll for testing.
struct DamageInput {
    ActorStats   attacker;
    ActorStats   target;
    Weapon       weapon;
    CommandType  command_type    = CommandType::BasicAttack;
    bool         is_crit         = false;
    double       random_roll_pct = 0.5;   // 0.5 = median roll
    // For weaponskills only:
    double       fTP             = 1.0;
    int          WSC             = 0;
};

// fSTR — the additive STR-vs-VIT bonus baked into base damage. Caps
// based on weapon rank. Returns integer in `[fSTR_lower_cap,
// fSTR_upper_cap]`.
//
// Formula (re-derived from XI cousin):
//   statDiff = attacker.str - target.vit
//   (clamped to [-(7+rank*2)*2, (14+rank*2)*2] for players)
//   piecewise add: 4 if statDiff >= 12, 6 if >= 6, ..., 13 if < -21
//   then floor((stat-adjusted) / 4), clamped to [rank*-1, rank+8]
//   (with weapon_rank=0 special-cased to lower=-1)
int compute_fSTR(int attacker_str, int target_vit, int weapon_rank);

// Map a weapon's DMG stat to its rank tier. 1.x weapon rank is a
// derivative property; this is the empirical curve from BG-wiki
// reference data.
int wpn_dmg_to_rank(int wpn_dmg);

// pDIF — physical damage modifier. Derived from cRatio (ATK/DEF)
// with weapon-type-specific cap, then a uniform random roll inside
// [cap_low, cap_high]. `random_roll_pct` lets callers pin the roll
// for deterministic testing.
//
// Critical hits add +1 to cRatio post-cap (NOT a separate
// multiplier — this matches the XI cousin behavior).
double compute_pDIF(int atk, int def, WeaponSkill skill,
                    bool is_crit, double random_roll_pct);

// Full physical damage roll:
//   baseDamage = wpnDmg + fSTR + (WSC if weaponskill, else 0)
//   rawDamage  = floor(baseDamage * fTP * pDIF)
//   rawDamage  = max(0, rawDamage)
int compute_physical_damage(const DamageInput& in);

// Returns the pre-randomizer pDIF cap for a weapon skill (3.0 to
// 4.0 typical). Used by both compute_pDIF and as a public API for
// damage-range estimation.
double pdif_cap_for_skill(WeaponSkill skill);

} // namespace ffxiv::battle

#endif // FFXIV_BATTLE_DAMAGE_FORMULA_H
