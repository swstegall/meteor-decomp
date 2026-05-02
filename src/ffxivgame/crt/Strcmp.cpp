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
// MSVC 2005 CRT — `strcmp`. Hand-tuned 4-byte unrolled compare with
// alignment-aware prologue (handles s1 not on a 4-byte boundary by
// peeling 1 or 2 bytes first). All "different byte" exits jump to a
// single tail (sbb/shl/add) that returns -1/+1.
//
// Recovered 2026-05-02 from byte signature vs strcmp.asm in
// <crt>/src/intel/.

extern "C" int __cdecl strcmp(const char *s1, const char *s2);

#pragma function(strcmp)

extern "C" {

// FUNCTION: ffxivgame 0x005d8870 — strcmp (135 B)

__declspec(naked) int __cdecl strcmp(const char *s1, const char *s2) {
    __asm {
        mov     edx, [esp+4]            ; s1
        mov     ecx, [esp+8]            ; s2
        test    edx, 3
        jnz     unaligned
    aligned_loop:
        mov     eax, [edx]              ; load 4 bytes from s1
        cmp     al, [ecx]
        jne     diff_done
        or      al, al
        jz      done_eq
        cmp     ah, [ecx+1]
        jne     diff_done
        or      ah, ah
        jz      done_eq
        shr     eax, 16
        cmp     al, [ecx+2]
        jne     diff_done
        or      al, al
        jz      done_eq
        cmp     ah, [ecx+3]
        jne     diff_done
        add     ecx, 4
        add     edx, 4
        or      ah, ah
        jnz     aligned_loop
        mov     edi, edi                ; 2-byte NOP for alignment
    done_eq:
        xor     eax, eax
        ret
        nop                              ; 1-byte alignment NOP
    diff_done:
        sbb     eax, eax
        shl     eax, 1
        add     eax, 1
        ret
    unaligned:
        test    edx, 1
        jz      two_byte_load        ; skip 1-byte AND skip test_2_align
                                       ; (when edx already even but not
                                       ; 4-aligned, must be 2-aligned, so
                                       ; the test would always fall through)
        mov     al, [edx]
        add     edx, 1
        cmp     al, [ecx]
        jne     diff_done
        add     ecx, 1
        or      al, al
        jz      done_eq
        test    edx, 2
        jz      aligned_loop
    two_byte_load:
        mov     ax, [edx]
        add     edx, 2
        cmp     al, [ecx]
        jne     diff_done
        or      al, al
        jz      done_eq
        cmp     ah, [ecx+1]
        jne     diff_done
        or      ah, ah
        jz      done_eq
        add     ecx, 2
        jmp     aligned_loop
    }
}

}  // extern "C"
