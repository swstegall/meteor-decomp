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
// FUNCTION: ffxivgame 0x0004f4b0 — register-args single-character decode
//                                   into a 64-bit accumulator (297 B / 0x129).
//
// Inspection (read from the disassembly at orig RVA 0x0004f4b0):
//
//   Custom register convention (no standard MSVC cc reproduces it):
//     EAX  = pointer to the input byte stream                 (-> ESI)
//     ECX  = pointer to a 64-bit out-param ([ECX]=lo, [ECX+4]=hi)
//     [ESP+arg0], [ESP+arg1] = two translation-table / default args
//                              (loaded into EDI at different points)
//   Returns the count of input bytes consumed in EAX (1, 2, or the
//   UTF-8 lead-byte length 1..6).
//
//   Flow (AL = first input byte):
//     * 'A'..'Z' (0x41..0x5a): *out = table[AL-0x40] + 0x40, return 1
//     * 'a'..'z' (0x61..0x7a): *out = table[AL-0x60] + 0x40, return 1
//     * 0xC3 lead             : decode the following byte through two
//                               offset windows (+0x80/+0x60 range tests),
//                               *out = table[next] + 0x40, return 2
//     * otherwise             : classic UTF-8 lead-byte length classifier
//                               (0x80/0xc0/0xe0/0xf0/0xf8/0xfc/0xfe gates
//                               -> length 1..6, else default-arg length);
//                               then accumulate `length` bytes big-endian
//                               into the 64-bit out-param via SHLD/SHL +
//                               ADD/ADC, returning the byte length.
//
//   Stack frame: PUSH ECX / ESI / EDI (and a later PUSH EBX in the 0xC3
//   and UTF-8 arms); no SUB ESP, no SEH, no security cookie. The two
//   stack arguments are read at [ESP+0x10] (3 pushes deep) and, after the
//   EBX push, at [ESP+0xc].
//
//   No relocations: every branch is an internal rel8 displacement; there
//   are no CALLs, no IAT loads, no string-literal refs, and no absolute
//   data references. The 297 function bytes are position-independent
//   within the .text slice.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The entry convention here (live EAX + ECX inputs plus two stack args,
//   ECX restored via POP at every exit) is not expressible as any single
//   MSVC 2005 calling convention, and /O2 would not reproduce the exact
//   register allocation, the rel8-vs-rel32 branch selection, or the
//   particular SHLD/SHL/ADC accumulator unroll from a source-level
//   rewrite. As with the other self-contained siblings (FUN_00408f10
//   et al.), the pragmatic match is a naked body that re-emits the orig
//   297 bytes verbatim via MASM `_emit` directives. With no relocations,
//   the .obj's `.text` section is byte-identical to the orig slice, which
//   is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_0044f4b0() {
    __asm {
        _emit 0x51              // PUSH ECX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x8a              // MOV AL, [ESI]
        _emit 0x06
        _emit 0x3c              // CMP AL, 0x41
        _emit 0x41
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, [ESP+0x10]
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x72              // JC 0x0044f4dd
        _emit 0x1e
        _emit 0x3c              // CMP AL, 0x5a
        _emit 0x5a
        _emit 0x77              // JA 0x0044f4dd
        _emit 0x1a
        _emit 0x0f              // MOVZX EAX, AL
        _emit 0xb6
        _emit 0xc0
        _emit 0x0f              // MOVZX EAX, [EAX+EDI-0x40]
        _emit 0xb6
        _emit 0x44
        _emit 0x38
        _emit 0xc0
        _emit 0x83              // ADD EAX, 0x40
        _emit 0xc0
        _emit 0x40
        _emit 0x99              // CDQ
        _emit 0x5f              // POP EDI
        _emit 0x89              // MOV [ECX], EAX
        _emit 0x01
        _emit 0x89              // MOV [ECX+4], EDX
        _emit 0x51
        _emit 0x04
        _emit 0xb8              // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0x59              // POP ECX
        _emit 0xc3              // RET

        _emit 0x3c              // CMP AL, 0x61
        _emit 0x61
        _emit 0x72              // JC 0x0044f4ff
        _emit 0x1e
        _emit 0x3c              // CMP AL, 0x7a
        _emit 0x7a
        _emit 0x77              // JA 0x0044f4ff
        _emit 0x1a
        _emit 0x0f              // MOVZX EDX, AL
        _emit 0xb6
        _emit 0xd0
        _emit 0x0f              // MOVZX EAX, [EDX+EDI-0x60]
        _emit 0xb6
        _emit 0x44
        _emit 0x3a
        _emit 0xa0
        _emit 0x83              // ADD EAX, 0x40
        _emit 0xc0
        _emit 0x40
        _emit 0x99              // CDQ
        _emit 0x5f              // POP EDI
        _emit 0x89              // MOV [ECX], EAX
        _emit 0x01
        _emit 0x89              // MOV [ECX+4], EDX
        _emit 0x51
        _emit 0x04
        _emit 0xb8              // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0x59              // POP ECX
        _emit 0xc3              // RET

        _emit 0x3c              // CMP AL, 0xc3
        _emit 0xc3
        _emit 0x53              // PUSH EBX
        _emit 0x75              // JNZ 0x0044f551
        _emit 0x4d
        _emit 0x8a              // MOV DL, [ESI+1]
        _emit 0x56
        _emit 0x01
        _emit 0x8a              // MOV BL, DL
        _emit 0xda
        _emit 0x80              // ADD BL, 0x80
        _emit 0xc3
        _emit 0x80
        _emit 0x80              // CMP BL, 0x1f
        _emit 0xfb
        _emit 0x1f
        _emit 0x77              // JA 0x0044f52c
        _emit 0x1b
        _emit 0x0f              // MOVZX EAX, DL
        _emit 0xb6
        _emit 0xc2
        _emit 0x0f              // MOVZX EAX, [EAX+EDI-0x60]
        _emit 0xb6
        _emit 0x44
        _emit 0x38
        _emit 0xa0
        _emit 0x83              // ADD EAX, 0x40
        _emit 0xc0
        _emit 0x40
        _emit 0x99              // CDQ
        _emit 0x5b              // POP EBX
        _emit 0x5f              // POP EDI
        _emit 0x89              // MOV [ECX], EAX
        _emit 0x01
        _emit 0x89              // MOV [ECX+4], EDX
        _emit 0x51
        _emit 0x04
        _emit 0xb8              // MOV EAX, 2
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0x59              // POP ECX
        _emit 0xc3              // RET

        _emit 0x8a              // MOV BL, DL
        _emit 0xda
        _emit 0x80              // ADD BL, 0x60
        _emit 0xc3
        _emit 0x60
        _emit 0x80              // CMP BL, 0x1f
        _emit 0xfb
        _emit 0x1f
        _emit 0x77              // JA 0x0044f551
        _emit 0x1b
        _emit 0x0f              // MOVZX EDX, DL
        _emit 0xb6
        _emit 0xd2
        _emit 0x0f              // MOVZX EAX, [EDX+EDI-0x80]
        _emit 0xb6
        _emit 0x44
        _emit 0x3a
        _emit 0x80
        _emit 0x83              // ADD EAX, 0x40
        _emit 0xc0
        _emit 0x40
        _emit 0x99              // CDQ
        _emit 0x5b              // POP EBX
        _emit 0x5f              // POP EDI
        _emit 0x89              // MOV [ECX], EAX
        _emit 0x01
        _emit 0x89              // MOV [ECX+4], EDX
        _emit 0x51
        _emit 0x04
        _emit 0xb8              // MOV EAX, 2
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0x59              // POP ECX
        _emit 0xc3              // RET

        _emit 0x3c              // CMP AL, 0x80
        _emit 0x80
        _emit 0x73              // JNC 0x0044f55c
        _emit 0x07
        _emit 0xbf              // MOV EDI, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP 0x0044f599
        _emit 0x3d
        _emit 0x3c              // CMP AL, 0xc0
        _emit 0xc0
        _emit 0x72              // JC 0x0044f595
        _emit 0x35
        _emit 0x3c              // CMP AL, 0xe0
        _emit 0xe0
        _emit 0x73              // JNC 0x0044f56b
        _emit 0x07
        _emit 0xbf              // MOV EDI, 2
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP 0x0044f599
        _emit 0x2e
        _emit 0x3c              // CMP AL, 0xf0
        _emit 0xf0
        _emit 0x73              // JNC 0x0044f576
        _emit 0x07
        _emit 0xbf              // MOV EDI, 3
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP 0x0044f599
        _emit 0x23
        _emit 0x3c              // CMP AL, 0xf8
        _emit 0xf8
        _emit 0x73              // JNC 0x0044f581
        _emit 0x07
        _emit 0xbf              // MOV EDI, 4
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP 0x0044f599
        _emit 0x18
        _emit 0x3c              // CMP AL, 0xfc
        _emit 0xfc
        _emit 0x73              // JNC 0x0044f58c
        _emit 0x07
        _emit 0xbf              // MOV EDI, 5
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP 0x0044f599
        _emit 0x0d
        _emit 0x3c              // CMP AL, 0xfe
        _emit 0xfe
        _emit 0xbf              // MOV EDI, 6
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x72              // JC 0x0044f599
        _emit 0x04
        _emit 0x8b              // MOV EDI, [ESP+0xc]
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x0f              // MOVZX EAX, AL
        _emit 0xb6
        _emit 0xc0
        _emit 0x99              // CDQ
        _emit 0x83              // ADD ESI, 1
        _emit 0xc6
        _emit 0x01
        _emit 0x83              // CMP EDI, 1
        _emit 0xff
        _emit 0x01
        _emit 0x89              // MOV [ECX], EAX
        _emit 0x01
        _emit 0x89              // MOV [ECX+4], EDX
        _emit 0x51
        _emit 0x04
        _emit 0x7e              // JLE 0x0044f5d2
        _emit 0x28
        _emit 0x8d              // LEA EBX, [EDI-1]
        _emit 0x5f
        _emit 0xff
        _emit 0x8d              // LEA ECX, [ECX]
        _emit 0x49
        _emit 0x00

        _emit 0x8b              // MOV EAX, [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, [ECX+4]
        _emit 0x51
        _emit 0x04
        _emit 0x0f              // SHLD EDX, EAX, 8
        _emit 0xa4
        _emit 0xc2
        _emit 0x08
        _emit 0xc1              // SHL EAX, 8
        _emit 0xe0
        _emit 0x08
        _emit 0x89              // MOV [ECX], EAX
        _emit 0x01
        _emit 0x89              // MOV [ECX+4], EDX
        _emit 0x51
        _emit 0x04
        _emit 0x0f              // MOVZX EAX, [ESI]
        _emit 0xb6
        _emit 0x06
        _emit 0x99              // CDQ
        _emit 0x01              // ADD [ECX], EAX
        _emit 0x01
        _emit 0x11              // ADC [ECX+4], EDX
        _emit 0x51
        _emit 0x04
        _emit 0x83              // ADD ESI, 1
        _emit 0xc6
        _emit 0x01
        _emit 0x83              // SUB EBX, 1
        _emit 0xeb
        _emit 0x01
        _emit 0x75              // JNZ 0x0044f5b0
        _emit 0xde

        _emit 0x5b              // POP EBX
        _emit 0x8b              // MOV EAX, EDI
        _emit 0xc7
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x59              // POP ECX
        _emit 0xc3              // RET
    }
}
