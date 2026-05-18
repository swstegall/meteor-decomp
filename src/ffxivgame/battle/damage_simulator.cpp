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
// Phase 5 — damage_simulator CLI.
//
// Reads a damage-roll setup either from CLI args or a minimal flat
// key=value JSON file, runs compute_physical_damage(), and prints
// the result alongside the damage band (min/median/max over the
// random-roll range) so callers can sanity-check against the
// YouTube atlas median samples.
//
// Exit criterion (per PLAN.md Phase 5): produce a damage number
// that matches a recorded sample within ±1 for the calibration
// fixtures shipped under fixtures/battle/*.json.

#include "damage_formula.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

namespace fb = ffxiv::battle;

namespace {

void usage() {
    std::printf(
"usage:\n"
"  damage_simulator <fixture.json>\n"
"  damage_simulator --inline \\\n"
"      str=<n> vit=<n> atk=<n> def=<n> \\\n"
"      [wpn_dmg=<n>] [wpn_skill=<HandToHand|Sword|...>] \\\n"
"      [roll=<0.0..1.0>] [crit=0|1] [fTP=<f>] [WSC=<n>]\n"
"\n"
"output:\n"
"  damage=<int>  band_min=<int>  band_med=<int>  band_max=<int>\n"
"\n"
"fixture json (flat key=value, one per line, '#' for comments):\n"
"  str=80\n"
"  vit=60\n"
"  atk=200\n"
"  def=150\n"
"  wpn_dmg=22\n"
"  wpn_skill=Sword\n"
"  roll=0.5\n"
"  crit=0\n"
"\n"
"see fixtures/battle/*.json for calibration cases against the\n"
"YouTube atlas damage samples.\n");
}

fb::WeaponSkill parse_skill(const std::string& s) {
    if (s == "HandToHand")   return fb::WeaponSkill::HandToHand;
    if (s == "Dagger")       return fb::WeaponSkill::Dagger;
    if (s == "Sword")        return fb::WeaponSkill::Sword;
    if (s == "GreatSword")   return fb::WeaponSkill::GreatSword;
    if (s == "Axe")          return fb::WeaponSkill::Axe;
    if (s == "GreatAxe")     return fb::WeaponSkill::GreatAxe;
    if (s == "Scythe")       return fb::WeaponSkill::Scythe;
    if (s == "Polearm")      return fb::WeaponSkill::Polearm;
    if (s == "Katana")       return fb::WeaponSkill::Katana;
    if (s == "GreatKatana")  return fb::WeaponSkill::GreatKatana;
    if (s == "Club")         return fb::WeaponSkill::Club;
    if (s == "Staff")        return fb::WeaponSkill::Staff;
    if (s == "Archery")      return fb::WeaponSkill::Archery;
    if (s == "Marksmanship") return fb::WeaponSkill::Marksmanship;
    if (s == "Throwing")     return fb::WeaponSkill::Throwing;
    return fb::WeaponSkill::None;
}

using KV = std::unordered_map<std::string, std::string>;

bool read_fixture(const std::string& path, KV& out) {
    std::ifstream f(path);
    if (!f) {
        std::fprintf(stderr, "error: cannot open %s\n", path.c_str());
        return false;
    }
    std::string line;
    while (std::getline(f, line)) {
        // strip leading/trailing whitespace
        size_t a = line.find_first_not_of(" \t");
        if (a == std::string::npos) continue;
        if (line[a] == '#') continue;
        size_t b = line.find_last_not_of(" \t\r");
        line = line.substr(a, b - a + 1);

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string k = line.substr(0, eq);
        std::string v = line.substr(eq + 1);
        out[k] = v;
    }
    return true;
}

bool parse_inline(int argc, char** argv, int start, KV& out) {
    for (int i = start; i < argc; ++i) {
        std::string s = argv[i];
        size_t eq = s.find('=');
        if (eq == std::string::npos) {
            std::fprintf(stderr, "error: arg %d not key=value\n", i);
            return false;
        }
        out[s.substr(0, eq)] = s.substr(eq + 1);
    }
    return true;
}

int get_int(const KV& kv, const std::string& k, int def_val) {
    auto it = kv.find(k);
    return (it != kv.end()) ? std::atoi(it->second.c_str()) : def_val;
}

double get_dbl(const KV& kv, const std::string& k, double def_val) {
    auto it = kv.find(k);
    return (it != kv.end()) ? std::atof(it->second.c_str()) : def_val;
}

std::string get_str(const KV& kv, const std::string& k,
                    const std::string& def_val) {
    auto it = kv.find(k);
    return (it != kv.end()) ? it->second : def_val;
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2 || std::strcmp(argv[1], "--help") == 0 ||
        std::strcmp(argv[1], "-h") == 0) {
        usage();
        return (argc < 2) ? 1 : 0;
    }

    KV kv;
    if (std::strcmp(argv[1], "--inline") == 0) {
        if (!parse_inline(argc, argv, 2, kv)) return 2;
    } else {
        if (!read_fixture(argv[1], kv)) return 2;
    }

    fb::DamageInput in;
    in.attacker.str  = get_int(kv, "str", 50);
    in.attacker.atk  = get_int(kv, "atk", 100);
    in.attacker.level= get_int(kv, "att_level", 1);
    in.target.vit    = get_int(kv, "vit", 50);
    in.target.def    = get_int(kv, "def", 100);
    in.target.level  = get_int(kv, "tgt_level", 1);
    in.weapon.dmg    = get_int(kv, "wpn_dmg", 3);
    in.weapon.rank   = get_int(kv, "wpn_rank",
                               fb::wpn_dmg_to_rank(in.weapon.dmg));
    in.weapon.skill  = parse_skill(get_str(kv, "wpn_skill", "HandToHand"));
    in.command_type  = (get_str(kv, "cmd", "BasicAttack") == "WeaponSkill")
                       ? fb::CommandType::WeaponSkill
                       : fb::CommandType::BasicAttack;
    in.is_crit       = get_int(kv, "crit", 0) != 0;
    in.fTP           = get_dbl(kv, "fTP", 1.0);
    in.WSC           = get_int(kv, "WSC", 0);
    in.random_roll_pct = get_dbl(kv, "roll", 0.5);

    // Compute the requested roll, then min/median/max for the band.
    const int dmg = fb::compute_physical_damage(in);

    fb::DamageInput in_min = in; in_min.random_roll_pct = 0.0;
    fb::DamageInput in_max = in; in_max.random_roll_pct = 1.0;
    fb::DamageInput in_med = in; in_med.random_roll_pct = 0.5;

    const int dmg_min = fb::compute_physical_damage(in_min);
    const int dmg_med = fb::compute_physical_damage(in_med);
    const int dmg_max = fb::compute_physical_damage(in_max);

    const int fSTR = fb::compute_fSTR(in.attacker.str, in.target.vit,
                                      in.weapon.rank);
    const double pdif_med = fb::compute_pDIF(in.attacker.atk, in.target.def,
                                             in.weapon.skill, in.is_crit,
                                             0.5);

    std::printf("damage=%d  band_min=%d  band_med=%d  band_max=%d\n",
                dmg, dmg_min, dmg_med, dmg_max);
    std::printf("  inputs: str=%d vit=%d atk=%d def=%d  "
                "wpn_dmg=%d wpn_rank=%d crit=%d roll=%.2f\n",
                in.attacker.str, in.target.vit,
                in.attacker.atk, in.target.def,
                in.weapon.dmg, in.weapon.rank,
                in.is_crit ? 1 : 0, in.random_roll_pct);
    std::printf("  derived: fSTR=%d  pDIF(med)=%.3f  pdif_cap=%.2f\n",
                fSTR, pdif_med, fb::pdif_cap_for_skill(in.weapon.skill));
    return 0;
}
