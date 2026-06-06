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
// FUNCTION: ffxivgame 0x00046250 — buffer compare over the shorter length (36 B)
//
// A __thiscall method on a {data ptr @+0x0, length @+0x8} buffer object.
// Computes n = min(this->size, other->size) (unsigned) and forwards
// (this->data, other->data, n) to the __cdecl helper FUN_00445b70
// (a memcmp-style routine), returning its result.
//
// Calling convention: __thiscall (this in ECX, one stack arg, RET 4).
// Frame: none (/Oy — only callee-save ESI is used to hold `other`).
//
// Asm (36 bytes @ orig RVA 0x00046250):
//   8b 41 08        MOV EAX, [ECX+0x8]     ; this->size
//   56              PUSH ESI
//   8b 74 24 08     MOV ESI, [ESP+0x8]     ; other
//   8b 56 08        MOV EDX, [ESI+0x8]     ; other->size
//   3b c2           CMP EAX, EDX
//   72 02           JC  +2                 ; if this->size < other->size keep it
//   8b c2           MOV EAX, EDX           ; else n = other->size
//   8b 09           MOV ECX, [ECX]         ; this->data
//   50              PUSH EAX               ; n
//   8b 06           MOV EAX, [ESI]         ; other->data
//   50              PUSH EAX               ; other->data
//   51              PUSH ECX               ; this->data
//   e8 03 f9 ff ff  CALL FUN_00445b70      ; __cdecl (reloc)
//   83 c4 0c        ADD ESP, 0xc
//   5e              POP ESI
//   c2 04 00        RET 0x4

extern "C" int __cdecl FUN_00445b70(const void *a, const void *b, unsigned int n);

struct Buf_00446250 {
    const void  *data;   // +0x00
    int          unk04;  // +0x04
    unsigned int size;   // +0x08
    int FUN_00446250(const Buf_00446250 *other) const;
};

int Buf_00446250::FUN_00446250(const Buf_00446250 *other) const
{
    unsigned int n = this->size;
    if (n >= other->size)
        n = other->size;
    return FUN_00445b70(this->data, other->data, n);
}
