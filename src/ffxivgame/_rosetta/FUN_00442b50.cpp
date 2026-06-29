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
// FUNCTION: ffxivgame 0x00042b50 — backward copy of 36-byte elements between
//                                  two circular-buffer / deque-map structures
//                                  (208 B / 0xd0, __cdecl, no /GS frame).
//
// Asm shape (read from asm/ffxivgame/00042b50_FUN_00442b50.s,
// RVA 0x00042b50..0x00042c1f, 208 bytes):
//
//   Roughly equivalent to copy_backward for a pair of ring-buffer containers
//   that store arrays of element pointers.  Each element is 36 bytes (0x24).
//
//   Arguments (10 args, __cdecl):
//     arg0  [esp+0x14 after 4 saves] — result iterator output ptr (3 fields written)
//     arg1  [esp+0x18]               — (unused)
//     arg2  [esp+0x1c]               — (unused)
//     arg3  [esp+0x20]               — loop termination source index
//     arg4  [esp+0x24]               — (unused)
//     arg5  [esp+0x28]               — source circular-buffer struct ptr
//     arg6  [esp+0x2c]               — source start index (decremented each iter)
//     arg7  [esp+0x30]               — (unused)
//     arg8  [esp+0x34] → EBP         — dest circular-buffer struct ptr
//     arg9  [esp+0x38] → EBX         — dest start index (decremented each iter)
//
//   Circular-buffer struct layout (ptrs at +0x4, indices at +0x8/0xc/0x10):
//     +0x4  T** data       — pointer array
//     +0x8  unsigned base  — base index (subtracted when idx >= base)
//     +0xc  unsigned bnd1  — first bound (lower bound of valid range)
//     +0x10 unsigned bnd2  — second bound
//   Valid element pointer: data[ idx >= base ? idx - base : idx ]
//
//   Copy per iteration (36 bytes):
//     x87 FLD/FSTP for the first two floats (+0x00, +0x04),
//     MOV dword for the next two dwords (+0x08, +0x0c),
//     MOVQ XMM0 for two qwords (+0x10..+0x17, +0x18..+0x1f),
//     MOV dword for the trailing int (+0x20).
//
//   At exit, writes result iterator to arg0:
//     [arg0+0x0] = 0
//     [arg0+0x4] = EBP (dest container ptr)
//     [arg0+0x8] = EBX (final dest index)
//
//   Four assert/range-error calls at orig RVAs 0x009d22b4 are REL32
//   relocations; the four 4-byte displacement slots are masked by
//   compare.py (base-reloc table of the original PE).  All other
//   imm32 fields are intra-function relative branches — no relocations.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The opening push/mov interleaving (PUSH EBX; MOV EBX,[esp+0x2c];
//   PUSH EBP; MOV EBP,[esp+0x2c]; ...) that pre-loads EBX and EBP
//   before the remaining two pushes, the 4-byte alignment NOP
//   (LEA ESP,[ESP] = 8d 64 24 00), the mixed x87/XMM copy sequence
//   (FLD/FSTP for floats, MOVQ F3 prefix for loads, 66 prefix for
//   stores), and the exact branch-short/branch-near encoding choices
//   make source-level C++ at /O2 impractical to pin.  Naked-asm byte
//   passthrough (as used by FUN_00408910 and FUN_00403a20) is the
//   reliable path.

extern "C" __declspec(naked) void FUN_00442b50() {
    __asm {
        // 0x00  PUSH EBX
        _emit 0x53
        // 0x01  MOV EBX,[ESP+0x2c]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x2c
        // 0x05  PUSH EBP
        _emit 0x55
        // 0x06  MOV EBP,[ESP+0x2c]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x2c
        // 0x0a  PUSH ESI
        _emit 0x56
        // 0x0b  PUSH EDI
        _emit 0x57
        // 0x0c  LEA ESP,[ESP]  (4-byte NOP / alignment)
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
        // 0x10  loop_top: MOV ESI,[ESP+0x2c]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x2c
        // 0x14  CMP dword ptr [ESP+0x20],ESI
        _emit 0x39
        _emit 0x74
        _emit 0x24
        _emit 0x20
        // 0x18  JZ exit (near, +0x9d -> offset 0xbb)
        _emit 0x0f
        _emit 0x84
        _emit 0x9d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x1e  MOV EAX,[ESP+0x28]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0x22  SUB ESI,1
        _emit 0x83
        _emit 0xee
        _emit 0x01
        // 0x25  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0x27  MOV [ESP+0x2c],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x2c
        // 0x2b  JNZ +9 (to offset 0x36)
        _emit 0x75
        _emit 0x09
        // 0x2d  CALL 0x009d22b4  (REL32 reloc — displacement wildcarded)
        _emit 0xe8
        _emit 0x32
        _emit 0xf7
        _emit 0x58
        _emit 0x00
        // 0x32  MOV EAX,[ESP+0x28]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0x36  MOV ECX,[EAX+0x10]
        _emit 0x8b
        _emit 0x48
        _emit 0x10
        // 0x39  ADD ECX,[EAX+0xc]
        _emit 0x03
        _emit 0x48
        _emit 0x0c
        // 0x3c  CMP ESI,ECX
        _emit 0x3b
        _emit 0xf1
        // 0x3e  JC +9 (to offset 0x49)
        _emit 0x72
        _emit 0x09
        // 0x40  CALL 0x009d22b4  (REL32 reloc)
        _emit 0xe8
        _emit 0x1f
        _emit 0xf7
        _emit 0x58
        _emit 0x00
        // 0x45  MOV EAX,[ESP+0x28]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0x49  MOV ECX,[EAX+0x8]
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        // 0x4c  CMP ECX,ESI
        _emit 0x3b
        _emit 0xce
        // 0x4e  JA +2 (to offset 0x52)
        _emit 0x77
        _emit 0x02
        // 0x50  SUB ESI,ECX
        _emit 0x2b
        _emit 0xf1
        // 0x52  MOV EDX,[EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0x55  MOV ESI,[EDX+ESI*4]
        _emit 0x8b
        _emit 0x34
        _emit 0xb2
        // 0x58  SUB EBX,1
        _emit 0x83
        _emit 0xeb
        _emit 0x01
        // 0x5b  TEST EBP,EBP
        _emit 0x85
        _emit 0xed
        // 0x5d  MOV EDI,EBX
        _emit 0x8b
        _emit 0xfb
        // 0x5f  JNZ +5 (to offset 0x66)
        _emit 0x75
        _emit 0x05
        // 0x61  CALL 0x009d22b4  (REL32 reloc)
        _emit 0xe8
        _emit 0xfe
        _emit 0xf6
        _emit 0x58
        _emit 0x00
        // 0x66  MOV EAX,[EBP+0x10]
        _emit 0x8b
        _emit 0x45
        _emit 0x10
        // 0x69  ADD EAX,[EBP+0xc]
        _emit 0x03
        _emit 0x45
        _emit 0x0c
        // 0x6c  CMP EBX,EAX
        _emit 0x3b
        _emit 0xd8
        // 0x6e  JC +5 (to offset 0x75)
        _emit 0x72
        _emit 0x05
        // 0x70  CALL 0x009d22b4  (REL32 reloc)
        _emit 0xe8
        _emit 0xef
        _emit 0xf6
        _emit 0x58
        _emit 0x00
        // 0x75  MOV EAX,[EBP+0x8]
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        // 0x78  CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // 0x7a  JA +4 (to offset 0x80)
        _emit 0x77
        _emit 0x04
        // 0x7c  MOV EDI,EBX
        _emit 0x8b
        _emit 0xfb
        // 0x7e  SUB EDI,EAX
        _emit 0x2b
        _emit 0xf8
        // 0x80  MOV ECX,[EBP+0x4]
        _emit 0x8b
        _emit 0x4d
        _emit 0x04
        // 0x83  FLD float ptr [ESI]
        _emit 0xd9
        _emit 0x06
        // 0x85  MOV EAX,[ECX+EDI*4]
        _emit 0x8b
        _emit 0x04
        _emit 0xb9
        // 0x88  FSTP float ptr [EAX]
        _emit 0xd9
        _emit 0x18
        // 0x8a  FLD float ptr [ESI+0x4]
        _emit 0xd9
        _emit 0x46
        _emit 0x04
        // 0x8d  FSTP float ptr [EAX+0x4]
        _emit 0xd9
        _emit 0x58
        _emit 0x04
        // 0x90  MOV EDX,[ESI+0x8]
        _emit 0x8b
        _emit 0x56
        _emit 0x08
        // 0x93  MOV [EAX+0x8],EDX
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 0x96  MOV ECX,[ESI+0xc]
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        // 0x99  MOV [EAX+0xc],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x0c
        // 0x9c  MOVQ XMM0,qword ptr [ESI+0x10]  (F3 0F 7E prefix)
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x46
        _emit 0x10
        // 0xa1  MOVQ qword ptr [EAX+0x10],XMM0  (66 0F D6 prefix)
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x10
        // 0xa6  MOVQ XMM0,qword ptr [ESI+0x18]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x46
        _emit 0x18
        // 0xab  MOVQ qword ptr [EAX+0x18],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x18
        // 0xb0  MOV EDX,[ESI+0x20]
        _emit 0x8b
        _emit 0x56
        _emit 0x20
        // 0xb3  MOV [EAX+0x20],EDX
        _emit 0x89
        _emit 0x50
        _emit 0x20
        // 0xb6  JMP loop_top (near, -0xab -> offset 0x10)
        _emit 0xe9
        _emit 0x55
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0xbb  exit: MOV EAX,[ESP+0x14]  (= arg0)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0xbf  POP EDI
        _emit 0x5f
        // 0xc0  POP ESI
        _emit 0x5e
        // 0xc1  MOV [EAX+0x4],EBP
        _emit 0x89
        _emit 0x68
        _emit 0x04
        // 0xc4  POP EBP
        _emit 0x5d
        // 0xc5  MOV [EAX+0x8],EBX
        _emit 0x89
        _emit 0x58
        _emit 0x08
        // 0xc8  MOV dword ptr [EAX],0x0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0xce  POP EBX
        _emit 0x5b
        // 0xcf  RET
        _emit 0xc3
    }
}
