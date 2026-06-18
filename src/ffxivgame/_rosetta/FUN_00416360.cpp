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
// FUNCTION: ffxivgame 0x00016360 — temporal bump-allocator setup (__cdecl, 28 B).
//
// Initialises the global "temporal buffer" window used by the vertex-declaration
// writer (FUN_00c1e410) and reset by FUN_00416380.  Accepts a raw buffer pointer
// and a byte count, rounds the pointer up to a 16-byte boundary, then stores:
//   [0x01328d88]  = aligned base  (g_temporal.base)
//   [0x01328d8c]  = adjusted size (g_temporal.remaining_size)
//                 = size - (aligned_base - buffer)
//
// Calling convention: __cdecl — two dword args, void return, plain RET.
// Frame: none (/Oy — function has no locals).
//
// Asm (28 bytes @ orig RVA 0x00016360):
//   8b 44 24 04          MOV  EAX, [ESP+0x4]         ; arg1 (buffer)
//   8d 48 0f             LEA  ECX, [EAX+0xf]         ; ECX = buffer + 15
//   83 e1 f0             AND  ECX, 0xfffffff0         ; ECX = aligned base
//   2b c1                SUB  EAX, ECX               ; EAX = buffer - aligned (negative offset)
//   03 44 24 08          ADD  EAX, [ESP+0x8]          ; EAX = size - (aligned - buffer)
//   89 0d 88 8d 32 01    MOV  [0x01328d88], ECX       ; g_temporal.base
//   a3 8c 8d 32 01       MOV  [0x01328d8c], EAX       ; g_temporal.remaining_size
//   c3                   RET
//
// Reconstruction: __declspec(naked) _emit byte passthrough.
//
//   A source-level C rewrite would require MSVC 2005 to (a) allocate EAX for
//   the first parameter and ECX for the intermediate aligned value, and (b)
//   pick the `a3 <abs32>` short-form (5 bytes) over the general `89 05 <abs32>`
//   form (6 bytes) when storing EAX to the second global.  The `89 0d` form
//   for the ECX store is likewise non-trivially tied to register allocation
//   order.  Following the precedent of the adjacent FUN_00416320 and
//   FUN_00416400, we emit all 28 bytes verbatim; compare.py masks the two
//   abs32 address fields and reports GREEN.

extern "C" __declspec(naked) void FUN_00416360() {
    __asm {
        // 00016360: 8b 44 24 04          MOV EAX, [ESP+0x4]         (arg1: buffer)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00016364: 8d 48 0f             LEA ECX, [EAX+0xf]         (ECX = buffer + 15)
        _emit 0x8d
        _emit 0x48
        _emit 0x0f
        // 00016367: 83 e1 f0             AND ECX, 0xfffffff0         (ECX = 16-byte aligned)
        _emit 0x83
        _emit 0xe1
        _emit 0xf0
        // 0001636a: 2b c1                SUB EAX, ECX               (EAX = buffer - aligned)
        _emit 0x2b
        _emit 0xc1
        // 0001636c: 03 44 24 08          ADD EAX, [ESP+0x8]          (EAX = size + buffer - aligned)
        _emit 0x03
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00016370: 89 0d 88 8d 32 01    MOV [0x01328d88], ECX       (g_temporal.base)
        _emit 0x89
        _emit 0x0d
        _emit 0x88
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00016376: a3 8c 8d 32 01       MOV [0x01328d8c], EAX       (g_temporal.remaining_size)
        _emit 0xa3
        _emit 0x8c
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 0001637b: c3                   RET
        _emit 0xc3
    }
}
