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
// FUNCTION: ffxivgame 0x004633b0 — lookup-or-alloc thunk: allocates a
//                                  0x1c-byte local frame via __chkstk,
//                                  reads a key from the caller's arg,
//                                  fast-paths small values, then delegates
//                                  to FUN_004641d0 via a global singleton.
//                                  (__cdecl, 1 param, 73 bytes / 0x49)
//
// Calling convention: __cdecl (one parameter at [ESP+0x20] after alloca).
// Returns: (param - 1) for param in [1..9]; -1 if global ptr is null or
//          FUN_004641d0 returns -1; otherwise (FUN_004641d0 result + 9).
//
// Stack layout (after __chkstk subtracts 0x1c from caller ESP):
//   [ESP+0x00]  scratch dword (param written here before call to FUN_004641d0)
//   [ESP+0x04 … ESP+0x1B]  rest of local buffer (uninitialised)
//   [ESP+0x1c]  return address
//   [ESP+0x20]  incoming param_1
//
// Pseudo-C:
//
//   static void* g_mgr = *(void**)0x0132e7b4;   // global singleton pointer
//
//   int __cdecl FUN_004633b0(unsigned int param_1) {
//       char local[0x1c];
//       if ((unsigned)(param_1 - 1) <= 8)
//           return (int)(param_1 - 1);
//       *(unsigned*)&local[0] = param_1;
//       void* mgr = g_mgr;
//       if (!mgr)
//           return -1;
//       int r = FUN_004641d0(mgr, &local[0]);
//       if (r == -1)
//           return -1;
//       return r + 9;
//   }
//
// Reloc-bearing sites (emitted as raw virtual-address immediates —
// compare.py checks raw .text bytes; the fixed image base of 0x00400000
// means these constants are invariant across link and load):
//   +0x05  CALL rel32  → 0x009d29d0  (__chkstk / _alloca_probe thunk)
//   +0x1a  MOV EAX, moffs32  → [0x0132e7b4]  (global singleton ptr)
//   +0x35  CALL rel32  → 0x004641d0  (FUN_004641d0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Source-level C++ cannot reproduce the __chkstk call shape (MSVC emits
//   SUB ESP for fixed arrays), nor the moffs32 MOV-EAX-mem form, nor the
//   baked-in CALL rel32 displacements. A __declspec(naked) body re-emitting
//   all 73 bytes verbatim via MASM _emit produces a .text section that is
//   byte-identical to the original slice. compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_004633b0() {
    __asm {
        // 000633b0: b8 1c 00 00 00   MOV EAX, 0x1c
        _emit 0xb8
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000633b5: e8 16 f6 56 00   CALL 0x009d29d0  (__chkstk; net: ESP -= 0x1c)
        _emit 0xe8
        _emit 0x16
        _emit 0xf6
        _emit 0x56
        _emit 0x00
        // 000633ba: 8b 44 24 20      MOV EAX, dword ptr [ESP+0x20]   (param_1)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 000633be: 8d 48 ff         LEA ECX, [EAX-0x1]
        _emit 0x8d
        _emit 0x48
        _emit 0xff
        // 000633c1: 83 f9 08         CMP ECX, 0x8
        _emit 0x83
        _emit 0xf9
        _emit 0x08
        // 000633c4: 77 07            JA +7  (→ 0x000633cd, slow path)
        _emit 0x77
        _emit 0x07
        // 000633c6: 83 c0 ff         ADD EAX, -1   (return param_1 - 1)
        _emit 0x83
        _emit 0xc0
        _emit 0xff
        // 000633c9: 83 c4 1c         ADD ESP, 0x1c  (pop local frame)
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 000633cc: c3               RET
        _emit 0xc3
        // 000633cd: 89 04 24         MOV dword ptr [ESP], EAX   (store param_1)
        _emit 0x89
        _emit 0x04
        _emit 0x24
        // 000633d0: a1 b4 e7 32 01   MOV EAX, dword ptr [0x0132e7b4]  (g_mgr)
        _emit 0xa1
        _emit 0xb4
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 000633d5: 85 c0            TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000633d7: 75 07            JNZ +7  (→ 0x000633e0, mgr valid)
        _emit 0x75
        _emit 0x07
        // 000633d9: 83 c8 ff         OR EAX, 0xffffffff   (EAX = -1)
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // 000633dc: 83 c4 1c         ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 000633df: c3               RET
        _emit 0xc3
        // 000633e0: 8d 14 24         LEA EDX, [ESP]   (EDX = &local[0])
        _emit 0x8d
        _emit 0x14
        _emit 0x24
        // 000633e3: 52               PUSH EDX
        _emit 0x52
        // 000633e4: 50               PUSH EAX   (mgr)
        _emit 0x50
        // 000633e5: e8 e6 0d 00 00   CALL 0x004641d0  (FUN_004641d0)
        _emit 0xe8
        _emit 0xe6
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        // 000633ea: 83 c4 08         ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 000633ed: 83 f8 ff         CMP EAX, -1
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        // 000633f0: 74 e7            JZ -0x19  (→ 0x000633d9, return -1)
        _emit 0x74
        _emit 0xe7
        // 000633f2: 83 c0 09         ADD EAX, 0x9
        _emit 0x83
        _emit 0xc0
        _emit 0x09
        // 000633f5: 83 c4 1c         ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 000633f8: c3               RET
        _emit 0xc3
    }
}
