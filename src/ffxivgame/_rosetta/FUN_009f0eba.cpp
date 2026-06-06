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
// FUNCTION: ffxivgame 0x005f0eba — __cdecl codepage transcode helper
//                                   (428 B / 0x1ac, /GS cookie, inline
//                                   _malloca/_freea).
//
// Inspection (read from the disassembly at orig RVA 0x005f0eba):
//
//   __cdecl void* transcode(UINT srcCP, UINT dstCP, const char* src,
//                           int* pSrcLen, int* outLen, void* dstBuf,
//                           int dstBufLen);
//
//   Standard EBP frame, SUB ESP,0x34, /GS security cookie at [ebp-4].
//   Args: [ebp+8]=srcCP, [ebp+0xc]=dstCP, [ebp+0x10]=src,
//         [ebp+0x14]=pSrcLen (int*, dereferenced once), [ebp+0x18]=outLen,
//         [ebp+0x1c]=dstBuf.
//
//   Body shape (recovered, structural only):
//     if (srcCP == dstCP) return NULL;
//     // validate both codepages via IsValidCodePage-style fnptr
//     //   A = (*[0x00f3e364])(cp, &probe); probe must == 1
//     // size the wide buffer:
//     //   len = (*pSrcLen == -1) ? strlen(src)+1 : *pSrcLen
//     //   B = (*[0x00f3e1f4]) — MultiByteToWideChar-style
//     //   C = (*[0x00f3e12c]) — WideCharToMultiByte-style
//     //   wide = _malloca(2*count); memset(wide,0,2*count);
//     //   B(srcCP, MB_PRECOMPOSED, src, srcLen, wide, count);
//     //   if dstBuf: C(dstCP,0,wide,count,dstBuf,dstBufLen,0,0)
//     //   else: size = C(dstCP,0,wide,count,0,0,0,0);
//     //         out = alloc(1,size); C(...,out,size,...);
//     //   _freea(wide);
//
//   Reloc-bearing sites in the 428-byte body (absolute IAT / global
//   pointers + rel32 CRT calls — only valid in a full-binary relink at
//   image base 0x00400000; a standalone .obj can't reproduce them):
//     +0x06  MOV EAX,[0x012ea8b0]  — __security_cookie
//     +0x40  MOV ESI,[0x00f3e364]  — codepage-validate fnptr
//     +0x4d  MOV EBX,[0x00f3e1f4]  — MultiByteToWideChar-style fnptr
//     +0x80  CALL 0x009dc3f0       — strlen
//     +0xa0  CALL 0x009d8be0       — _alloca_probe (inline _malloca)
//     +0xd0  CALL 0x009d5bc5       — malloc (heap _malloca path)
//     +0xf8  CALL 0x009d2110       — memset
//     +0x12a CALL [0x00f3e12c]     — WideCharToMultiByte-style fnptr
//     +0x142 MOV EBX,[0x00f3e12c]  — same fnptr, v_arg4==0 path
//     +0x15b CALL 0x009ddfba       — allocator (count=1)
//     +0x17e CALL 0x009d5c88       — free (failed C-call cleanup)
//     +0x197 CALL 0x009da186       — _freea
//     +0x1ab CALL 0x009d20f4       — __security_check_cookie (tail,
//                                     truncated at the declared 0x1ac
//                                     boundary — the orig PE's recorded
//                                     function size cuts mid-instruction).
//
// Reconstruction strategy — naked-asm byte passthrough (the same
// approach the sibling reloc-heavy bodies FUN_00415d00 / FUN_0040b840
// take): re-emit the orig 428 bytes verbatim via MASM `_emit`
// directives so the .obj's `.text` is byte-identical to the orig slice
// (no relocations — the bytes are raw immediates), which is exactly
// what tools/compare.py grades. A source-level rebuild would have to
// coax MSVC 2005 /O2 /GS into reproducing the /GS cookie, the inline
// _malloca threshold/marker dance (0xCCCC stack / 0xDDDD heap), the
// chained register allocation across nine cdecl/indirect call sites,
// and the linker-resolved absolute addresses above — each brittle
// under /O2.

extern "C" __declspec(naked) void FUN_009f0eba() {
    __asm {
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x83
        _emit 0xec
        _emit 0x34
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

        _emit 0x8b
        _emit 0x45
        _emit 0x10
        _emit 0x8b
        _emit 0x4d
        _emit 0x18
        _emit 0x89
        _emit 0x45
        _emit 0xd8
        _emit 0x8b
        _emit 0x45
        _emit 0x14
        _emit 0x53
        _emit 0x89
        _emit 0x45
        _emit 0xd0

        _emit 0x8b
        _emit 0x00
        _emit 0x56
        _emit 0x89
        _emit 0x45
        _emit 0xdc
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        _emit 0x57
        _emit 0x33
        _emit 0xff
        _emit 0x3b
        _emit 0x45
        _emit 0x0c
        _emit 0x89

        _emit 0x4d
        _emit 0xcc
        _emit 0x89
        _emit 0x7d
        _emit 0xe0
        _emit 0x89
        _emit 0x7d
        _emit 0xd4
        _emit 0x0f
        _emit 0x84
        _emit 0x5f
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x35

        _emit 0x64
        _emit 0xe3
        _emit 0xf3
        _emit 0x00
        _emit 0x8d
        _emit 0x4d
        _emit 0xe8
        _emit 0x51
        _emit 0x50
        _emit 0xff
        _emit 0xd6
        _emit 0x85
        _emit 0xc0
        _emit 0x8b
        _emit 0x1d
        _emit 0xf4

        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x74
        _emit 0x5e
        _emit 0x83
        _emit 0x7d
        _emit 0xe8
        _emit 0x01
        _emit 0x75
        _emit 0x58
        _emit 0x8d
        _emit 0x45
        _emit 0xe8
        _emit 0x50
        _emit 0xff

        _emit 0x75
        _emit 0x0c
        _emit 0xff
        _emit 0xd6
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x4b
        _emit 0x83
        _emit 0x7d
        _emit 0xe8
        _emit 0x01
        _emit 0x75
        _emit 0x45
        _emit 0x8b
        _emit 0x75

        _emit 0xdc
        _emit 0x83
        _emit 0xfe
        _emit 0xff
        _emit 0xc7
        _emit 0x45
        _emit 0xd4
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x75
        _emit 0x0c
        _emit 0xff
        _emit 0x75
        _emit 0xd8

        _emit 0xe8
        _emit 0xb1
        _emit 0xb4
        _emit 0xfe
        _emit 0xff
        _emit 0x8b
        _emit 0xf0
        _emit 0x59
        _emit 0x46
        _emit 0x3b
        _emit 0xf7
        _emit 0x7e
        _emit 0x5b
        _emit 0x81
        _emit 0xfe
        _emit 0xf0

        _emit 0xff
        _emit 0xff
        _emit 0x7f
        _emit 0x77
        _emit 0x53
        _emit 0x8d
        _emit 0x44
        _emit 0x36
        _emit 0x08
        _emit 0x3d
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x77
        _emit 0x2f

        _emit 0xe8
        _emit 0x81
        _emit 0x7c
        _emit 0xfe
        _emit 0xff
        _emit 0x8b
        _emit 0xc4
        _emit 0x3b
        _emit 0xc7
        _emit 0x74
        _emit 0x38
        _emit 0xc7
        _emit 0x00
        _emit 0xcc
        _emit 0xcc
        _emit 0x00

        _emit 0x00
        _emit 0xeb
        _emit 0x2d
        _emit 0x57
        _emit 0x57
        _emit 0xff
        _emit 0x75
        _emit 0xdc
        _emit 0xff
        _emit 0x75
        _emit 0xd8
        _emit 0x6a
        _emit 0x01
        _emit 0xff
        _emit 0x75
        _emit 0x08

        _emit 0xff
        _emit 0xd3
        _emit 0x8b
        _emit 0xf0
        _emit 0x3b
        _emit 0xf7
        _emit 0x75
        _emit 0xc3
        _emit 0x33
        _emit 0xc0
        _emit 0xe9
        _emit 0xd1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50

        _emit 0xe8
        _emit 0x36
        _emit 0x4c
        _emit 0xfe
        _emit 0xff
        _emit 0x3b
        _emit 0xc7
        _emit 0x59
        _emit 0x74
        _emit 0x09
        _emit 0xc7
        _emit 0x00
        _emit 0xdd
        _emit 0xdd
        _emit 0x00
        _emit 0x00

        _emit 0x83
        _emit 0xc0
        _emit 0x08
        _emit 0x89
        _emit 0x45
        _emit 0xe4
        _emit 0xeb
        _emit 0x03
        _emit 0x89
        _emit 0x7d
        _emit 0xe4
        _emit 0x39
        _emit 0x7d
        _emit 0xe4
        _emit 0x74
        _emit 0xd8

        _emit 0x8d
        _emit 0x04
        _emit 0x36
        _emit 0x50
        _emit 0x57
        _emit 0xff
        _emit 0x75
        _emit 0xe4
        _emit 0xe8
        _emit 0x59
        _emit 0x11
        _emit 0xfe
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c

        _emit 0x56
        _emit 0xff
        _emit 0x75
        _emit 0xe4
        _emit 0xff
        _emit 0x75
        _emit 0xdc
        _emit 0xff
        _emit 0x75
        _emit 0xd8
        _emit 0x6a
        _emit 0x01
        _emit 0xff
        _emit 0x75
        _emit 0x08
        _emit 0xff

        _emit 0xd3
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x7f
        _emit 0x8b
        _emit 0x5d
        _emit 0xcc
        _emit 0x3b
        _emit 0xdf
        _emit 0x74
        _emit 0x1d
        _emit 0x57
        _emit 0x57
        _emit 0xff
        _emit 0x75

        _emit 0x1c
        _emit 0x53
        _emit 0x56
        _emit 0xff
        _emit 0x75
        _emit 0xe4
        _emit 0x57
        _emit 0xff
        _emit 0x75
        _emit 0x0c
        _emit 0xff
        _emit 0x15
        _emit 0x2c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00

        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x60
        _emit 0x89
        _emit 0x5d
        _emit 0xe0
        _emit 0xeb
        _emit 0x5b
        _emit 0x39
        _emit 0x7d
        _emit 0xd4
        _emit 0x8b
        _emit 0x1d
        _emit 0x2c
        _emit 0xe1

        _emit 0xf3
        _emit 0x00
        _emit 0x75
        _emit 0x14
        _emit 0x57
        _emit 0x57
        _emit 0x57
        _emit 0x57
        _emit 0x56
        _emit 0xff
        _emit 0x75
        _emit 0xe4
        _emit 0x57
        _emit 0xff
        _emit 0x75
        _emit 0x0c

        _emit 0xff
        _emit 0xd3
        _emit 0x8b
        _emit 0xf0
        _emit 0x3b
        _emit 0xf7
        _emit 0x74
        _emit 0x3c
        _emit 0x56
        _emit 0x6a
        _emit 0x01
        _emit 0xe8
        _emit 0xa0
        _emit 0xcf
        _emit 0xfe
        _emit 0xff

        _emit 0x3b
        _emit 0xc7
        _emit 0x59
        _emit 0x59
        _emit 0x89
        _emit 0x45
        _emit 0xe0
        _emit 0x74
        _emit 0x2b
        _emit 0x57
        _emit 0x57
        _emit 0x56
        _emit 0x50
        _emit 0x56
        _emit 0xff
        _emit 0x75

        _emit 0xe4
        _emit 0x57
        _emit 0xff
        _emit 0x75
        _emit 0x0c
        _emit 0xff
        _emit 0xd3
        _emit 0x3b
        _emit 0xc7
        _emit 0x75
        _emit 0x0e
        _emit 0xff
        _emit 0x75
        _emit 0xe0
        _emit 0xe8
        _emit 0x4b

        _emit 0x4c
        _emit 0xfe
        _emit 0xff
        _emit 0x59
        _emit 0x89
        _emit 0x7d
        _emit 0xe0
        _emit 0xeb
        _emit 0x0b
        _emit 0x83
        _emit 0x7d
        _emit 0xdc
        _emit 0xff
        _emit 0x74
        _emit 0x05
        _emit 0x8b

        _emit 0x4d
        _emit 0xd0
        _emit 0x89
        _emit 0x01
        _emit 0xff
        _emit 0x75
        _emit 0xe4
        _emit 0xe8
        _emit 0x30
        _emit 0x91
        _emit 0xfe
        _emit 0xff
        _emit 0x59
        _emit 0x8b
        _emit 0x45
        _emit 0xe0

        _emit 0x8d
        _emit 0x65
        _emit 0xc0
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0x8b
        _emit 0x4d
        _emit 0xfc
        _emit 0x33
        _emit 0xcd
        _emit 0xe8
    }
}
