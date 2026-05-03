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
// Inheritance chain (recovered 2026-05-02 by chasing chained dtors):
//
//   SQEX::CDev::Engine::Fw::SceneObject::Actor (vtable 0xc9ca94, 89 slots)
//       └── App::Scene::RaptureActor             (vtable 0xbea50c, 160 slots) [+71]
//           └── App::Scene::Actor::CDevActor     (vtable 0xbbc03c, 164 slots) [+4]
//               └── App::Scene::Actor::Chara::CharaActor (vtable 0xbc0d34, 188 slots) [+24]
//
// Layer roles:
//   - SceneObject::Actor — CDev engine's base scene object (89 slots
//     of generic engine behaviour: lifecycle, transform, draw, etc.).
//   - RaptureActor — game-application "Rapture" layer that adds
//     71 game-specific virtual hooks (the bulk of the slots).
//   - CDevActor — adds 4 slots related to Excel-table-driven
//     resource loading (CDevActorResourceEvent,
//     CDevActorSetResourceEvent, CDevActorSetResourceWithExcelEvent,
//     CDevActorExcelWaiter all live in this namespace).
//   - CharaActor — adds 24 slots specific to characters (player
//     and NPC); the slot-specific behaviours are still TBD.
//
// Sibling CDevActor subclasses (also extend CDevActor):
//   Chara::WeaponActor (165 slots), Map::BgModelActor (167),
//   Map::BgObjActor (167), Map::BgPlateActor (167),
//   Map::MapLayoutActor (160), System::CommonResourceActor (164),
//   System::GameManagerActor (164), System::BootupActor (164),
//   System::ScreenshotManagerActor (164), System::CutManagerActor
//   (164), System::CameraActor (164), System::TargetActor (160),
//   Light::LightActor (164), Effect::EffectDebugActor (164),
//   Effect::EffectActor (164), Window::WindowActor (160).
//
// vtable: RVA 0xbc0d34 (= VA 0xfc0d34), 188 slots
// ctor:   FUN_0065f180 (1942 B at file 0x25f180)
// dtor:   FUN_00666130 (968 B at file 0x266130) — wrapped by slot 0
//         (FUN_00669e20, 34 B scalar deleting destructor)
//
// Chained parent dtors (last CALL in each):
//   CharaActor::~CharaActor → CDevActor::~CDevActor      (FUN_006325e0, 117 B)
//   CDevActor::~CDevActor   → RaptureActor::~RaptureActor (FUN_007ced70, 235 B)
//   RaptureActor::~Rapture  → SceneObject::Actor::~Actor (FUN_00a60f30, 507 B)
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
//
// === Inheritance demarcation ===
//
// Bytes 0x0000..0x0118 belong to RaptureActor (the parent class)
// and its parent SceneObject::Actor. RaptureActor's ctor
// (FUN_007cef80, 376 B) writes/touches 18 field offsets in this
// range — see RAPTURE_OFFSET below. Bytes 0x0118+ are CharaActor's
// own + CDevActor's (CDevActor adds only 4 vtable slots and very
// little data — most of the 0x118..0x2ba0 range is CharaActor's
// own state).

namespace RAPTURE_OFFSET {
    // RaptureActor (vtable 0xbea50c, 160 slots, ctor FUN_007cef80,
    // dtor FUN_007ced70) — size ≥ 0x11c (284 bytes). 18 fields
    // touched in ctor+dtor.

    static const size_t vtable                     = 0x0000;  // dword, init = 0xfea50c (RaptureActor's vftable;
                                                              //   in a CharaActor instance, this slot holds
                                                              //   CharaActor's own vtable 0xfc0d34 because the
                                                              //   most-derived class wins)
    // Inline sub-object pattern: pointer at +0x90 → object body
    // at +0x94 (back-pointer linked-list node).
    static const size_t subobj_back_ptr_0090       = 0x0090;  // dword, init = ESI+0x94 (= subobj_0094 addr)
    static const size_t subobj_0094                = 0x0094;  // inline sub-object body
    static const size_t subobj_back_ptr_009c       = 0x009c;  // dword, init = ESI+0xa0
    static const size_t subobj_00a0                = 0x00a0;  // inline sub-object with vtable 0xfb7af4
    static const size_t subobj_00a4                = 0x00a4;  // inline sub-object (dtor: FUN_0080bcf0)
    static const size_t subobj_00b8                = 0x00b8;  // inline sub-object
    static const size_t subobj_00c8                = 0x00c8;  // inline sub-object
    // Small scalar fields (init = 0):
    static const size_t field_00d8                 = 0x00d8;  // dword
    static const size_t field_00dc                 = 0x00dc;  // dword
    static const size_t field_00e0                 = 0x00e0;  // dword
    static const size_t field_00e4                 = 0x00e4;
    static const size_t flag_00e8                  = 0x00e8;  // byte
    static const size_t field_00ec                 = 0x00ec;  // dword
    static const size_t field_00f0                 = 0x00f0;
    static const size_t field_0110                 = 0x0110;  // dword
    static const size_t field_0114                 = 0x0114;  // dword
    static const size_t field_0118                 = 0x0118;  // dword (last RaptureActor field — also seen
                                                              //   accessed by CharaActor's own ctor as
                                                              //   parent_or_owner)
}  // namespace RAPTURE_OFFSET

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
    // +0x1170 (init 0xED=237): settable property with dirty-bit
    //   tracking. Setter at FUN_0065aa70 (53 B): compares-with-current,
    //   on change OR's `0x400000` into flags_2b70 (dirty bit), writes
    //   new value, then optionally zeroes if `[+0x2b5c]+0x4c & 0x1`.
    //   166 callers, mostly in switch-table dispatchers, passing
    //   integer literals in the 0xC0..0xF0 (192..240) range — likely
    //   action / motion / animation / state IDs. Each setter call is
    //   typically followed by `MOV [ESI+0x5ae], <byte>` setting a
    //   correlated state byte. The init 0xED is a placeholder default,
    //   replaced from game data at load time.
    static const size_t value_1170                 = 0x1170;  // dword; default 0xED, runtime range ~0xC0..0xF0
    static const size_t field_1174                 = 0x1174;  // dword, init = 0
    // +0x1178 (init 0xC9=201): paired property to +0x1170. Setter
    //   at FUN_0065ab90 (222 B) is more elaborate — broadcasts
    //   change via callback (calls a logger/notifier with format
    //   string referencing 0x1a0-byte buffer). Likely the "secondary"
    //   or "previous" state companion to +0x1170. Init 0xC9 is also
    //   a placeholder default.
    static const size_t value_1178                 = 0x1178;  // dword; default 0xC9, paired with +0x1170
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
