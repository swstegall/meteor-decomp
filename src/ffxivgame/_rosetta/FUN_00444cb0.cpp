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
// FUNCTION: ffxivgame 0x00044cb0 — `__thiscall` bounds-checked element
//                                  accessor for a fixed-stride container
//                                  (59 B / 0x3b)
//
// Signature (inferred):
//   __thiscall void* FUN_00444cb0(this, int idx)
//     ECX = this
//     [ESP+4] before prologue = int idx  (1 DWORD stack arg; callee cleans via RET 4)
//
// Layout (inferred from field accesses):
//   this+0x00   (unknown / padding)
//   this+0x04   T* begin   — pointer to first element of the array
//   this+0x08   T* end     — pointer one-past-last element
//
// Each element is 0x54 (84) bytes wide.  The count is computed as
// (end - begin) / 84 using MSVC 2005's magic-multiply idiom for signed
// division by 84:
//   IMUL ECX (magic 0x30C30C31 in EAX)  →  EDX:EAX = signed product
//   SAR  EDX, 4                          →  EDX = high-half >> 4 = quotient (biased)
//   SHR  EAX, 0x1f                       →  EAX = sign bit of quotient
//   ADD  EAX, EDX                        →  EAX = corrected quotient (round-toward-zero)
//
// Control flow:
//   if (begin == NULL)           → call panic (FUN_009d22b4)
//   if ((uint)idx >= count)      → call panic (FUN_009d22b4)
//   return (byte*)begin + idx*84
//
//   The two failure paths share a single CALL site: JZ jumps directly to
//   the CALL; the unsigned-bounds JC skips it; if neither branch fires the
//   code falls through into the CALL.
//
// Return value (EAX): pointer to the idx-th element.
//
// Asm (59 bytes):
//   56                       PUSH ESI
//   8b f1                    MOV  ESI, ECX                    ; this
//   8b 46 04                 MOV  EAX, [ESI + 0x4]            ; begin
//   85 c0                    TEST EAX, EAX
//   57                       PUSH EDI
//   8b 7c 24 0c              MOV  EDI, [ESP + 0xc]            ; idx
//   74 1a                    JZ   fail                        ; begin==NULL → panic
//   8b 4e 08                 MOV  ECX, [ESI + 0x8]            ; end
//   2b c8                    SUB  ECX, EAX                    ; byte_diff = end-begin
//   b8 31 0c c3 30           MOV  EAX, 0x30c30c31             ; magic for /84
//   f7 e9                    IMUL ECX
//   c1 fa 04                 SAR  EDX, 4
//   8b c2                    MOV  EAX, EDX
//   c1 e8 1f                 SHR  EAX, 0x1f
//   03 c2                    ADD  EAX, EDX                    ; EAX = count
//   3b f8                    CMP  EDI, EAX
//   72 05                    JC   ok                          ; idx < count → ok
// fail:
//   e8 XX XX XX XX           CALL FUN_009d22b4                ; panic (reloc masked)
// ok:
//   8b c7                    MOV  EAX, EDI
//   6b c0 54                 IMUL EAX, EAX, 0x54              ; idx * 84
//   03 46 04                 ADD  EAX, [ESI + 0x4]            ; begin + idx*84
//   5f                       POP  EDI
//   5e                       POP  ESI
//   c2 04 00                 RET  0x4
//
// The CALL rel32 at the fail site carries a linker relocation;
// compare.py masks those 4 displacement bytes (reports GREEN).

extern "C" void FUN_009d22b4();   // panic / bounds-fail handler (RVA 0x5d22b4)

extern "C" __declspec(naked) void FUN_00444cb0() {
    __asm {
        push    esi
        mov     esi, ecx
        mov     eax, dword ptr [esi + 0x4]
        test    eax, eax
        push    edi
        mov     edi, dword ptr [esp + 0xc]
        jz      fail
        mov     ecx, dword ptr [esi + 0x8]
        sub     ecx, eax
        mov     eax, 0x30c30c31
        imul    ecx
        sar     edx, 4
        mov     eax, edx
        shr     eax, 0x1f
        add     eax, edx
        cmp     edi, eax
        jc      ok
    fail:
        call    FUN_009d22b4
    ok:
        mov     eax, edi
        imul    eax, eax, 0x54
        add     eax, dword ptr [esi + 0x4]
        pop     edi
        pop     esi
        ret     0x4
    }
}
