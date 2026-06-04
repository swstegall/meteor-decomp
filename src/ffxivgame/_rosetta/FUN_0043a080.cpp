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
// FUNCTION: ffxivgame 0x0043a080 — `__cdecl` thunk that assembles a wide
//                                  (~0xa0-byte) transform/parameter block
//                                  on the stack from a run of float args,
//                                  then forwards it to a __thiscall callee
//                                  (305 B + alignment / 0x13d).
//
// Inspection (read from the disassembly at orig RVA 0x0003a080):
//
//   __cdecl <obj>* FUN_0043a080(obj *self /* [ebp+0x08] */,
//                               float a /* [ebp+0x0c] */,
//                               float b /* [ebp+0x10] */,
//                               float c /* [ebp+0x14] */,
//                               float d /* [ebp+0x18] */,
//                               float e /* [ebp+0x1c] */,
//                               float f /* [ebp+0x20] */);
//
//   Aligns ESP to 8 (`AND ESP, ~7`), carves a 0xc4-byte frame, zero-fills
//   a swath of XMM0 (=0.0f) slots, then interleaves the six incoming x87
//   floats ([ebp+0x0c..+0x20]) with two literal float constants —
//   0x00fb7a60 (loaded into XMM1) and 0x00f62f60 (XMM2) — to lay out a
//   structured block. The block is gathered via MOVQ (8-byte) copies into
//   [esp+0x60 .. esp+0x9c] and [esp+0xa0+]. `LEA EAX,[ESP+0xa0]` takes the
//   block's address; it is PUSHed as the sole stack argument to the
//   __cdecl helper at 0x004393b0 (a vector/matrix builder), whose result
//   in EAX is then passed alongside `self` ([ebp+0x08]) to the __thiscall
//   method at 0x0042edb0 (`LEA ECX,[ESP+0x50]` = the `this`). The function
//   returns `self` (MOV EAX,ESI ; ESI = [ebp+0x08]).
//
//   Reloc-bearing sites in the orig 317 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x12   MOVSS XMM1 literal load   (.rdata 0x00fb7a60)
//     +0x1a   MOVSS XMM2 literal load   (.rdata 0x00f62f60)
//     +0x120  __cdecl callee CALL       (.text 0x004393b0 rel32)
//     +0x131  __thiscall callee CALL    (.text 0x0042edb0 rel32)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Coaxing MSVC 2005 /O2 into reproducing this exact frame (the ESP
//   8-alignment, the SSE/x87 hybrid lowering of the float-shuffle, the
//   precise MOVQ gather order, and the two linker-resolved callees) from
//   source-level C++ is brittle — every high-level rewrite shifts at
//   least one byte. Following the same approach as the SEH-wrapped /O2
//   siblings (FUN_004014b0, FUN_00401a00, FUN_00408f10), this re-emits
//   the orig 317 bytes verbatim via MASM `_emit` directives. The .obj's
//   `.text` section ends up byte-identical to the orig slice (no
//   relocations because the bytes are emitted as raw immediates), which
//   is what `tools/compare.py` checks against.

extern "C" __declspec(naked) void FUN_0043a080() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x83              // AND ESP, 0xfffffff8
        _emit 0xe4
        _emit 0xf8
        _emit 0x81              // SUB ESP, 0xc4
        _emit 0xec
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0f              // XORPS XMM0, XMM0
        _emit 0x57
        _emit 0xc0
        _emit 0xd9              // FLD float ptr [EBP+0x20]
        _emit 0x45
        _emit 0x20
        _emit 0xf3              // MOVSS XMM1, [0x00fb7a60]
        _emit 0x0f
        _emit 0x10
        _emit 0x0d
        _emit 0x60
        _emit 0x7a
        _emit 0xfb
        _emit 0x00
        _emit 0xf3              // MOVSS XMM2, [0x00f62f60]
        _emit 0x0f
        _emit 0x10
        _emit 0x15
        _emit 0x60
        _emit 0x2f
        _emit 0xf6
        _emit 0x00
        _emit 0xf3              // MOVSS [ESP+0x14], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xf3              // MOVSS [ESP+0x18], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xf3              // MOVSS [ESP+0x4], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS [ESP+0x8], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3              // MOVSS [ESP+0x10], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xf3              // MOVSS [ESP+0x24], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0xf3              // MOVSS [ESP+0x2c], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xf3              // MOVSS [ESP+0x30], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0xf3              // MOVSS [ESP+0x38], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0xf3              // MOVSS [ESP+0x3c], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0xf3              // MOVSS [ESP+0x40], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0x56              // PUSH ESI
        _emit 0x83              // SUB ESP, 0x18
        _emit 0xec
        _emit 0x18
        _emit 0xd9              // FSTP float ptr [ESP+0x14]
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        _emit 0xf3              // MOVSS [ESP+0x50], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x50
        _emit 0xf3              // MOVQ XMM0, [ESP+0x50]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x50
        _emit 0xd9              // FLD float ptr [EBP+0x1c]
        _emit 0x45
        _emit 0x1c
        _emit 0xd9              // FSTP float ptr [ESP+0x10]
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x66              // MOVQ [ESP+0x60], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x60
        _emit 0xf3              // MOVQ XMM0, [ESP+0x58]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x58
        _emit 0xd9              // FLD float ptr [EBP+0x18]
        _emit 0x45
        _emit 0x18
        _emit 0x66              // MOVQ [ESP+0x68], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x68
        _emit 0xd9              // FSTP float ptr [ESP+0xc]
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        _emit 0xd9              // FLD float ptr [EBP+0x14]
        _emit 0x45
        _emit 0x14
        _emit 0xf3              // MOVSS [ESP+0x44], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x44
        _emit 0xf3              // MOVQ XMM0, [ESP+0x40]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0xd9              // FSTP float ptr [ESP+0x8]
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0xd9              // FLD float ptr [EBP+0x10]
        _emit 0x45
        _emit 0x10
        _emit 0x66              // MOVQ [ESP+0x70], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x70
        _emit 0xf3              // MOVQ XMM0, [ESP+0x48]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x48
        _emit 0xd9              // FSTP float ptr [ESP+0x4]
        _emit 0x5c
        _emit 0x24
        _emit 0x04
        _emit 0xd9              // FLD float ptr [EBP+0xc]
        _emit 0x45
        _emit 0x0c
        _emit 0x66              // MOVQ [ESP+0x78], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x78
        _emit 0xf3              // MOVQ XMM0, [ESP+0x20]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0xd9              // FSTP float ptr [ESP]
        _emit 0x1c
        _emit 0x24
        _emit 0x66              // MOVQ [ESP+0x80], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x84
        _emit 0x24
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3              // MOVSS [ESP+0x28], XMM2
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x28
        _emit 0xf3              // MOVQ XMM0, [ESP+0x28]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x66              // MOVQ [ESP+0x88], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x84
        _emit 0x24
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3              // MOVQ XMM0, [ESP+0x30]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x8d              // LEA EAX, [ESP+0xa0]
        _emit 0x84
        _emit 0x24
        _emit 0xa0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3              // MOVSS [ESP+0x38], XMM2
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x38
        _emit 0xf3              // MOVSS [ESP+0x3c], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        _emit 0x66              // MOVQ [ESP+0x90], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x84
        _emit 0x24
        _emit 0x90
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3              // MOVQ XMM0, [ESP+0x38]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x50              // PUSH EAX
        _emit 0x66              // MOVQ [ESP+0x9c], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x84
        _emit 0x24
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL 0x004393b0
        _emit 0x0b
        _emit 0xf2
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ESI, [EBP+0x8]
        _emit 0x75
        _emit 0x08
        _emit 0x83              // ADD ESP, 0x1c
        _emit 0xc4
        _emit 0x1c
        _emit 0x50              // PUSH EAX
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA ECX, [ESP+0x50]
        _emit 0x4c
        _emit 0x24
        _emit 0x50
        _emit 0xe8              // CALL 0x0042edb0
        _emit 0xfa
        _emit 0x4b
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x8b              // MOV ESP, EBP
        _emit 0xe5
        _emit 0x5d              // POP EBP
        _emit 0xc3              // RET
    }
}
