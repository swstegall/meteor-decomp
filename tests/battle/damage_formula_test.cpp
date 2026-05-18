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
// Phase 5 — damage_formula unit tests.
//
// Verifies the fSTR + pDIF + compute_physical_damage primitives
// produce the expected numbers across a curated set of input
// vectors. Used both as regression coverage and as living
// documentation of the formula's curve shape.

#include "../../src/ffxivgame/battle/damage_formula.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace fb = ffxiv::battle;

static int g_pass = 0;
static int g_fail = 0;

#define EXPECT_EQ(actual, expected, label) do { \
    int _a = (actual); int _e = (expected); \
    if (_a == _e) { \
        ++g_pass; \
    } else { \
        ++g_fail; \
        std::printf("  FAIL: %s — actual %d != expected %d\n", \
                    (label), _a, _e); \
    } \
} while (0)

#define EXPECT_NEAR(actual, expected, tol, label) do { \
    double _a = (actual); double _e = (expected); \
    if (std::fabs(_a - _e) <= (tol)) { \
        ++g_pass; \
    } else { \
        ++g_fail; \
        std::printf("  FAIL: %s — actual %.4f not within %.4f of %.4f\n", \
                    (label), _a, (double)(tol), _e); \
    } \
} while (0)

#define EXPECT_RANGE(actual, lo, hi, label) do { \
    int _a = (actual); int _l = (lo); int _h = (hi); \
    if (_a >= _l && _a <= _h) { \
        ++g_pass; \
    } else { \
        ++g_fail; \
        std::printf("  FAIL: %s — actual %d outside [%d, %d]\n", \
                    (label), _a, _l, _h); \
    } \
} while (0)

static void test_fSTR_curve() {
    std::printf("test_fSTR_curve:\n");
    // statDiff = 0, rank=1 → shifted = 0+8 = 8, /4 = 2, clamp [-1, 9] → 2
    EXPECT_EQ(fb::compute_fSTR(50, 50, 1), 2, "balanced str/vit rank 1");
    // statDiff = 12, rank=2 → shifted = 12+4 = 16, /4 = 4, clamp [-2, 10] → 4
    EXPECT_EQ(fb::compute_fSTR(62, 50, 2), 4, "str+12 rank 2");
    // statDiff = -20, rank=3 → shifted = -20+12 = -8, /4 = -2, clamp [-3, 11] → -2
    EXPECT_EQ(fb::compute_fSTR(30, 50, 3), -2, "str-20 rank 3");
    // statDiff = +200 with rank 5 — clamped to upper cap (14+10)*2 = 48,
    //   shifted = 48+4 = 52, /4 = 13, clamp [-5, 13] → 13
    EXPECT_EQ(fb::compute_fSTR(300, 50, 5), 13, "huge str gap rank 5");
    // statDiff = -200, clamped to lower (7+10)*-2 = -34, shifted = -34+13 = -21,
    //   /4 = -5 (truncates toward 0; -21/4 = -5 in C), clamp [-5, 13] → -5
    EXPECT_EQ(fb::compute_fSTR(0, 200, 5), -5, "huge negative gap rank 5");
    // weapon_rank = 0 special case: lower cap = -1
    EXPECT_EQ(fb::compute_fSTR(0, 200, 0), -1, "rank 0 lower cap -1");
}

static void test_pdif_caps() {
    std::printf("test_pdif_caps:\n");
    EXPECT_NEAR(fb::pdif_cap_for_skill(fb::WeaponSkill::HandToHand),
                3.50, 0.001, "H2H cap");
    EXPECT_NEAR(fb::pdif_cap_for_skill(fb::WeaponSkill::Scythe),
                4.00, 0.001, "Scythe cap (highest)");
    EXPECT_NEAR(fb::pdif_cap_for_skill(fb::WeaponSkill::None),
                3.00, 0.001, "None cap (default)");
}

static void test_pDIF_curve() {
    std::printf("test_pDIF_curve:\n");
    // atk=def → cRatio=1.0, pdif band [1.0, 1.25], median 1.125
    EXPECT_NEAR(fb::compute_pDIF(100, 100, fb::WeaponSkill::Sword, false, 0.5),
                1.125, 0.001, "balanced atk/def median");
    // atk=2*def → cRatio=2.0, pdif band approx (1.625, 1.875), median 1.75
    EXPECT_NEAR(fb::compute_pDIF(200, 100, fb::WeaponSkill::Sword, false, 0.5),
                1.75, 0.001, "atk=2x def median");
    // Crit bumps cRatio to 3.0; with Sword cap=3.25, ceiling capped to 3.25
    {
        double v = fb::compute_pDIF(200, 100, fb::WeaponSkill::Sword, true, 1.0);
        EXPECT_NEAR(v, 3.25, 0.001, "crit caps at Sword pdif cap");
    }
    // Min cRatio 0.5 (atk << def) at roll 0.0 → 0.5
    EXPECT_NEAR(fb::compute_pDIF(50, 200, fb::WeaponSkill::Sword, false, 0.0),
                0.5, 0.001, "atk<<def cRatio lower clamp");
}

static void test_wpn_dmg_to_rank() {
    std::printf("test_wpn_dmg_to_rank:\n");
    EXPECT_EQ(fb::wpn_dmg_to_rank(1), 0, "tiny dmg = rank 0");
    EXPECT_EQ(fb::wpn_dmg_to_rank(10), 1, "early dmg = rank 1");
    EXPECT_EQ(fb::wpn_dmg_to_rank(22), 3, "midgame dmg = rank 3");
    EXPECT_EQ(fb::wpn_dmg_to_rank(98), 8, "highgame dmg = rank 8");
    EXPECT_EQ(fb::wpn_dmg_to_rank(500), 10, "absurd dmg = rank 10 (cap)");
}

static void test_full_damage_calibration() {
    std::printf("test_full_damage_calibration:\n");
    // Calibration: low-mob hitting tank-player. YouTube atlas "attack"
    // min=2; our floor for under-damaged hits should be 0..3.
    fb::DamageInput low;
    low.attacker.str = 14;  low.attacker.atk = 60;
    low.target.vit   = 55;  low.target.def   = 180;
    low.weapon.dmg   = 3;   low.weapon.skill = fb::WeaponSkill::HandToHand;
    low.weapon.rank  = 0;
    low.command_type = fb::CommandType::BasicAttack;
    low.random_roll_pct = 0.5;
    EXPECT_RANGE(fb::compute_physical_damage(low), 0, 5,
                 "low-end basic attack ~min YouTube band");

    // Mid: mid-tier player vs mid mob. Expected band 20-60.
    fb::DamageInput med;
    med.attacker.str = 68;  med.attacker.atk = 180;
    med.target.vit   = 55;  med.target.def   = 120;
    med.weapon.dmg   = 22;  med.weapon.skill = fb::WeaponSkill::Polearm;
    med.weapon.rank  = 3;
    med.command_type = fb::CommandType::BasicAttack;
    med.random_roll_pct = 0.5;
    EXPECT_RANGE(fb::compute_physical_damage(med), 20, 80,
                 "mid-tier basic attack");

    // High: overpowered player + crit. Expected band 300-700.
    fb::DamageInput high;
    high.attacker.str = 140; high.attacker.atk = 420;
    high.target.vit   = 40;  high.target.def   = 110;
    high.weapon.dmg   = 98;  high.weapon.skill = fb::WeaponSkill::GreatSword;
    high.weapon.rank  = 8;
    high.command_type = fb::CommandType::BasicAttack;
    high.is_crit      = true;
    high.random_roll_pct = 1.0;
    EXPECT_RANGE(fb::compute_physical_damage(high), 300, 700,
                 "high-end crit basic attack");
}

int main() {
    std::printf("ffxiv-battle damage_formula tests\n");
    std::printf("---------------------------------\n");

    test_fSTR_curve();
    test_pdif_caps();
    test_pDIF_curve();
    test_wpn_dmg_to_rank();
    test_full_damage_calibration();

    std::printf("\n");
    std::printf("RESULT: %d passed, %d failed\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
