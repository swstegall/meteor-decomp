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
// FUNCTION: ffxivgame 0x0045ad20 — SHA-1 context initialisation (52 B / 0x34)
//
// __thiscall void FUN_0045ad20(this)
//   ECX = SHA1 context pointer; no stack arguments; plain RET (callee cleans 0).
//
// SHA-1 context layout (offsets relative to ECX / `this`):
//   +0x00  uint32  H[0]      — initial hash word 0  (0x67452301)
//   +0x04  uint32  H[1]      — initial hash word 1  (0xefcdab89)
//   +0x08  uint32  H[2]      — initial hash word 2  (0x98badcfe)
//   +0x0c  uint32  H[3]      — initial hash word 3  (0x10325476)
//   +0x10  uint32  H[4]      — initial hash word 4  (0xc3d2e1f0)
//   +0x14  uint32  count[0]  — partial-block byte count word 0  (zeroed)
//   +0x18  uint32  count[1]  — partial-block byte count word 1  (zeroed)
//   +0x1c  uint32  count[2]  — partial-block byte count word 2  (zeroed)
//   +0x20..+0x5f  (64-byte message buffer — not touched here)
//   +0x60  uint32  length_lo — total message bit-length low word  (zeroed)
//   +0x64  uint32  length_hi — total message bit-length high word (zeroed)
//
// The five magic constants are the SHA-1 initial hash values from FIPS 180-4:
//   H0 = 0x67452301, H1 = 0xEFCDAB89, H2 = 0x98BADCFE,
//   H3 = 0x10325476, H4 = 0xC3D2E1F0
//
// Source shape (reconstructed):
//   void SHA1Context::Init() {
//       count[0] = count[1] = count[2] = 0;
//       H[0] = 0x67452301;  H[1] = 0xefcdab89;  H[2] = 0x98badcfe;
//       H[3] = 0x10325476;  H[4] = 0xc3d2e1f0;
//       length_lo = length_hi = 0;
//   }
//
// MSVC 2005 lowering notes:
//   - XOR EAX,EAX once to materialise 0; the same EAX value is reused for
//     all five zero-stores (count[0..2] + length_lo + length_hi).
//     The immediate-MOV stores for H[0..4] do not clobber EAX, so EAX
//     remains 0 through the entire function and is reused for the final pair.
//   - Store order follows source-code order: count[] first, then H[], then
//     length[], matching the leftmost-first evaluation of the C assignments.
//   - All [ECX+disp] encodings use disp8 (0x14, 0x18, 0x1c, 0x04, 0x08,
//     0x0c, 0x10, 0x60, 0x64 all fit in a signed byte [-128..+127]).
//   - No stack frame, no callee-saved registers, no relocations.
//
// Calling convention: __thiscall (ECX = this; no stack args; `ret` / C3).
//
// Frame: none (no push/pop, no sub esp).
//
// No REL32 or DIR32 relocations — raw inline __asm is byte-identical to
// the orig without any masking by tools/compare.py.

extern "C" __declspec(naked) void FUN_0045ad20() {
    __asm {
        xor     eax, eax
        mov     dword ptr [ecx + 0x14], eax
        mov     dword ptr [ecx + 0x18], eax
        mov     dword ptr [ecx + 0x1c], eax
        mov     dword ptr [ecx], 0x67452301
        mov     dword ptr [ecx + 0x4], 0xefcdab89
        mov     dword ptr [ecx + 0x8], 0x98badcfe
        mov     dword ptr [ecx + 0xc], 0x10325476
        mov     dword ptr [ecx + 0x10], 0xc3d2e1f0
        mov     dword ptr [ecx + 0x60], eax
        mov     dword ptr [ecx + 0x64], eax
        ret
    }
}
