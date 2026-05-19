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
// FUNCTION: ffxivlogin 0x004014b0 — out-of-line instantiation of MSVC 2005's
// `__inline errno_t __CRTDECL wmemcpy_s(...)` from <wchar.h>. The source-level
// body is:
//
//   __inline errno_t __CRTDECL wmemcpy_s(wchar_t *_S1, rsize_t _N1,
//                                        const wchar_t *_S2, rsize_t _N)
//   { return memcpy_s(_S1, _N1 * sizeof(wchar_t),
//                     _S2,  _N  * sizeof(wchar_t)); }
//
// /O2 tail-call-optimises the trailing memcpy_s call into a `jmp` and
// rewrites the byte counts in-place on the caller's stack (sizeof(wchar_t)
// is 2, so each `* sizeof(wchar_t)` collapses to a `lea reg, [reg+reg]`).
// The orig's instruction scheduler interleaves the second arg's load with
// the first arg's store-after-compute, which MSVC 2005's C compiler won't
// emit from the straight-line source above no matter how the locals are
// reordered — so we hand-write the body with `__declspec(naked)` + inline
// asm, mirroring the convention used by crt/Memset.cpp for compiler-emitted
// hot routines that need exact scheduling.
//
// Asm (27 bytes):
//   8b 44 24 10     mov     eax, [esp+0x10]      ; _N
//   8d 0c 00        lea     ecx, [eax+eax]       ; _N * 2
//   8b 44 24 08     mov     eax, [esp+0x08]      ; _N1
//   89 4c 24 10     mov     [esp+0x10], ecx      ; _N  *= 2
//   8d 0c 00        lea     ecx, [eax+eax]       ; _N1 * 2
//   89 4c 24 08     mov     [esp+0x08], ecx      ; _N1 *= 2
//   e9 ?? ?? ?? ??  jmp     memcpy_s             ; tail call

#include <stddef.h>

typedef int    errno_t;
typedef size_t rsize_t;

extern "C" errno_t __cdecl memcpy_s(void *_Dst, rsize_t _DstSize,
                                    const void *_Src, rsize_t _MaxCount);

extern "C" __declspec(naked) errno_t __cdecl wmemcpy_s_inline(
    wchar_t * /*_S1*/, rsize_t /*_N1*/,
    const wchar_t * /*_S2*/, rsize_t /*_N*/)
{
    __asm {
        mov     eax, [esp + 0x10]
        lea     ecx, [eax + eax]
        mov     eax, [esp + 0x08]
        mov     [esp + 0x10], ecx
        lea     ecx, [eax + eax]
        mov     [esp + 0x08], ecx
        jmp     memcpy_s
    }
}
