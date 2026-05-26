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
// FUNCTION: ffxivgame 0x00016360 — temporal-allocator window setup (__cdecl, 28 B)
//
// Carves a 16-byte-aligned bump-allocator window out of a caller-supplied
// (buffer, size) pair. The aligned base is stashed in DAT_01328d88 and
// the residual usable size (size minus alignment padding) in DAT_01328d8c.
//
// Co-located callers:
//   - FUN_00416380 zeroes both globals (allocator reset).
//   - FUN_00c1e410 (VfxVertexInfo::VertexDeclarationData::CreateData)
//     checks DAT_01328d8c >> 3 against the requested vertex count and
//     populates the window via DAT_01328d88; on overflow it logs
//     "give me more temporal buffer.".
//
// Calling convention: __cdecl (stack args [ESP+4]=buffer, [ESP+8]=size,
// no callee-saves, plain RET). Frame: none (/Oy — function is a leaf
// with no locals).
//
// Asm (28 bytes @ orig RVA 0x00016360):
//   8b 44 24 04         MOV EAX, [ESP+0x4]          ; buffer (param_1)
//   8d 48 0f            LEA ECX, [EAX+0xF]          ; ECX = buffer + 15
//   83 e1 f0            AND ECX, 0xFFFFFFF0         ; ECX = aligned-up base
//   2b c1               SUB EAX, ECX                ; EAX = buffer - aligned
//   03 44 24 08         ADD EAX, [ESP+0x8]          ; EAX += size (param_2)
//   89 0d 88 8d 32 01   MOV [DAT_01328d88], ECX     ; store aligned base
//   a3 8c 8d 32 01      MOV [DAT_01328d8c], EAX     ; store residual size
//   c3                  RET
//
// The LEA + AND alignment idiom and the SUB / ADD reuse of EAX (rather
// than computing `size - (aligned - buffer)` directly) are MSVC 8.0 /O2
// signatures — emitting both stores last keeps the data hazards short.

extern "C" {
    void*        DAT_01328d88;   // 16-byte-aligned bump-allocator base
    unsigned int DAT_01328d8c;   // residual size after alignment padding
}

void __cdecl FUN_00416360(unsigned int buffer, unsigned int size)
{
    unsigned int aligned = (buffer + 0xFu) & 0xFFFFFFF0u;
    unsigned int delta   = (buffer - aligned) + size;
    DAT_01328d88 = reinterpret_cast<void*>(aligned);
    DAT_01328d8c = delta;
}
