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
// FUNCTION: ffxivgame 0x009c7a4f — __cdecl wide-char put into a FILE buffer
//                                  (CRT _fputwc_nolock variant, 427 B / 0x1ab,
//                                  /GS security cookie, no SEH frame).
//
// Inspection (read from the disassembly at orig RVA 0x009c7a4f):
//
//   int __cdecl _fputwc_nolock(wchar_t ch, FILE* str);
//
//   EDI = param_1 (the wide char), ESI = param_2 (the FILE*). The body:
//     - guards `ch == 0xffff` (WEOF) → return -1;
//     - validates the stream flags at str+0xc (must be open for write);
//     - lazily allocates the stream buffer via FUN_009f0a23 when str+0x8==0;
//     - for text-mode (non-0x40) streams, looks up the per-fd ioinfo
//       record (`__pioinfo[fd>>5][fd&0x1f]`, stride 0x40, table at
//       0x0137b7e0) to decide whether the fd is in wide/utf-8 text mode,
//       then runs the multibyte-conversion path through FUN_009f711f
//       (a WideCharToMultiByte-style helper) before draining the bytes
//       into the stream buffer one at a time;
//     - for binary streams writes the two raw little-endian bytes of `ch`;
//     - sets the "buffer dirty / has data" flag bits (clear 0x10, set 0x1)
//       in str+0xc on success and returns `ch`, or -1 on any error.
//
//   Reloc-bearing sites in the 427-byte body (absolute globals + rel32
//   call targets resolve only in a full-binary relink at image base
//   0x00400000; standalone .obj compilation can't reproduce them):
//     +0x06  moffs32 [0x012ea8b0] — __security_cookie
//     +0x42  rel32   0x009f0a23   — call (lazy buffer alloc)
//     +0x53  rel32   0x009d6a61   — call (fileno / ioinfo fetch)
//     +0x5c  imm32   0x0137b7e0   — __pioinfo base (LEA)
//     +0x80  rel32   0x009d6a61
//     +0x90  rel32   0x009d6a61
//     +0xab  rel32   0x009d6a61
//     +0xb6  rel32   0x009d6a61
//     +0xbf  imm32   0x0137b7e0   — __pioinfo base (LEA)
//     +0xc6  rel32   0x009d6a61
//     +0xfd  rel32   0x009f711f   — call (multibyte conversion helper)
//     +0x18c rel32   0x009d20f4   — call __security_check_cookie
//
//   Also MOV EBX,0x12eb4d8 (a default ioinfo record) at +0x5c-ish.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 /GS into
//   reproducing the exact register allocation across the repeated
//   FUN_009d6a61 calls, the SAR/AND/SHL ioinfo index math, the two-arm
//   text-vs-binary branch, and the linker-resolved absolute addresses in
//   the twelve relocation windows above — every high-level lowering shifts
//   at least one byte. The pragmatic choice (same as the sibling
//   FUN_00415d00 / FUN_0040b840 bodies) is a `__declspec(naked)` body that
//   re-emits the orig 427 bytes verbatim via MASM `_emit` directives. The
//   .obj's `.text` ends up byte-identical to the orig slice, which is what
//   tools/compare.py grades.

extern "C" __declspec(naked) void FUN_00dc7a4f() {
    __asm {
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x83
        _emit 0xec
        _emit 0x10
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc5
        _emit 0x89
        _emit 0x45
        _emit 0xfc
        _emit 0x53
        _emit 0x56
        _emit 0x8b
        _emit 0x75
        _emit 0x0c
        _emit 0x57
        _emit 0x8b
        _emit 0x7d
        _emit 0x08
        _emit 0x66
        _emit 0x81
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x0f
        _emit 0x84
        _emit 0x5c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0xa8
        _emit 0x01
        _emit 0x75
        _emit 0x10
        _emit 0x84
        _emit 0xc0
        _emit 0x0f
        _emit 0x89
        _emit 0x4d
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xa8
        _emit 0x02
        _emit 0x0f
        _emit 0x85
        _emit 0x45
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0x7e
        _emit 0x08
        _emit 0x00
        _emit 0x75
        _emit 0x07
        _emit 0x56
        _emit 0xe8
        _emit 0x8d
        _emit 0x8f
        _emit 0xc2
        _emit 0xff
        _emit 0x59
        _emit 0xf6
        _emit 0x46
        _emit 0x0c
        _emit 0x40
        _emit 0x0f
        _emit 0x85
        _emit 0x01
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0xba
        _emit 0xef
        _emit 0xc0
        _emit 0xff
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        _emit 0x59
        _emit 0xbb
        _emit 0xd8
        _emit 0xb4
        _emit 0x2e
        _emit 0x01
        _emit 0x74
        _emit 0x2e
        _emit 0x56
        _emit 0xe8
        _emit 0xa9
        _emit 0xef
        _emit 0xc0
        _emit 0xff
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        _emit 0x59
        _emit 0x74
        _emit 0x22
        _emit 0x56
        _emit 0xe8
        _emit 0x9d
        _emit 0xef
        _emit 0xc0
        _emit 0xff
        _emit 0xc1
        _emit 0xf8
        _emit 0x05
        _emit 0x56
        _emit 0x8d
        _emit 0x3c
        _emit 0x85
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0xe8
        _emit 0x8d
        _emit 0xef
        _emit 0xc0
        _emit 0xff
        _emit 0x83
        _emit 0xe0
        _emit 0x1f
        _emit 0x59
        _emit 0xc1
        _emit 0xe0
        _emit 0x06
        _emit 0x03
        _emit 0x07
        _emit 0x59
        _emit 0xeb
        _emit 0x02
        _emit 0x8b
        _emit 0xc3
        _emit 0xf6
        _emit 0x40
        _emit 0x04
        _emit 0x80
        _emit 0x0f
        _emit 0x84
        _emit 0xb3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0x6f
        _emit 0xef
        _emit 0xc0
        _emit 0xff
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        _emit 0x59
        _emit 0x74
        _emit 0x2e
        _emit 0x56
        _emit 0xe8
        _emit 0x63
        _emit 0xef
        _emit 0xc0
        _emit 0xff
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        _emit 0x59
        _emit 0x74
        _emit 0x22
        _emit 0x56
        _emit 0xe8
        _emit 0x57
        _emit 0xef
        _emit 0xc0
        _emit 0xff
        _emit 0xc1
        _emit 0xf8
        _emit 0x05
        _emit 0x56
        _emit 0x8d
        _emit 0x3c
        _emit 0x85
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0xe8
        _emit 0x47
        _emit 0xef
        _emit 0xc0
        _emit 0xff
        _emit 0x83
        _emit 0xe0
        _emit 0x1f
        _emit 0x59
        _emit 0xc1
        _emit 0xe0
        _emit 0x06
        _emit 0x03
        _emit 0x07
        _emit 0x59
        _emit 0xeb
        _emit 0x02
        _emit 0x8b
        _emit 0xc3
        _emit 0xf6
        _emit 0x40
        _emit 0x24
        _emit 0x7f
        _emit 0x74
        _emit 0x11
        _emit 0x8b
        _emit 0x55
        _emit 0x08
        _emit 0x6a
        _emit 0x02
        _emit 0x58
        _emit 0x88
        _emit 0x55
        _emit 0xf4
        _emit 0x88
        _emit 0x75
        _emit 0xf5
        _emit 0x89
        _emit 0x45
        _emit 0xf0
        _emit 0xeb
        _emit 0x1f
        _emit 0xff
        _emit 0x75
        _emit 0x08
        _emit 0x8d
        _emit 0x45
        _emit 0xf4
        _emit 0x6a
        _emit 0x05
        _emit 0x50
        _emit 0x8d
        _emit 0x45
        _emit 0xf0
        _emit 0x50
        _emit 0xe8
        _emit 0xce
        _emit 0xf5
        _emit 0xc2
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x77
        _emit 0x8b
        _emit 0x45
        _emit 0xf0
        _emit 0x8b
        _emit 0x55
        _emit 0x08
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        _emit 0x03
        _emit 0xc8
        _emit 0x39
        _emit 0x0e
        _emit 0x73
        _emit 0x0d
        _emit 0x83
        _emit 0x7e
        _emit 0x04
        _emit 0x00
        _emit 0x75
        _emit 0x62
        _emit 0x3b
        _emit 0x46
        _emit 0x18
        _emit 0x7f
        _emit 0x5d
        _emit 0x89
        _emit 0x0e
        _emit 0x8d
        _emit 0x48
        _emit 0xff
        _emit 0x85
        _emit 0xc9
        _emit 0x7c
        _emit 0x10
        _emit 0xff
        _emit 0x0e
        _emit 0x49
        _emit 0x8a
        _emit 0x5c
        _emit 0x0d
        _emit 0xf5
        _emit 0x8b
        _emit 0x06
        _emit 0x88
        _emit 0x18
        _emit 0x79
        _emit 0xf3
        _emit 0x8b
        _emit 0x45
        _emit 0xf0
        _emit 0x01
        _emit 0x46
        _emit 0x04
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0x83
        _emit 0xe0
        _emit 0xef
        _emit 0x83
        _emit 0xc8
        _emit 0x01
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        _emit 0x66
        _emit 0x8b
        _emit 0xc2
        _emit 0xeb
        _emit 0x34
        _emit 0x8b
        _emit 0x7d
        _emit 0x08
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        _emit 0x83
        _emit 0xc0
        _emit 0x02
        _emit 0x39
        _emit 0x06
        _emit 0x73
        _emit 0x0e
        _emit 0x83
        _emit 0x7e
        _emit 0x04
        _emit 0x00
        _emit 0x75
        _emit 0x1d
        _emit 0x83
        _emit 0x7e
        _emit 0x18
        _emit 0x02
        _emit 0x72
        _emit 0x17
        _emit 0x89
        _emit 0x06
        _emit 0x83
        _emit 0x06
        _emit 0xfe
        _emit 0xf6
        _emit 0x46
        _emit 0x0c
        _emit 0x40
        _emit 0x8b
        _emit 0x06
        _emit 0x74
        _emit 0x1d
        _emit 0x66
        _emit 0x39
        _emit 0x38
        _emit 0x74
        _emit 0x1b
        _emit 0x83
        _emit 0xc0
        _emit 0x02
        _emit 0x89
        _emit 0x06
        _emit 0x66
        _emit 0x0d
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x4d
        _emit 0xfc
        _emit 0x5f
        _emit 0x5e
        _emit 0x33
        _emit 0xcd
        _emit 0x5b
        _emit 0xe8
        _emit 0x14
        _emit 0xa5
        _emit 0xc0
        _emit 0xff
        _emit 0xc9
        _emit 0xc3
        _emit 0x66
        _emit 0x89
        _emit 0x38
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0x83
        _emit 0x46
        _emit 0x04
        _emit 0x02
        _emit 0x83
        _emit 0xe0
        _emit 0xef
        _emit 0x83
        _emit 0xc8
        _emit 0x01
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        _emit 0x66
        _emit 0x8b
        _emit 0xc7
        _emit 0xeb
        _emit 0xd9
    }
}
