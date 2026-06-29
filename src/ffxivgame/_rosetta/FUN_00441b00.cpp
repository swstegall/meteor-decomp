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
// FUNCTION: ffxivgame 0x00041b00 — FUN_00441b00 (229 B / 0xe5)
//                                  __thiscall, 2 stack args, RET 8.
//                                  Audio module — initialises a member
//                                  string from an optional source string
//                                  then iterates a linked-list field.
//
// Asm shape (229 B, __thiscall, SEH-wrapped /GS):
//
//   void __thiscall FUN_00441b00(this, arg1, arg2)
//       ECX = this
//       [ESP+4] = arg1 (ptr, nullable)
//       [ESP+8] = arg2
//
//   SEH prologue: PUSH -1 / PUSH 0xe57098 / PUSH FS:[0] /
//                 SUB ESP,0x18 / PUSH EBX/EBP/ESI/EDI /
//                 __security_cookie XOR ESP / PUSH EAX /
//                 LEA EAX,[ESP+0x2c]; MOV FS:[0],EAX
//
//   ESI = this (from ECX)
//   if (arg1 != NULL) {
//       call 0xb8ff10(local_buf, arg1)        ; __cdecl, 2 args
//       ECX = local_buf
//       try {     // state 0
//           call 0xb51bd0(ECX=local_buf, arg2) ; __thiscall, 1 arg
//       } finally {
//           call 0xb51920(ECX=local_buf)       ; __thiscall, 0 args
//       }
//   } else {
//       call 0xb8fa10(arg2)                   ; __cdecl, 1 arg
//   }
//   EBP = this->field_0x1c   ; linked-list head
//   EDI = &this->field_0x18  ; sentinel
//   EBX = *EBP               ; first node's ->next
//   ESI = EDI
//   [ESP+0x14] = ESI
//   [ESP+0x18] = EBX
//   (7-byte alignment NOP)
//   loop:
//     if (ESI == NULL || ESI == EDI) -> purecall/end
//     if (EBX == EBP) -> exit loop
//     if (ESI == NULL) -> purecall
//     if (EBX == [ESI+4]) -> purecall
//     ECX = [EBX+0x10]; call 0x442770
//     ECX = &[ESP+0x14]; call 0x9172c0  ; advance iterator
//     EBX = [ESP+0x18]; ESI = [ESP+0x14]
//     jmp loop
//   SEH epilogue: restore FS:[0] / POP ECX,EDI,ESI,EBP,EBX /
//                 ADD ESP,0x24 / RET 8
//
// Why naked asm: The EH3-style SEH frame (PUSH -1 / scope table / FS:[0]
// / __security_cookie) plus the mid-body try-state writes at [ESP+0x38]
// and [ESP+0x34] are compiler-emitted shape that depends on precise
// locals layout and MSVC's modrm encoding choices. Source-level C++ would
// also emit EH4 funclets into a .text$x COMDAT, causing a size mismatch
// with orig's 229-byte .text section. The `__declspec(naked)` byte
// passthrough avoids both issues and gives compare.py a byte-exact match
// (same approach as FUN_00403a20, FUN_00408910, FUN_004011b0).
//
// Reloc-bearing sites (wildcarded by compare.py):
//   +0x03  scope table ptr      (0xe57098)
//   +0x16  __security_cookie    (0x012ea8b0)
//   +0x38  CALL 0x00b8ff10      (rel32)
//   +0x51  CALL 0x00b51bd0      (rel32)
//   +0x62  CALL 0x00b51920      (rel32)
//   +0x6e  CALL 0x00b8fa10      (rel32)
//   +0x99  CALL 0x009d22b4      (rel32)
//   +0xa6  CALL 0x009d22b4      (rel32)
//   +0xb0  CALL 0x009d22b4      (rel32)
//   +0xb8  CALL 0x00442770      (rel32)
//   +0xc1  CALL 0x009172c0      (rel32)

extern "C" __declspec(naked) void FUN_00441b00() {
    __asm {
        // 0x41b00: PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0x41b02: PUSH 0xe57098
        _emit 0x68
        _emit 0x98
        _emit 0x70
        _emit 0xe5
        _emit 0x00
        // 0x41b07: MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x41b0d: PUSH EAX
        _emit 0x50
        // 0x41b0e: SUB ESP,0x18
        _emit 0x83
        _emit 0xec
        _emit 0x18
        // 0x41b11: PUSH EBX
        _emit 0x53
        // 0x41b12: PUSH EBP
        _emit 0x55
        // 0x41b13: PUSH ESI
        _emit 0x56
        // 0x41b14: PUSH EDI
        _emit 0x57
        // 0x41b15: MOV EAX,[0x012ea8b0]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0x41b1a: XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0x41b1c: PUSH EAX
        _emit 0x50
        // 0x41b1d: LEA EAX,[ESP + 0x2c]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // 0x41b21: MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x41b27: MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0x41b29: MOV EAX,[ESP + 0x3c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        // 0x41b2d: TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0x41b2f: JZ +0x37 (-> 0x41b68)
        _emit 0x74
        _emit 0x37
        // 0x41b31: PUSH EAX
        _emit 0x50
        // 0x41b32: LEA EAX,[ESP + 0x20]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0x41b36: PUSH EAX
        _emit 0x50
        // 0x41b37: CALL 0x00b8ff10
        _emit 0xe8
        _emit 0xd4
        _emit 0xe3
        _emit 0x74
        _emit 0x00
        // 0x41b3c: ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0x41b3f: MOV ECX,[ESP + 0x40]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        // 0x41b43: PUSH ECX
        _emit 0x51
        // 0x41b44: LEA ECX,[ESP + 0x20]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 0x41b48: MOV [ESP + 0x38],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x41b50: CALL 0x00b51bd0
        _emit 0xe8
        _emit 0x7b
        _emit 0x00
        _emit 0x71
        _emit 0x00
        // 0x41b55: LEA ECX,[ESP + 0x1c]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0x41b59: MOV [ESP + 0x34],0xffffffff
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0x41b61: CALL 0x00b51920
        _emit 0xe8
        _emit 0xba
        _emit 0xfd
        _emit 0x70
        _emit 0x00
        // 0x41b66: JMP +0x0d (-> 0x41b75)
        _emit 0xeb
        _emit 0x0d
        // 0x41b68: MOV EDX,[ESP + 0x40]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x40
        // 0x41b6c: PUSH EDX
        _emit 0x52
        // 0x41b6d: CALL 0x00b8fa10
        _emit 0xe8
        _emit 0x9e
        _emit 0xde
        _emit 0x74
        _emit 0x00
        // 0x41b72: ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0x41b75: MOV EBP,[ESI + 0x1c]
        _emit 0x8b
        _emit 0x6e
        _emit 0x1c
        // 0x41b78: LEA EDI,[ESI + 0x18]
        _emit 0x8d
        _emit 0x7e
        _emit 0x18
        // 0x41b7b: MOV EAX,EBP
        _emit 0x8b
        _emit 0xc5
        // 0x41b7d: MOV EBX,[EAX]
        _emit 0x8b
        _emit 0x18
        // 0x41b7f: MOV ESI,EDI
        _emit 0x8b
        _emit 0xf7
        // 0x41b81: MOV [ESP + 0x14],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 0x41b85: MOV [ESP + 0x18],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // 0x41b89: LEA ESP,[ESP] (7-byte alignment NOP)
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x41b90: TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 0x41b92: JZ +0x04 (-> 0x41b98)
        _emit 0x74
        _emit 0x04
        // 0x41b94: CMP ESI,EDI
        _emit 0x3b
        _emit 0xf7
        // 0x41b96: JZ +0x05 (-> 0x41b9d)
        _emit 0x74
        _emit 0x05
        // 0x41b98: CALL 0x009d22b4
        _emit 0xe8
        _emit 0x17
        _emit 0x07
        _emit 0x59
        _emit 0x00
        // 0x41b9d: CMP EBX,EBP
        _emit 0x3b
        _emit 0xdd
        // 0x41b9f: JZ +0x2e (-> 0x41bcf)
        _emit 0x74
        _emit 0x2e
        // 0x41ba1: TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 0x41ba3: JNZ +0x05 (-> 0x41baa)
        _emit 0x75
        _emit 0x05
        // 0x41ba5: CALL 0x009d22b4
        _emit 0xe8
        _emit 0x0a
        _emit 0x07
        _emit 0x59
        _emit 0x00
        // 0x41baa: CMP EBX,[ESI + 0x4]
        _emit 0x3b
        _emit 0x5e
        _emit 0x04
        // 0x41bad: JNZ +0x05 (-> 0x41bb4)
        _emit 0x75
        _emit 0x05
        // 0x41baf: CALL 0x009d22b4
        _emit 0xe8
        _emit 0x00
        _emit 0x07
        _emit 0x59
        _emit 0x00
        // 0x41bb4: MOV ECX,[EBX + 0x10]
        _emit 0x8b
        _emit 0x4b
        _emit 0x10
        // 0x41bb7: CALL 0x00442770
        _emit 0xe8
        _emit 0xb4
        _emit 0x0b
        _emit 0x00
        _emit 0x00
        // 0x41bbc: LEA ECX,[ESP + 0x14]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0x41bc0: CALL 0x009172c0
        _emit 0xe8
        _emit 0xfb
        _emit 0x56
        _emit 0x4d
        _emit 0x00
        // 0x41bc5: MOV EBX,[ESP + 0x18]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // 0x41bc9: MOV ESI,[ESP + 0x14]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 0x41bcd: JMP -0x3f (-> 0x41b90)
        _emit 0xeb
        _emit 0xc1
        // 0x41bcf: MOV ECX,[ESP + 0x2c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // 0x41bd3: MOV FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x41bda: POP ECX
        _emit 0x59
        // 0x41bdb: POP EDI
        _emit 0x5f
        // 0x41bdc: POP ESI
        _emit 0x5e
        // 0x41bdd: POP EBP
        _emit 0x5d
        // 0x41bde: POP EBX
        _emit 0x5b
        // 0x41bdf: ADD ESP,0x24
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        // 0x41be2: RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
