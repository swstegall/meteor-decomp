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
// FUNCTION: ffxivgame 0x00091e87 (VA 0x00491e87) — modular-inverse of
//                                  argument modulo 65537 (0x10001)
//                                  (68 B / 0x44)
//
// Calling convention: __fastcall (single int argument in ECX, return in EAX).
// No stack arguments; `ret` (no `ret N`).
//
// Algorithm — extended Euclidean:
//   Computes ECX^-1 mod 0x10001 (mod 65537).
//   Returns the canonical representative in [0, 65536].
//
//   Init:
//     EBX = 0x10001  (the fixed modulus, a₀)
//     ESI = 1        (Bezout coefficient t₁)
//     EBP = 0        (Bezout coefficient t₀)
//     ECX = n        (the value whose inverse we want, b₀)
//
//   Each iteration:
//     r = a % b      (first IDIV: discard quotient, keep EDX=remainder in EDI)
//     q = (a-r) / b  (second IDIV: recompute quotient cleanly)
//     if r == 0: exit
//     t₀ -= q * t₁
//     rotate: EBX=ECX, ECX=EDI (a=b, b=r), ESI=EBP, EBP=old_ESI (swap t)
//
//   Post-loop:
//     if ESI < 0: ESI += 0x10001 (reduce into [0, 65536])
//     return ESI
//
// Disassembly (verbatim, 68 bytes at RVA 0x00091e87):
//
//   00091e87: 53                 push    ebx
//   00091e88: 55                 push    ebp
//   00091e89: 56                 push    esi
//   00091e8a: 57                 push    edi
//   00091e8b: bb 01 00 01 00     mov     ebx, 0x00010001
//   00091e90: be 01 00 00 00     mov     esi, 0x00000001
//   00091e95: 33 ed              xor     ebp, ebp
//   00091e97: 8b c3              mov     eax, ebx        ; loop_top
//   00091e99: 99                 cdq
//   00091e9a: f7 f9              idiv    ecx
//   00091e9c: 8b c3              mov     eax, ebx        ; restore for 2nd IDIV
//   00091e9e: 8b fa              mov     edi, edx        ; EDI = r = a%b
//   00091ea0: 2b c7              sub     eax, edi        ; EAX = a - r = q*b
//   00091ea2: 99                 cdq
//   00091ea3: f7 f9              idiv    ecx             ; EAX = q = (a-r)/b
//   00091ea5: 85 ff              test    edi, edi
//   00091ea7: 74 11              jz      0x00091eba      ; if r==0 exit
//   00091ea9: 0f af c6           imul    eax, esi        ; EAX = q * t₁
//   00091eac: 2b e8              sub     ebp, eax        ; t₀ -= q * t₁
//   00091eae: 8b d6              mov     edx, esi        ; save old t₁
//   00091eb0: 8b d9              mov     ebx, ecx        ; a = b
//   00091eb2: 8b f5              mov     esi, ebp        ; t₁ = new t₀
//   00091eb4: 8b cf              mov     ecx, edi        ; b = r
//   00091eb6: 8b ea              mov     ebp, edx        ; t₀ = old t₁
//   00091eb8: eb dd              jmp     0x00091e97      ; -> loop_top
//   00091eba: 85 f6              test    esi, esi        ; done:
//   00091ebc: 7d 06              jge     0x00091ec4      ; if t₁>=0 skip adjust
//   00091ebe: 81 c6 01 00 01 00  add     esi, 0x00010001 ; t₁ += 65537
//   00091ec4: 5f                 pop     edi             ; skip:
//   00091ec5: 8b c6              mov     eax, esi        ; return t₁
//   00091ec7: 5e                 pop     esi
//   00091ec8: 5d                 pop     ebp
//   00091ec9: 5b                 pop     ebx
//   00091eca: c3                 ret
//
// No relocations — all immediates are inline constants. Naked asm reproduces
// the 68 bytes verbatim; no masking required by tools/compare.py.

extern "C" __declspec(naked) void FUN_00491e87() {
    __asm {
        push    ebx
        push    ebp
        push    esi
        push    edi
        mov     ebx, 0x00010001
        mov     esi, 1
        xor     ebp, ebp
    loop_top:
        mov     eax, ebx
        cdq
        idiv    ecx
        mov     eax, ebx
        mov     edi, edx
        sub     eax, edi
        cdq
        idiv    ecx
        test    edi, edi
        je      done
        imul    eax, esi
        sub     ebp, eax
        mov     edx, esi
        mov     ebx, ecx
        mov     esi, ebp
        mov     ecx, edi
        mov     ebp, edx
        jmp     loop_top
    done:
        test    esi, esi
        jge     skip
        add     esi, 0x00010001
    skip:
        pop     edi
        mov     eax, esi
        pop     esi
        pop     ebp
        pop     ebx
        ret
    }
}
