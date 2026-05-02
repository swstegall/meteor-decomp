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
// MSVC 2005 CRT — `memset`. Hand-tuned asm that broadcasts the byte
// value to a dword and uses `rep stosd` for the bulk fill, with
// byte-alignment prologue + tail-byte epilogue. Tail-calls
// `_VEC_memset` (SSE-accelerated variant) for large zero-fills when
// the runtime SSE check is enabled. Recovered 2026-05-02 from byte
// signature vs memset.asm in <crt>/src/intel/.

extern "C" void * __cdecl _VEC_memset(void *dst, int value, unsigned count);
extern int __isa_available;
extern "C" void * __cdecl memset(void *dst, int value, unsigned count);

// MSVC treats memset as intrinsic — disable so we can define it
// from source rather than have the compiler inline its own version.
#pragma function(memset)

extern "C" {

// FUNCTION: ffxivgame 0x005d2110 — memset (122 B)

__declspec(naked) void * __cdecl memset(void *dst, int value, unsigned count) {
    __asm {
        mov     edx, [esp+0xc]
        mov     ecx, [esp+4]
        test    edx, edx
        jz      done_zero
        xor     eax, eax
        mov     al, [esp+8]
        test    al, al
        jnz     non_zero_value
        cmp     edx, 0x100
        jb      non_zero_value
        cmp     __isa_available, 0
        jz      non_zero_value
        jmp     _VEC_memset
    non_zero_value:
        push    edi
        mov     edi, ecx
        cmp     edx, 4
        jb      tail_bytes
        neg     ecx
        and     ecx, 3
        jz      aligned_main
        sub     edx, ecx
    align_loop:
        mov     [edi], al
        add     edi, 1
        sub     ecx, 1
        jnz     align_loop
    aligned_main:
        mov     ecx, eax
        shl     eax, 8
        add     eax, ecx
        mov     ecx, eax
        shl     eax, 16
        add     eax, ecx
        mov     ecx, edx
        and     edx, 3
        shr     ecx, 2
        jz      tail_bytes
        rep     stosd
        test    edx, edx
        jz      done_pop
    tail_bytes:
        mov     [edi], al
        add     edi, 1
        sub     edx, 1
        jnz     tail_bytes
    done_pop:
        mov     eax, [esp+8]
        pop     edi
        ret
    done_zero:
        mov     eax, [esp+4]
        ret
    }
}

}  // extern "C"
