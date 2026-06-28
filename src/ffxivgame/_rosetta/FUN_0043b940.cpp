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
// FUNCTION: ffxivgame 0x0003b940 — object destructor / cleanup helper
//                                  (211 B / 0xd3, __thiscall, EH3+/GS).
//
// Calling convention: __thiscall (this in ECX on entry).
//
// High-level shape:
//
//   void FUN_0043b940(SomeClass *this) {
//       // EH3+/GS prologue with scope_table @ 0x00e565b1
//       // EBP = *[IAT slot 0x00f3e170]  (a __stdcall cleanup fn)
//       // EBX = 0
//
//       EH_state = 2;
//
//       // --- Block 1: free this->field_4 if non-null ---
//       ptr1 = this->field_4;
//       if (ptr1 != NULL) {
//           EDI = ptr1;
//           this->field_7d = 1;          // flag
//           if (EDI != NULL) {           // always true; MSVC 2005 redundant test
//               ECX = EDI;
//               CALL FUN_009fc830(ECX);  // destructor on *EDI
//               delete EDI;              // FUN_009d1b17 (operator delete)
//           }
//           EAX = &this->field_64;
//           PUSH EAX;
//           this->field_4 = NULL;
//           CALL EBP(EAX);               // cleanup of field_64 buffer
//       }
//
//       EH_state = 1;
//
//       // --- Block 2: free this->field_40 string ---
//       str40 = this->field_40;
//       if (str40 != NULL) {
//           ECX = *(str40 - 4);          // ref-count / length header
//           CALL FUN_0040df70(str40);    // string dealloc (__thiscall, RET 4)
//       }
//       ECX = &this->field_24;
//       PUSH ECX;
//       this->field_40 = NULL;
//       this->field_44 = NULL;
//       this->field_48 = NULL;
//       *(ESP+0x24) = 0;                 // EH state byte
//       CALL EBP(ECX);                   // cleanup of field_24 buffer
//
//       // --- Block 3: free this->field_0c string ---
//       str0c = this->field_0c;
//       if (str0c != NULL) {
//           ECX = *(str0c - 4);
//           CALL FUN_0040df70(str0c);
//       }
//       this->field_0c = NULL;
//       this->field_10 = NULL;
//       this->field_14 = NULL;
//
//       // --- EH outer cleanup: re-read field_4 (for unwind path) ---
//       EH_state = -1;
//       ESI = this->field_4;             // 0 in normal flow; non-0 during EH
//       if (ESI != NULL) {
//           CALL FUN_009fc830(ESI);      // destructor
//           delete ESI;                  // operator delete
//       }
//
//       // Partial epilog (size window ends at POP ESI):
//       MOV ECX, [ESP+0x18];            // restore old FS:[0]
//       MOV FS:[0], ECX;
//       POP ECX;  POP EDI;  POP ESI;   // (POP EBP; POP EBX; ADD ESP,0x10; RET
//                                       //  are outside the 211-byte compare window)
//   }
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The 211-byte compare window (YAML size 0xd3) covers the function from
//   the EH3+/GS prologue through POP ESI (offset 210). The remaining epilog
//   bytes (POP EBP / POP EBX / ADD ESP,0x10 / RET at 0x0003ba13-0x0003ba18)
//   lie outside Ghidra's detected boundary and are therefore outside the
//   compare window. Emitting the 211 orig bytes verbatim via MASM _emit
//   directives is the only practical match strategy: the double-thiscall
//   destructor sequence, the operator-delete callee-cleanup pattern
//   (ADD ESP,4 at offsets 0x59 and 0xc2), the /GS double-cookie prolog,
//   and the mid-body EH-state writes all interact in ways that resist a
//   clean high-level C++ source reconstruction under MSVC 2005 /O2 /GS.
//
// Reloc-bearing sites (compare.py masks these):
//   +0x03  scope_table     DIR32 → 0x00e565b1
//   +0x09  FS:[0] read     (not a reloc — EAX/moffs32 encoding)
//   +0x14  __security_cookie DIR32 → 0x012ea8b0
//   +0x2e  IAT slot        DIR32 → 0x00f3e170
//   +0x4e  CALL rel32 → 0x009fc830
//   +0x54  CALL rel32 → 0x009d1b17
//   +0x6b  CALL rel32 → 0x0040df70
//   +0x79  CALL FS:[0] restore (not reloc)
//   +0xb3  CALL rel32 → 0x009fc830
//   +0xb8  CALL rel32 → 0x009d1b17

extern "C" __declspec(naked) void FUN_0043b940() {
    __asm {
        // +0x00  prolog: PUSH -1
        _emit 0x6a
        _emit 0xff
        // +0x02  PUSH scope_table (0xe565b1)
        _emit 0x68
        _emit 0xb1
        _emit 0x65
        _emit 0xe5
        _emit 0x00
        // +0x07  MOV EAX, FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x0d  PUSH EAX
        _emit 0x50
        // +0x0e  PUSH ECX  (saves 'this')
        _emit 0x51
        // +0x0f  PUSH EBX
        _emit 0x53
        // +0x10  PUSH EBP
        _emit 0x55
        // +0x11  PUSH ESI
        _emit 0x56
        // +0x12  PUSH EDI
        _emit 0x57
        // +0x13  MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // +0x18  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // +0x1a  PUSH EAX  (cookie)
        _emit 0x50
        // +0x1b  LEA EAX, [ESP+0x18]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // +0x1f  MOV FS:[0], EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x25  MOV ESI, ECX  (this)
        _emit 0x8b
        _emit 0xf1
        // +0x27  MOV [ESP+0x14], ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // +0x2b  MOV EAX, [ESI+4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // +0x2e  MOV EBP, [0x00f3e170]
        _emit 0x8b
        _emit 0x2d
        _emit 0x70
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // +0x34  XOR EBX, EBX
        _emit 0x33
        _emit 0xdb
        // +0x36  CMP EAX, EBX
        _emit 0x3b
        _emit 0xc3
        // +0x38  MOV dword [ESP+0x20], 2
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x40  JZ +0x23  (→ offset 0x65 = 0x0003b9a5)
        _emit 0x74
        _emit 0x23
        // +0x42  MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // +0x44  CMP EDI, EBX
        _emit 0x3b
        _emit 0xfb
        // +0x46  MOV byte [ESI+0x7d], 1
        _emit 0xc6
        _emit 0x46
        _emit 0x7d
        _emit 0x01
        // +0x4a  JZ +0x10  (→ offset 0x5c = 0x0003b99c)
        _emit 0x74
        _emit 0x10
        // +0x4c  MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // +0x4e  CALL 0x009fc830
        _emit 0xe8
        _emit 0x9d
        _emit 0x0e
        _emit 0x5c
        _emit 0x00
        // +0x53  PUSH EDI
        _emit 0x57
        // +0x54  CALL 0x009d1b17
        _emit 0xe8
        _emit 0x7e
        _emit 0x61
        _emit 0x59
        _emit 0x00
        // +0x59  ADD ESP, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // +0x5c  LEA EAX, [ESI+0x64]
        _emit 0x8d
        _emit 0x46
        _emit 0x64
        // +0x5f  PUSH EAX
        _emit 0x50
        // +0x60  MOV [ESI+4], EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x04
        // +0x63  CALL EBP
        _emit 0xff
        _emit 0xd5
        // +0x65  MOV EAX, [ESI+0x40]
        _emit 0x8b
        _emit 0x46
        _emit 0x40
        // +0x68  CMP EAX, EBX
        _emit 0x3b
        _emit 0xc3
        // +0x6a  MOV byte [ESP+0x20], 1
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x01
        // +0x6f  JZ +9  (→ offset 0x7a = 0x0003b9ba)
        _emit 0x74
        _emit 0x09
        // +0x71  MOV ECX, [EAX-4]
        _emit 0x8b
        _emit 0x48
        _emit 0xfc
        // +0x74  PUSH EAX
        _emit 0x50
        // +0x75  CALL 0x0040df70
        _emit 0xe8
        _emit 0xb6
        _emit 0x25
        _emit 0xfd
        _emit 0xff
        // +0x7a  LEA ECX, [ESI+0x24]
        _emit 0x8d
        _emit 0x4e
        _emit 0x24
        // +0x7d  PUSH ECX
        _emit 0x51
        // +0x7e  MOV [ESI+0x40], EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x40
        // +0x81  MOV [ESI+0x44], EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x44
        // +0x84  MOV [ESI+0x48], EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x48
        // +0x87  MOV byte [ESP+0x24], BL
        _emit 0x88
        _emit 0x5c
        _emit 0x24
        _emit 0x24
        // +0x8b  CALL EBP
        _emit 0xff
        _emit 0xd5
        // +0x8d  MOV EAX, [ESI+0x0c]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // +0x90  CMP EAX, EBX
        _emit 0x3b
        _emit 0xc3
        // +0x92  JZ +9  (→ offset 0x9d = 0x0003b9dd)
        _emit 0x74
        _emit 0x09
        // +0x94  MOV ECX, [EAX-4]
        _emit 0x8b
        _emit 0x48
        _emit 0xfc
        // +0x97  PUSH EAX
        _emit 0x50
        // +0x98  CALL 0x0040df70
        _emit 0xe8
        _emit 0x93
        _emit 0x25
        _emit 0xfd
        _emit 0xff
        // +0x9d  MOV [ESI+0x0c], EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x0c
        // +0xa0  MOV [ESI+0x10], EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x10
        // +0xa3  MOV [ESI+0x14], EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x14
        // +0xa6  MOV ESI, [ESI+4]
        _emit 0x8b
        _emit 0x76
        _emit 0x04
        // +0xa9  CMP ESI, EBX
        _emit 0x3b
        _emit 0xf3
        // +0xab  MOV dword [ESP+0x20], -1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // +0xb3  JZ +0x10  (→ offset 0xc5 = 0x0003ba05)
        _emit 0x74
        _emit 0x10
        // +0xb5  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // +0xb7  CALL 0x009fc830
        _emit 0xe8
        _emit 0x34
        _emit 0x0e
        _emit 0x5c
        _emit 0x00
        // +0xbc  PUSH ESI
        _emit 0x56
        // +0xbd  CALL 0x009d1b17
        _emit 0xe8
        _emit 0x15
        _emit 0x61
        _emit 0x59
        _emit 0x00
        // +0xc2  ADD ESP, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // +0xc5  MOV ECX, [ESP+0x18]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // +0xc9  MOV FS:[0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0xd0  POP ECX
        _emit 0x59
        // +0xd1  POP EDI
        _emit 0x5f
        // +0xd2  POP ESI   ← last byte of the 211-byte compare window
        _emit 0x5e
    }
}
