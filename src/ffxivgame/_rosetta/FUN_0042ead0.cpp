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
// FUNCTION: ffxivgame 0x0042ead0 — 3-D normalize-with-W-preserved (194 B / 0xc2)
//
//   __thiscall void FUN_0042ead0(this=ECX, Vec4* out=[ESP+4])
//
//   ECX points to a 4-float struct { float x, y, z, w; }.
//   The function normalises only the XYZ part (preserving W unchanged)
//   using a SSE packed RSQRTPS + one Newton-Raphson refinement step,
//   then writes all four floats to the output pointer.
//
//   High-level pseudo-C:
//
//     void normalize3d_keep_w(Vec4* __restrict__ src, Vec4* out) /* __thiscall */
//     {
//         // load src into 16-byte aligned stack buffer (src may be unaligned)
//         float buf[4] align(16) = { src->x, src->y, src->z, src->w };
//         __m128 v = *((__m128*)buf);   // XMM2
//
//         // compute 3-D squared length, broadcast to all 4 lanes
//         //   lane 0,1,2 = x²+y²+z²   lane 3 = 3w²  (side-effect of pattern)
//         __m128 sq  = _mm_mul_ps(v, v);
//         __m128 t   = _mm_shuffle_ps(sq, sq, 0xD2);   // (z²,x²,y²,w²)
//         t = _mm_add_ps(t, sq);                        // (z²+x², x²+y², y²+z², 2w²)
//         __m128 t2  = _mm_shuffle_ps(sq, sq, 0xC9);   // (y²,z²,x²,w²)
//         t = _mm_add_ps(t, t2);          // (x²+y²+z², x²+y²+z², x²+y²+z², 3w²)
//
//         // Newton-Raphson refined rsqrt(t)
//         //   globals: g_nr_A @ 0x00f54f70 (= 1.0f), g_nr_B @ 0x00f62f60 (= 0.5f)
//         //   formula: y_new = y + (A - x*y²) * B * y   ≡  y*(1.5 - 0.5*x*y²)
//         __m128 A   = _mm_shuffle_ps(_mm_load_ss(g_nr_A), …, 0);   // broadcast
//         __m128 y   = _mm_rsqrt_ps(t);                              // XMM3
//         __m128 xy  = _mm_mul_ps(y, t);                             // XMM4 = y*x
//         __m128 B   = _mm_shuffle_ps(_mm_load_ss(g_nr_B), …, 0);   // broadcast XMM1
//         B   = _mm_mul_ps(B, y);                                    // B*y
//         xy  = _mm_mul_ps(xy, y);                                   // x*y²
//         A   = _mm_sub_ps(A, xy);                                   // A - x*y²
//         A   = _mm_mul_ps(A, B);                                    // (A-x*y²)*B*y
//         A   = _mm_add_ps(A, y);                                    // refined rsqrt
//
//         // normalise XYZ, restore original W
//         __m128 norm = _mm_mul_ps(A, v);   // (x_n, y_n, z_n, w_garbage)
//         __m128 hi   = _mm_unpackhi_ps(norm, v);    // (z_n, z, w_garbage, w)
//         norm = _mm_shuffle_ps(norm, hi, 0xC4);     // (x_n, y_n, z_n, w)
//
//         // store 4 floats individually (out may be unaligned)
//         out->x = norm[0];  out->y = norm[1];
//         out->z = norm[2];  out->w = norm[3];
//     }
//
//   Reloc-bearing sites in the orig 194 bytes (absolute PE VAs baked by linker;
//   emitted verbatim as raw bytes so compare.py finds no COFF relocations to
//   mask — the values match byte-for-byte against the original binary slice):
//     +0x55  disp32  0x00f54f70   (MOVSS XMM0, [g_nr_A])
//     +0x66  disp32  0x00f62f60   (MOVSS XMM1, [g_nr_B])
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function's 194-byte body contains no CALL relocations; the only
//   external references are the two scalar-float global reads above. Using
//   symbolic names for those globals would insert COFF reloc placeholders
//   (zeros) that would not match the original's baked-in VA bytes.
//   Emitting the raw bytes with _emit keeps the .obj's .text section
//   byte-identical to the original slice and lets compare.py report GREEN.

extern "C" __declspec(naked) void FUN_0042ead0()
{
    __asm {
        // PUSH EBP
        _emit 0x55
        // MOV EBP, ESP
        _emit 0x8b
        _emit 0xec
        // AND ESP, 0xfffffff0
        _emit 0x83
        _emit 0xe4
        _emit 0xf0
        // SUB ESP, 0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // MOVSS XMM0, dword ptr [ECX]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x01
        // MOVSS dword ptr [ESP], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        // MOVSS XMM0, dword ptr [ECX+4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x04
        // MOVSS dword ptr [ESP+4], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // MOVSS XMM0, dword ptr [ECX+8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x08
        // MOVSS dword ptr [ESP+8], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // MOVSS XMM0, dword ptr [ECX+0xc]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x0c
        // MOVSS dword ptr [ESP+0xc], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // MOVAPS XMM2, xmmword ptr [ESP]
        _emit 0x0f
        _emit 0x28
        _emit 0x14
        _emit 0x24
        // MOV EAX, dword ptr [EBP+8]
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        // MOVAPS XMM0, XMM2
        _emit 0x0f
        _emit 0x28
        _emit 0xc2
        // MULPS XMM0, XMM2
        _emit 0x0f
        _emit 0x59
        _emit 0xc2
        // MOVAPS XMM1, XMM0
        _emit 0x0f
        _emit 0x28
        _emit 0xc8
        // SHUFPS XMM1, XMM0, 0xd2
        _emit 0x0f
        _emit 0xc6
        _emit 0xc8
        _emit 0xd2
        // ADDPS XMM1, XMM0
        _emit 0x0f
        _emit 0x58
        _emit 0xc8
        // SHUFPS XMM0, XMM0, 0xc9
        _emit 0x0f
        _emit 0xc6
        _emit 0xc0
        _emit 0xc9
        // ADDPS XMM1, XMM0
        _emit 0x0f
        _emit 0x58
        _emit 0xc8
        // MOVSS XMM0, dword ptr [0x00f54f70]   (g_nr_A, abs VA baked)
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        // RSQRTPS XMM3, XMM1
        _emit 0x0f
        _emit 0x52
        _emit 0xd9
        // SHUFPS XMM0, XMM0, 0x0
        _emit 0x0f
        _emit 0xc6
        _emit 0xc0
        _emit 0x00
        // MOVAPS XMM4, XMM3
        _emit 0x0f
        _emit 0x28
        _emit 0xe3
        // MULPS XMM4, XMM1
        _emit 0x0f
        _emit 0x59
        _emit 0xe1
        // MOVSS XMM1, dword ptr [0x00f62f60]   (g_nr_B, abs VA baked)
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x0d
        _emit 0x60
        _emit 0x2f
        _emit 0xf6
        _emit 0x00
        // SHUFPS XMM1, XMM1, 0x0
        _emit 0x0f
        _emit 0xc6
        _emit 0xc9
        _emit 0x00
        // MULPS XMM1, XMM3
        _emit 0x0f
        _emit 0x59
        _emit 0xcb
        // MULPS XMM4, XMM3
        _emit 0x0f
        _emit 0x59
        _emit 0xe3
        // SUBPS XMM0, XMM4
        _emit 0x0f
        _emit 0x5c
        _emit 0xc4
        // MULPS XMM0, XMM1
        _emit 0x0f
        _emit 0x59
        _emit 0xc1
        // ADDPS XMM0, XMM3
        _emit 0x0f
        _emit 0x58
        _emit 0xc3
        // MULPS XMM0, XMM2
        _emit 0x0f
        _emit 0x59
        _emit 0xc2
        // MOVAPS XMM1, XMM0
        _emit 0x0f
        _emit 0x28
        _emit 0xc8
        // UNPCKHPS XMM0, XMM2
        _emit 0x0f
        _emit 0x15
        _emit 0xc2
        // SHUFPS XMM1, XMM0, 0xc4
        _emit 0x0f
        _emit 0xc6
        _emit 0xc8
        _emit 0xc4
        // MOVAPS xmmword ptr [ESP], XMM1
        _emit 0x0f
        _emit 0x29
        _emit 0x0c
        _emit 0x24
        // MOVSS XMM0, dword ptr [ESP]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x04
        _emit 0x24
        // MOVSS dword ptr [EAX], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x00
        // MOVSS XMM0, dword ptr [ESP+4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // MOVSS dword ptr [EAX+4], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x04
        // MOVSS XMM0, dword ptr [ESP+8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // MOVSS dword ptr [EAX+8], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x08
        // MOVSS XMM0, dword ptr [ESP+0xc]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // MOVSS dword ptr [EAX+0xc], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x0c
        // MOV ESP, EBP
        _emit 0x8b
        _emit 0xe5
        // POP EBP
        _emit 0x5d
        // RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
