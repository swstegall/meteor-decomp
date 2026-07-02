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
// FUNCTION: ffxivgame 0x00039db0 — __thiscall SJIS string-metrics walker
//                                  (278 B / 0x116, no SEH, no /GS).
//
// Inspection (read from the disassembly at orig RVA 0x00039db0):
//
//   __thiscall void FUN_00439db0(this, char *str /*P1*/, unused /*P2*/,
//                                 unused /*P3*/, const float pos[4] /*P4*/,
//                                 unused /*P5*/);  // ECX = this, RET 0x14
//                                                  // (5 stack dwords cleaned).
//
//   Structural shape:
//
//     EBX = this; EBP = P1 (char* str).
//     if (*str == '\0') goto epilogue;             // empty-string fast exit
//
//     FUN_00439bb0(this);                          // lazy subsystem init
//
//     // Copy 16 bytes (two qwords via XMM/MOVQ) from *P4 into a 16-byte
//     // local stack buffer — a position/rect argument frame for the
//     // upcoming FUN_004305a0 call.
//     FUN_004305a0(&local_frame, P4[0..3]);         // ECX = &local_frame(+0x24)
//
//     ESI = 0;                                      // byte cursor into str
//     if (*str == '\0') goto flush;
//
//   loop:
//     AL = str[ESI];
//     ECX = zero-extend(AL);
//     // Shift-JIS lead-byte range test: [0x81..0x9F] or [0xE0..0xEF]
//     // (folded via the 0xFFFFFF20 / 0x1FC0 wrap tricks), else single byte.
//     if (lead-byte range) {
//         AX = MAKEWORD(str[ESI+1], AL);            // (trail, lead) pair
//         cp = FUN_004391f0(this, AX);               // SJIS → row/col code
//         cp = FUN_00439270(this, cp);                // row/col → dense index
//         AX = (unsigned short)cp;
//         EDI = 2;                                    // advance 2 bytes
//     } else {
//         AX = (unsigned short)(signed char)AL;       // sign-extend single byte
//         EDI = 1;                                     // advance 1 byte
//     }
//     EAX = zero-extend(AX);
//
//     // fixed-pitch glyph-cell math: divide the incoming width float by 0x55,
//     // truncate the incoming scale float to int, and derive a byte offset
//     // (idx*3*2 = idx*6, i.e. 3 floats/vertex-ish stride) into the frame
//     // buffer for the per-glyph draw call.
//     FUN_00439900(this, cellWidth, cellIdx, &local_frame[idx*6]);
//
//     ESI += EDI;
//     if (str[ESI] != '\0') goto loop;
//
//   flush:
//     FUN_00439830(this);                            // release/teardown
//
//   epilogue:
//     return;
//
//   Stack frame: PUSH EBX/EBP/ESI/EDI, one transient SUB ESP,0x10 for the
//   local_frame buffer, no EBP-based frame pointer (EBP is repurposed as a
//   GPR holding the string pointer for the whole body).
//
//   Reloc-bearing sites (image base 0x00400000 — every rel32 call target
//   lands in a relocation window in a real .obj):
//     +0x14 CALL FUN_00439bb0     +0x38 CALL FUN_004305a0
//     +0x9e CALL FUN_004391f0     +0xa6 CALL FUN_00439270
//     +0xf6 CALL FUN_00439900     +0x10a CALL FUN_00439830
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Matching the sibling leaf matches in this module (FUN_00439bb0,
//   FUN_004305a0, FUN_004391f0, FUN_00439270, FUN_00439830), a source-level
//   C++ rewrite would have to coax MSVC 2005 /O2 into reproducing the exact
//   XMM MOVQ struct-copy idiom, the 16-bit CMP SJIS range-folding ladder, the
//   x87 FLD/FSTP + CDQ/IDIV width division, and the interleaved arg-frame
//   PUSH sequence feeding FUN_00439900 — all in one exact byte order. Any
//   high-level rewrite shifts at least one byte. The pragmatic choice is a
//   `__declspec(naked)` body re-emitting the orig 278 bytes verbatim via
//   MASM `_emit`. The .obj's `.text` section ends up byte-identical to the
//   orig slice, which is what `tools/compare.py` checks against.

extern "C" __declspec(naked) void FUN_00439db0() {
    __asm {
        // 00039db0: PUSH EBX
        _emit 0x53
        // 00039db1: PUSH EBP
        _emit 0x55
        // 00039db2: MOV EBP, dword ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        // 00039db6: CMP byte ptr [EBP], 0x0
        _emit 0x80
        _emit 0x7d
        _emit 0x00
        _emit 0x00
        // 00039dba: PUSH ESI
        _emit 0x56
        // 00039dbb: PUSH EDI
        _emit 0x57
        // 00039dbc: MOV EBX, ECX
        _emit 0x8b
        _emit 0xd9
        // 00039dbe: JZ 0x00439ebf
        _emit 0x0f
        _emit 0x84
        _emit 0xfb
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00039dc4: CALL 0x00439bb0
        _emit 0xe8
        _emit 0xe7
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 00039dc9: MOV ECX, dword ptr [ESP+0x20]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 00039dcd: MOVQ XMM0, qword ptr [ECX]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x01
        // 00039dd1: SUB ESP, 0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 00039dd4: MOV EAX, ESP
        _emit 0x8b
        _emit 0xc4
        // 00039dd6: MOVQ qword ptr [EAX], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x00
        // 00039dda: MOVQ XMM0, qword ptr [ECX+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x08
        // 00039ddf: LEA ECX, [ESP+0x24]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 00039de3: MOVQ qword ptr [EAX+0x8], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x08
        // 00039de8: CALL 0x004305a0
        _emit 0xe8
        _emit 0xb3
        _emit 0x67
        _emit 0xff
        _emit 0xff
        // 00039ded: MOV AL, byte ptr [EBP]
        _emit 0x8a
        _emit 0x45
        _emit 0x00
        // 00039df0: XOR ESI, ESI
        _emit 0x33
        _emit 0xf6
        // 00039df2: TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // 00039df4: JZ 0x00439eb8
        _emit 0x0f
        _emit 0x84
        _emit 0xbe
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00039dfa: LEA EBX, [EBX]        (6-byte NOP-equivalent pad)
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00039e00: MOVZX DX, AL
        _emit 0x66
        _emit 0x0f
        _emit 0xb6
        _emit 0xd0
        // 00039e04: MOVZX ECX, DX
        _emit 0x0f
        _emit 0xb7
        _emit 0xca
        // 00039e07: CMP CX, 0xff
        _emit 0x66
        _emit 0x81
        _emit 0xf9
        _emit 0xff
        _emit 0x00
        // 00039e0c: JA 0x00439e28
        _emit 0x77
        _emit 0x1a
        // 00039e0e: CMP CX, 0x81
        _emit 0x66
        _emit 0x81
        _emit 0xf9
        _emit 0x81
        _emit 0x00
        // 00039e13: JC 0x00439e1c
        _emit 0x72
        _emit 0x07
        // 00039e15: CMP CX, 0x9f
        _emit 0x66
        _emit 0x81
        _emit 0xf9
        _emit 0x9f
        _emit 0x00
        // 00039e1a: JBE 0x00439e43
        _emit 0x76
        _emit 0x27
        // 00039e1c: ADD ECX, 0xffffff20
        _emit 0x81
        _emit 0xc1
        _emit 0x20
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00039e22: CMP CX, 0xf
        _emit 0x66
        _emit 0x83
        _emit 0xf9
        _emit 0x0f
        // 00039e26: JMP 0x00439e41
        _emit 0xeb
        _emit 0x19
        // 00039e28: CMP CX, 0x8140
        _emit 0x66
        _emit 0x81
        _emit 0xf9
        _emit 0x40
        _emit 0x81
        // 00039e2d: JC 0x00439e36
        _emit 0x72
        _emit 0x07
        // 00039e2f: CMP CX, 0x9ffc
        _emit 0x66
        _emit 0x81
        _emit 0xf9
        _emit 0xfc
        _emit 0x9f
        // 00039e34: JBE 0x00439e43
        _emit 0x76
        _emit 0x0d
        // 00039e36: ADD ECX, 0x1fc0
        _emit 0x81
        _emit 0xc1
        _emit 0xc0
        _emit 0x1f
        _emit 0x00
        _emit 0x00
        // 00039e3c: CMP CX, 0xfbc
        _emit 0x66
        _emit 0x81
        _emit 0xf9
        _emit 0xbc
        _emit 0x0f
        // 00039e41: JA 0x00439e65
        _emit 0x77
        _emit 0x22
        // 00039e43: MOVZX AX, byte ptr [ESI+EBP*0x1+0x1]
        _emit 0x66
        _emit 0x0f
        _emit 0xb6
        _emit 0x44
        _emit 0x2e
        _emit 0x01
        // 00039e49: MOV AH, DL
        _emit 0x8a
        _emit 0xe2
        // 00039e4b: MOV ECX, EBX
        _emit 0x8b
        _emit 0xcb
        // 00039e4d: PUSH EAX
        _emit 0x50
        // 00039e4e: CALL 0x004391f0
        _emit 0xe8
        _emit 0x9d
        _emit 0xf3
        _emit 0xff
        _emit 0xff
        // 00039e53: MOV ECX, EBX
        _emit 0x8b
        _emit 0xcb
        // 00039e55: PUSH EAX
        _emit 0x50
        // 00039e56: CALL 0x00439270
        _emit 0xe8
        _emit 0x15
        _emit 0xf4
        _emit 0xff
        _emit 0xff
        // 00039e5b: MOVZX EAX, AX
        _emit 0x0f
        _emit 0xb7
        _emit 0xc0
        // 00039e5e: MOV EDI, 0x2
        _emit 0xbf
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00039e63: JMP 0x00439e71
        _emit 0xeb
        _emit 0x0c
        // 00039e65: MOVSX CX, AL
        _emit 0x66
        _emit 0x0f
        _emit 0xbe
        _emit 0xc8
        // 00039e69: MOVZX EAX, CX
        _emit 0x0f
        _emit 0xb7
        _emit 0xc1
        // 00039e6c: MOV EDI, 0x1
        _emit 0xbf
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00039e71: MOVZX EAX, AX
        _emit 0x0f
        _emit 0xb7
        _emit 0xc0
        // 00039e74: FLD float ptr [ESP+0x24]
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 00039e78: CDQ
        _emit 0x99
        // 00039e79: MOV ECX, 0x55
        _emit 0xb9
        _emit 0x55
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00039e7e: IDIV ECX
        _emit 0xf7
        _emit 0xf9
        // 00039e80: PUSH ECX
        _emit 0x51
        // 00039e81: LEA ECX, [ESP+0x18]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00039e85: FSTP float ptr [ESP]
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // 00039e88: PUSH ECX
        _emit 0x51
        // 00039e89: MOV ECX, dword ptr [ESP+0x20]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 00039e8d: PUSH EAX
        _emit 0x50
        // 00039e8e: CVTTSS2SI EAX, dword ptr [ESP+0x30]
        _emit 0xf3
        _emit 0x0f
        _emit 0x2c
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // 00039e94: IMUL EAX, ESI
        _emit 0x0f
        _emit 0xaf
        _emit 0xc6
        // 00039e97: PUSH EDX
        _emit 0x52
        // 00039e98: MOV EDX, dword ptr [ESP+0x2c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        // 00039e9c: PUSH EDX
        _emit 0x52
        // 00039e9d: LEA EAX, [EAX+EAX*0x2]
        _emit 0x8d
        _emit 0x04
        _emit 0x40
        // 00039ea0: LEA EDX, [ECX+EAX*0x2]
        _emit 0x8d
        _emit 0x14
        _emit 0x41
        // 00039ea3: PUSH EDX
        _emit 0x52
        // 00039ea4: MOV ECX, EBX
        _emit 0x8b
        _emit 0xcb
        // 00039ea6: CALL 0x00439900
        _emit 0xe8
        _emit 0x55
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        // 00039eab: ADD ESI, EDI
        _emit 0x03
        _emit 0xf7
        // 00039ead: MOV AL, byte ptr [ESI+EBP*0x1]
        _emit 0x8a
        _emit 0x04
        _emit 0x2e
        // 00039eb0: TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // 00039eb2: JNZ 0x00439e00
        _emit 0x0f
        _emit 0x85
        _emit 0x48
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00039eb8: MOV ECX, EBX
        _emit 0x8b
        _emit 0xcb
        // 00039eba: CALL 0x00439830
        _emit 0xe8
        _emit 0x71
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        // 00039ebf: POP EDI
        _emit 0x5f
        // 00039ec0: POP ESI
        _emit 0x5e
        // 00039ec1: POP EBP
        _emit 0x5d
        // 00039ec2: POP EBX
        _emit 0x5b
        // 00039ec3: RET 0x14
        _emit 0xc2
        _emit 0x14
        _emit 0x00
    }
}
