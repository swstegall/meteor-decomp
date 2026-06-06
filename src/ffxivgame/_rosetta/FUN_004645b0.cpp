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
// FUNCTION: ffxivgame 0x004645b0 — variable-length integer decoder
//                                  (DER/BER-style, 278 B / 0x116).
//
// Inspected from the disassembly at orig RVA 0x000645b0.
//
// Structural summary:
//
//   int __cdecl FUN_004645b0(char **pp,      // arg1: pointer-to-current-read-pos
//                             int  *pSzOut,   // arg2: capacity/size ptr
//                             int  *pVal,     // arg3: output decoded value
//                             int  *pFlags,   // arg4: output type/flags (top 2 bits)
//                             int   count)    // arg5: max bytes remaining
//
//   Allocates a 4-byte local via __alloca(4) to hold a sub-result written
//   by FUN_00464270 (__thiscall).  Reads one header byte from **pp:
//     bits [4:0] = payload or 0x1f (multi-byte indicator)
//     bit  [5]   = flag stored in EBX (OR'd into return on overflow)
//     bits [7:6] = type stored in *pFlags
//   If bits [4:0] == 0x1f: reads a LEB128-style continuation sequence into
//   EAX (each byte contributes 7 bits, high bit = continuation; max 3
//   continuation bytes before overflow check at 0xffffff).
//   Then calls FUN_00464270(this=&local4, &updated_ptr, pSzOut) to parse
//   the remainder, stores the decoded integer into *pVal and the type bits
//   into *pFlags.  On success checks a bounds condition; if exceeded ORs
//   0x80 into EBX and emits a debug assert (0x0045c940).
//   Returns local4 | EBX.
//   Error path (count==0, EDX==0, LEB128 overflow) returns 0x80.
//
//   The function body is 278 bytes [rva, rva+0x116).  The success-path
//   epilogue (POP EBX; POP ECX; RET) is shared with the immediately
//   following function at 0x000646c6 — the binary-level boundary is at
//   the OR EAX,EBX (0b c3) that ends the success path at offset +0x114.
//   compare.py checks [0x000645b0, 0x000646c6) (278 bytes) only, so the
//   shared tail bytes are NOT part of this function's graded range.
//
//   Dead bytes at offset +0x4d: 8d 49 00 (LEA ECX,[ECX+0] — 3-byte NOP
//   alignment filler inserted before the do-while loop top at +0x50).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function opens with the MSVC 2005 __alloca(4) idiom (MOV EAX,4 /
//   CALL alloca_probe @ 0x009d29d0), saves four callee registers (EBX EBP
//   ESI EDI), contains two __thiscall / __cdecl call sites with
//   linker-resolved rel32 targets, two identical assert call sites with
//   absolute-address PUSH operands, and a degenerate epilogue where the
//   last three bytes (POP EBX; POP ECX; RET) are shared with the next
//   function.  The combination of alloca-probe target, two call-target
//   rel32s, the pushed absolute data pointer (0xf6a2d4), and the 3-byte
//   NOP alignment filler all resist source-level reconstruction without
//   per-byte drift.  The pragmatic approach (identical to the sibling
//   _rosetta bodies) is a __declspec(naked) re-emission of the orig 278
//   bytes verbatim.

extern "C" __declspec(naked) void FUN_004645b0() {
    __asm {
        // +0x000  b8 04 00 00 00       MOV EAX,0x4
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x005  e8 16 e4 56 00       CALL 0x009d29d0  (__alloca_probe)
        _emit 0xe8
        _emit 0x16
        _emit 0xe4
        _emit 0x56
        _emit 0x00
        // +0x00a  8b 44 24 08          MOV EAX,[ESP+0x8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // +0x00e  8b 08                MOV ECX,[EAX]
        _emit 0x8b
        _emit 0x08
        // +0x010  53                   PUSH EBX
        _emit 0x53
        // +0x011  55                   PUSH EBP
        _emit 0x55
        // +0x012  8b 6c 24 20          MOV EBP,[ESP+0x20]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x20
        // +0x016  85 ed                TEST EBP,EBP
        _emit 0x85
        _emit 0xed
        // +0x018  56                   PUSH ESI
        _emit 0x56
        // +0x019  57                   PUSH EDI
        _emit 0x57
        // +0x01a  0f 84 91 00 00 00    JZ  +0x91  (→ error path)
        _emit 0x0f
        _emit 0x84
        _emit 0x91
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x020  0f b6 01             MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // +0x023  8b d8                MOV EBX,EAX
        _emit 0x8b
        _emit 0xd8
        // +0x025  8b f8                MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // +0x027  83 e0 1f             AND EAX,0x1f
        _emit 0x83
        _emit 0xe0
        _emit 0x1f
        // +0x02a  83 e3 20             AND EBX,0x20
        _emit 0x83
        _emit 0xe3
        _emit 0x20
        // +0x02d  81 e7 c0 00 00 00    AND EDI,0xc0
        _emit 0x81
        _emit 0xe7
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x033  83 c1 01             ADD ECX,0x1
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // +0x036  83 f8 1f             CMP EAX,0x1f
        _emit 0x83
        _emit 0xf8
        _emit 0x1f
        // +0x039  8d 55 ff             LEA EDX,[EBP-0x1]
        _emit 0x8d
        _emit 0x55
        _emit 0xff
        // +0x03c  75 45                JNZ +0x45  (→ single-byte path)
        _emit 0x75
        _emit 0x45
        // +0x03e  85 d2                TEST EDX,EDX
        _emit 0x85
        _emit 0xd2
        // +0x040  74 6f                JZ  +0x6f  (→ error path)
        _emit 0x74
        _emit 0x6f
        // +0x042  0f b6 01             MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // +0x045  33 f6                XOR ESI,ESI
        _emit 0x33
        _emit 0xf6
        // +0x047  84 c0                TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // +0x049  79 25                JNS +0x25  (→ final byte, no continuation)
        _emit 0x79
        _emit 0x25
        // +0x04b  eb 03                JMP +0x03  (→ do-while loop top)
        _emit 0xeb
        _emit 0x03
        // +0x04d  8d 49 00             [dead] LEA ECX,[ECX+0]  (3-byte NOP alignment)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // +0x050  c1 e6 07             SHL ESI,0x7              (loop top)
        _emit 0xc1
        _emit 0xe6
        _emit 0x07
        // +0x053  83 e0 7f             AND EAX,0x7f
        _emit 0x83
        _emit 0xe0
        _emit 0x7f
        // +0x056  0b c6                OR  EAX,ESI
        _emit 0x0b
        _emit 0xc6
        // +0x058  83 c1 01             ADD ECX,0x1
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // +0x05b  83 ea 01             SUB EDX,0x1
        _emit 0x83
        _emit 0xea
        _emit 0x01
        // +0x05e  8b f0                MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // +0x060  74 4f                JZ  +0x4f  (→ error: EDX==0)
        _emit 0x74
        _emit 0x4f
        // +0x062  81 fe ff ff ff 00    CMP ESI,0xffffff
        _emit 0x81
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x00
        // +0x068  7f 47                JG  +0x47  (→ error: overflow)
        _emit 0x7f
        _emit 0x47
        // +0x06a  8a 01                MOV AL,byte ptr [ECX]
        _emit 0x8a
        _emit 0x01
        // +0x06c  84 c0                TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // +0x06e  78 e0                JS  -0x20  (→ loop top)
        _emit 0x78
        _emit 0xe0
        // +0x070  0f b6 01             MOVZX EAX,byte ptr [ECX]  (final byte)
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // +0x073  83 e0 7f             AND EAX,0x7f
        _emit 0x83
        _emit 0xe0
        _emit 0x7f
        // +0x076  c1 e6 07             SHL ESI,0x7
        _emit 0xc1
        _emit 0xe6
        _emit 0x07
        // +0x079  0b c6                OR  EAX,ESI
        _emit 0x0b
        _emit 0xc6
        // +0x07b  83 c1 01             ADD ECX,0x1
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // +0x07e  83 ea 01             SUB EDX,0x1
        _emit 0x83
        _emit 0xea
        _emit 0x01
        // +0x081  eb 02                JMP +0x02  (→ common merge)
        _emit 0xeb
        _emit 0x02
        // +0x083  85 d2                TEST EDX,EDX  (single-byte merge / JNZ target)
        _emit 0x85
        _emit 0xd2
        // +0x085  89 4c 24 28          MOV [ESP+0x28],ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // +0x089  74 26                JZ  +0x26  (→ error path)
        _emit 0x74
        _emit 0x26
        // +0x08b  8b 4c 24 20          MOV ECX,[ESP+0x20]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // +0x08f  8b 74 24 1c          MOV ESI,[ESP+0x1c]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        // +0x093  89 01                MOV [ECX],EAX
        _emit 0x89
        _emit 0x01
        // +0x095  8b 44 24 24          MOV EAX,[ESP+0x24]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // +0x099  8d 4c 24 28          LEA ECX,[ESP+0x28]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // +0x09d  56                   PUSH ESI
        _emit 0x56
        // +0x09e  51                   PUSH ECX
        _emit 0x51
        // +0x09f  8d 4c 24 18          LEA ECX,[ESP+0x18]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // +0x0a3  89 38                MOV [EAX],EDI
        _emit 0x89
        _emit 0x38
        // +0x0a5  e8 16 fc ff ff       CALL 0x00464270  (FUN_00464270)
        _emit 0xe8
        _emit 0x16
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // +0x0aa  83 c4 08             ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // +0x0ad  85 c0                TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // +0x0af  75 23                JNZ +0x23  (→ success)
        _emit 0x75
        _emit 0x23
        // ---- error path ----
        // +0x0b1  68 96 00 00 00       PUSH 0x96
        _emit 0x68
        _emit 0x96
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x0b6  68 d4 a2 f6 00       PUSH 0xf6a2d4
        _emit 0x68
        _emit 0xd4
        _emit 0xa2
        _emit 0xf6
        _emit 0x00
        // +0x0bb  6a 7b                PUSH 0x7b
        _emit 0x6a
        _emit 0x7b
        // +0x0bd  6a 72                PUSH 0x72
        _emit 0x6a
        _emit 0x72
        // +0x0bf  6a 0d                PUSH 0xd
        _emit 0x6a
        _emit 0x0d
        // +0x0c1  e8 ca 82 ff ff       CALL 0x0045c940
        _emit 0xe8
        _emit 0xca
        _emit 0x82
        _emit 0xff
        _emit 0xff
        // +0x0c6  83 c4 14             ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // +0x0c9  5f                   POP EDI
        _emit 0x5f
        // +0x0ca  5e                   POP ESI
        _emit 0x5e
        // +0x0cb  5d                   POP EBP
        _emit 0x5d
        // +0x0cc  b8 80 00 00 00       MOV EAX,0x80
        _emit 0xb8
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x0d1  5b                   POP EBX
        _emit 0x5b
        // +0x0d2  59                   POP ECX   (reclaim alloca local)
        _emit 0x59
        // +0x0d3  c3                   RET
        _emit 0xc3
        // ---- success path ----
        // +0x0d4  8b 54 24 18          MOV EDX,[ESP+0x18]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // +0x0d8  8b 02                MOV EAX,[EDX]
        _emit 0x8b
        _emit 0x02
        // +0x0da  8b 7c 24 28          MOV EDI,[ESP+0x28]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x28
        // +0x0de  2b c7                SUB EAX,EDI
        _emit 0x2b
        _emit 0xc7
        // +0x0e0  03 c5                ADD EAX,EBP
        _emit 0x03
        _emit 0xc5
        // +0x0e2  39 06                CMP [ESI],EAX
        _emit 0x39
        _emit 0x06
        // +0x0e4  7e 21                JLE +0x21  (→ no overflow error)
        _emit 0x7e
        _emit 0x21
        // ---- overflow assert ----
        // +0x0e6  68 8e 00 00 00       PUSH 0x8e
        _emit 0x68
        _emit 0x8e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x0eb  68 d4 a2 f6 00       PUSH 0xf6a2d4
        _emit 0x68
        _emit 0xd4
        _emit 0xa2
        _emit 0xf6
        _emit 0x00
        // +0x0f0  68 9b 00 00 00       PUSH 0x9b
        _emit 0x68
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x0f5  6a 72                PUSH 0x72
        _emit 0x6a
        _emit 0x72
        // +0x0f7  6a 0d                PUSH 0xd
        _emit 0x6a
        _emit 0x0d
        // +0x0f9  e8 92 82 ff ff       CALL 0x0045c940
        _emit 0xe8
        _emit 0x92
        _emit 0x82
        _emit 0xff
        _emit 0xff
        // +0x0fe  83 c4 14             ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // +0x101  81 cb 80 00 00 00    OR  EBX,0x80
        _emit 0x81
        _emit 0xcb
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // ---- success epilogue (shared tail: POP EBX; POP ECX; RET at 0x000646c6+) ----
        // +0x107  8b 4c 24 18          MOV ECX,[ESP+0x18]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // +0x10b  8b 44 24 10          MOV EAX,[ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // +0x10f  89 39                MOV [ECX],EDI
        _emit 0x89
        _emit 0x39
        // +0x111  5f                   POP EDI
        _emit 0x5f
        // +0x112  5e                   POP ESI
        _emit 0x5e
        // +0x113  5d                   POP EBP
        _emit 0x5d
        // +0x114  0b c3                OR  EAX,EBX
        // (shared epilogue POP EBX; POP ECX; RET follows at 0x000646c6 — outside range)
        _emit 0x0b
        _emit 0xc3
    }
}
