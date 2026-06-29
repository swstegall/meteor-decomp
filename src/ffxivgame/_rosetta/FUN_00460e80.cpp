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
// FUNCTION: ffxivgame 0x00060e80 — FUN_00460e80 (__cdecl, 175 B)
//
// Two-argument dispatch on a state-flagged object pointer.  arg2 points
// to a struct whose first DWORD is a bitmask and whose DWORD at +0x10 is
// a function pointer.  The function calls that pointer, reads the flags,
// then routes to one of several sub-handlers:
//
//   int FUN_00460e80(int **pOut, Obj *pObj) {
//       int r = pObj->fn10();          // call fn at [pObj+0x10]
//       unsigned flags = pObj->flags;  // read [pObj]
//       if (flags & 1) {
//           if (flags & 0x306) { *pOut = NULL; return 1; }
//           int r2 = pObj->fn10();
//           /* ESI = pOut (implicit) */ FUN_00460dd0(r2);
//           return 1;
//       }
//       if (flags & 0x300) { *pOut = NULL; return 1; }
//       if (flags & 0x6) {
//           int x = FUN_004640e0();
//           if (x) { *pOut = (int*)x; return 1; }
//           FUN_0045c940(0xd, 0x85, 0x41, s_f69350, 0x125);
//           return 0;
//       }
//       /* EDI = pOut (implicit) */
//       return FUN_00460bb0(flags & 0x400);  // thiscall: this = r
//   }
//
// Non-standard calling conventions used by callees:
//   FUN_00460dd0 — reads ESI as an implicit output pointer (in addition
//                  to its normal stack arg).  The caller must set ESI = pOut
//                  before the CALL.
//   FUN_00460bb0 — __thiscall (ECX = object from first fn10 call) and
//                  reads EDI as implicit output pointer.  Caller must set
//                  EDI = pOut before the CALL.
//
// Both callee conventions are irrepresentable in standard C++, so the
// whole body is emitted as naked asm.  All jump offsets are short (within
// ±127 B); the assembler computes them from the labels.
//
// Reloc-bearing sites (offsets within the function — wildcarded by compare.py):
//   +0x35   CALL rel32 → FUN_00460dd0
//   +0x62   CALL rel32 → FUN_004640e0
//   +0x6f   PUSH imm32 (DIR32)  → s_f69350
//   +0x7d   CALL rel32 → FUN_0045c940
//   +0xa4   CALL rel32 → FUN_00460bb0

extern "C" {
    // Callee functions (standard stack args; implicit register args handled
    // manually in the naked asm below).
    int  FUN_00460dd0(int);
    int  FUN_004640e0();
    void FUN_0045c940(int, int, int, void *, int);
    int  FUN_00460bb0(int);

    // String literal at VA 0x00f69350 — exact address relocated at link time;
    // compare.py wildcards the 4-byte window.
    extern char s_f69350[];
}

extern "C" __declspec(naked) void FUN_00460e80()
{
    __asm {
        push    esi
        mov     esi, dword ptr [esp + 0xc]       // ESI = arg2 (pObj)
        mov     eax, dword ptr [esi + 0x10]      // EAX = pObj->fn10
        call    eax                              // r = fn10()
        mov     edx, dword ptr [esi]             // EDX = flags
        test    dl, 0x1                          // bit 0?
        jz      bit0_clear                       // 74 32
        test    edx, 0x306                       // bits 1,2,8,9?
        jz      bit0_set_no306                   // 74 11
        mov     ecx, dword ptr [esp + 0x8]       // ECX = arg1
        mov     dword ptr [ecx], 0
        mov     eax, 1
        pop     esi
        ret
    bit0_set_no306:
        mov     edx, dword ptr [esi + 0x10]      // reload fn10
        call    edx                              // r2 = fn10()
        mov     esi, dword ptr [esp + 0x8]       // ESI = arg1 (implicit for FUN_00460dd0)
        push    eax
        call    FUN_00460dd0
        add     esp, 4
        mov     eax, 1
        pop     esi
        ret
    bit0_clear:
        test    edx, 0x300                       // bits 8,9?
        jz      no_bits89                        // 74 11
        mov     eax, dword ptr [esp + 0x8]
        mov     dword ptr [eax], 0
        mov     eax, 1
        pop     esi
        ret
    no_bits89:
        test    dl, 0x6                          // bits 1,2?
        jz      no_bits12                        // 74 35
        call    FUN_004640e0
        test    eax, eax
        jnz     fn_nonzero                       // 75 1f
        push    0x125
        push    offset s_f69350
        push    0x41
        push    0x85
        push    0xd
        call    FUN_0045c940
        add     esp, 0x14
        xor     eax, eax
        pop     esi
        ret
    fn_nonzero:
        mov     ecx, dword ptr [esp + 0x8]
        mov     dword ptr [ecx], eax
        mov     eax, 1
        pop     esi
        ret
    no_bits12:
        push    edi
        mov     edi, dword ptr [esp + 0xc]       // EDI = arg1 (implicit for FUN_00460bb0)
        and     edx, 0x400
        push    edx
        mov     ecx, eax                         // ECX = r (this for FUN_00460bb0)
        call    FUN_00460bb0
        add     esp, 4
        pop     edi
        pop     esi
        ret
    }
}
