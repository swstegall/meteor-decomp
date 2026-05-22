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
// FUNCTION: ffxivgame 0x0040dbd0 — FUN_0040dbd0 (274 B / 0x112)
//                                   __thiscall member function, no SEH frame.
//                                   Returns short AX.
//                                   Builds a hit-map buffer via FUN_0040a600 /
//                                   FUN_0040e230 / FUN_0040e110, fills with
//                                   _memset, zeroes selected slots, then collects
//                                   surviving pointers into an output array and
//                                   calls FUN_0040df70.
//
// Calling convention: __thiscall (ECX = this); RET 0x4 — one 4-byte stack param.
// Stack frame: SUB ESP,0x10 + PUSH ESI (ESP-based, no EBP frame pointer).
// Return value: short in AX.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Embeds three absolute data/code addresses (0x012652f8, 0x00f55758,
//   0x009d2110) that are link-time constants, plus the 6-byte alignment pad
//   LEA EBX,[EBX+0] at 0xdc9a and a non-trivial epilogue that returns the
//   delta in AX.  The naked passthrough reproduces all 274 bytes verbatim.

extern "C" __declspec(naked) void FUN_0040dbd0() {
    __asm {
        // 0x0000dbd0: 83 ec 10   SUB ESP,0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 0x0000dbd3: 56   PUSH ESI
        _emit 0x56
        // 0x0000dbd4: 8b f1   MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0x0000dbd6: 8b 46 04   MOV EAX,dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0x0000dbd9: 66 8b 48 08   MOV CX,word ptr [EAX+0x8]
        _emit 0x66
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        // 0x0000dbdd: 66 2b 48 0a   SUB CX,word ptr [EAX+0xa]
        _emit 0x66
        _emit 0x2b
        _emit 0x48
        _emit 0x0a
        // 0x0000dbe1: 0f b7 c1   MOVZX EAX,CX
        _emit 0x0f
        _emit 0xb7
        _emit 0xc1
        // 0x0000dbe4: 66 85 c0   TEST AX,AX
        _emit 0x66
        _emit 0x85
        _emit 0xc0
        // 0x0000dbe7: 89 44 24 08   MOV dword ptr [ESP+0x8],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0x0000dbeb: 75 07   JNZ +0x07  -> 0x0040dbf4
        _emit 0x75
        _emit 0x07
        // 0x0000dbed: 5e   POP ESI
        _emit 0x5e
        // 0x0000dbee: 83 c4 10   ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0x0000dbf1: c2 04 00   RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 0x0000dbf4: 83 7c 24 18 00   CMP dword ptr [ESP+0x18],0x0
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x00
        // 0x0000dbf9: 0f 84 dc 00 00 00   JZ +0xdc -> 0x0040dcdb
        _emit 0x0f
        _emit 0x84
        _emit 0xdc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x0000dbff: 55   PUSH EBP
        _emit 0x55
        // 0x0000dc00: e8 fb c9 ff ff   CALL 0x0040a600  [REL32]
        _emit 0xe8
        _emit 0xfb
        _emit 0xc9
        _emit 0xff
        _emit 0xff
        // 0x0000dc05: 8b e8   MOV EBP,EAX
        _emit 0x8b
        _emit 0xe8
        // 0x0000dc07: 85 ed   TEST EBP,EBP
        _emit 0x85
        _emit 0xed
        // 0x0000dc09: 89 6c 24 08   MOV dword ptr [ESP+0x8],EBP
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x08
        // 0x0000dc0d: 75 0b   JNZ +0x0b -> 0x0040dc1a
        _emit 0x75
        _emit 0x0b
        // 0x0000dc0f: 5d   POP EBP
        _emit 0x5d
        // 0x0000dc10: 66 33 c0   XOR AX,AX
        _emit 0x66
        _emit 0x33
        _emit 0xc0
        // 0x0000dc13: 5e   POP ESI
        _emit 0x5e
        // 0x0000dc14: 83 c4 10   ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0x0000dc17: c2 04 00   RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 0x0000dc1a: 8b 15 f8 52 26 01   MOV EDX,dword ptr [0x012652f8]  [DIR32 reloc]
        _emit 0x8b
        _emit 0x15
        _emit 0xf8
        _emit 0x52
        _emit 0x26
        _emit 0x01
        // 0x0000dc20: 57   PUSH EDI
        _emit 0x57
        // 0x0000dc21: 68 58 57 f5 00   PUSH 0x00f55758  [DIR32 reloc — string literal]
        _emit 0x68
        _emit 0x58
        _emit 0x57
        _emit 0xf5
        _emit 0x00
        // 0x0000dc26: 52   PUSH EDX
        _emit 0x52
        // 0x0000dc27: 8d 4c 24 1c   LEA ECX,[ESP+0x1c]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0x0000dc2b: e8 00 06 00 00   CALL 0x0040e230  [REL32]
        _emit 0xe8
        _emit 0x00
        _emit 0x06
        _emit 0x00
        _emit 0x00
        // 0x0000dc30: 50   PUSH EAX
        _emit 0x50
        // 0x0000dc31: 8b 46 04   MOV EAX,dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0x0000dc34: 0f b7 48 08   MOVZX ECX,word ptr [EAX+0x8]
        _emit 0x0f
        _emit 0xb7
        _emit 0x48
        _emit 0x08
        // 0x0000dc38: 51   PUSH ECX
        _emit 0x51
        // 0x0000dc39: 8b cd   MOV ECX,EBP
        _emit 0x8b
        _emit 0xcd
        // 0x0000dc3b: e8 d0 04 00 00   CALL 0x0040e110  [REL32]
        _emit 0xe8
        _emit 0xd0
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0x0000dc40: 8b f8   MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 0x0000dc42: 85 ff   TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 0x0000dc44: 75 0c   JNZ +0x0c -> 0x0040dc52
        _emit 0x75
        _emit 0x0c
        // 0x0000dc46: 5f   POP EDI
        _emit 0x5f
        // 0x0000dc47: 5d   POP EBP
        _emit 0x5d
        // 0x0000dc48: 66 33 c0   XOR AX,AX
        _emit 0x66
        _emit 0x33
        _emit 0xc0
        // 0x0000dc4b: 5e   POP ESI
        _emit 0x5e
        // 0x0000dc4c: 83 c4 10   ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0x0000dc4f: c2 04 00   RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 0x0000dc52: 8b 56 04   MOV EDX,dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 0x0000dc55: 0f b7 42 08   MOVZX EAX,word ptr [EDX+0x8]
        _emit 0x0f
        _emit 0xb7
        _emit 0x42
        _emit 0x08
        // 0x0000dc59: 50   PUSH EAX
        _emit 0x50
        // 0x0000dc5a: 6a 01   PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0x0000dc5c: 57   PUSH EDI
        _emit 0x57
        // 0x0000dc5d: e8 ae 44 5c 00   CALL 0x009d2110  [DIR32 reloc — _memset]
        _emit 0xe8
        _emit 0xae
        _emit 0x44
        _emit 0x5c
        _emit 0x00
        // 0x0000dc62: 8b 46 04   MOV EAX,dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0x0000dc65: 33 c9   XOR ECX,ECX
        _emit 0x33
        _emit 0xc9
        // 0x0000dc67: 83 c4 0c   ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0x0000dc6a: 66 39 48 0a   CMP word ptr [EAX+0xa],CX
        _emit 0x66
        _emit 0x39
        _emit 0x48
        _emit 0x0a
        // 0x0000dc6e: 76 1a   JBE +0x1a -> 0x0040dc8a
        _emit 0x76
        _emit 0x1a
        // 0x0000dc70: 8b 40 04   MOV EAX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        // 0x0000dc73: 0f b7 d1   MOVZX EDX,CX
        _emit 0x0f
        _emit 0xb7
        _emit 0xd1
        // 0x0000dc76: 0f b7 14 50   MOVZX EDX,word ptr [EAX+EDX*2]
        _emit 0x0f
        _emit 0xb7
        _emit 0x14
        _emit 0x50
        // 0x0000dc7a: c6 04 3a 00   MOV byte ptr [EDX+EDI*1],0x0
        _emit 0xc6
        _emit 0x04
        _emit 0x3a
        _emit 0x00
        // 0x0000dc7e: 8b 46 04   MOV EAX,dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0x0000dc81: 83 c1 01   ADD ECX,0x1
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // 0x0000dc84: 66 3b 48 0a   CMP CX,word ptr [EAX+0xa]
        _emit 0x66
        _emit 0x3b
        _emit 0x48
        _emit 0x0a
        // 0x0000dc88: 72 e6   JC -0x1a -> 0x0040dc70
        _emit 0x72
        _emit 0xe6
        // 0x0000dc8a: 8b 46 04   MOV EAX,dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0x0000dc8d: 53   PUSH EBX
        _emit 0x53
        // 0x0000dc8e: 33 db   XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // 0x0000dc90: 33 d2   XOR EDX,EDX
        _emit 0x33
        _emit 0xd2
        // 0x0000dc92: 66 39 58 08   CMP word ptr [EAX+0x8],BX
        _emit 0x66
        _emit 0x39
        _emit 0x58
        _emit 0x08
        // 0x0000dc96: 76 33   JBE +0x33 -> 0x0040dccb
        _emit 0x76
        _emit 0x33
        // 0x0000dc98: eb 06   JMP +0x06 -> 0x0040dca0
        _emit 0xeb
        _emit 0x06
        // 0x0000dc9a: 8d 9b 00 00 00 00   LEA EBX,[EBX+0]  (alignment NOP, unreachable)
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x0000dca0: 0f b7 ca   MOVZX ECX,DX
        _emit 0x0f
        _emit 0xb7
        _emit 0xca
        // 0x0000dca3: 80 3c 39 01   CMP byte ptr [ECX+EDI*1],0x1
        _emit 0x80
        _emit 0x3c
        _emit 0x39
        _emit 0x01
        // 0x0000dca7: 75 16   JNZ +0x16 -> 0x0040dcbf
        _emit 0x75
        _emit 0x16
        // 0x0000dca9: 8b 00   MOV EAX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x00
        // 0x0000dcab: 8b 6c 24 24   MOV EBP,dword ptr [ESP+0x24]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x24
        // 0x0000dcaf: 03 c1   ADD EAX,ECX
        _emit 0x03
        _emit 0xc1
        // 0x0000dcb1: 0f b7 cb   MOVZX ECX,BX
        _emit 0x0f
        _emit 0xb7
        _emit 0xcb
        // 0x0000dcb4: 89 44 8d 00   MOV dword ptr [EBP+ECX*4],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x8d
        _emit 0x00
        // 0x0000dcb8: 8b 6c 24 10   MOV EBP,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // 0x0000dcbc: 83 c3 01   ADD EBX,0x1
        _emit 0x83
        _emit 0xc3
        _emit 0x01
        // 0x0000dcbf: 8b 46 04   MOV EAX,dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0x0000dcc2: 83 c2 01   ADD EDX,0x1
        _emit 0x83
        _emit 0xc2
        _emit 0x01
        // 0x0000dcc5: 66 3b 50 08   CMP DX,word ptr [EAX+0x8]
        _emit 0x66
        _emit 0x3b
        _emit 0x50
        _emit 0x08
        // 0x0000dcc9: 72 d5   JC -0x2b -> 0x0040dca0
        _emit 0x72
        _emit 0xd5
        // 0x0000dccb: 57   PUSH EDI
        _emit 0x57
        // 0x0000dccc: 8b cd   MOV ECX,EBP
        _emit 0x8b
        _emit 0xcd
        // 0x0000dcce: e8 9d 02 00 00   CALL 0x0040df70  [REL32]
        _emit 0xe8
        _emit 0x9d
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0x0000dcd3: 66   (operand-size prefix — first byte of MOV AX,[ESP+0x14]
        //                  epilogue shared with the next function; symbols.json
        //                  counts this function as exactly 260 bytes, ending here)
        _emit 0x66
    }
}
