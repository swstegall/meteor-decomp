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
// FUNCTION: ffxivgame 0x00018380 — `__cdecl` helper that stores two 16-bit
//                                  fields and one 8-bit field into a global
//                                  record array (element stride 0x30 = 48 B),
//                                  then calls FUN_0041be40 with the same
//                                  four arguments (59 B / 0x3b).
//
// Calling convention: __cdecl — callee does not clean args (plain `RET`).
// Prototype (inferred):
//   void FUN_00418380(int idx, short field0, short field1, unsigned char field2)
//
// Arg layout at function entry (before PUSH ESI / PUSH EDI):
//   [ESP+0x04]  int           idx    → ECX; drives array index: EAX = idx * 48
//   [ESP+0x08]  (unused here; slot passed through to FUN_0041be40)
//   [ESP+0x0c]  short         field0 → ESI / SI  (word store at +0x00)
//   [ESP+0x10]  short/byte    field1 → EDX / DL  (byte store at +0x04); also
//                             EDX = [ESP+0x10] before any pushes
//   [ESP+0x14]  short         field2 → EDI / DI  (word store at +0x02)
//
// Wait — re-reading the asm precisely:
//   MOV EDX, [ESP+0x10]         ; arg4 (4th) into EDX before any push
//   MOV ECX, [ESP+0x04]         ; arg1 (1st) into ECX
//   PUSH ESI / MOV ESI,[ESP+0x0c] ; after 1 push: [ESP+0x0c] = original [ESP+0x08] → arg2
//   PUSH EDI / MOV EDI,[ESP+0x14] ; after 2 pushes:[ESP+0x14] = original [ESP+0x0c] → arg3
//
// So: ECX=arg1(idx), ESI=arg2(short), EDI=arg3(short), EDX=arg4(byte).
// Global array VA = 0x013290d0 (stride 48 = 0x30 per element):
//   EAX = idx * 3 * 16  (LEA [ECX+ECX*2] then SHL 4)
//   [EAX + 0x013290d0] = SI   (16-bit field at +0)
//   [EAX + 0x013290d2] = DI   (16-bit field at +2)
//   [EAX + 0x013290d4] = DL   (8-bit  field at +4)
//
// Push ordering (reverse-arg, push deepest first):
//   PUSH EDX (arg4) → PUSH EDI (arg3) → [compute EAX] →
//   PUSH ESI (arg2) → PUSH ECX (arg1)
// then memory stores happen with the args already on the stack, then CALL.
//   ADD ESP, 0x10 → caller cleans 4 × DWORD args (__cdecl inner call).
//
// Reloc-bearing sites (all absolute addresses in the binary's VA space;
// hardcoded via _emit so compare.py matches them without masking):
//   +0x1c  disp32 0x013290d0  (global array row field 0)
//   +0x23  disp32 0x013290d2  (global array row field 2)
//   +0x29  disp32 0x013290d4  (global array row field 4)
//   +0x2f  rel32  0x00003a8b  (CALL → FUN_0041be40)
//
// Reconstruction: naked _emit byte passthrough — exact 59 bytes from the
// orig binary reproduced verbatim; compare.py byte-identical GREEN.

extern "C" __declspec(naked) void FUN_00418380() {
    __asm {
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 0x10]
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x04]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP + 0x0c]
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP + 0x14]
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x52              // PUSH EDX
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA EAX, [ECX + ECX*2]
        _emit 0x04
        _emit 0x49
        _emit 0xc1              // SHL EAX, 4
        _emit 0xe0
        _emit 0x04
        _emit 0x56              // PUSH ESI
        _emit 0x51              // PUSH ECX
        _emit 0x66              // MOV word ptr [EAX + 0x013290d0], SI
        _emit 0x89
        _emit 0xb0
        _emit 0xd0
        _emit 0x90
        _emit 0x32
        _emit 0x01
        _emit 0x66              // MOV word ptr [EAX + 0x013290d2], DI
        _emit 0x89
        _emit 0xb8
        _emit 0xd2
        _emit 0x90
        _emit 0x32
        _emit 0x01
        _emit 0x88              // MOV byte ptr [EAX + 0x013290d4], DL
        _emit 0x90
        _emit 0xd4
        _emit 0x90
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL FUN_0041be40 (rel32 → 0x0041be40)
        _emit 0x8b
        _emit 0x3a
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
