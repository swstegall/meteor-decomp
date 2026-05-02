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
// MSVC 2005 CRT — `_initterm_e`. Walks an array of init-list function
// pointers, calling each that returns int (vs `_initterm` which calls
// void-returning ones). Stops at the first non-zero return.
//
// Recovered 2026-05-02 from byte signature vs initterm.c in the MSVC
// 2005 SP1 CRT source.

typedef int (__cdecl *_PIFV)(void);

extern "C" {

// FUNCTION: ffxivgame 0x005d8ecb — _initterm_e (32 B)
//
//   56              push    esi
//   8b 74 24 08     mov     esi, [esp+8]    ; pfbegin
//   33 c0           xor     eax, eax
//   eb 0f           jmp     test
// loop:
//   85 c0           test    eax, eax
//   75 11           jnz     done
//   8b 0e           mov     ecx, [esi]
//   85 c9           test    ecx, ecx
//   74 02           jz      skip
//   ff d1           call    ecx
// skip:
//   83 c6 04        add     esi, 4
// test:
//   3b 74 24 0c     cmp     esi, [esp+0xc]
//   72 eb           jb      loop
// done:
//   5e              pop     esi
//   c3              ret

__declspec(naked) int __cdecl _initterm_e(_PIFV *pfbegin, _PIFV *pfend) {
    __asm {
        push    esi
        mov     esi, [esp+8]
        xor     eax, eax
        jmp     test_loop
    loop_body:
        test    eax, eax
        jnz     done
        mov     ecx, [esi]
        test    ecx, ecx
        jz      skip_call
        call    ecx
    skip_call:
        add     esi, 4
    test_loop:
        cmp     esi, [esp+0xc]
        jb      loop_body
    done:
        pop     esi
        ret
    }
}

}  // extern "C"
