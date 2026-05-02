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
// MSVC 2005 CRT — `strlen`. Hand-tuned 4-byte-at-a-time scan using
// the Mycroft "(x - 0x01010101) & ~x & 0x80808080" zero-byte
// detection trick. Aligns input to 4 bytes via byte-by-byte prologue,
// then loops 4 bytes per iteration in the hot path.
//
// Recovered 2026-05-02 from byte signature vs strlen.asm in
// <crt>/src/intel/.

extern "C" unsigned __cdecl strlen(const char *s);

#pragma function(strlen)

extern "C" {

// FUNCTION: ffxivgame 0x005dc3f0 — strlen (139 B)

__declspec(naked) unsigned __cdecl strlen(const char *s) {
    __asm {
        mov     ecx, [esp+4]            ; ptr
        test    ecx, 3
        jz      aligned_loop_entry
    unaligned_loop:
        mov     al, [ecx]
        add     ecx, 1
        test    al, al
        jz      done_minus_1
        test    ecx, 3
        jnz     unaligned_loop
        ; alignment NOPs to put aligned_loop_entry at 16-byte boundary
        ; 5-byte NOP — `add eax, imm32` (long form; MSVC default
        ; compresses `add eax, 0` to 3-byte `83 c0 00`)
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        ; 7-byte NOP — `lea esp, [esp+EIZ*1+0]` (full SIB+disp32 form)
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        ; another 7-byte NOP
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    aligned_loop_entry:
        mov     eax, [ecx]              ; load 4 bytes
        mov     edx, 0x7efefeff
        add     edx, eax
        xor     eax, 0xffffffff
        xor     eax, edx
        add     ecx, 4
        test    eax, 0x81010100
        jz      aligned_loop_entry
        ; zero byte found — figure out which one
        mov     eax, [ecx-4]
        test    al, al
        jz      done_minus_4
        test    ah, ah
        jz      done_minus_3
        test    eax, 0x00ff0000
        jz      done_minus_2
        test    eax, 0xff000000
        jz      done_minus_1
        jmp     aligned_loop_entry      ; false alarm — continue
    done_minus_1:
        lea     eax, [ecx-1]
        mov     ecx, [esp+4]
        sub     eax, ecx
        ret
    done_minus_2:
        lea     eax, [ecx-2]
        mov     ecx, [esp+4]
        sub     eax, ecx
        ret
    done_minus_3:
        lea     eax, [ecx-3]
        mov     ecx, [esp+4]
        sub     eax, ecx
        ret
    done_minus_4:
        lea     eax, [ecx-4]
        mov     ecx, [esp+4]
        sub     eax, ecx
        ret
    }
}

}  // extern "C"
