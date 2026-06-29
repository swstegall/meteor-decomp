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
// FUNCTION: ffxivgame 0x00045b70 — __cdecl bounded signed-byte string
//                                  comparison (strncmp variant, 58 bytes).
//
// int FUN_00445b70(const char *str1, const char *str2, unsigned int count)
//
// Compares up to `count` characters of str1 and str2 using signed byte
// arithmetic (MOVSX). Returns the signed difference of the first differing
// byte pair, or 0 if all compared bytes are equal or a null terminator is
// reached first.
//
// Calling convention: __cdecl (plain RET, caller cleans 3 DWORDs).
//
// Frame:
//   PUSH EBX            ; save EBX
//   MOV  EBX,[ESP+0x10] ; load count into EBX early (1 push at this point,
//                       ; so arg3 = ESP+0x10 after only 1 push)
//   PUSH EBP            ; save EBP (used as signed-byte temp, not frame ptr)
//   PUSH ESI            ; save ESI (str2 walking pointer)
//   PUSH EDI            ; save EDI (loop counter i)
//   [no ESP adjustment]
//
// MSVC 2005 interleaves the register save and load: EBX is loaded into
// count BEFORE EBP/ESI/EDI are saved, so the other two pointer arguments
// are only accessible at ESP+0x14 / ESP+0x18 after all four saves finish.
//
// Loop structure (do-while over i < count):
//   XOR  EDI, EDI                  ; i = 0
//   TEST EBX, EBX; JBE zero_ret   ; if count == 0 → return 0
//   MOV  ESI, [ESP+0x18]           ; str2
//   MOV  EDX, [ESP+0x14]           ; str1
// loop:
//   MOV  CL,  [EDX]                ; cl = *str1
//   MOVSX EBP, [ESI]               ; ebp = (signed char)*str2
//   MOVSX EAX, CL                  ; eax = (signed char)*str1
//   SUB  EAX, EBP                  ; eax = *str1 - *str2
//   JNZ  diff_ret                  ; if diff → return diff
//   TEST CL, CL; JZ zero_ret      ; if *str1 == 0 → return 0
//   ADD  EDI, 1; ADD EDX, 1; ADD ESI, 1
//   CMP  EDI, EBX; JC loop        ; unsigned: if i < count → loop
// zero_ret:
//   XOR  EAX, EAX                  ; return 0
// diff_ret:
//   POP EDI; POP ESI; POP EBP; POP EBX; RET

extern "C" __declspec(naked) int FUN_00445b70(const char *, const char *, unsigned int) {
    __asm {
        push    ebx
        mov     ebx, dword ptr [esp + 0x10]
        push    ebp
        push    esi
        push    edi
        xor     edi, edi
        test    ebx, ebx
        jbe     zero_ret
        mov     esi, dword ptr [esp + 0x18]
        mov     edx, dword ptr [esp + 0x14]
    loop_start:
        mov     cl, byte ptr [edx]
        movsx   ebp, byte ptr [esi]
        movsx   eax, cl
        sub     eax, ebp
        jnz     diff_ret
        test    cl, cl
        jz      zero_ret
        add     edi, 1
        add     edx, 1
        add     esi, 1
        cmp     edi, ebx
        jc      loop_start
    zero_ret:
        xor     eax, eax
    diff_ret:
        pop     edi
        pop     esi
        pop     ebp
        pop     ebx
        ret
    }
}
