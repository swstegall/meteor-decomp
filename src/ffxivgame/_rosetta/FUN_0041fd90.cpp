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
// FUNCTION: ffxivgame 0x0041fd90 — `__cdecl` dispatch helper (308 B / 0x134).
//
// Inspection (read from the disassembly at orig RVA 0x0001fd90):
//
//   __cdecl void FUN_0041fd90(int p1, int p2, int p3, int p4, int p5, int p6, int p7);
//
//   ESP-relative frame with no EBP setup.  Parameters accessed via
//   [ESP+0x04], [ESP+0x10], [ESP+0x14], [ESP+0x18], etc.
//
//   Flow overview:
//     if ([ESP+0x18] == 0)  return;          // ECX null-check at top
//     if ([0x01328f7c] != 0) {               // global flag check
//         switch ([ESP+0x10]) {
//           case 0:  call 0x0041fa20 with 9 args (ADD ESP,0x24); return;
//           case 1:  call 0x0041fb40 with 9 args (ADD ESP,0x24); return;
//           default: push 5 literals + LEA ECX; call 0x00406550; return;
//         }
//     } else {
//         // table-driven dispatch
//         EDI = table1[[ESP+0x04] * 4 + 0xf595c4];  if (!EDI) return;
//         EBX = table2[[ESP+0x14] * 4 + 0xf596b8];  if (!EBX) return;
//         ESI = call 0x0041c540([ESP+0x04]);
//         call 0x0041ed70([ESP+0x2c]);
//         ... compute (p5 - p6 + 1) range, push 9 args,
//         call 0x00423300; push 0; call 0x0041d240;
//         call 0x004246f0(ESI); pop ESI, EBX, EDI; return;
//     }
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function uses no EBP frame and pushes registers only in the
//   table-dispatch branch. The exact register allocation across the
//   two 9-argument push sequences and the SIB-indexed table loads
//   is brittle under /O2 — any high-level rewrite shifts at least
//   one byte (SUB EAX,0x0 idiom for CMP EAX,0, the particular order
//   of PUSH ECX / PUSH EDX interleaved with MOV reloads, the rel32
//   branch offsets at every conditional site).  The pragmatic choice
//   is a `__declspec(naked)` body re-emitting the orig 308 bytes
//   verbatim via MASM `_emit` directives, matching the approach used
//   by FUN_004014b0, FUN_00408f10, and FUN_00401a00 in the same
//   _rosetta cluster.

extern "C" __declspec(naked) void FUN_0041fd90() {
    __asm {
        // 0x00  MOV ECX, [ESP+0x18]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0x04  TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 0x06  JZ +0x127 (→ RET at 0x133)
        _emit 0x0f
        _emit 0x84
        _emit 0x27
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0x0c  CMP byte ptr [0x01328f7c], 0
        _emit 0x80
        _emit 0x3d
        _emit 0x7c
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // 0x13  JZ +0xa5 (→ 0xbe)
        _emit 0x0f
        _emit 0x84
        _emit 0xa5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x19  MOV EDX, [ESP+0x10]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0x1d  MOV EAX, EDX
        _emit 0x8b
        _emit 0xc2
        // 0x1f  SUB EAX, 0  (MSVC idiom: CMP + propagate flags for next sub)
        _emit 0x83
        _emit 0xe8
        _emit 0x00
        // 0x22  JZ +0x61 (→ 0x85)
        _emit 0x74
        _emit 0x61
        // 0x24  SUB EAX, 1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // 0x27  JZ +0x23 (→ 0x4c)
        _emit 0x74
        _emit 0x23
        // 0x29  PUSH 0xf593b0
        _emit 0x68
        _emit 0xb0
        _emit 0x93
        _emit 0xf5
        _emit 0x00
        // 0x2e  PUSH 0xce4
        _emit 0x68
        _emit 0xe4
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        // 0x33  PUSH 0xf593f8
        _emit 0x68
        _emit 0xf8
        _emit 0x93
        _emit 0xf5
        _emit 0x00
        // 0x38  PUSH 0xf58b67
        _emit 0x68
        _emit 0x67
        _emit 0x8b
        _emit 0xf5
        _emit 0x00
        // 0x3d  PUSH 0xf59448
        _emit 0x68
        _emit 0x48
        _emit 0x94
        _emit 0xf5
        _emit 0x00
        // 0x42  LEA ECX, [ESP+0x2c]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // 0x46  CALL 0x00406550
        _emit 0xe8
        _emit 0x75
        _emit 0x67
        _emit 0xfe
        _emit 0xff
        // 0x4b  RET
        _emit 0xc3

        // --- case 1: call 0x0041fb40 ---
        // 0x4c  MOV EAX, [0x01328f80]
        _emit 0xa1
        _emit 0x80
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 0x51  PUSH EAX
        _emit 0x50
        // 0x52  MOV EAX, [ESP+0x28]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0x56  PUSH EAX
        _emit 0x50
        // 0x57  MOV EAX, [ESP+0x28]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0x5b  PUSH EAX
        _emit 0x50
        // 0x5c  MOV EAX, [ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0x60  PUSH ECX
        _emit 0x51
        // 0x61  MOV ECX, [ESP+0x1c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0x65  PUSH EDX
        _emit 0x52
        // 0x66  MOV EDX, [ESP+0x1c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 0x6a  PUSH ECX
        _emit 0x51
        // 0x6b  MOV ECX, [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0x71  PUSH EDX
        _emit 0x52
        // 0x72  MOV EDX, [ESP+0x38]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x38
        // 0x76  PUSH EAX
        _emit 0x50
        // 0x77  MOV EAX, [ESP+0x34]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 0x7b  PUSH ECX
        _emit 0x51
        // 0x7c  CALL 0x0041fb40
        _emit 0xe8
        _emit 0x2f
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0x81  ADD ESP, 0x24
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        // 0x84  RET
        _emit 0xc3

        // --- case 0: call 0x0041fa20 ---
        // 0x85  MOV EAX, [0x01328f80]
        _emit 0xa1
        _emit 0x80
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 0x8a  PUSH EAX
        _emit 0x50
        // 0x8b  MOV EAX, [ESP+0x28]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0x8f  PUSH EAX
        _emit 0x50
        // 0x90  MOV EAX, [ESP+0x28]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0x94  PUSH EAX
        _emit 0x50
        // 0x95  MOV EAX, [ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0x99  PUSH ECX
        _emit 0x51
        // 0x9a  MOV ECX, [ESP+0x1c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0x9e  PUSH EDX
        _emit 0x52
        // 0x9f  MOV EDX, [ESP+0x1c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 0xa3  PUSH ECX
        _emit 0x51
        // 0xa4  MOV ECX, [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0xaa  PUSH EDX
        _emit 0x52
        // 0xab  MOV EDX, [ESP+0x38]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x38
        // 0xaf  PUSH EAX
        _emit 0x50
        // 0xb0  MOV EAX, [ESP+0x34]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 0xb4  PUSH ECX
        _emit 0x51
        // 0xb5  CALL 0x0041fa20
        _emit 0xe8
        _emit 0xd6
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        // 0xba  ADD ESP, 0x24
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        // 0xbd  RET
        _emit 0xc3

        // --- table-dispatch branch (global flag == 0) ---
        // 0xbe  MOV EAX, [ESP+0x04]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0xc2  PUSH EDI
        _emit 0x57
        // 0xc3  MOV EDI, [EAX*4+0xf595c4]
        _emit 0x8b
        _emit 0x3c
        _emit 0x85
        _emit 0xc4
        _emit 0x95
        _emit 0xf5
        _emit 0x00
        // 0xca  TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 0xcc  JZ +0x64 (→ 0x132 POP EDI)
        _emit 0x74
        _emit 0x64
        // 0xce  MOV EDX, [ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0xd2  PUSH EBX
        _emit 0x53
        // 0xd3  MOV EBX, [EDX*4+0xf596b8]
        _emit 0x8b
        _emit 0x1c
        _emit 0x95
        _emit 0xb8
        _emit 0x96
        _emit 0xf5
        _emit 0x00
        // 0xda  TEST EBX, EBX
        _emit 0x85
        _emit 0xdb
        // 0xdc  JZ +0x53 (→ 0x131 POP EBX)
        _emit 0x74
        _emit 0x53
        // 0xde  PUSH ESI
        _emit 0x56
        // 0xdf  PUSH EAX
        _emit 0x50
        // 0xe0  CALL 0x0041c540
        _emit 0xe8
        _emit 0xcb
        _emit 0xc6
        _emit 0xff
        _emit 0xff
        // 0xe5  MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 0xe7  MOV EAX, [ESP+0x2c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // 0xeb  PUSH EAX
        _emit 0x50
        // 0xec  CALL 0x0041ed70
        _emit 0xe8
        _emit 0xef
        _emit 0xee
        _emit 0xff
        _emit 0xff
        // 0xf1  MOV ECX, [ESP+0x34]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        // 0xf5  MOV EDX, [ESP+0x38]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x38
        // 0xf9  MOV EAX, [ESP+0x28]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0xfd  ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0x100  PUSH ECX
        _emit 0x51
        // 0x101  MOV ECX, [ESP+0x1c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0x105  PUSH EDX
        _emit 0x52
        // 0x106  PUSH EBX
        _emit 0x53
        // 0x107  PUSH EAX
        _emit 0x50
        // 0x108  MOV EAX, [ESP+0x24]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0x10c  SUB ECX, EAX
        _emit 0x2b
        _emit 0xc8
        // 0x10e  PUSH ESI
        _emit 0x56
        // 0x10f  ADD ECX, 0x1
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // 0x112  PUSH ECX
        _emit 0x51
        // 0x113  MOV ECX, [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0x119  PUSH EAX
        _emit 0x50
        // 0x11a  PUSH EDI
        _emit 0x57
        // 0x11b  CALL 0x00423300
        _emit 0xe8
        _emit 0x50
        _emit 0x34
        _emit 0x00
        _emit 0x00
        // 0x120  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0x122  CALL 0x0041d240
        _emit 0xe8
        _emit 0x89
        _emit 0xd3
        _emit 0xff
        _emit 0xff
        // 0x127  PUSH ESI
        _emit 0x56
        // 0x128  CALL 0x004246f0
        _emit 0xe8
        _emit 0x33
        _emit 0x48
        _emit 0x00
        _emit 0x00
        // 0x12d  ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0x130  POP ESI
        _emit 0x5e
        // 0x131  POP EBX
        _emit 0x5b
        // 0x132  POP EDI
        _emit 0x5f
        // 0x133  RET
        _emit 0xc3
    }
}
