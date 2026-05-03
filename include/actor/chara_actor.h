// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Application::Scene::Actor::Chara::CharaActor — recovered from
// ffxivgame.exe RTTI + ctor/dtor extraction.
//
// vtable: RVA 0xbc0d34 (= VA 0xfc0d34), 188 slots
// ctor:   FUN_0065f180 (1942 B at file 0x25f180)
// dtor:   FUN_00666130 (968 B at file 0x266130) — wrapped by slot 0
//         (FUN_00669e20, 34 B scalar deleting destructor)
//
// Class size: ≥ 0x2ba4 (= 11,172 bytes) — derived from the highest
// field offset (`+0x2ba0`, dword) cleared in the ctor. Could be
// larger; the dtor doesn't access anything past +0x2b94.
//
// 139 distinct field offsets accessed across ctor + dtor.
//
// This header is a FIELD-OFFSET CATALOG (not a packed struct), since
// reconstructing the exact C++ class layout with correct padding is
// fragile and not necessary — garlemald just needs to know which
// offset corresponds to which property when constructing
// SetActorProperty packets.
//
// Naming convention:
//   * KNOWN field — meaningful name (`hp`, `level`, `class_id`)
//   * UNKNOWN field — `field_<HEX_OFFSET>`
//   * SUB-OBJECT — `subobj_<HEX_OFFSET>`
//   * VTABLE-BEARING POINTER — `vtable_obj_<HEX_OFFSET>`
//
// Use these constants in garlemald when building a packet that
// targets a CharaActor field, e.g.:
//
//   set_actor_property(actor_id,
//                      CHARA_ACTOR_OFFSET::flags_2b70,
//                      bit_set);

#ifndef METEOR_DECOMP_ACTOR_CHARA_ACTOR_H
#define METEOR_DECOMP_ACTOR_CHARA_ACTOR_H

#include <stddef.h>

namespace meteor_decomp {
namespace actor {
namespace chara_actor {

// Vtable identification.
static const size_t VTABLE_RVA = 0xbc0d34;
static const size_t VTABLE_VA  = 0xfc0d34;
static const size_t VTABLE_SLOT_COUNT = 188;

// Ctor + dtor anchors.
static const size_t CTOR_FILE_OFFSET = 0x25f180;
static const size_t CTOR_SIZE        = 1942;
static const size_t DTOR_FILE_OFFSET = 0x266130;
static const size_t DTOR_SIZE        = 968;
static const size_t DTOR_SLOT0_FILE_OFFSET = 0x269e20;  // scalar deleting dtor

// Class size lower bound (highest field offset + 4).
static const size_t CLASS_SIZE_MIN = 0x2ba4;            // 11,172 bytes

// === Field offsets ===
//
// Each constant is the BYTE OFFSET of the field within a CharaActor
// instance. Comments capture: type (if known from the writer's
// instruction width), initial value (from ctor), and any other
// hints from cross-referenced behaviour.
//
// Many fields' SEMANTIC PURPOSE is unknown — the offset is real
// (the binary writes/reads here) but what the value MEANS depends
// on which methods read/write it. Filled in incrementally as more
// methods are decompiled.

namespace OFFSET {

    // ---- Header / inheritance ----
    static const size_t vtable                     = 0x0000;  // dword, init = 0xfc0d34 (CharaActor::vftable)

    // ---- Early fields (likely inherited from base Actor class) ----
    static const size_t field_0114                 = 0x0114;  // (dtor-only; type unknown)
    static const size_t parent_or_owner            = 0x0118;  // dword, accessed early in ctor; likely a back-ref
    static const size_t some_ptr_138               = 0x0138;  // dword, init = 0
    static const size_t field_0154                 = 0x0154;
    static const size_t field_0158                 = 0x0158;
    static const size_t field_015c_w               = 0x015c;  // word, init = 0
    static const size_t field_0160                 = 0x0160;  // dword, init = 0
    static const size_t field_0164_b               = 0x0164;  // byte, init = 0
    static const size_t field_0168_b               = 0x0168;  // byte, init = 0
    static const size_t flag_0169                  = 0x0169;  // byte, init = 1 (LITERAL)
    static const size_t field_016c                 = 0x016c;
    static const size_t field_01bc                 = 0x01bc;

    // ---- Mid-class block (~0x300..~0xc00) ----
    // Mostly sub-objects + opaque state. Each touched in dtor with
    // a sub-dtor call.
    static const size_t subobj_035c                = 0x035c;
    static const size_t subobj_0390                = 0x0390;
    static const size_t subobj_03ac                = 0x03ac;
    static const size_t subobj_0438                = 0x0438;
    static const size_t subobj_04c0                = 0x04c0;
    static const size_t subobj_0590                = 0x0590;
    static const size_t subobj_0b50                = 0x0b50;
    static const size_t subobj_0b80                = 0x0b80;
    static const size_t subobj_0bf0                = 0x0bf0;

    // ---- 0x1000..0x1400 block ----
    static const size_t subobj_0fc0                = 0x0fc0;
    static const size_t subobj_1030                = 0x1030;
    static const size_t subobj_1070                = 0x1070;
    static const size_t subobj_1110                = 0x1110;
    static const size_t value_1170                 = 0x1170;  // dword, init = 0xED (237) — INTERESTING LITERAL
    static const size_t field_1174                 = 0x1174;  // dword, init = 0
    static const size_t value_1178                 = 0x1178;  // dword, init = 0xC9 (201) — INTERESTING LITERAL
    static const size_t field_117c                 = 0x117c;  // dword, init = 0
    static const size_t field_1180                 = 0x1180;  // dword, init = 0
    static const size_t field_1184                 = 0x1184;  // dword, init = 0
    static const size_t field_118c                 = 0x118c;
    static const size_t field_11b4                 = 0x11b4;
    static const size_t field_11b8                 = 0x11b8;  // dword, init = 0
    static const size_t field_1200                 = 0x1200;
    static const size_t field_1264                 = 0x1264;
    static const size_t field_12c8                 = 0x12c8;
    static const size_t vtable_obj_12f0            = 0x12f0;  // dword, init = 0; owned (freed via vtable[0])
    static const size_t vtable_obj_12f4            = 0x12f4;  // dword, init = 0; owned
    static const size_t field_12f8                 = 0x12f8;
    static const size_t field_134c                 = 0x134c;  // dword, init = 0
    static const size_t flag_1350                  = 0x1350;  // byte, init = 0

    // ---- 0x1400..0x1700 block ----
    static const size_t field_143c                 = 0x143c;
    static const size_t field_1530                 = 0x1530;
    static const size_t field_1650                 = 0x1650;
    // 10-dword array of pointers, all cleared to 0 in ctor.
    // Likely a fixed-size pointer array (slots? sub-actors? buffs?)
    static const size_t array_1690                 = 0x1690;  // [10] dwords, init = 0 each
    static const size_t array_1690_end             = 0x16b8;  // = 0x1690 + 10*4

    // ---- 0x1900..0x2200 block ----
    static const size_t field_1960                 = 0x1960;
    static const size_t value_1958                 = 0x1958;  // dword, init = 0x10 (16)
    static const size_t field_2130                 = 0x2130;
    static const size_t field_21ac                 = 0x21ac;
    static const size_t field_21d4                 = 0x21d4;

    // ---- 0x2500..0x2900 block ----
    static const size_t field_25e4                 = 0x25e4;
    static const size_t field_2608                 = 0x2608;
    static const size_t field_2620                 = 0x2620;
    static const size_t field_2830                 = 0x2830;
    static const size_t field_2858                 = 0x2858;

    // ---- Tail block (0x2a00+) — many sub-objects + flags ----
    static const size_t field_2a8c                 = 0x2a8c;
    static const size_t field_2acc                 = 0x2acc;
    static const size_t field_2ae0                 = 0x2ae0;
    static const size_t subobj_2aec                = 0x2aec;  // small sub-object (dtor: FUN_00856ea0)
    static const size_t subobj_2af0                = 0x2af0;  // small sub-object (dtor: FUN_00856ea0)
    static const size_t subobj_2afc                = 0x2afc;  // small sub-object (dtor: FUN_00856df0)
    static const size_t field_2b0c                 = 0x2b0c;  // dword, init = 0
    static const size_t field_2b10                 = 0x2b10;  // dword, init = 0
    static const size_t field_2b14                 = 0x2b14;  // dword, init = 0
    static const size_t field_2b38                 = 0x2b38;  // dword, init = 0
    static const size_t field_2b3c                 = 0x2b3c;  // dword, init = 0
    static const size_t field_2b40                 = 0x2b40;  // dword, init = 0
    static const size_t field_2b44                 = 0x2b44;  // dword, init = 0
    static const size_t subobj_2b48                = 0x2b48;  // larger sub-object (dtor: FUN_00631be0)
    static const size_t vtable_obj_2b5c            = 0x2b5c;  // dword, init = 0; owned
    static const size_t subobj_2b60                = 0x2b60;  // sub-object (dtor: FUN_00855970)
    static const size_t flags_2b70                 = 0x2b70;  // dword, init = 0
                                                              // bit 0x40000 = "destruction in progress"
    static const size_t field_2b74                 = 0x2b74;  // dword, init = 0
    static const size_t vtable_obj_2b84            = 0x2b84;  // dword, init = 0; owned
    static const size_t field_2b88                 = 0x2b88;  // dword, init = 0
    static const size_t field_2b8c                 = 0x2b8c;  // dword, init = 0
    static const size_t field_2b90                 = 0x2b90;  // dword, init = 0
    static const size_t vtable_obj_2b94            = 0x2b94;  // dword, init = 0; owned (back-ref via target.field_2b98 = this)
    static const size_t field_2b98                 = 0x2b98;  // dword, init = 0 (back-ref slot)
    static const size_t field_2b9c                 = 0x2b9c;  // dword, init = 0
    static const size_t field_2ba0                 = 0x2ba0;  // dword, init = 0 (last observed field)

}  // namespace OFFSET

}  // namespace chara_actor
}  // namespace actor
}  // namespace meteor_decomp

#endif  // METEOR_DECOMP_ACTOR_CHARA_ACTOR_H
