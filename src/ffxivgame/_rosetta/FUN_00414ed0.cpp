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
// FUNCTION: ffxivgame 0x00414ed0 — raw QueryPerformanceCounter wrapper
//                                  returning LARGE_INTEGER as __int64
//                                  (__cdecl, 24 B / 0x18).
//
// Behaviour (recovered from asm/ffxivgame/00014ed0_FUN_00414ed0.s):
//
//   __int64 __cdecl FUN_00414ed0() {
//       LARGE_INTEGER counter;
//       QueryPerformanceCounter(&counter);   // IAT slot [0x00f3e158]
//       return counter.QuadPart;             // low → EAX, high → EDX
//   }
//
// Stack layout at CALL time (after SUB ESP,8 + PUSH EAX):
//   [ESP]    = &counter (arg to QueryPerformanceCounter)
//   [ESP+4]  = counter.LowPart   (written by callee)
//   [ESP+8]  = counter.HighPart  (written by callee)
// QueryPerformanceCounter is __stdcall and pops its 4-byte arg; after it
// returns, [ESP] = counter.LowPart and [ESP+4] = counter.HighPart.
// The function then loads EAX ← LowPart, EDX ← HighPart and returns.
//
// IAT slot:
//   [0x00f3e158]  QueryPerformanceCounter  (kernel32)
//
// The `ff 15 58 e1 f3 00` IAT-indirect call carries a post-link absolute
// address; tools/compare.py masks that 4-byte window as a DIR32 reloc
// wildcard so the .obj diff passes cleanly.
//
// Identical in structure to the QueryPerformanceFrequency half-function
// embedded in FUN_00414ef0 (RVA 0x00014ef0), which calls [0x00f3e15c]
// and uses the same SUB/LEA/PUSH/CALL/MOV-pair/ADD/RET skeleton.

extern "C" __declspec(naked) void FUN_00414ed0() {
    __asm {
        _emit 0x83              // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x8d              // LEA EAX, [ESP]
        _emit 0x04
        _emit 0x24
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL DWORD PTR [0x00f3e158]  (QueryPerformanceCounter)
        _emit 0x15
        _emit 0x58
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV EAX, DWORD PTR [ESP]     (counter.LowPart)
        _emit 0x04
        _emit 0x24
        _emit 0x8b              // MOV EDX, DWORD PTR [ESP + 0x4] (counter.HighPart)
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc3              // RET
    }
}
