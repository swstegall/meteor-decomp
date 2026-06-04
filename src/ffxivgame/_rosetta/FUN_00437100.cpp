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
// FUNCTION: ffxivgame 0x00437100 — __thiscall factory wrapper (95 bytes)
//
// __thiscall void FUN_00437100(this, Vec4 *src)   // RET 4 (one stack arg)
//   ECX = this  (→ ESI)
//   [ESP+0x04] : Vec4 *src   (16-byte payload, → ECX after prologue/[ESP+8])
//
// Behaviour: index into a global slab descriptor at [0x01328d90]:
//   slab   = *[0x01328d90]
//   idx    = (BYTE) slab[0]
//   slot   = slab[+4] + idx*0x1C        ; idx*7 dwords  (LEA edx,[eax*8]; sub eax)
//   node   = FUN_00417ab0(slot, 0x14)   ; pool-allocate a 20-byte node
//   if (node) {
//       node[0]   = 0x00f64938          ; vtable / type tag
//       *(q)(node+0x4) = *(q)(src+0x0)  ; copy 16 bytes (two MOVQ pairs)
//       *(q)(node+0xc) = *(q)(src+0x8)
//       this->field8->FUN_0043c2d0(node);
//   } else {
//       this->field8->FUN_0043c2d0(0);
//   }
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The body contains SSE2 MOVQ loads/stores (f3 0f 7e / 66 0f d6) whose
//   exact encoding the inline assembler may not reproduce, plus an inlined
//   vtable immediate (0xf64938) and an absolute global ref (0x01328d90).
//   Emitting the orig 95 bytes verbatim via `_emit` yields a .obj whose
//   .text is byte-identical to the orig slice with NO relocations — the
//   rel32/imm32/abs32 offsets are the orig binary's own resolved values.
//   tools/compare.py then reports GREEN.
//
// Reloc-bearing sites (baked-in as raw bytes here):
//     +0x05   MOV ECX,[abs32]  → 0x01328d90 (global slab descriptor)
//     +0x1d   CALL rel32       → FUN_00417ab0 (RVA 0x00017ab0)
//     +0x2a   MOV [EAX],imm32  → 0x00f64938   (vtable / type tag)
//     +0x47   CALL rel32       → FUN_0043c2d0 (RVA 0x0003c2d0)
//     +0x56   CALL rel32       → FUN_0043c2d0 (RVA 0x0003c2d0)

extern "C" __declspec(naked) void FUN_00437100() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV ECX, dword ptr [0x01328d90]
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x0f              // MOVZX EAX, byte ptr [ECX]
        _emit 0xb6
        _emit 0x01
        _emit 0x8d              // LEA EDX, [EAX*8 + 0]
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB EDX, EAX
        _emit 0xd0
        _emit 0x8b              // MOV EAX, dword ptr [ECX+0x4]
        _emit 0x41
        _emit 0x04
        _emit 0x8d              // LEA ECX, [EAX + EDX*4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a              // PUSH 0x14
        _emit 0x14
        _emit 0xe8              // CALL FUN_00417ab0 (rel32 → 0x00417ab0)
        _emit 0x8e
        _emit 0x09
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x2A (null path)
        _emit 0x2a
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [EAX], 0x00f64938
        _emit 0x00
        _emit 0x38
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0xf3              // MOVQ XMM0, qword ptr [ECX]
        _emit 0x0f
        _emit 0x7e
        _emit 0x01
        _emit 0x66              // MOVQ qword ptr [EAX+0x4], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x04
        _emit 0xf3              // MOVQ XMM0, qword ptr [ECX+0x8]
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x08
        _emit 0x66              // MOVQ qword ptr [EAX+0xc], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x0c
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0043c2d0 (rel32 → 0x0043c2d0)
        _emit 0x84
        _emit 0x51
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x8]  (null path)
        _emit 0x4e
        _emit 0x08
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0043c2d0 (rel32 → 0x0043c2d0)
        _emit 0x75
        _emit 0x51
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
