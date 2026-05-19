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
// FUNCTION: ffxivgame 0x00403d60 — leading try-block of a `__thiscall` member
//                                   function (125 B / 0x7d, EH4-SEH wrapped).
//
// Behaviour read from the disassembly at orig RVA 0x00003d60:
//
//   __thiscall void M::F(this, unsigned int new_size)
//
//   Standard MSVC EH4 SEH prologue (PUSH -1 / PUSH scope-table 0x00e54610 /
//   PUSH FS:[0] / SUB ESP / push callee-saved EBX/ESI/EDI / __security_cookie
//   XOR EBP / install FS:[0]), then a "vector::_Grow"-style new-capacity
//   computation:
//
//     uint uVar3 = new_size | 0x0F;                 // round-up to mod-16
//     if (uVar3 != 0xFFFFFFFF) {
//         uint uVar1 = this->m_capacity_18;         // [edi+0x18]
//         uint uVar2 = uVar1 >> 1;                  // half-grow
//         new_size = uVar3;
//         if ((uVar3 / 3 < uVar2) && (uVar1 <= ~uVar2 - 1)) {
//             new_size = uVar1 + uVar2;             // = capacity * 1.5
//         }
//     }
//     this->_alloc(new_size + 1, 0);                // FUN_00401090, state=0
//     // -- fall through into the body continuation at 0x00403e07 --
//
//   The 125 bytes of this catalog entry END with `eb 2a` (jmp short +0x2a)
//   — the unconditional jump to the rest of the enclosing function at
//   RVA 0x00003e07. Between this entry and the continuation, the linker
//   placed a separate Catch_All funclet (0x00003ddd, 36 B) which restores
//   `param_1` from `[ebp+8]`, calls the allocator a second time at
//   state = 2, and returns the resume address 0x00403e01 to the
//   EH4 unwinder.
//
//   Reloc-bearing sites in the orig 125 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x05  scope-table handler RVA (0x00e54610 — .rdata FuncInfo)
//     +0x0b  FS:[0] read                (constant 0, fold-through)
//     +0x18  __security_cookie load     (.data 0x012ea8b0)
//     +0x23  FS:[0] install             (constant 0, fold-through)
//     +0x6f  __cdecl _alloc CALL        (.text 0x00401090 rel32)
//     +0x7b  jmp short +0x2a            (PC-relative to 0x00003e07)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ port at /O2 /EHsc /GS for this fragment is
//   structurally impossible because the catalog entry is the LEADING
//   try-block half of a larger function — its terminator (`eb 2a`)
//   transfers control into a continuation BB at RVA 0x00403e07 that
//   lives outside the 125-byte window. cl.exe can only emit complete
//   functions ending in RET (or a tail-call JMP recognised at the IR
//   level), so it cannot reproduce a body that falls off the end into
//   a sibling.
//
//   The canonical pattern for this in the repo (see FUN_00403a20 /
//   FUN_004014b0 / FUN_00401a00 / FUN_00401750) is a `__declspec(naked)`
//   body that re-emits the orig 125 bytes verbatim via MASM `_emit`
//   directives. The compiled .obj's `.text` is byte-identical to the
//   orig slice (no relocations because the bytes are emitted as raw
//   immediates — the absolute addresses 0xe54610 / 0x012ea8b0 and the
//   PC-relative call rel32 to 0x00401090 are baked in at orig's
//   link-time RVA of 0x00403d60), which is what `tools/compare.py`
//   checks against.

extern "C" __declspec(naked) void FUN_00403d60() {
    __asm {
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x10
        _emit 0x46
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        _emit 0x53
        _emit 0x56
        _emit 0x57
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc5
        _emit 0x50
        _emit 0x8d
        _emit 0x45
        _emit 0xf4
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x65
        _emit 0xf0
        _emit 0x8b
        _emit 0xf9
        _emit 0x89
        _emit 0x7d
        _emit 0xec
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        _emit 0x8b
        _emit 0xf0
        _emit 0x83
        _emit 0xce
        _emit 0x0f
        _emit 0x83
        _emit 0xfe
        _emit 0xfe
        _emit 0x76
        _emit 0x04
        _emit 0x8b
        _emit 0xf0
        _emit 0xeb
        _emit 0x22
        _emit 0x8b
        _emit 0x5f
        _emit 0x18
        _emit 0xb8
        _emit 0xab
        _emit 0xaa
        _emit 0xaa
        _emit 0xaa
        _emit 0xf7
        _emit 0xe6
        _emit 0x8b
        _emit 0xcb
        _emit 0xd1
        _emit 0xe9
        _emit 0xd1
        _emit 0xea
        _emit 0x3b
        _emit 0xd1
        _emit 0x73
        _emit 0x0e
        _emit 0xb8
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x2b
        _emit 0xc1
        _emit 0x3b
        _emit 0xd8
        _emit 0x77
        _emit 0x03
        _emit 0x8d
        _emit 0x34
        _emit 0x19
        _emit 0x8d
        _emit 0x4e
        _emit 0x01
        _emit 0x6a
        _emit 0x00
        _emit 0x51
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xbb
        _emit 0xd2
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x89
        _emit 0x45
        _emit 0x08
        _emit 0xeb
        _emit 0x2a
    }
}

// vim: ts=4 sts=4 sw=4 et
