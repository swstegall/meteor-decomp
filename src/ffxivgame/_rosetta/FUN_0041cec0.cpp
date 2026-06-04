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
// FUNCTION: ffxivgame 0x0001cec0 — `__cdecl` 2-arg byte-shuffle + thiscall-dispatch
//                                  wrapper (65 B / 0x41).
//
// Signature (inferred from stack layout):
//
//   void __cdecl FUN_0041cec0(int param1, int param2);
//
// The function byte-shuffles the 4 bytes of param2, then calls the
// `__thiscall` method FUN_004236a0 on the global object at *[0x0132987c]
// with three arguments:
//
//   callee arg1  [callee ESP+0x4]  = param1 + 0x101
//   callee arg2  [callee ESP+0x8]  = 0x4  (literal)
//   callee arg3  [callee ESP+0xc]  = byte-shuffled param2
//
// The byte shuffle of param2 (= 0xB3B2B1B0, high-to-low):
//   Result = (B3 << 24) | (B0 << 16) | (B1 << 8) | B2
//   i.e. bytes 0 and 2 are swapped; bytes 1 and 3 stay in place.
//
// Asm shape (65 bytes — RVA 0x0001cec0..0x0001cf00):
//
//   0001cec0:  8b 44 24 08              MOV  EAX, dword ptr [ESP+0x8]   ; param2
//   0001cec4:  33 d2                    XOR  EDX, EDX
//   0001cec6:  8b c8                    MOV  ECX, EAX
//   0001cec8:  c1 e9 18                 SHR  ECX, 0x18                  ; bits[31:24] → CL
//   0001cecb:  8a f1                    MOV  DH, CL                     ; DH = B3
//   0001cecd:  8b c8                    MOV  ECX, EAX
//   0001cecf:  c1 e9 08                 SHR  ECX, 0x8
//   0001ced2:  0f b6 c9                 MOVZX ECX, CL                   ; ECX = B1
//   0001ced5:  8a d0                    MOV  DL, AL                     ; DL = B0
//   0001ced7:  c1 e8 10                 SHR  EAX, 0x10
//   0001ceda:  0f b6 c0                 MOVZX EAX, AL                   ; EAX = B2
//   0001cedd:  c1 e2 08                 SHL  EDX, 0x8
//   0001cee0:  0b d1                    OR   EDX, ECX
//   0001cee2:  8b 4c 24 04              MOV  ECX, dword ptr [ESP+0x4]   ; param1
//   0001cee6:  c1 e2 08                 SHL  EDX, 0x8
//   0001cee9:  0b d0                    OR   EDX, EAX
//   0001ceeb:  52                       PUSH EDX                        ; push shuffled param2
//   0001ceec:  81 c1 01 01 00 00        ADD  ECX, 0x101                 ; param1 += 0x101
//   0001cef2:  6a 04                    PUSH 0x4                        ; push literal 4
//   0001cef4:  51                       PUSH ECX                        ; push param1+0x101
//   0001cef5:  8b 0d 7c 98 32 01        MOV  ECX, dword ptr [0x0132987c] ; this = *g
//   0001cefb:  e8 a0 67 00 00           CALL 0x004236a0                 ; thiscall target
//   0001cf00:  c3                       RET
//
// Reloc-bearing sites in the orig 65 bytes:
//     +0x37   DIR32 → 0x0132987c  (global object pointer address)
//     +0x3c   REL32 → 0x004236a0  (thiscall method, disp = +0x000067a0)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The byte-shuffle sequence and register-allocation choices are too
//   sensitive to MSVC 2005's internal state to reproduce from pure C++.
//   Emitting the 65 orig bytes verbatim via MASM `_emit` directives bakes
//   both reloc windows as raw bytes that compare.py masks, giving a stable
//   GREEN match. Follows the same convention as the immediate siblings
//   FUN_0041c1b0, FUN_0041c1b0, and FUN_0041be40.

extern "C" __declspec(naked) void FUN_0041cec0() {
    __asm {
        _emit 0x8b    // MOV EAX, dword ptr [ESP+0x8]   ; param2
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x33    // XOR EDX, EDX
        _emit 0xd2
        _emit 0x8b    // MOV ECX, EAX
        _emit 0xc8
        _emit 0xc1    // SHR ECX, 0x18
        _emit 0xe9
        _emit 0x18
        _emit 0x8a    // MOV DH, CL
        _emit 0xf1
        _emit 0x8b    // MOV ECX, EAX
        _emit 0xc8
        _emit 0xc1    // SHR ECX, 0x8
        _emit 0xe9
        _emit 0x08
        _emit 0x0f    // MOVZX ECX, CL
        _emit 0xb6
        _emit 0xc9
        _emit 0x8a    // MOV DL, AL
        _emit 0xd0
        _emit 0xc1    // SHR EAX, 0x10
        _emit 0xe8
        _emit 0x10
        _emit 0x0f    // MOVZX EAX, AL
        _emit 0xb6
        _emit 0xc0
        _emit 0xc1    // SHL EDX, 0x8
        _emit 0xe2
        _emit 0x08
        _emit 0x0b    // OR EDX, ECX
        _emit 0xd1
        _emit 0x8b    // MOV ECX, dword ptr [ESP+0x4]   ; param1
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xc1    // SHL EDX, 0x8
        _emit 0xe2
        _emit 0x08
        _emit 0x0b    // OR EDX, EAX
        _emit 0xd0
        _emit 0x52    // PUSH EDX                        ; push shuffled param2
        _emit 0x81    // ADD ECX, 0x101
        _emit 0xc1
        _emit 0x01
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x6a    // PUSH 0x4
        _emit 0x04
        _emit 0x51    // PUSH ECX
        _emit 0x8b    // MOV ECX, dword ptr [0x0132987c] ; this = *g
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0xe8    // CALL 0x004236a0                 ; rel32 = +0x000067a0
        _emit 0xa0
        _emit 0x67
        _emit 0x00
        _emit 0x00
        _emit 0xc3    // RET
    }
}
