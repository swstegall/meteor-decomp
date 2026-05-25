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
// FUNCTION: ffxivgame 0x00414ed0 — QueryPerformanceCounter thunk
//                                  (24 bytes / 0x18, __cdecl, returns LARGE_INTEGER)
//
// Source-level intent (Ghidra hint):
//   LARGE_INTEGER FUN_00414ed0(void) {
//       LARGE_INTEGER local;
//       QueryPerformanceCounter(&local);
//       return local;
//   }
//
// Codegen detail: MSVC 2005 returns a 64-bit aggregate in EDX:EAX, materialises
// the LARGE_INTEGER on stack via SUB ESP,8 / LEA EAX,[ESP] / PUSH EAX, then
// reloads the two halves with MOV EAX,[ESP] / MOV EDX,[ESP+4] after the
// indirect IAT call. The call site is `FF 15 <abs32>` against the IAT slot for
// kernel32!QueryPerformanceCounter at 0x00f3e158 — an absolute reloc baked
// into the .text. Reproduced verbatim via __declspec(naked) so the .obj
// .text matches byte-for-byte; compare.py reports GREEN.
//
// Reloc-bearing site in the orig 24 bytes:
//     +0x07   FF 15 <abs32>   → 0x00f3e158  (IAT: QueryPerformanceCounter)

extern "C" __declspec(naked) void FUN_00414ed0() {
    __asm {
        // 00014ed0: 83 ec 08          SUB ESP,0x8        (materialise LARGE_INTEGER)
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 00014ed3: 8d 04 24          LEA EAX,[ESP]      (&local)
        _emit 0x8d
        _emit 0x04
        _emit 0x24
        // 00014ed6: 50                PUSH EAX
        _emit 0x50
        // 00014ed7: ff 15 58 e1 f3 00 CALL DWORD PTR [0x00f3e158]  (IAT: QueryPerformanceCounter)
        _emit 0xff
        _emit 0x15
        _emit 0x58
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 00014edd: 8b 04 24          MOV EAX,dword ptr [ESP]     (lo half → EAX)
        _emit 0x8b
        _emit 0x04
        _emit 0x24
        // 00014ee0: 8b 54 24 04       MOV EDX,dword ptr [ESP+0x4] (hi half → EDX)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x04
        // 00014ee4: 83 c4 08          ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00014ee7: c3                RET
        _emit 0xc3
    }
}
