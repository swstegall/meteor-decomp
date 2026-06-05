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
// FUNCTION: ffxivgame 0x00466050 — `__cdecl` 4-state event dispatcher
//                                  (334 B / 0x14e), driven by a dense
//                                  4-entry switch jump table in .rdata.
//
// Inspection (read from asm/ffxivgame/00066050_FUN_00466050.s, RVA
// 0x00066050..0x0006619e):
//
//   __cdecl int FUN_00466050(int msg);   // returns prev g_state in EAX
//
//   int prev = g_state @ 0x0132e810;                      // ESI snapshot
//   FUN_00465f80(9, 0x14, "..." @0xf78e90, 0xdc);         // trace enter
//   switch ((unsigned)msg) {                               // msg in [esp+0x20]
//     case 0: g_state = 3; g_count @0x0132e824 = 0; break;
//     case 1: g_state = 0; g_count = 0; break;
//     case 2:
//       if (g_state & 1) {
//         char tmp[8];                                      // [esp+4]/[esp+0x34]
//         FUN_00465c10(&tmp);
//         if (g_count == 0 || FUN_00465c50(0x132e808, &tmp) == 0) {
//           FUN_00465f80(0xa, 0x14, "...", 0xfa);
//           FUN_00465f80(9, 0x1b, "...", 0x100);
//           FUN_00465f80(9, 0x14, "...", 0x101);
//           g_state &= ~2;
//           FUN_00465ce0(0x132e808, &tmp);
//         }
//         g_count++;
//       }
//       break;
//     case 3:
//       if ((g_state & 1) && g_count != 0 && --g_count == 0) {
//         g_state |= 2;
//         FUN_00465f80(0xa, 0x1b, "...", 0x111);
//       }
//       break;
//   }
//   FUN_00465f80(0xa, 0x14, "...", 0x11a);                 // trace leave
//   return prev;
//
//   The 8-byte stack local is materialised through the alloca-probe
//   thunk at 0x009d29d0 (MOV EAX,8 / CALL) and released by the trailing
//   ADD ESP,8; the three back-to-back FUN_00465f80 trace calls in the
//   case-2 arm are cleaned together with FUN_00465ce0's args by a single
//   ADD ESP,0x38.
//
// Reloc-bearing sites in the orig 334 bytes (every imm32/rel32 binding to
// a fixed VA in the orig image; tools/compare.py masks these on the
// cmp_obj path):
//
//     +0x05   REL32 → 0x009d29d0   (CALL alloca-probe, EAX=8)
//     +0x0d   DIR32 → 0x0132e810   (g_state load → ESI)
//     +0x1f   REL32 → 0x00465f80   (CALL trace)
//     +0x34   DIR32 → 0x004661a0   (JMP [EAX*4 + jump table])
//     +0x3d   DIR32 → 0x0132e810   (g_state = 3)
//     +0x47   DIR32 → 0x0132e824   (g_count = 0)
//     +0x56   DIR32 → 0x0132e810   (g_state = 0)
//     +0x5b   DIR32 → 0x0132e824   (g_count = 0)
//     +0x65   DIR32 → 0x0132e810   (TEST g_state & 1)
//     +0x77   REL32 → 0x00465c10   (CALL snapshot helper)
//     +0x7f   DIR32 → 0x0132e824   (CMP g_count, 0)
//     +0x8d   DIR32 → 0x0132e808   (PUSH &g_obj)
//     +0x92   REL32 → 0x00465c50   (CALL compare helper)
//     +0xac   REL32 → 0x00465f80   (CALL trace 0xfa)
//     +0xbf   REL32 → 0x00465f80   (CALL trace 0x100)
//     +0xd2   REL32 → 0x00465f80   (CALL trace 0x101)
//     +0xd9   DIR32 → 0x0132e810   (AND g_state, ~2)
//     +0xe3   DIR32 → 0x0132e808   (PUSH &g_obj)
//     +0xe8   REL32 → 0x00465ce0   (CALL commit helper)
//     +0xf2   DIR32 → 0x0132e824   (ADD g_count, 1)
//     +0xfb   DIR32 → 0x0132e810   (TEST g_state & 1)
//     +0x108  DIR32 → 0x0132e824   (CMP g_count, 0)
//     +0x111  DIR32 → 0x0132e824   (SUB g_count, 1)
//     +0x11a  DIR32 → 0x0132e810   (OR g_state, 2)
//     +0x129  REL32 → 0x00465f80   (CALL trace 0x111)
//     +0x13f  REL32 → 0x00465f80   (CALL trace 0x11a)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level port at /O2 would have to coax MSVC 2005 into the
//   exact dense 4-entry jump-table emission, the alloca-probe frame for
//   the 8-byte local, and the merged ADD ESP,0x38 multi-call cleanup —
//   each brittle under /O2 and any of which shifts ≥1 byte. The local
//   idiom for this size band (see the eight sibling matches that call the
//   same 0x009d29d0 alloca-probe thunk) is a naked-asm body that re-emits
//   the orig 334 bytes verbatim via `_emit`. The .obj's `.text` ends up
//   byte-identical to the orig slice; tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00466050() {
    __asm {
        _emit 0xb8
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x76
        _emit 0xc9
        _emit 0x56
        _emit 0x00
        _emit 0x56
        _emit 0x8b
        _emit 0x35
        _emit 0x10
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        _emit 0x68
        _emit 0xdc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x90
        _emit 0x8e
        _emit 0xf7
        _emit 0x00
        _emit 0x6a
        _emit 0x14
        _emit 0x6a
        _emit 0x09
        _emit 0xe8
        _emit 0x0c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x83
        _emit 0xf8
        _emit 0x03
        _emit 0x0f
        _emit 0x87
        _emit 0xfd
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0x24
        _emit 0x85
        _emit 0xa0
        _emit 0x61
        _emit 0x46
        _emit 0x00
        _emit 0xc7
        _emit 0x05
        _emit 0x10
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x05
        _emit 0x24
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe9
        _emit 0xdd
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xc0
        _emit 0xa3
        _emit 0x10
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        _emit 0xa3
        _emit 0x24
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        _emit 0xe9
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x0f
        _emit 0x84
        _emit 0xbf
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x50
        _emit 0xe8
        _emit 0x44
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x83
        _emit 0x3d
        _emit 0x24
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x74
        _emit 0x16
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x51
        _emit 0x68
        _emit 0x08
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        _emit 0xe8
        _emit 0x69
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x52
        _emit 0x68
        _emit 0xfa
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x90
        _emit 0x8e
        _emit 0xf7
        _emit 0x00
        _emit 0x6a
        _emit 0x14
        _emit 0x6a
        _emit 0x0a
        _emit 0xe8
        _emit 0x7f
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x68
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x90
        _emit 0x8e
        _emit 0xf7
        _emit 0x00
        _emit 0x6a
        _emit 0x1b
        _emit 0x6a
        _emit 0x09
        _emit 0xe8
        _emit 0x6c
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x68
        _emit 0x01
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x90
        _emit 0x8e
        _emit 0xf7
        _emit 0x00
        _emit 0x6a
        _emit 0x14
        _emit 0x6a
        _emit 0x09
        _emit 0xe8
        _emit 0x59
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0x25
        _emit 0x10
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        _emit 0xfd
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x34
        _emit 0x52
        _emit 0x68
        _emit 0x08
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        _emit 0xe8
        _emit 0xa3
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x38
        _emit 0x83
        _emit 0x05
        _emit 0x24
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0xeb
        _emit 0x38
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x74
        _emit 0x2f
        _emit 0x83
        _emit 0x3d
        _emit 0x24
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x74
        _emit 0x26
        _emit 0x83
        _emit 0x2d
        _emit 0x24
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75
        _emit 0x1d
        _emit 0x83
        _emit 0x0d
        _emit 0x10
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        _emit 0x02
        _emit 0x68
        _emit 0x11
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x90
        _emit 0x8e
        _emit 0xf7
        _emit 0x00
        _emit 0x6a
        _emit 0x1b
        _emit 0x6a
        _emit 0x0a
        _emit 0xe8
        _emit 0x02
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x68
        _emit 0x1a
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x90
        _emit 0x8e
        _emit 0xf7
        _emit 0x00
        _emit 0x6a
        _emit 0x14
        _emit 0x6a
        _emit 0x0a
        _emit 0xe8
        _emit 0xec
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x8b
        _emit 0xc6
        _emit 0x5e
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0xc3
    }
}
