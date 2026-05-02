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
// MSVC 2005 CRT — `strncmp`. 4-byte unrolled compare with count-aware
// loop, plus a byte-by-byte fallback for the tail. 4 byte-position
// "diff or zero" handlers each MOVZX both bytes and JMP to a shared
// SUB tail.
//
// Recovered 2026-05-02 from byte signature vs strncmp.asm in
// <crt>/src/intel/.

extern "C" int __cdecl strncmp(const char *s1, const char *s2, unsigned count);

#pragma function(strncmp)

extern "C" {

// FUNCTION: ffxivgame 0x005d5475 — strncmp (190 B)

__declspec(naked) int __cdecl strncmp(const char *s1, const char *s2, unsigned count) {
    __asm {
        push    ebp
        mov     ebp, esp
        push    ecx                                 ; local at [ebp-4] (counter)
        and     dword ptr [ebp-4], 0
        push    ebx
        mov     ebx, [ebp+0x10]                     ; count
        test    ebx, ebx
        jnz     have_count
        xor     eax, eax
        jmp     done_pop_ebx                        ; skip POP EDI — it
                                                     ; wasn't pushed yet
                                                     ; on this path
    have_count:
        cmp     ebx, 4
        push    edi
        jb      byte_loop_init
        lea     edi, [ebx-4]
        test    edi, edi
        jbe     byte_loop_init
        mov     ecx, [ebp+0xc]
        mov     eax, [ebp+8]
    main_loop:
        mov     dl, [eax]
        add     eax, 4
        add     ecx, 4
        test    dl, dl
        jz      pos0_done
        cmp     dl, [ecx-4]
        jne     pos0_done
        mov     dl, [eax-3]
        test    dl, dl
        jz      pos1_done
        cmp     dl, [ecx-3]
        jne     pos1_done
        mov     dl, [eax-2]
        test    dl, dl
        jz      pos2_done
        cmp     dl, [ecx-2]
        jne     pos2_done
        mov     dl, [eax-1]
        test    dl, dl
        jz      pos3_done
        cmp     dl, [ecx-1]
        jne     pos3_done
        add     dword ptr [ebp-4], 4
        cmp     [ebp-4], edi
        jb      main_loop
        jmp     byte_loop_check                     ; not byte_loop_init —
                                                     ; eax/ecx are already
                                                     ; advanced past the
                                                     ; unrolled portion
    pos3_done:
        movzx   eax, byte ptr [eax-1]
        movzx   ecx, byte ptr [ecx-1]
        jmp     diff_to_done
    pos2_done:
        movzx   eax, byte ptr [eax-2]
        movzx   ecx, byte ptr [ecx-2]
        jmp     diff_to_done
    pos1_done:
        movzx   eax, byte ptr [eax-3]
        movzx   ecx, byte ptr [ecx-3]
        jmp     diff_to_done
    pos0_done:
        movzx   eax, byte ptr [eax-4]
        movzx   ecx, byte ptr [ecx-4]
        jmp     diff_to_done
    byte_loop_init:
        mov     ecx, [ebp+0xc]
        mov     eax, [ebp+8]
        jmp     byte_loop_check
    byte_loop_body:
        mov     dl, [eax]
        test    dl, dl
        jz      diff_at_eax_ecx
        cmp     dl, [ecx]
        jne     diff_at_eax_ecx
        inc     eax
        inc     ecx
        inc     dword ptr [ebp-4]
    byte_loop_check:
        cmp     [ebp-4], ebx
        jb      byte_loop_body
        xor     eax, eax
    done:
        pop     edi
    done_pop_ebx:
        pop     ebx
        leave
        ret
    diff_at_eax_ecx:
        movzx   eax, byte ptr [eax]
        movzx   ecx, byte ptr [ecx]
    diff_to_done:
        sub     eax, ecx
        jmp     done
    }
}

}  // extern "C"
