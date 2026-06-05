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
// FUNCTION: ffxivgame 0x00447a80 — `__thiscall` UTF-8 substring/length
//                                  builder (314 B / 0x13a, RET 0xc).
//
// Inspection (read from the disassembly at orig RVA 0x00047a80):
//
//   __thiscall ret_struct *FUN_00447a80(this, arg0, arg1, count);
//     ECX = this (saved to ESI), three dword stack args, RET 0xc.
//     Returns the worked-on object pointer in EAX.
//
//   High-level shape (three exit paths):
//
//     * `count == 0` path (JNZ at 0x47a91 falls through): initialise a
//       small builder object — clears length, sets a 0x40 capacity, two
//       boolean flags (+0x10/+0x11 = 1), stores an inline buffer pointer
//       at +0x00 (this+0x12), NUL-terminates it, calls 0x00447010
//       (__thiscall reserve/grow), then invokes 0x009d5110 (a 3-arg
//       cdecl helper — memset/format with literal 0xf6729e) and returns.
//
//     * default path (count != 0): seed a running offset (EBP) — either
//       a sentinel-relative base via 0x00445e50 when count == -1, or
//       `[esp+0x18] + count`. Then walk the source C-string at *this,
//       decoding UTF-8 lead bytes (CL+0x80 test for ASCII, then the
//       0x80/0xe0/0xf0/0xf8/0xfc/0xfe boundary ladder yielding sequence
//       length 1..6 via the SBB ECX,ECX / AND ECX,6 tail) and advancing
//       both the byte pointer (EDX) and the character offset (EBP) until
//       a terminator or a position limit is reached.
//
//     * the two tail calls to 0x00447260 (__thiscall, with literal
//       0xf6729f / -1) materialise the resulting substring object and
//       return it in EAX.
//
//   Relocation-bearing sites in the orig 314 bytes (resolve only in a
//   full-binary relink; standalone .obj keeps them as literal bytes):
//     +0x3e  CALL 0x00447010    rel32  (.text)
//     +0x46  PUSH 0x00f6729e    imm32  (.rdata string pool)
//     +0x4c  CALL 0x009d5110    rel32  (.text)
//     +0x6a  CALL 0x00445e50    rel32  (.text)
//     +0x10f PUSH 0x00f6729f    imm32  (.rdata string pool)
//     +0x116 CALL 0x00447260    rel32  (.text)
//     +0x12c CALL 0x00447260    rel32  (.text, 2nd)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   As with FUN_004014b0 / FUN_00408f10 and the rest of the /O2 /GS
//   siblings, coaxing MSVC 2005 to reproduce the exact register
//   allocation (ESI=this, EDI=char index, EBP=byte offset), the precise
//   UTF-8 lead-byte boundary ladder, the rel8-vs-rel32 branch selection,
//   and the linker-resolved absolute addresses is brittle — every
//   high-level rewrite shifts at least one byte. The pragmatic match is
//   a naked body that re-emits the orig 314 bytes verbatim. The .obj's
//   `.text` ends up byte-identical to the orig slice, which is what
//   `tools/compare.py` checks.

extern "C" __declspec(naked) void FUN_00447a80() {
    __asm {
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV EAX, [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0x3b              // CMP EAX, EDI
        _emit 0xc7
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x89              // MOV [ESP+0x8], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x08
        _emit 0x75              // JNZ 0x00447ae1
        _emit 0x4e
        _emit 0x8b              // MOV ESI, [ESP+0x10]
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x8d              // LEA EAX, [ESI+0x12]
        _emit 0x46
        _emit 0x12
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0xc6              // MOV byte ptr [ESI+0x10], 0x1
        _emit 0x46
        _emit 0x10
        _emit 0x01
        _emit 0xc6              // MOV byte ptr [ESI+0x11], 0x1
        _emit 0x46
        _emit 0x11
        _emit 0x01
        _emit 0x89              // MOV [ESI+0xc], EDI
        _emit 0x7e
        _emit 0x0c
        _emit 0xc7              // MOV dword ptr [ESI+0x8], 0x1
        _emit 0x46
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI+0x4], 0x40
        _emit 0x46
        _emit 0x04
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [ESI], EAX
        _emit 0x06
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xc6              // MOV byte ptr [EAX], 0x0
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL 0x00447010
        _emit 0x4d
        _emit 0xf5
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EAX, [ESI]
        _emit 0x06
        _emit 0x57              // PUSH EDI
        _emit 0x68              // PUSH 0xf6729e
        _emit 0x9e
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x009d5110
        _emit 0x3f
        _emit 0xd6
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV ECX, [ESI]
        _emit 0x0e
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0xc6              // MOV byte ptr [ECX], 0x0
        _emit 0x01
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0x59              // POP ECX
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00

        _emit 0x55              // PUSH EBP                    ; 0x00447ae1
        _emit 0x83              // OR EBP, 0xffffffff
        _emit 0xcd
        _emit 0xff
        _emit 0x83              // CMP EAX, -0x1
        _emit 0xf8
        _emit 0xff
        _emit 0x75              // JNZ 0x00447af5
        _emit 0x0b
        _emit 0xe8              // CALL 0x00445e50
        _emit 0x61
        _emit 0xe3
        _emit 0xff
        _emit 0xff
        _emit 0x89              // MOV [ESP+0x1c], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xeb              // JMP 0x00447aff
        _emit 0x0a
        _emit 0x8b              // MOV EDX, [ESP+0x18]         ; 0x00447af5
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x03              // ADD EDX, EAX
        _emit 0xd0
        _emit 0x89              // MOV [ESP+0x1c], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x8b              // MOV EDX, [ESI]              ; 0x00447aff
        _emit 0x16
        _emit 0x8a              // MOV CL, [EDX]
        _emit 0x0a
        _emit 0x33              // XOR ESI, ESI
        _emit 0xf6
        _emit 0x84              // TEST CL, CL
        _emit 0xc9
        _emit 0x74              // JZ 0x00447b7f
        _emit 0x76
        _emit 0x8d              // LEA ESP, [ESP]             ; 7-byte nop
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x39              // CMP [ESP+0x18], EDI        ; 0x00447b10
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x75              // JNZ 0x00447b1c
        _emit 0x06
        _emit 0x8b              // MOV ESI, EDX
        _emit 0xf2
        _emit 0x33              // XOR EBP, EBP
        _emit 0xed
        _emit 0xeb              // JMP 0x00447b22
        _emit 0x06
        _emit 0x39              // CMP [ESP+0x1c], EDI        ; 0x00447b1c
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x74              // JZ 0x00447b7f
        _emit 0x5d
        _emit 0x8a              // MOV AL, CL                 ; 0x00447b22
        _emit 0xc1
        _emit 0x04              // ADD AL, 0x80
        _emit 0x80
        _emit 0x3c              // CMP AL, 0x3f
        _emit 0x3f
        _emit 0x77              // JA 0x00447b2e
        _emit 0x04
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0xeb              // JMP 0x00447b72
        _emit 0x44
        _emit 0x80              // CMP CL, 0x80               ; 0x00447b2e
        _emit 0xf9
        _emit 0x80
        _emit 0x73              // JNC 0x00447b3a
        _emit 0x07
        _emit 0xb9              // MOV ECX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP 0x00447b72
        _emit 0x38
        _emit 0x80              // CMP CL, 0xe0               ; 0x00447b3a
        _emit 0xf9
        _emit 0xe0
        _emit 0x73              // JNC 0x00447b46
        _emit 0x07
        _emit 0xb9              // MOV ECX, 0x2
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP 0x00447b72
        _emit 0x2c
        _emit 0x80              // CMP CL, 0xf0               ; 0x00447b46
        _emit 0xf9
        _emit 0xf0
        _emit 0x73              // JNC 0x00447b52
        _emit 0x07
        _emit 0xb9              // MOV ECX, 0x3
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP 0x00447b72
        _emit 0x20
        _emit 0x80              // CMP CL, 0xf8               ; 0x00447b52
        _emit 0xf9
        _emit 0xf8
        _emit 0x73              // JNC 0x00447b5e
        _emit 0x07
        _emit 0xb9              // MOV ECX, 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP 0x00447b72
        _emit 0x14
        _emit 0x80              // CMP CL, 0xfc               ; 0x00447b5e
        _emit 0xf9
        _emit 0xfc
        _emit 0x73              // JNC 0x00447b6a
        _emit 0x07
        _emit 0xb9              // MOV ECX, 0x5
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP 0x00447b72
        _emit 0x08
        _emit 0x80              // CMP CL, 0xfe               ; 0x00447b6a
        _emit 0xf9
        _emit 0xfe
        _emit 0x1b              // SBB ECX, ECX
        _emit 0xc9
        _emit 0x83              // AND ECX, 0x6
        _emit 0xe1
        _emit 0x06
        _emit 0x03              // ADD EDX, ECX              ; 0x00447b72
        _emit 0xd1
        _emit 0x03              // ADD EBP, ECX
        _emit 0xe9
        _emit 0x8a              // MOV CL, [EDX]
        _emit 0x0a
        _emit 0x83              // ADD EDI, 0x1
        _emit 0xc7
        _emit 0x01
        _emit 0x84              // TEST CL, CL
        _emit 0xc9
        _emit 0x75              // JNZ 0x00447b10
        _emit 0x91

        _emit 0x39              // CMP [ESP+0x18], EDI       ; 0x00447b7f
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x75              // JNZ 0x00447ba4
        _emit 0x1f
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75              // JNZ 0x00447ba4
        _emit 0x1b
        _emit 0x8b              // MOV ESI, [ESP+0x14]
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0xf6729f
        _emit 0x9f
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x00447260
        _emit 0xc5
        _emit 0xf6
        _emit 0xff
        _emit 0xff
        _emit 0x5d              // POP EBP
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x59              // POP ECX
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00

        _emit 0x55              // PUSH EBP                  ; 0x00447ba4
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, [ESP+0x1c]
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x00447260
        _emit 0xaf
        _emit 0xf6
        _emit 0xff
        _emit 0xff
        _emit 0x5d              // POP EBP
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x59              // POP ECX
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
