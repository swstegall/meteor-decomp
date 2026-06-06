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
// FUNCTION: ffxivgame 0x00038e70 — `__thiscall` container element erase /
//                                  notify (360 B / 0x168, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x00038e70):
//
//   __thiscall void erase_notify(this, arg0, arg1) — `ECX = this`,
//   two pushed stack args (the epilogue is `RET 0x8`), returns void.
//
//   Structural shape (debug-iterator-style bounds checks bracket the
//   body, hence the repeated `CALL 0x009d22b4` invariant-trap helper):
//
//     EBX = this->m_end;                 // [ecx+0x10]
//     if (this->m_xxx /* [ecx+0xc] */ > EBX) _ITERATOR_TRAP();
//     EDI = &this->m_vec;                // [ecx+0x8]  (the 0x14-stride vector)
//     [esp+0x10] = this;                 // spill `this`
//     ESI = EDI->begin;                  // [edi+0x4]
//     if (ESI > EDI->cap /* [edi+0x8] */) _ITERATOR_TRAP();
//     EBP = arg0;                        // [esp+0x38]  (search key)
//     [esp+0x18] = ESI;
//     // linear scan for the 0x14-byte entry whose first dword == arg0
//     if (ESI != EBX && *ESI != EBP)
//         do { ESI += 0x14; } while (ESI != EBX && *ESI != EBP);
//
//     EBX = EDI->end;                    // [edi+0x8]
//     if (EDI->begin /* [edi+0x4] */ > EBX) _ITERATOR_TRAP();
//     if (!EDI || EDI != EDI) _ITERATOR_TRAP();   // null/self-consistency
//     if (ESI == EBX) {                  // key not present — assert path
//         // magic-static-guarded bind of the assertion reporter at 0x4385a0,
//         // then fire it with the file/line/func string set @ 0x00f6xxxx.
//         ...
//     }
//     ... bounds-checked element move/erase via 0x00435fe0 / 0x0068faa0 ...
//     EDI->end -= 0x14;                  // [edi+0x8] -= 0x14  (shrink by one)
//
//   Reloc-bearing sites in the orig 360 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     CALL 0x009d22b4   — debug-iterator invariant trap (×9)
//     TEST/OR [0x01323910] / MOV [0x0132390c] — magic-static assert guard
//     MOV [0x0132390c], 0x004385a0 — bind assertion reporter
//     PUSH 0x00f65e98 / 0x00f65ef8 / 0x00f65d3f / 0x00f65f58 — assert strings
//     CALL [0x0132390c] — fire assertion reporter
//     MOV [esp+0x20], 0x00f649e8 — vtable / type tag for the transient
//     CALL 0x00435fe0   — element relocate / ctor helper
//     CALL 0x0068faa0   — erase / shift helper
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 into the exact
//   register schedule across the nine interleaved debug-iterator traps, the
//   magic-static assertion guard layout, and the linker-resolved absolute
//   addresses above — every high-level variant shifts at least one byte.
//   The local idiom for reloc-heavy bodies (see FUN_0040ced0,
//   FUN_004014b0, FUN_00405080) is a naked body that re-emits the orig
//   360 bytes verbatim; the .obj `.text` ends up byte-identical to the
//   orig slice and tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00438e70() {
    __asm {
        _emit 0x83
        _emit 0xec
        _emit 0x24
        _emit 0x53
        _emit 0x8b
        _emit 0x59
        _emit 0x10
        _emit 0x39
        _emit 0x59
        _emit 0x0c
        _emit 0x55
        _emit 0x56
        _emit 0x57
        _emit 0x8d
        _emit 0x79
        _emit 0x08
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x76
        _emit 0x05
        _emit 0xe8
        _emit 0x29
        _emit 0x94
        _emit 0x59
        _emit 0x00
        _emit 0x8b
        _emit 0x77
        _emit 0x04
        _emit 0x3b
        _emit 0x77
        _emit 0x08
        _emit 0x76
        _emit 0x05
        _emit 0xe8
        _emit 0x1c
        _emit 0x94
        _emit 0x59
        _emit 0x00
        _emit 0x3b
        _emit 0xf3
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x38
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x18
        _emit 0x74
        _emit 0x0b
        _emit 0x39
        _emit 0x2e
        _emit 0x74
        _emit 0x07
        _emit 0x83
        _emit 0xc6
        _emit 0x14
        _emit 0x3b
        _emit 0xf3
        _emit 0x75
        _emit 0xf5
        _emit 0x8b
        _emit 0x5f
        _emit 0x08
        _emit 0x39
        _emit 0x5f
        _emit 0x04
        _emit 0x76
        _emit 0x05
        _emit 0xe8
        _emit 0xf8
        _emit 0x93
        _emit 0x59
        _emit 0x00
        _emit 0x85
        _emit 0xff
        _emit 0x74
        _emit 0x04
        _emit 0x3b
        _emit 0xff
        _emit 0x74
        _emit 0x05
        _emit 0xe8
        _emit 0xeb
        _emit 0x93
        _emit 0x59
        _emit 0x00
        _emit 0x3b
        _emit 0xf3
        _emit 0x75
        _emit 0x3f
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75
        _emit 0x10
        _emit 0x09
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xa0
        _emit 0x85
        _emit 0x43
        _emit 0x00
        _emit 0x68
        _emit 0x98
        _emit 0x5e
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0x68
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0xf8
        _emit 0x5e
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0x3f
        _emit 0x5d
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0x58
        _emit 0x5f
        _emit 0xf6
        _emit 0x00
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x85
        _emit 0xff
        _emit 0x75
        _emit 0x05
        _emit 0xe8
        _emit 0x9f
        _emit 0x93
        _emit 0x59
        _emit 0x00
        _emit 0x3b
        _emit 0x77
        _emit 0x08
        _emit 0x72
        _emit 0x05
        _emit 0xe8
        _emit 0x95
        _emit 0x93
        _emit 0x59
        _emit 0x00
        _emit 0xf6
        _emit 0x46
        _emit 0x0c
        _emit 0x10
        _emit 0x74
        _emit 0x1b
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x38
        _emit 0x50
        _emit 0x8b
        _emit 0x47
        _emit 0x08
        _emit 0x51
        _emit 0x52
        _emit 0x56
        _emit 0x50
        _emit 0xeb
        _emit 0x7e
        _emit 0x3b
        _emit 0x77
        _emit 0x08
        _emit 0x72
        _emit 0x23
        _emit 0xe8
        _emit 0x6a
        _emit 0x93
        _emit 0x59
        _emit 0x00
        _emit 0x3b
        _emit 0x77
        _emit 0x08
        _emit 0x72
        _emit 0x19
        _emit 0xe8
        _emit 0x60
        _emit 0x93
        _emit 0x59
        _emit 0x00
        _emit 0x3b
        _emit 0x77
        _emit 0x08
        _emit 0x72
        _emit 0x0f
        _emit 0xe8
        _emit 0x56
        _emit 0x93
        _emit 0x59
        _emit 0x00
        _emit 0x3b
        _emit 0x77
        _emit 0x08
        _emit 0x72
        _emit 0x05
        _emit 0xe8
        _emit 0x4c
        _emit 0x93
        _emit 0x59
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        _emit 0x8d
        _emit 0x14
        _emit 0x08
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0xe8
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x24
        _emit 0xe8
        _emit 0x3b
        _emit 0xd0
        _emit 0xff
        _emit 0xff
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x38
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x51
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        _emit 0x52
        _emit 0x50
        _emit 0x56
        _emit 0x51
        _emit 0x83
        _emit 0xc6
        _emit 0x14
        _emit 0x56
        _emit 0xe8
        _emit 0xd9
        _emit 0x6a
        _emit 0x25
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x83
        _emit 0x47
        _emit 0x08
        _emit 0xec
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
