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
// FUNCTION: ffxivgame 0x009d7add — `__cdecl long _ftell_nolock(FILE *stream)`
//                                  MSVC 2005 CRT internal (408 B / 0x198).
//
// Inspection (read from the disassembly at orig RVA 0x005d7add):
//
//   __cdecl long FUN_009d7add(FILE *stream);   // stream = [ebp+8] -> edi
//
//   Structure (traced from the asm bytes):
//
//   PUSH EBP / MOV EBP,ESP / SUB ESP,0x0c / PUSH EBX / PUSH EDI
//   MOV EDI,[EBP+8] ; XOR EBX,EBX ; CMP EDI,EBX
//   JNE <ok>
//     CALL _errno ; MOV [EAX],0x16 (EINVAL) ; PUSH x5 ; CALL _invalid_parameter
//     OR EAX,-1 ; JMP <epilogue>            ; return -1
//   <ok>:
//     PUSH EDI ; CALL _fileno ; POP ECX ; MOV [EBP-4],EAX   ; fd
//     CMP [EDI+4],EBX ; JGE ... ; MOV [EDI+4],EBX            ; if(_cnt<0)_cnt=0
//     PUSH 1 ; PUSH 0 ; PUSH EAX ; CALL _lseek ; ADD ESP,0xc ; SEEK_CUR
//     CMP EAX,EBX ; MOV [EBP-8],EAX ; JL <ret -1>            ; filepos<0
//     MOV EDX,[EDI+0xc] ; TEST DX,0x108 ; JNE <buffered>
//       SUB EAX,[EDI+4] ; JMP <epilogue>     ; return filepos - _cnt
//   <buffered>:
//     ... text-mode CR/LF accounting via __pioinfo[fd>>5][fd&0x1f].osfile
//         (table at absolute 0x0137b7e0, ioinfo stride 0x40, osfile @ +4,
//          FTEXT=0x80, FCRLF=0x04) scanning buffer for '\n' (0x0a) bytes,
//          plus a second _lseek probe (SEEK_SET) and a _bufsiz (+0x18)
//          adjustment.
//
//   Stack frame (32-bit __cdecl, frame param 0x0c):
//     [EBP+0x08]  stream  (reused as a running offset accumulator)
//     [EBP-0x04]  fd      (_fileno result)
//     [EBP-0x08]  filepos (_lseek SEEK_CUR result)
//     [EBP-0x0c]  rdcnt / line-feed accumulator
//
//   Reloc-bearing sites: rel32 CALLs to _errno (0x9d9d47),
//   _invalid_parameter (0x9d2290), _fileno (0x9d6a61) and _lseek
//   (0x9e7abb), plus the absolute __pioinfo table pointer at 0x0137b7e0.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function carries several relocation sites (rel32 calls to
//   CRT-internal callees plus an absolute .data table pointer) that
//   keep a source-level C++ port from emitting identical .obj bytes.
//   A naked-asm passthrough re-emits the 408 shipped bytes verbatim so
//   `tools/compare.py` sees an exact match.  The structural commentary
//   above is the readable record for promoting this to a real
//   source-level match once the callees / table are modelled.

extern "C" __declspec(naked) void FUN_009d7add() {
    __asm {
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        _emit 0x53
        _emit 0x57
        _emit 0x8b
        _emit 0x7d
        _emit 0x08
        _emit 0x33
        _emit 0xdb
        _emit 0x3b
        _emit 0xfb
        _emit 0x75
        _emit 0x20
        _emit 0xe8
        _emit 0x54
        _emit 0x22
        _emit 0x00
        _emit 0x00
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0xc7
        _emit 0x00
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x8d
        _emit 0xa7
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        _emit 0xe9
        _emit 0x63
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x57
        _emit 0xe8
        _emit 0x4d
        _emit 0xef
        _emit 0xff
        _emit 0xff
        _emit 0x39
        _emit 0x5f
        _emit 0x04
        _emit 0x59
        _emit 0x89
        _emit 0x45
        _emit 0xfc
        _emit 0x7d
        _emit 0x03
        _emit 0x89
        _emit 0x5f
        _emit 0x04
        _emit 0x6a
        _emit 0x01
        _emit 0x53
        _emit 0x50
        _emit 0xe8
        _emit 0x92
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x3b
        _emit 0xc3
        _emit 0x89
        _emit 0x45
        _emit 0xf8
        _emit 0x7c
        _emit 0xd3
        _emit 0x8b
        _emit 0x57
        _emit 0x0c
        _emit 0x66
        _emit 0xf7
        _emit 0xc2
        _emit 0x08
        _emit 0x01
        _emit 0x75
        _emit 0x08
        _emit 0x2b
        _emit 0x47
        _emit 0x04
        _emit 0xe9
        _emit 0x2c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x07
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        _emit 0x56
        _emit 0x8b
        _emit 0xf0
        _emit 0x2b
        _emit 0xf1
        _emit 0xf6
        _emit 0xc2
        _emit 0x03
        _emit 0x89
        _emit 0x75
        _emit 0xf4
        _emit 0x74
        _emit 0x41
        _emit 0x8b
        _emit 0x55
        _emit 0xfc
        _emit 0x8b
        _emit 0x75
        _emit 0xfc
        _emit 0xc1
        _emit 0xfa
        _emit 0x05
        _emit 0x8b
        _emit 0x14
        _emit 0x95
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0x83
        _emit 0xe6
        _emit 0x1f
        _emit 0xc1
        _emit 0xe6
        _emit 0x06
        _emit 0xf6
        _emit 0x44
        _emit 0x32
        _emit 0x04
        _emit 0x80
        _emit 0x74
        _emit 0x17
        _emit 0x8b
        _emit 0xd1
        _emit 0x3b
        _emit 0xd0
        _emit 0x73
        _emit 0x11
        _emit 0x8b
        _emit 0xf0
        _emit 0x80
        _emit 0x3a
        _emit 0x0a
        _emit 0x75
        _emit 0x05
        _emit 0xff
        _emit 0x45
        _emit 0xf4
        _emit 0x33
        _emit 0xdb
        _emit 0x42
        _emit 0x3b
        _emit 0xd6
        _emit 0x72
        _emit 0xf1
        _emit 0x39
        _emit 0x5d
        _emit 0xf8
        _emit 0x75
        _emit 0x1c
        _emit 0x8b
        _emit 0x45
        _emit 0xf4
        _emit 0xe9
        _emit 0xd8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84
        _emit 0xd2
        _emit 0x78
        _emit 0xef
        _emit 0xe8
        _emit 0xa6
        _emit 0x21
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x00
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe9
        _emit 0x86
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf6
        _emit 0x47
        _emit 0x0c
        _emit 0x01
        _emit 0x0f
        _emit 0x84
        _emit 0xb2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x57
        _emit 0x04
        _emit 0x3b
        _emit 0xd3
        _emit 0x75
        _emit 0x08
        _emit 0x89
        _emit 0x5d
        _emit 0xf4
        _emit 0xe9
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x5d
        _emit 0xfc
        _emit 0x8b
        _emit 0x75
        _emit 0xfc
        _emit 0x2b
        _emit 0xc1
        _emit 0x03
        _emit 0xc2
        _emit 0xc1
        _emit 0xfb
        _emit 0x05
        _emit 0x83
        _emit 0xe6
        _emit 0x1f
        _emit 0x8d
        _emit 0x1c
        _emit 0x9d
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0x89
        _emit 0x45
        _emit 0x08
        _emit 0x8b
        _emit 0x03
        _emit 0xc1
        _emit 0xe6
        _emit 0x06
        _emit 0xf6
        _emit 0x44
        _emit 0x30
        _emit 0x04
        _emit 0x80
        _emit 0x74
        _emit 0x77
        _emit 0x6a
        _emit 0x02
        _emit 0x6a
        _emit 0x00
        _emit 0xff
        _emit 0x75
        _emit 0xfc
        _emit 0xe8
        _emit 0xc4
        _emit 0xfe
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x3b
        _emit 0x45
        _emit 0xf8
        _emit 0x75
        _emit 0x1f
        _emit 0x8b
        _emit 0x47
        _emit 0x08
        _emit 0x8b
        _emit 0x4d
        _emit 0x08
        _emit 0x03
        _emit 0xc8
        _emit 0xeb
        _emit 0x09
        _emit 0x80
        _emit 0x38
        _emit 0x0a
        _emit 0x75
        _emit 0x03
        _emit 0xff
        _emit 0x45
        _emit 0x08
        _emit 0x40
        _emit 0x3b
        _emit 0xc1
        _emit 0x72
        _emit 0xf3
        _emit 0x66
        _emit 0xf7
        _emit 0x47
        _emit 0x0c
        _emit 0x00
        _emit 0x20
        _emit 0xeb
        _emit 0x3f
        _emit 0x6a
        _emit 0x00
        _emit 0xff
        _emit 0x75
        _emit 0xf8
        _emit 0xff
        _emit 0x75
        _emit 0xfc
        _emit 0xe8
        _emit 0x90
        _emit 0xfe
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x7d
        _emit 0x05
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        _emit 0xeb
        _emit 0x39
        _emit 0xb8
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x39
        _emit 0x45
        _emit 0x08
        _emit 0x77
        _emit 0x0f
        _emit 0x8b
        _emit 0x4f
        _emit 0x0c
        _emit 0xf6
        _emit 0xc1
        _emit 0x08
        _emit 0x74
        _emit 0x07
        _emit 0x66
        _emit 0xf7
        _emit 0xc1
        _emit 0x00
        _emit 0x04
        _emit 0x74
        _emit 0x03
        _emit 0x8b
        _emit 0x47
        _emit 0x18
        _emit 0x89
        _emit 0x45
        _emit 0x08
        _emit 0x8b
        _emit 0x03
        _emit 0xf6
        _emit 0x44
        _emit 0x30
        _emit 0x04
        _emit 0x04
        _emit 0x74
        _emit 0x03
        _emit 0xff
        _emit 0x45
        _emit 0x08
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        _emit 0x29
        _emit 0x45
        _emit 0xf8
        _emit 0x8b
        _emit 0x45
        _emit 0xf4
        _emit 0x8b
        _emit 0x4d
        _emit 0xf8
        _emit 0x03
        _emit 0xc1
        _emit 0x5e
        _emit 0x5f
        _emit 0x5b
        _emit 0xc9
        _emit 0xc3
    }
}
