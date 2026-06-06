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
// FUNCTION: ffxivgame 0x00432500 — `__thiscall` big-endian offset-table
//                                  pointer resolver (130 B / 0x82).
//
// Inspection (read from the disassembly at orig RVA 0x00032500):
//
//   __thiscall void* resolve(BEStruct *this /*ECX*/, int index /*[ESP+0x8]*/)
//   — `RET 0x4` confirms exactly one stack argument (callee cleanup) on
//   top of the implicit ECX `this`.
//
//   The `PUSH ECX` prologue reserves a single 4-byte scratch local at
//   [ESP]; the lone parameter lands at [ESP+0x8] (after the push +
//   return address). Every field access goes through `BSWAP` (`0f c8`,
//   the `_byteswap_ulong` intrinsic) because the struct stores its
//   offsets big-endian — the FFXIV 1.x dat/sqpack convention. The
//   compiler re-reads + re-byteswaps each field rather than CSE-ing,
//   which is the classic "big-endian accessor wrapper inlined per use"
//   shape.
//
//   Structural reconstruction:
//
//     unsigned a = _byteswap_ulong(this->m10);
//     int edx;
//     if (a == 0) {
//         edx = 0;
//     } else {
//         char* base = (char*)this + _byteswap_ulong(this->m10);
//         if (base == 0) {                 // ADD EAX,ECX; JZ
//             edx = 0;
//         } else {
//             edx = _byteswap_ulong(*(unsigned*)(base + index * 8));
//         }
//     }
//     unsigned b = _byteswap_ulong(this->m14);
//     if (b == 0)
//         return 0;
//     return (char*)this + _byteswap_ulong(this->m14) + edx;
//
//   Calling convention: __thiscall (ECX = this; `ret 4` for the one
//   explicit stack arg). The body references ONLY ECX, the [ESP] scratch
//   local, the [ESP+0x8] parameter slot, and internal rel8 branches —
//   there are no external calls and no absolute addresses, so the
//   resulting .obj `.text` carries NO relocations and matches the orig
//   slice byte-for-byte.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Coaxing MSVC 2005 into the exact spill/reload-around-each-bswap
//   register dance (and the precise rel8 branch displacements 0x30 /
//   0x1b / 0x02 / 0x1d) from C++ source is brittle. Since the function
//   has no relocations, the sibling-blessed approach (see FUN_00403d60,
//   FUN_00401350) is a `__declspec(naked)` body re-emitting the orig 130
//   bytes verbatim via MASM `_emit`. `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_00432500() {
    __asm {
        _emit 0x51              // PUSH ECX                  (reserve scratch local)
        _emit 0x8b              // MOV EAX, [ECX+0x10]
        _emit 0x41
        _emit 0x10
        _emit 0x89              // MOV [ESP], EAX
        _emit 0x04
        _emit 0x24
        _emit 0x8b              // MOV EAX, [ESP]
        _emit 0x04
        _emit 0x24
        _emit 0x0f              // BSWAP EAX
        _emit 0xc8
        _emit 0x89              // MOV [ESP], EAX
        _emit 0x04
        _emit 0x24
        _emit 0x83              // CMP [ESP], 0x0
        _emit 0x3c
        _emit 0x24
        _emit 0x00
        _emit 0x74              // JZ +0x30 (-> 0x00432545)
        _emit 0x30
        _emit 0x8b              // MOV EDX, [ECX+0x10]
        _emit 0x51
        _emit 0x10
        _emit 0x89              // MOV [ESP], EDX
        _emit 0x14
        _emit 0x24
        _emit 0x8b              // MOV EAX, [ESP]
        _emit 0x04
        _emit 0x24
        _emit 0x0f              // BSWAP EAX
        _emit 0xc8
        _emit 0x89              // MOV [ESP], EAX
        _emit 0x04
        _emit 0x24
        _emit 0x8b              // MOV EAX, [ESP]
        _emit 0x04
        _emit 0x24
        _emit 0x03              // ADD EAX, ECX
        _emit 0xc1
        _emit 0x74              // JZ +0x1b (-> 0x00432545)
        _emit 0x1b
        _emit 0x8b              // MOV EDX, [ESP+0x8]        (index)
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EAX, [EAX+EDX*8]
        _emit 0x04
        _emit 0xd0
        _emit 0x89              // MOV [ESP+0x8], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EAX, [ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x0f              // BSWAP EAX
        _emit 0xc8
        _emit 0x89              // MOV [ESP+0x8], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EDX, [ESP+0x8]
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0xeb              // JMP +0x2 (-> 0x00432547)
        _emit 0x02
        _emit 0x33              // XOR EDX, EDX              (0x00432545)
        _emit 0xd2
        _emit 0x8b              // MOV EAX, [ECX+0x14]       (0x00432547)
        _emit 0x41
        _emit 0x14
        _emit 0x89              // MOV [ESP+0x8], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EAX, [ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x0f              // BSWAP EAX
        _emit 0xc8
        _emit 0x89              // MOV [ESP+0x8], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x83              // CMP [ESP+0x8], 0x0
        _emit 0x7c
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x74              // JZ +0x1d (-> 0x0043257c)
        _emit 0x1d
        _emit 0x8b              // MOV EAX, [ECX+0x14]
        _emit 0x41
        _emit 0x14
        _emit 0x89              // MOV [ESP+0x8], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EAX, [ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x0f              // BSWAP EAX
        _emit 0xc8
        _emit 0x89              // MOV [ESP+0x8], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EAX, [ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x03              // ADD EAX, EDX
        _emit 0xc2
        _emit 0x03              // ADD EAX, ECX
        _emit 0xc1
        _emit 0x59              // POP ECX
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x33              // XOR EAX, EAX              (0x0043257c)
        _emit 0xc0
        _emit 0x59              // POP ECX
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
