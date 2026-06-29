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
// FUNCTION: ffxivgame 0x0003f210 — __thiscall size() returning (field_8 - field_4) / 28
//
// Small 33-byte __thiscall member: returns 0 immediately if this->field_4 is
// zero, otherwise computes the signed integer division (field_8 - field_4) / 28
// via the standard MSVC 2005 magic-multiplier sequence.
//
// Asm (33 bytes @ RVA 0x0003f210):
//   8b 41 04            MOV  EAX, dword ptr [ECX+0x4]   ; field_4
//   85 c0               TEST EAX, EAX
//   75 01               JNZ  skip_ret
//   c3                  RET                              ; return 0 (EAX == 0)
//   8b 49 08            MOV  ECX, dword ptr [ECX+0x8]   ; field_8
//   2b c8               SUB  ECX, EAX                   ; ECX = field_8 - field_4
//   b8 93 24 49 92      MOV  EAX, 0x92492493             ; magic for /28 (over-full)
//   f7 e9               IMUL ECX                        ; EDX:EAX = magic * (b-a)
//   03 d1               ADD  EDX, ECX                   ; adjust: effective mult += 2^32
//   c1 fa 04            SAR  EDX, 0x4                   ; >> 4 (total shift 36)
//   8b c2               MOV  EAX, EDX
//   c1 e8 1f            SHR  EAX, 0x1f                  ; sign bit
//   03 c2               ADD  EAX, EDX                   ; round-toward-zero correction
//   c3                  RET
//
// Calling convention: __thiscall (ECX = this, no stack args, plain RET).

extern "C" __declspec(naked) void FUN_0043f210()
{
    __asm {
        mov     eax, dword ptr [ecx + 0x4]
        test    eax, eax
        jnz     skip_ret
        ret
    skip_ret:
        mov     ecx, dword ptr [ecx + 0x8]
        sub     ecx, eax
        mov     eax, 0x92492493
        imul    ecx
        add     edx, ecx
        sar     edx, 0x4
        mov     eax, edx
        shr     eax, 0x1f
        add     eax, edx
        ret
    }
}
