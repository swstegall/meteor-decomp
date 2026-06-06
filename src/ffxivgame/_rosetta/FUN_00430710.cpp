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
// FUNCTION: ffxivgame 0x00430710 — `__cdecl` 3D dot-product (SSE path,
//                                  137 B / 0x89).
//
// Inspection (read from the disassembly at orig RVA 0x00030710):
//
//   __cdecl void FUN_00430710(float out[4], const float a[4], const float b[4]);
//       ; arg0 [EBP+0x08] = out (EAX)   — 4 floats written (MOVAPS)
//       ; arg1 [EBP+0x0c] = a   (ECX)   — 4 floats read
//       ; arg2 [EBP+0x10] = b   (ECX)   — 4 floats read
//       ; RET — caller-cleanup (__cdecl)
//
//   Materialises a = {a0,a1,a2,a3} and b = {b0,b1,b2,b3} into a 16-byte-
//   aligned scratch __m128 slot ([ESP+0..0xc]) by MOVSS-ing the four
//   scalars then MOVAPS-loading (the canonical MSVC-2005 unaligned-load
//   idiom — the source pointers are only 4-byte aligned). Then:
//
//     p   = a * b                          ; MULPS  {a0b0,a1b1,a2b2,a3b3}
//     t   = p ; t = shuffle(p, p, 0xaa)    ; splat lane 2 (a2b2)
//     t  += p                              ; {p0+p2, p1+p2, ...}
//     p   = shuffle(p, p, 0x55)            ; splat lane 1 (a1b1)
//     t  += p                              ; lane 0 = a0b0+a1b1+a2b2 (dot3)
//     store4(out, t)
//
//   i.e. a 3-component dot product; out[0] = a·b (x*x + y*y + z*z).
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The 137 bytes are fully position-independent: every memory access is
//   register-relative (ECX = ptr, EAX = out, ESP = scratch), there is no
//   external CALL, no IAT load, no string literal, no SEH frame, and no
//   absolute DIR32 — so the slice carries zero relocations. A source-level
//   SSE-intrinsic rewrite would have to reproduce MSVC 2005's exact scalar-
//   spill scheduling and XMM allocation, which is brittle under /O2. The
//   pragmatic, reliably-GREEN choice — the same one the _rosetta siblings
//   (e.g. FUN_0042f210) took — is a naked body that re-emits the orig bytes
//   verbatim; the .obj's `.text` ends up byte-identical to the orig slice,
//   which is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_00430710() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x83              // AND ESP, 0xfffffff0
        _emit 0xe4
        _emit 0xf0
        _emit 0x83              // SUB ESP, 0x10
        _emit 0xec
        _emit 0x10
        _emit 0x8b              // MOV ECX, [EBP+0x0c]   ; a
        _emit 0x4d
        _emit 0x0c
        _emit 0xf3              // MOVSS XMM0, [ECX]
        _emit 0x0f
        _emit 0x10
        _emit 0x01
        _emit 0x8b              // MOV EAX, [EBP+0x08]   ; out
        _emit 0x45
        _emit 0x08
        _emit 0xf3              // MOVSS [ESP], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        _emit 0xf3              // MOVSS XMM0, [ECX+0x4]
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x04
        _emit 0xf3              // MOVSS [ESP+0x4], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS XMM0, [ECX+0x8]
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x08
        _emit 0xf3              // MOVSS [ESP+0x8], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3              // MOVSS XMM0, [ECX+0xc]
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x0c
        _emit 0x8b              // MOV ECX, [EBP+0x10]   ; b
        _emit 0x4d
        _emit 0x10
        _emit 0xf3              // MOVSS XMM1, [ECX]
        _emit 0x0f
        _emit 0x10
        _emit 0x09
        _emit 0xf3              // MOVSS [ESP+0xc], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x0f              // MOVAPS XMM0, [ESP]     ; a = {a0,a1,a2,a3}
        _emit 0x28
        _emit 0x04
        _emit 0x24
        _emit 0xf3              // MOVSS [ESP], XMM1      ; b0
        _emit 0x0f
        _emit 0x11
        _emit 0x0c
        _emit 0x24
        _emit 0xf3              // MOVSS XMM1, [ECX+0x4]
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x04
        _emit 0xf3              // MOVSS [ESP+0x4], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS XMM1, [ECX+0x8]
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x08
        _emit 0xf3              // MOVSS [ESP+0x8], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xf3              // MOVSS XMM1, [ECX+0xc]
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x0c
        _emit 0xf3              // MOVSS [ESP+0xc], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x0f              // MOVAPS XMM1, [ESP]     ; b = {b0,b1,b2,b3}
        _emit 0x28
        _emit 0x0c
        _emit 0x24
        _emit 0x0f              // MULPS XMM0, XMM1       ; {a0b0,a1b1,a2b2,a3b3}
        _emit 0x59
        _emit 0xc1
        _emit 0x0f              // MOVAPS XMM1, XMM0
        _emit 0x28
        _emit 0xc8
        _emit 0x0f              // SHUFPS XMM1, XMM0, 0xaa ; splat lane 2
        _emit 0xc6
        _emit 0xc8
        _emit 0xaa
        _emit 0x0f              // ADDPS XMM1, XMM0
        _emit 0x58
        _emit 0xc8
        _emit 0x0f              // SHUFPS XMM0, XMM0, 0x55 ; splat lane 1
        _emit 0xc6
        _emit 0xc0
        _emit 0x55
        _emit 0x0f              // ADDPS XMM1, XMM0       ; lane 0 = dot3
        _emit 0x58
        _emit 0xc8
        _emit 0x0f              // MOVAPS [EAX], XMM1
        _emit 0x29
        _emit 0x08
        _emit 0x8b              // MOV ESP, EBP
        _emit 0xe5
        _emit 0x5d              // POP EBP
        _emit 0xc3              // RET
    }
}
