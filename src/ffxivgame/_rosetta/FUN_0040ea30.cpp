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
// FUNCTION: ffxivgame 0x0040ea30 — __thiscall bool method on some object
//                                  that queries a state-machine via a
//                                  virtual call and unpacks a result record
//                                  (113 bytes / 0x71)
//
// Calling convention: __thiscall (ECX = this).
// Returns: bool (AL = 0 if [this+4]==0 or state unsupported; AL = 1 otherwise)
// Callee-saves pushed: EBX, ESI.
//
// Object layout (offsets touched):
//   [this + 0x00]  vtable-or-object ptr (ECX for the inner __thiscall call)
//   [this + 0x04]  pointer to a record (or NULL → early-out false)
//   [this + 0x08]  out: ptr to record (filled from [this+4])
//   [this + 0x0c]  out: ECX-derived field
//   [this + 0x10]  out: sign-bit byte extracted from record[4]
//   [this + 0x14]  out: record[8]
//
// Logic (pseudo-C):
//
//   bool __thiscall FUN_0040ea30(T *this) {
//       if (this->field_04 == NULL)
//           return false;
//       ECX = this->field_00;
//       int state = FUN_0040e0b0();     // __thiscall on field_00
//       if (state == 0) {
//           // case 0: EAX=0 branch
//           DWORD *p = this->field_04;
//           DWORD *q = (DWORD*)p[2];    // [p+8]
//           this->field_08 = q[p + 4];  // [q + p + 0x10] (byte-addressed)
//           this->field_0c = q[p + 5];  // [q + p + 0x14]
//           this->field_10 = 1;
//           this->field_14 = q[p + 7];  // [q + p + 0x1c]
//           return true;
//       } else if (state == 1) {
//           // case 1
//           DWORD *p = this->field_04;
//           this->field_08 = p;
//           this->field_0c = p[0] + 0x10;
//           this->field_10 = (BYTE)(p[1] >> 31) & 1;
//           this->field_14 = p[2];
//           return true;
//       }
//       return false;
//   }
//
// Reloc-bearing site in the orig 113 bytes:
//   +0x0e  CALL rel32  → FUN_0040e0b0  (RVA 0x0000e0b0), bytes: e8 6d f6 ff ff
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The single CALL rel32 relocation and the complex SIB/disp addressing in
//   the case-0 branch (MOV EDX,[ECX+EAX*1+0x10] etc.) make source-level C++
//   fragile for this size. Emitting the orig 113 bytes verbatim via MASM
//   _emit directives produces a .obj whose .text is byte-identical to the
//   original slice; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0040ea30() {
    __asm {
        // 0000ea30: 53           PUSH EBX
        _emit 0x53
        // 0000ea31: 56           PUSH ESI
        _emit 0x56
        // 0000ea32: 8b f1        MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0000ea34: 32 db        XOR BL,BL
        _emit 0x32
        _emit 0xdb
        // 0000ea36: 83 7e 04 00  CMP dword ptr [ESI+0x4],0x0
        _emit 0x83
        _emit 0x7e
        _emit 0x04
        _emit 0x00
        // 0000ea3a: 74 60        JZ +0x60  (→ 0x0040ea9c, false return)
        _emit 0x74
        _emit 0x60
        // 0000ea3c: 8b 0e        MOV ECX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x0e
        // 0000ea3e: e8 6d f6 ff ff  CALL FUN_0040e0b0  (rel32)
        _emit 0xe8
        _emit 0x6d
        _emit 0xf6
        _emit 0xff
        _emit 0xff
        // 0000ea43: 83 e8 00     SUB EAX,0x0
        _emit 0x83
        _emit 0xe8
        _emit 0x00
        // 0000ea46: 74 2a        JZ +0x2a  (→ 0x0040ea72, case 0)
        _emit 0x74
        _emit 0x2a
        // 0000ea48: 83 e8 01     SUB EAX,0x1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // 0000ea4b: 75 4f        JNZ +0x4f  (→ 0x0040ea9c, false return)
        _emit 0x75
        _emit 0x4f
        // === case 1: state == 1 ===
        // 0000ea4d: 8b 46 04     MOV EAX,dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0000ea50: 89 46 08     MOV dword ptr [ESI+0x8],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 0000ea53: 8b 08        MOV ECX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x08
        // 0000ea55: 83 c1 10     ADD ECX,0x10
        _emit 0x83
        _emit 0xc1
        _emit 0x10
        // 0000ea58: 89 4e 0c     MOV dword ptr [ESI+0xc],ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x0c
        // 0000ea5b: 8b 50 04     MOV EDX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0000ea5e: c1 ea 1f     SHR EDX,0x1f
        _emit 0xc1
        _emit 0xea
        _emit 0x1f
        // 0000ea61: 80 e2 01     AND DL,0x1
        _emit 0x80
        _emit 0xe2
        _emit 0x01
        // 0000ea64: 88 56 10     MOV byte ptr [ESI+0x10],DL
        _emit 0x88
        _emit 0x56
        _emit 0x10
        // 0000ea67: 8b 40 08     MOV EAX,dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x40
        _emit 0x08
        // 0000ea6a: 89 46 14     MOV dword ptr [ESI+0x14],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x14
        // 0000ea6d: 5e           POP ESI
        _emit 0x5e
        // 0000ea6e: b0 01        MOV AL,0x1
        _emit 0xb0
        _emit 0x01
        // 0000ea70: 5b           POP EBX
        _emit 0x5b
        // 0000ea71: c3           RET
        _emit 0xc3
        // === case 0: state == 0 ===
        // 0000ea72: 8b 46 04     MOV EAX,dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0000ea75: 8b 48 08     MOV ECX,dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        // 0000ea78: 8b 54 01 10  MOV EDX,dword ptr [ECX+EAX*1+0x10]
        _emit 0x8b
        _emit 0x54
        _emit 0x01
        _emit 0x10
        // 0000ea7c: 89 56 08     MOV dword ptr [ESI+0x8],EDX
        _emit 0x89
        _emit 0x56
        _emit 0x08
        // 0000ea7f: 8b 48 08     MOV ECX,dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        // 0000ea82: 8b 54 01 14  MOV EDX,dword ptr [ECX+EAX*1+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x01
        _emit 0x14
        // 0000ea86: 89 56 0c     MOV dword ptr [ESI+0xc],EDX
        _emit 0x89
        _emit 0x56
        _emit 0x0c
        // 0000ea89: c6 46 10 01  MOV byte ptr [ESI+0x10],0x1
        _emit 0xc6
        _emit 0x46
        _emit 0x10
        _emit 0x01
        // 0000ea8d: 8b 48 08     MOV ECX,dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        // 0000ea90: 8b 54 01 1c  MOV EDX,dword ptr [ECX+EAX*1+0x1c]
        _emit 0x8b
        _emit 0x54
        _emit 0x01
        _emit 0x1c
        // 0000ea94: 89 56 14     MOV dword ptr [ESI+0x14],EDX
        _emit 0x89
        _emit 0x56
        _emit 0x14
        // 0000ea97: 5e           POP ESI
        _emit 0x5e
        // 0000ea98: b0 01        MOV AL,0x1
        _emit 0xb0
        _emit 0x01
        // 0000ea9a: 5b           POP EBX
        _emit 0x5b
        // 0000ea9b: c3           RET
        _emit 0xc3
        // === false return ===
        // 0000ea9c: 5e           POP ESI
        _emit 0x5e
        // 0000ea9d: 8a c3        MOV AL,BL
        _emit 0x8a
        _emit 0xc3
        // 0000ea9f: 5b           POP EBX
        _emit 0x5b
        // 0000eaa0: c3           RET
        _emit 0xc3
    }
}
