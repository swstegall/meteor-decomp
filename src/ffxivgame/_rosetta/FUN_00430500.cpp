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
// FUNCTION: ffxivgame 0x00030500 — __cdecl convert-int-coords-to-scaled-float-vector
//                                  149 B / 0x95, ret 0 (no stack args cleaned).
//
// __cdecl void FUN_00430500(float *out, int x, int y, int z):
//   Converts three integer coordinates (x, y, z) to floats, divides each by
//   a global scale factor loaded from VA 0xF62F68, appends w = scale/scale = 1.0,
//   and stores the resulting 4-float vector to *out.
//
//   Pseudocode:
//     float scale = *g_f62f68;
//     __m128 v = { (float)x, (float)y, (float)z, scale };
//     __m128 s = _mm_set1_ps(scale);    // SHUFPS broadcast
//     v = _mm_div_ps(v, s);             // { x/s, y/s, z/s, 1.0f }
//     *((__m64*)out)     = *(__m64*)&v;
//     *((__m64*)(out+2)) = *(__m64*)(&v+1);
//
//   The SSE1/SSE2 code uses a 16-byte-aligned frame (AND ESP,0xFFFFFFF0 +
//   SUB ESP,0x20) to satisfy MOVAPS alignment requirements.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function is a compact SSE sequence with no branches or calls. The
//   compiler generated explicit vector loads/stores and a SHUFPS broadcast
//   idiom (MOVSS xmm0,xmm0 / SHUFPS xmm0,xmm0,0) that source-level
//   _mm_set1_ps() or auto-vectorisation would not reproduce byte-for-byte
//   (MSVC 2005 picks different scratch registers and shuffles the unpack
//   order depending on whether the argument is a local or an SSE intrinsic
//   argument). Naked asm is the canonical workaround for the ffxivgame
//   _rosetta tree.
//
// Reloc-bearing sites:
//   +0x20   MOVSS XMM0,[g_f62f68]  — DIR32 reloc (4 bytes of address)
//           wildcarded by tools/compare.py.

// Global scale factor at VA 0x00F62F68 (data section).
extern float g_f62f68;

extern "C" __declspec(naked) void FUN_00430500() {
    __asm {
        // --- prologue (16-byte-aligned frame) --------------------------------
        push    ebp                                 // 55
        mov     ebp, esp                            // 8b ec
        and     esp, 0xfffffff0                     // 83 e4 f0
        sub     esp, 0x20                           // 83 ec 20

        // --- convert integer args to scalar floats ---------------------------
        cvtsi2ss xmm0, dword ptr [ebp + 0xc]       // f3 0f 2a 45 0c  (float)x
        cvtsi2ss xmm1, dword ptr [ebp + 0x10]      // f3 0f 2a 4d 10  (float)y
        cvtsi2ss xmm2, dword ptr [ebp + 0x14]      // f3 0f 2a 55 14  (float)z
        mov     eax, dword ptr [ebp + 0x8]          // 8b 45 08        out ptr

        // --- build 4-float vector {x, y, z, scale} on stack -----------------
        movss   dword ptr [esp], xmm0               // f3 0f 11 04 24  [esp]   = (float)x
        movss   xmm0, dword ptr [g_f62f68]          // f3 0f 10 05 ??  xmm0   = scale
        movss   dword ptr [esp + 0xc], xmm0         // f3 0f 11 44 24 0c  [esp+C] = scale

        // --- broadcast scale to all 4 lanes ----------------------------------
        movss   xmm0, xmm0                          // f3 0f 10 c0     (zero upper 96 bits)
        shufps  xmm0, xmm0, 0                       // 0f c6 c0 00     xmm0 = {s,s,s,s}

        // --- store y and z into the vector -----------------------------------
        movss   dword ptr [esp + 0x4], xmm1         // f3 0f 11 4c 24 04  [esp+4] = (float)y
        movss   dword ptr [esp + 0x8], xmm2         // f3 0f 11 54 24 08  [esp+8] = (float)z

        // --- packed divide: {x,y,z,scale} / {s,s,s,s} = {x/s,y/s,z/s,1} ---
        movaps  xmm1, xmmword ptr [esp]             // 0f 28 0c 24
        divps   xmm1, xmm0                          // 0f 5e c8
        movaps  xmmword ptr [esp + 0x10], xmm1      // 0f 29 4c 24 10  store result

        // --- scatter result back to [esp..esp+F] for MOVQ stores -------------
        movss   xmm0, dword ptr [esp + 0x10]        // f3 0f 10 44 24 10
        movss   dword ptr [esp], xmm0               // f3 0f 11 04 24
        movss   xmm0, dword ptr [esp + 0x14]        // f3 0f 10 44 24 14
        movss   dword ptr [esp + 0x4], xmm0         // f3 0f 11 44 24 04
        movss   xmm0, dword ptr [esp + 0x18]        // f3 0f 10 44 24 18
        movss   dword ptr [esp + 0x8], xmm0         // f3 0f 11 44 24 08
        movss   xmm0, dword ptr [esp + 0x1c]        // f3 0f 10 44 24 1c
        movss   dword ptr [esp + 0xc], xmm0         // f3 0f 11 44 24 0c

        // --- write 16 bytes to *out via two 64-bit MOVQ stores ---------------
        movq    xmm0, qword ptr [esp]               // f3 0f 7e 04 24
        movq    qword ptr [eax], xmm0               // 66 0f d6 00
        movq    xmm0, qword ptr [esp + 0x8]         // f3 0f 7e 44 24 08
        movq    qword ptr [eax + 0x8], xmm0         // 66 0f d6 40 08

        // --- epilogue --------------------------------------------------------
        mov     esp, ebp                            // 8b e5
        pop     ebp                                 // 5d
        ret                                         // c3
    }
}
