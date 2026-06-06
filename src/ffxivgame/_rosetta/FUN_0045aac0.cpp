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
// FUNCTION: ffxivgame 0x0005aac0 — Blowfish 16-round encryption core
//                                  (145 B / 0x91, ret 8)
//
// __thiscall void FUN_0045aac0(this, DWORD *xL, DWORD *xR):
//   ECX = this (BlowfishContext: P-array DWORD[18] at offset 0,
//               S-boxes DWORD[4][256] at offset 0x48).
//   arg1 = pointer to left half (in/out).
//   arg2 = pointer to right half (in/out).
//
// Performs the standard 16-round Blowfish encryption:
//   for i in 0..15:
//     L ^= P[i]
//     R ^= F(L)        F = (S0[b3] + S1[b2]) ^ S2[b1] + S3[b0]
//     swap(L, R)       (implicit via register assignment)
//   *xL = L ^ P[17];  *xR = R ^ P[16];
//
// Context layout inferred from code:
//   [this +  0x00 .. 0x44]  P[0..17]   DWORD[18] — round keys
//   [this +  0x48 .. 0x447] S0[0..255] DWORD[256]
//   [this + 0x448 .. 0x847] S1[0..255] DWORD[256]
//   [this + 0x848 .. 0xc47] S2[0..255] DWORD[256]
//   [this + 0xc48 .. 0x1047] S3[0..255] DWORD[256]
//
// Frame: SUB ESP,8 (2 locals) + push EBX/EBP/ESI/EDI = 24 bytes total.
//   local0 [ESP+0x10] = loop counter (initialised to 16)
//   local1 [ESP+0x14] = temp save of EDX (L ^ P[i], before F transform)
//
// Reconstruction notes:
//   The 3-byte alignment NOP at offset 0x001d (`LEA ECX,[ECX+0]` = 8D 49 00)
//   that aligns the loop head to a 16-byte boundary cannot be coaxed from
//   C++ source; it is emitted verbatim via _emit. The rest of the function
//   assembles identically from straightforward MASM operands because there
//   are no REL32 call-site relocations.

extern "C" __declspec(naked) void FUN_0045aac0() {
    __asm {
        sub     esp, 8
        mov     eax, dword ptr [esp + 0x0c]      ; EAX = arg1 (xL pointer)
        mov     edx, dword ptr [eax]              ; EDX = *xL  (initial L)
        mov     eax, dword ptr [esp + 0x10]      ; EAX = arg2 (xR pointer)
        push    ebx
        push    ebp
        push    esi
        mov     esi, dword ptr [eax]              ; ESI = *xR  (initial R)
        push    edi
        mov     edi, ecx                          ; EDI = this (P-array cursor)
        mov     dword ptr [esp + 0x10], 0x10      ; counter = 16
        ; 8D 49 00 — LEA ECX,[ECX+0] (3-byte loop-alignment NOP)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
    blowfish_round:
        xor     edx, dword ptr [edi]              ; L ^= P[i]
        add     edi, 4
        mov     eax, edx
        shr     eax, 8
        movzx   ebx, dl                           ; EBX = byte0 (LSB)
        mov     dword ptr [esp + 0x14], edx       ; save L (before F)
        movzx   ebp, al                           ; EBP = byte1
        shr     eax, 8
        mov     edx, eax
        shr     edx, 8
        and     edx, 0ffh                         ; EDX = byte3 (MSB)
        mov     edx, dword ptr [ecx + edx*4 + 0x48]    ; S0[byte3]
        movzx   eax, al                           ; EAX = byte2
        add     edx, dword ptr [ecx + eax*4 + 0x448]   ; S1[byte2]
        movzx   eax, bp                           ; EAX = byte1
        xor     edx, dword ptr [ecx + eax*4 + 0x848]   ; S2[byte1]
        movzx   eax, bx                           ; EAX = byte0
        add     edx, dword ptr [ecx + eax*4 + 0xc48]   ; S3[byte0]
        xor     edx, esi                          ; R' = F(L) ^ R
        sub     dword ptr [esp + 0x10], 1
        mov     esi, dword ptr [esp + 0x14]       ; ESI = L (implicit swap)
        jnz     blowfish_round
        ; post-loop: XOR with P[16] and P[17]
        mov     eax, dword ptr [ecx + 0x40]       ; EAX = P[16]
        mov     ecx, dword ptr [ecx + 0x44]       ; ECX = P[17]
        xor     eax, edx                          ; EAX = P[16] ^ R_final
        mov     edx, dword ptr [esp + 0x1c]       ; EDX = arg1 (xL ptr)
        pop     edi
        xor     ecx, esi                          ; ECX = P[17] ^ L_final
        pop     esi
        mov     dword ptr [edx], ecx              ; *xL = P[17] ^ L
        mov     ecx, dword ptr [esp + 0x18]       ; ECX = arg2 (xR ptr) [after 2 pops]
        pop     ebp
        mov     dword ptr [ecx], eax              ; *xR = P[16] ^ R
        pop     ebx
        add     esp, 8
        ret     8
    }
}
