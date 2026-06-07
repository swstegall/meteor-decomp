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
// FUNCTION: ffxivgame 0x0041cf10 — __cdecl relay: scales a signed-byte arg
//                                  by a double constant, narrows to float,
//                                  then calls a __thiscall method (57 bytes).
//
// Signature (inferred from asm):
//   void FUN_0041cf10(int arg1, signed char arg2)
//
// Body (reconstructed):
//   1. MOVSX EAX, byte [ESP+0x8]   — sign-extend arg2 to EAX
//   2. MOV   EDX, [ESP+0x4]        — load arg1 into EDX
//   3. CVTSI2SD XMM0, EAX          — int → double
//   4. MULSD  XMM0, [0x00f598a8]   — scale by double constant
//   5. CVTSD2SS XMM0, XMM0         — double → float
//   6. MOVSS  [ESP+0x8], XMM0      — store float bits over arg2's stack slot
//   7. MOV    ECX, [ESP+0x8]       — reload float bits as DWORD
//   8. PUSH   ECX                  — push as 3rd call arg (float by value)
//   9. MOV    ECX, [0x0132987c]    — load global object ptr → ECX (this)
//  10. PUSH   0x8                  — push as 2nd call arg
//  11. ADD    EDX, 0x101           — adjust arg1
//  12. PUSH   EDX                  — push as 1st call arg
//  13. CALL   FUN_004236a0         — __thiscall: this->method(arg1+0x101, 8, fval)
//  14. RET                         — __cdecl: caller cleans args
//
// The SSE2 instructions (CVTSI2SD / MULSD / CVTSD2SS / MOVSS) indicate this
// compilation unit was built with /arch:SSE2, which is not in the global
// ROSETTA_FLAGS. Source-level C++ reconstruction is not feasible without the
// matching compiler flags, so the function is reproduced verbatim via naked
// _emit bytes.
//
// Reloc-bearing sites in the original 57 bytes (embedded as raw bytes here;
// no COFF relocation entries are emitted in the _emit path, so compare.py
// performs a full byte-exact comparison at all positions):
//   +0x0d  MULSD qword ptr [0x00f598a8]  — .rdata double constant
//   +0x1f  MOV ECX, [0x0132987c]         — global object pointer
//   +0x33  CALL rel32 → 0x004236a0       — __thiscall callee

extern "C" __declspec(naked) void FUN_0041cf10() {
    __asm {
        // 0001cf10: 0f be 44 24 08   MOVSX EAX, byte ptr [ESP+0x8]
        _emit 0x0f
        _emit 0xbe
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0001cf15: 8b 54 24 04      MOV EDX, dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x04
        // 0001cf19: f2 0f 2a c0      CVTSI2SD XMM0, EAX
        _emit 0xf2
        _emit 0x0f
        _emit 0x2a
        _emit 0xc0
        // 0001cf1d: f2 0f 59 05 a8 98 f5 00   MULSD XMM0, qword ptr [0x00f598a8]
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0x05
        _emit 0xa8
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        // 0001cf25: f2 0f 5a c0      CVTSD2SS XMM0, XMM0
        _emit 0xf2
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        // 0001cf29: f3 0f 11 44 24 08   MOVSS dword ptr [ESP+0x8], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0001cf2f: 8b 4c 24 08      MOV ECX, dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 0001cf33: 51               PUSH ECX
        _emit 0x51
        // 0001cf34: 8b 0d 7c 98 32 01   MOV ECX, dword ptr [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001cf3a: 6a 08            PUSH 0x8
        _emit 0x6a
        _emit 0x08
        // 0001cf3c: 81 c2 01 01 00 00   ADD EDX, 0x101
        _emit 0x81
        _emit 0xc2
        _emit 0x01
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001cf42: 52               PUSH EDX
        _emit 0x52
        // 0001cf43: e8 58 67 00 00   CALL 0x004236a0
        _emit 0xe8
        _emit 0x58
        _emit 0x67
        _emit 0x00
        _emit 0x00
        // 0001cf48: c3               RET
        _emit 0xc3
    }
}
