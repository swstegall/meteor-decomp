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
// FUNCTION: ffxivgame 0x0043a3f0 — __thiscall format-stride query
//                                  (70 B / 0x46, zero relocs)
//
// Calling convention: __thiscall (ECX = this, two stack args, RET 8).
//
//   int SomeClass::GetStride(int a, int b)
//
//   Reads this->field8 (a format/type enum), then dispatches:
//
//     field <= 4        →  field * a * b
//     field == 0x881a   →  8 * a * b
//     field == 0x8820   →  2 * a * b
//     otherwise         →  0
//
//   The "not_simple" arm cascades two SUBs to test for 0x881a and then
//   0x8820 (= 0x881a + 6) without a jump table, since there are only two
//   special-cased values. EAX is zeroed at entry so the default-return
//   path simply falls to the shared `RET 8`.
//
// Instruction notes (all bytes fixed, zero linker fixups needed):
//
//   8b 49 08            MOV ECX, [ECX+8]         ; ECX = this->field8
//   33 c0               XOR EAX, EAX             ; EAX = 0 (default return)
//   83 f9 04            CMP ECX, 4               ; unsigned compare
//   77 0f               JA  +0x0f (not_simple)
//   0f af 4c 24 04      IMUL ECX, [ESP+4]        ; ECX *= a
//   0f af 4c 24 08      IMUL ECX, [ESP+8]        ; ECX *= b
//   8b c1               MOV EAX, ECX
//   c2 08 00            RET 8
//   81 e9 1a 88 00 00   SUB ECX, 0x881a          ; imm32 form
//   74 13               JZ  +0x13 (case_881a)
//   83 e9 06            SUB ECX, 6               ; imm8 form
//   75 1d               JNZ +0x1d (default_case)
//   8b 44 24 04         MOV EAX, [ESP+4]         ; case 0x8820: 2*a*b
//   0f af 44 24 08      IMUL EAX, [ESP+8]
//   03 c0               ADD EAX, EAX
//   c2 08 00            RET 8
//   8b 44 24 04         MOV EAX, [ESP+4]         ; case 0x881a: 8*a*b
//   0f af 44 24 08      IMUL EAX, [ESP+8]
//   03 c0               ADD EAX, EAX
//   03 c0               ADD EAX, EAX
//   03 c0               ADD EAX, EAX
//   c2 08 00            RET 8                    ; default_case (EAX=0)

extern "C" __declspec(naked) void FUN_0043a3f0() {
    __asm {
        mov     ecx, dword ptr [ecx + 0x8]
        xor     eax, eax
        cmp     ecx, 0x4
        ja      not_simple
        imul    ecx, dword ptr [esp + 0x4]
        imul    ecx, dword ptr [esp + 0x8]
        mov     eax, ecx
        ret     0x8
    not_simple:
        sub     ecx, 0x881a
        jz      case_881a
        sub     ecx, 0x6
        jnz     default_case
        mov     eax, dword ptr [esp + 0x4]
        imul    eax, dword ptr [esp + 0x8]
        add     eax, eax
        ret     0x8
    case_881a:
        mov     eax, dword ptr [esp + 0x4]
        imul    eax, dword ptr [esp + 0x8]
        add     eax, eax
        add     eax, eax
        add     eax, eax
    default_case:
        ret     0x8
    }
}
