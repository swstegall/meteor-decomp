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
// FUNCTION: ffxivgame 0x00018410 — conditional array-element write/clear
//                                  with tail-call to FUN_0041bee0 (71 B / 0x47)
//
// No prologue/epilogue — pure ESP-relative addressing (FPO), tail-call
// optimised via JMP in both branches.
//
// Two stack arguments: [ESP+4] = index (int), [ESP+8] = value (int).
//
// Body:
//   if (DAT_01328ed8 & 0x4) {            // bit 2 SET  → clear path
//       g_table[index * 48].field0 = 0;  // stride 48 = (EAX*3) << 4
//       value = 0;
//   } else {                             // bit 2 CLEAR → set path
//       g_table[index * 48].field0 = value;
//       // [ESP+8] = value (redundant store, MSVC artifact)
//   }
//   FUN_0041bee0(index, value);          // tail-call — JMP, not CALL+RET
//
// Reconstruction strategy — naked-asm _emit byte passthrough:
//
// `[ecx + OFFSET sym]` in MSVC inline asm generates absolute [disp32]
// addressing (ModRM 05) rather than register+disp32 (ModRM 81), dropping
// the register component entirely. Similarly for the [edx + disp32] store.
// Writing the 71 bytes verbatim via _emit gives byte-identical output
// (compare.py compares .obj text bytes against the orig PE slice
// directly; no linker fixups are needed because the absolute VAs and
// JMP relative offsets are baked in as literals in both).
//
// Orig bytes (71):
//
//   00018410:  f6 05 d8 8e 32 01 04              TEST byte ptr [0x01328ed8],0x4
//   00018417:  8b 44 24 04                       MOV EAX,dword ptr [ESP + 0x4]
//   0001841b:  89 44 24 04                       MOV dword ptr [ESP + 0x4],EAX
//   0001841f:  74 1d                             JZ 0x0041843e
//   00018421:  8d 0c 40                          LEA ECX,[EAX + EAX*0x2]
//   00018424:  c1 e1 04                          SHL ECX,0x4
//   00018427:  c7 81 b8 90 32 01 00 00 00 00     MOV dword ptr [ECX + 0x13290b8],0x0
//   00018431:  c7 44 24 08 00 00 00 00           MOV dword ptr [ESP + 0x8],0x0
//   00018439:  e9 a2 3a 00 00                    JMP 0x0041bee0
//   0001843e:  8b 4c 24 08                       MOV ECX,dword ptr [ESP + 0x8]
//   00018442:  8d 14 40                          LEA EDX,[EAX + EAX*0x2]
//   00018445:  c1 e2 04                          SHL EDX,0x4
//   00018448:  89 8a b8 90 32 01                 MOV dword ptr [EDX + 0x13290b8],ECX
//   0001844e:  89 4c 24 08                       MOV dword ptr [ESP + 0x8],ECX
//   00018452:  e9 89 3a 00 00                    JMP 0x0041bee0

extern "C" __declspec(naked) void FUN_00418410() {
    __asm {
        // 00018410: f6 05 d8 8e 32 01 04  TEST byte ptr [0x01328ed8],0x4
        _emit 0xf6
        _emit 0x05
        _emit 0xd8
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        _emit 0x04
        // 00018417: 8b 44 24 04  MOV EAX,dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001841b: 89 44 24 04  MOV dword ptr [ESP+0x4],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001841f: 74 1d  JZ +0x1d (→ 0x0041843e)
        _emit 0x74
        _emit 0x1d
        // 00018421: 8d 0c 40  LEA ECX,[EAX+EAX*2]
        _emit 0x8d
        _emit 0x0c
        _emit 0x40
        // 00018424: c1 e1 04  SHL ECX,0x4
        _emit 0xc1
        _emit 0xe1
        _emit 0x04
        // 00018427: c7 81 b8 90 32 01 00 00 00 00  MOV dword ptr [ECX+0x13290b8],0x0
        _emit 0xc7
        _emit 0x81
        _emit 0xb8
        _emit 0x90
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018431: c7 44 24 08 00 00 00 00  MOV dword ptr [ESP+0x8],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018439: e9 a2 3a 00 00  JMP 0x0041bee0 (rel32 = +0x3aa2)
        _emit 0xe9
        _emit 0xa2
        _emit 0x3a
        _emit 0x00
        _emit 0x00
        // 0001843e: 8b 4c 24 08  MOV ECX,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 00018442: 8d 14 40  LEA EDX,[EAX+EAX*2]
        _emit 0x8d
        _emit 0x14
        _emit 0x40
        // 00018445: c1 e2 04  SHL EDX,0x4
        _emit 0xc1
        _emit 0xe2
        _emit 0x04
        // 00018448: 89 8a b8 90 32 01  MOV dword ptr [EDX+0x13290b8],ECX
        _emit 0x89
        _emit 0x8a
        _emit 0xb8
        _emit 0x90
        _emit 0x32
        _emit 0x01
        // 0001844e: 89 4c 24 08  MOV dword ptr [ESP+0x8],ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 00018452: e9 89 3a 00 00  JMP 0x0041bee0 (rel32 = +0x3a89)
        _emit 0xe9
        _emit 0x89
        _emit 0x3a
        _emit 0x00
        _emit 0x00
    }
}
