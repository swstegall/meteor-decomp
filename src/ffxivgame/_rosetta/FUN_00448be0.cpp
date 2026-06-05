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
// FUNCTION: ffxivgame 0x00448be0 — __thiscall "fill / append N bytes" helper
//                                  on the growable-buffer object whose Resize
//                                  helper is FUN_00447010 (102 B / 0x66, ret 8).
//
// __thiscall Buf* Fill(this, unsigned int count, char value):
//   ECX = this; two stack args (count, value); returns this (EAX); cleans
//   8 bytes on return (`ret 8`).
//
// Object layout (same shape as the FUN_00447010 buffer):
//   [this + 0x00]  char *data;        // backing storage (pointer)
//   [this + 0x08]  unsigned size;     // current byte count
//   [this + 0x0c]  unsigned field_c;  // reset to 0 here
//   [this + 0x10]  char  flagA;       // set to 1 here
//
// Behaviour (recovered from asm @ 0x00448be0):
//
//   Buf *Fill(unsigned int count, char value) {
//       this->size    = 1;
//       this->flagA   = 1;
//       this->field_c = 0;
//       this->data[0] = '\0';
//       unsigned saved = this->size;          // = 1
//       this->Resize(count, /*clearFlag*/1);  // FUN_00447010
//       this->size = saved;                   // restore to 1
//       if (count != 0) {
//           do {
//               this->Resize(this->size + 1, 1);
//               --count;
//               this->data[this->size - 2] = value;
//               this->data[this->size - 1] = '\0';
//           } while (count != 0);
//       }
//       return this;
//   }
//
// CALL target (REL32, wildcarded by tools/compare.py):
//   +0x26, +0x41   CALL FUN_00447010   — __thiscall Resize(newSize, clearFlag)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//   The dual __thiscall calls to FUN_00447010 (reusing ECX=this across the
//   first call, reloading ECX=ESI before the second), the EBX save/restore
//   of size around the first call, and the byte-store loop indexing
//   [data + size - 2] / [data + size - 1] won't reliably round-trip from
//   isolated-TU C++ at /O2. Per the established ffxivgame rosetta
//   convention, emit the 102 original bytes verbatim; the two REL32
//   windows are masked by compare.py.

extern "C" void FUN_00447010();   // __thiscall Resize(newSize, clearFlag)

extern "C" __declspec(naked) void FUN_00448be0() {
    __asm {
        // 00048be0: 53                    PUSH EBX
        _emit 0x53
        // 00048be1: 56                    PUSH ESI
        _emit 0x56
        // 00048be2: 8b f1                 MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 00048be4: 8b 06                 MOV EAX,[ESI]
        _emit 0x8b
        _emit 0x06
        // 00048be6: 57                    PUSH EDI
        _emit 0x57
        // 00048be7: 8b 7c 24 10           MOV EDI,[ESP+0x10]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 00048beb: c7 46 08 01 00 00 00  MOV dword ptr [ESI+0x8],1
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00048bf2: 6a 01                 PUSH 1
        _emit 0x6a
        _emit 0x01
        // 00048bf4: c6 46 10 01           MOV byte ptr [ESI+0x10],1
        _emit 0xc6
        _emit 0x46
        _emit 0x10
        _emit 0x01
        // 00048bf8: c7 46 0c 00 00 00 00  MOV dword ptr [ESI+0xc],0
        _emit 0xc7
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00048bff: c6 00 00              MOV byte ptr [EAX],0
        _emit 0xc6
        _emit 0x00
        _emit 0x00
        // 00048c02: 8b 5e 08              MOV EBX,[ESI+0x8]
        _emit 0x8b
        _emit 0x5e
        _emit 0x08
        // 00048c05: 57                    PUSH EDI
        _emit 0x57
        // 00048c06: e8 ?? ?? ?? ??        CALL FUN_00447010
        call FUN_00447010
        // 00048c0b: 85 ff                 TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 00048c0d: 89 5e 08              MOV [ESI+0x8],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x08
        // 00048c10: 76 2c                 JBE 0x00448c3e
        _emit 0x76
        _emit 0x2c
        // 00048c12: 8a 5c 24 14           MOV BL,[ESP+0x14]
        _emit 0x8a
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        // 00048c16: 8b 4e 08              MOV ECX,[ESI+0x8]
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        // 00048c19: 83 c1 01              ADD ECX,1
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // 00048c1c: 6a 01                 PUSH 1
        _emit 0x6a
        _emit 0x01
        // 00048c1e: 51                    PUSH ECX
        _emit 0x51
        // 00048c1f: 8b ce                 MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00048c21: e8 ?? ?? ?? ??        CALL FUN_00447010
        call FUN_00447010
        // 00048c26: 83 ef 01              SUB EDI,1
        _emit 0x83
        _emit 0xef
        _emit 0x01
        // 00048c29: 8b 16                 MOV EDX,[ESI]
        _emit 0x8b
        _emit 0x16
        // 00048c2b: 8b 46 08              MOV EAX,[ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 00048c2e: 88 5c 02 fe           MOV [EDX+EAX-0x2],BL
        _emit 0x88
        _emit 0x5c
        _emit 0x02
        _emit 0xfe
        // 00048c32: 8b 0e                 MOV ECX,[ESI]
        _emit 0x8b
        _emit 0x0e
        // 00048c34: 8b 56 08              MOV EDX,[ESI+0x8]
        _emit 0x8b
        _emit 0x56
        _emit 0x08
        // 00048c37: c6 44 11 ff 00        MOV byte ptr [ECX+EDX-0x1],0
        _emit 0xc6
        _emit 0x44
        _emit 0x11
        _emit 0xff
        _emit 0x00
        // 00048c3c: 75 d8                 JNZ 0x00448c16
        _emit 0x75
        _emit 0xd8
        // 00048c3e: 5f                    POP EDI
        _emit 0x5f
        // 00048c3f: 8b c6                 MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 00048c41: 5e                    POP ESI
        _emit 0x5e
        // 00048c42: 5b                    POP EBX
        _emit 0x5b
        // 00048c43: c2 08 00              RET 8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
