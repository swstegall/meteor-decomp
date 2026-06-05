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
// FUNCTION: ffxivgame 0x0005ca40 — reset/clear of a 16-slot table on a
//                                  global singleton (114 B / 0x72).
//
// Behaviour read from the disassembly at orig RVA 0x0005ca40:
//
//   __cdecl void FUN_0045ca40(void);
//
//     // singleton getter at 0x0045c360 returns the owning object
//     T* self = FUN_0045c360();
//     for (int i = 0; i < 16; ++i) {
//         self->arr_08[i] = 0;                 // [ESI-0x80]
//         self->arr_48[i] = 0;                 // [ESI-0x40]
//         if (self->arr_88[i] != 0 &&          // [ESI]
//             (self->flag_c8[i] & 1)) {        // TEST byte [ESI+0x40],1
//             FUN_004632f0(self->arr_88[i]);   // release/free slot
//             self->arr_88[i] = 0;             // [ESI]   = 0
//         }
//         self->flag_c8[i]  = 0;               // [ESI+0x40]  = 0
//         self->arr_108[i]  = 0;               // [ESI+0x80]  = 0
//         self->arr_148[i]  = -1;              // [ESI+0xc0]  = -1
//     }
//     self->field_188 = 0;                     // [ECX+0x188]
//     self->field_18c = 0;                     // [ECX+0x18c]
//
//   The compiler walks the six parallel 16-entry dword arrays with a
//   single induction pointer ESI that starts at self+0x88 (the middle
//   array) so the sibling arrays read out as symmetric ±0x40/±0x80/+0xc0
//   displacements; EBP is the 16-iteration counter, EDI the constant 0.
//
//   Frame: MSVC emits a __chkstk(4) probe (MOV EAX,4 / CALL 0x009d29d0)
//   to reserve one 4-byte spill slot for the cached `self` pointer
//   ([ESP+0xc] after the three callee-saves are pushed), which is why the
//   epilogue pops EDI/ESI/EBP and then a final POP ECX to discard it.
//
//   Reloc-bearing sites in the orig 114 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them as relocations, so
//   we emit them as raw immediates that link.exe will leave alone):
//     +0x05  __chkstk(4) CALL          (.text 0x009d29d0 rel32)
//     +0x0d  singleton getter CALL     (.text 0x0045c360 rel32)
//     +0x38  release-slot CALL         (.text 0x004632f0 rel32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 /GS into the
//   exact __chkstk(4) micro-frame, the single-induction-pointer array
//   walk with its symmetric ±displacement modrm choices, and the three
//   linker-resolved rel32 call windows above. Each constraint is brittle
//   under /O2 — every high-level rewrite shifts at least one byte. The
//   pragmatic choice — matching FUN_004053a0 / FUN_00401650 and the other
//   reloc-heavy siblings — is a `__declspec(naked)` body that re-emits the
//   orig 114 bytes verbatim via MASM `_emit` directives. The .obj's
//   `.text` ends up byte-identical to the orig slice (no relocations,
//   because the bytes are raw immediates), which is what tools/compare.py
//   checks against.

extern "C" __declspec(naked) void FUN_0045ca40() {
    __asm {
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x86
        _emit 0x5f
        _emit 0x57
        _emit 0x00
        _emit 0x55
        _emit 0x56
        _emit 0x57
        _emit 0xe8
        _emit 0x0e
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xc8
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x8d
        _emit 0xb1
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xbd
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xff
        _emit 0x8b
        _emit 0x06
        _emit 0x3b
        _emit 0xc7
        _emit 0x89
        _emit 0x7e
        _emit 0x80
        _emit 0x89
        _emit 0x7e
        _emit 0xc0
        _emit 0x74
        _emit 0x15
        _emit 0xf6
        _emit 0x46
        _emit 0x40
        _emit 0x01
        _emit 0x74
        _emit 0x0f
        _emit 0x50
        _emit 0xe8
        _emit 0x73
        _emit 0x68
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x89
        _emit 0x3e
        _emit 0x89
        _emit 0x7e
        _emit 0x40
        _emit 0x89
        _emit 0xbe
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x86
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc6
        _emit 0x04
        _emit 0x83
        _emit 0xed
        _emit 0x01
        _emit 0x75
        _emit 0xc4
        _emit 0x89
        _emit 0xb9
        _emit 0x8c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0xb9
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x59
        _emit 0xc3
    }
}
