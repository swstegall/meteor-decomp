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

#include "damage_formula.h"

#include <algorithm>
#include <cmath>

namespace ffxiv::battle {

// Per-skill pDIF cap (pre-randomizer ceiling). Re-derived from the
// XI-cousin reference (BG-wiki "FFXI pdif" article values that LSB
// codifies in `pDifWeaponCapTable`). FFXIV 1.x reused this design
// grammar for its physical damage variance.
double pdif_cap_for_skill(WeaponSkill skill) {
    switch (skill) {
        case WeaponSkill::HandToHand:  return 3.50;
        case WeaponSkill::Dagger:      return 3.25;
        case WeaponSkill::Sword:       return 3.25;
        case WeaponSkill::GreatSword:  return 3.75;
        case WeaponSkill::Axe:         return 3.25;
        case WeaponSkill::GreatAxe:    return 3.75;
        case WeaponSkill::Scythe:      return 4.00;
        case WeaponSkill::Polearm:     return 3.75;
        case WeaponSkill::Katana:      return 3.25;
        case WeaponSkill::GreatKatana: return 3.50;
        case WeaponSkill::Club:        return 3.25;
        case WeaponSkill::Staff:       return 3.75;
        case WeaponSkill::Archery:     return 3.25;
        case WeaponSkill::Marksmanship:return 3.50;
        case WeaponSkill::Throwing:    return 3.25;
        case WeaponSkill::None:
        default:                       return 3.00;
    }
}

// Map raw weapon DMG → weapon-rank tier. The relationship is
// empirically log-linear in the XI cousin; this approximation
// matches the observed clamping behavior for 1.x-era item DMG
// ranges (1..120 typical for level-50 cap).
int wpn_dmg_to_rank(int wpn_dmg) {
    if (wpn_dmg < 5)   return 0;
    if (wpn_dmg < 13)  return 1;
    if (wpn_dmg < 22)  return 2;
    if (wpn_dmg < 33)  return 3;
    if (wpn_dmg < 47)  return 4;
    if (wpn_dmg < 62)  return 5;
    if (wpn_dmg < 79)  return 6;
    if (wpn_dmg < 98)  return 7;
    if (wpn_dmg < 119) return 8;
    if (wpn_dmg < 142) return 9;
    return 10;  // very-high-tier weapons; 1.x rarely exceeds rank 10
}

namespace {

int clamp_int(int v, int lo, int hi) {
    return std::min(std::max(v, lo), hi);
}

double clamp_dbl(double v, double lo, double hi) {
    return std::min(std::max(v, lo), hi);
}

} // namespace

int compute_fSTR(int attacker_str, int target_vit, int weapon_rank) {
    int statDiff = attacker_str - target_vit;

    // Player path (mob path is simpler; documented below if needed).
    const int statLowerCap = (7  + weapon_rank * 2) * -2;
    const int statUpperCap = (14 + weapon_rank * 2) *  2;
    statDiff = clamp_int(statDiff, statLowerCap, statUpperCap);

    // Piecewise-linear shift. Brackets reproduce XI's known curve
    // (8 thresholds at +12, +6, +1, -2, -7, -15, -21, else).
    int shifted;
    if      (statDiff >= 12)  shifted = statDiff +  4;
    else if (statDiff >=  6)  shifted = statDiff +  6;
    else if (statDiff >=  1)  shifted = statDiff +  7;
    else if (statDiff >= -2)  shifted = statDiff +  8;
    else if (statDiff >= -7)  shifted = statDiff +  9;
    else if (statDiff >= -15) shifted = statDiff + 10;
    else if (statDiff >= -21) shifted = statDiff + 12;
    else                      shifted = statDiff + 13;

    // Final clamp on /4 result.
    const int fSTRupperCap = weapon_rank + 8;
    int fSTRlowerCap = (weapon_rank == 0) ? -1 : -weapon_rank;

    // Integer division by 4 (truncates toward zero — matches MSVC
    // 2005's signed-int div behavior the engine uses).
    int fSTR_div = shifted / 4;
    return clamp_int(fSTR_div, fSTRlowerCap, fSTRupperCap);
}

double compute_pDIF(int atk, int def, WeaponSkill skill,
                    bool is_crit, double random_roll_pct) {
    // cRatio = ATK / DEF, capped on both ends.
    double cRatio = (def > 0) ? (double)atk / (double)def : 1.0;

    // Player cRatio caps: lower 0.5, upper 2.0 (BG-wiki convention).
    cRatio = clamp_dbl(cRatio, 0.5, 2.0);

    // Critical hit bumps cRatio by +1.0 post-cap (XI behavior).
    if (is_crit) cRatio += 1.0;

    // Per-skill pDIF cap (3.0..4.0 range).
    const double cap = pdif_cap_for_skill(skill);

    // pDIF random component: uniform in [cRatio_low, cRatio_high].
    // Low/high derived from cRatio with weapon's cap as ceiling.
    double pdif_low, pdif_high;
    if (cRatio < 0.5) {
        pdif_low  = cRatio;
        pdif_high = cRatio + 0.05;
    } else if (cRatio < 0.7) {
        pdif_low  = cRatio;
        pdif_high = cRatio + 0.10;
    } else if (cRatio < 1.2) {
        pdif_low  = cRatio;
        pdif_high = cRatio + 0.25;
    } else if (cRatio < 1.5) {
        pdif_low  = (cRatio - 0.375);
        pdif_high = std::min(cRatio + 0.05, cap);
    } else if (cRatio < 2.625) {
        pdif_low  = (cRatio - 0.375);
        pdif_high = std::min(cRatio - 0.375 + 0.25, cap);
    } else {
        pdif_low  = std::min(cRatio - 0.375, cap - 0.25);
        pdif_high = cap;
    }

    // Clamp band to weapon cap.
    pdif_low  = clamp_dbl(pdif_low,  0.0, cap);
    pdif_high = clamp_dbl(pdif_high, pdif_low, cap);

    // Linear interpolation through the random roll.
    const double r = clamp_dbl(random_roll_pct, 0.0, 1.0);
    return pdif_low + (pdif_high - pdif_low) * r;
}

int compute_physical_damage(const DamageInput& in) {
    // Weapon damage portion + fSTR + (WSC if weaponskill).
    const int fSTR = compute_fSTR(in.attacker.str, in.target.vit,
                                  in.weapon.rank);

    int baseDamage = in.weapon.dmg + fSTR;
    if (in.command_type == CommandType::WeaponSkill) {
        baseDamage += in.WSC;
    }
    if (baseDamage < 1) baseDamage = 1;

    // Apply fTP (weaponskill multiplier; 1.0 for basic attack).
    const double pDIF = compute_pDIF(in.attacker.atk, in.target.def,
                                     in.weapon.skill, in.is_crit,
                                     in.random_roll_pct);

    const double scaled = (double)baseDamage * in.fTP * pDIF;
    int damage = (int)std::floor(scaled);
    if (damage < 0) damage = 0;
    return damage;
}

} // namespace ffxiv::battle
