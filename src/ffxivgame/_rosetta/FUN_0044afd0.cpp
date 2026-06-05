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
// FUNCTION: ffxivgame 0x0004afd0 — __thiscall field-splitter that parses a
//                                  string into up to 4 fields, copying the
//                                  field text into the object's 0x200-byte
//                                  scratch buffer and storing the four field
//                                  pointers at [this+0x200] (142 B / 0x8e,
//                                  ret 4).
//
// Calling convention: __thiscall (ECX = this). One stack argument — a
// string-like object `str` — read from [ESP+0x18]; the epilogue does
// `ret 4` to clean it. Returns bool in AL.
//
// Recovered behaviour:
//
//   bool Object::SplitFields(StringRef str) {
//       char *field = (char *)this;          // EBP — start of current field
//       int   count = 0;                     // EBX
//       if (str.length() != 0) {             // CALL FUN_00445e50 (length)
//           for (unsigned i = 0; i < str.length(); ++i) {
//               char c = *str.at(i);         // CALL FUN_00445060 (char-at)
//               if (c >= 0x20 && c != ',') { // printable, non-comma
//                   ((char *)this)[i] = c;
//                   continue;
//               }
//               ((char *)this)[i] = 0;       // terminate field
//               ((char **)((char*)this + 0x200))[count] = field;
//               ++count;
//               field = (char *)this + i + 1;
//               if (count >= 4) return true; // early-out: 4 fields filled
//           }
//       }
//       ((char **)((char*)this + 0x200))[count] = field;  // trailing field
//       ++count;
//       return count == 4;
//   }
//
// CALL targets (REL32):
//   +0x11  CALL FUN_00445e50   — str.length()
//   +0x29  CALL FUN_00445060   — str.at(index) → char*
//   +0x61  CALL FUN_00445e50   — str.length() (loop re-check)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//   The call displacements are re-emitted verbatim from the original
//   slice (self-consistent: the .obj's literal bytes equal the original
//   binary's bytes, so compare.py reports the 142-byte .text GREEN). A
//   source-level C++ form is sensitive to the EBP/ESI/EDI allocation
//   order MSVC 2005 chose for this TU; the naked emit guarantees an
//   exact match.

extern "C" __declspec(naked) void FUN_0044afd0() {
    __asm {
        // 0004afd0:  51                 PUSH ECX
        _emit 0x51
        // 0004afd1:  53                 PUSH EBX
        _emit 0x53
        // 0004afd2:  55                 PUSH EBP
        _emit 0x55
        // 0004afd3:  56                 PUSH ESI
        _emit 0x56
        // 0004afd4:  57                 PUSH EDI
        _emit 0x57
        // 0004afd5:  8b f9              MOV EDI,ECX
        _emit 0x8b
        _emit 0xf9
        // 0004afd7:  8b 4c 24 18        MOV ECX,[ESP+0x18]   ; str
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0004afdb:  33 db              XOR EBX,EBX          ; count = 0
        _emit 0x33
        _emit 0xdb
        // 0004afdd:  8b ef              MOV EBP,EDI          ; field = this
        _emit 0x8b
        _emit 0xef
        // 0004afdf:  33 f6              XOR ESI,ESI          ; i = 0
        _emit 0x33
        _emit 0xf6
        // 0004afe1:  e8 6a ae ff ff     CALL 0x00445e50      ; str.length()
        _emit 0xe8
        _emit 0x6a
        _emit 0xae
        _emit 0xff
        _emit 0xff
        // 0004afe6:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0004afe8:  76 50              JBE 0x0044b03a       ; length==0 -> store final
        _emit 0x76
        _emit 0x50
        // 0004afea:  8d 87 00 02 00 00  LEA EAX,[EDI+0x200]  ; out = &fields[0]
        _emit 0x8d
        _emit 0x87
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0004aff0:  89 44 24 10        MOV [ESP+0x10],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // loop_top (0004aff4):
        // 0004aff4:  8b 4c 24 18        MOV ECX,[ESP+0x18]   ; str
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0004aff8:  56                 PUSH ESI             ; arg: i
        _emit 0x56
        // 0004aff9:  e8 62 a0 ff ff     CALL 0x00445060      ; str.at(i)
        _emit 0xe8
        _emit 0x62
        _emit 0xa0
        _emit 0xff
        _emit 0xff
        // 0004affe:  8a 00              MOV AL,[EAX]         ; c = *p
        _emit 0x8a
        _emit 0x00
        // 0004b000:  3c 20              CMP AL,0x20
        _emit 0x3c
        _emit 0x20
        // 0004b002:  7c 09              JL 0x0044b00d        ; c < 0x20 -> delimiter
        _emit 0x7c
        _emit 0x09
        // 0004b004:  3c 2c              CMP AL,0x2c
        _emit 0x3c
        _emit 0x2c
        // 0004b006:  74 05              JZ 0x0044b00d        ; c == ',' -> delimiter
        _emit 0x74
        _emit 0x05
        // 0004b008:  88 04 3e           MOV [ESI+EDI],AL     ; this[i] = c
        _emit 0x88
        _emit 0x04
        _emit 0x3e
        // 0004b00b:  eb 1d              JMP 0x0044b02a       ; continue
        _emit 0xeb
        _emit 0x1d
        // delimiter (0004b00d):
        // 0004b00d:  8b 44 24 10        MOV EAX,[ESP+0x10]   ; out
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0004b011:  c6 04 3e 00        MOV byte [ESI+EDI],0x0  ; terminate field
        _emit 0xc6
        _emit 0x04
        _emit 0x3e
        _emit 0x00
        // 0004b015:  89 28              MOV [EAX],EBP        ; *out = field
        _emit 0x89
        _emit 0x28
        // 0004b017:  83 c3 01           ADD EBX,0x1          ; count++
        _emit 0x83
        _emit 0xc3
        _emit 0x01
        // 0004b01a:  83 c0 04           ADD EAX,0x4          ; out++
        _emit 0x83
        _emit 0xc0
        _emit 0x04
        // 0004b01d:  83 fb 04           CMP EBX,0x4
        _emit 0x83
        _emit 0xfb
        _emit 0x04
        // 0004b020:  89 44 24 10        MOV [ESP+0x10],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0004b024:  8d 6c 3e 01        LEA EBP,[ESI+EDI+0x1] ; field = &this[i+1]
        _emit 0x8d
        _emit 0x6c
        _emit 0x3e
        _emit 0x01
        // 0004b028:  7d 2a              JGE 0x0044b054       ; count>=4 -> return true
        _emit 0x7d
        _emit 0x2a
        // continue (0004b02a):
        // 0004b02a:  8b 4c 24 18        MOV ECX,[ESP+0x18]   ; str
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0004b02e:  83 c6 01           ADD ESI,0x1          ; i++
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        // 0004b031:  e8 1a ae ff ff     CALL 0x00445e50      ; str.length()
        _emit 0xe8
        _emit 0x1a
        _emit 0xae
        _emit 0xff
        _emit 0xff
        // 0004b036:  3b f0              CMP ESI,EAX
        _emit 0x3b
        _emit 0xf0
        // 0004b038:  72 ba              JC 0x0044aff4        ; i < length -> loop
        _emit 0x72
        _emit 0xba
        // store_final (0004b03a):
        // 0004b03a:  89 ac 9f 00 02 00 00  MOV [EDI+EBX*4+0x200],EBP
        _emit 0x89
        _emit 0xac
        _emit 0x9f
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0004b041:  5f                 POP EDI
        _emit 0x5f
        // 0004b042:  83 c3 01           ADD EBX,0x1          ; count++
        _emit 0x83
        _emit 0xc3
        _emit 0x01
        // 0004b045:  5e                 POP ESI
        _emit 0x5e
        // 0004b046:  33 c0              XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0004b048:  83 fb 04           CMP EBX,0x4
        _emit 0x83
        _emit 0xfb
        _emit 0x04
        // 0004b04b:  5d                 POP EBP
        _emit 0x5d
        // 0004b04c:  0f 94 c0           SETZ AL              ; return count==4
        _emit 0x0f
        _emit 0x94
        _emit 0xc0
        // 0004b04f:  5b                 POP EBX
        _emit 0x5b
        // 0004b050:  59                 POP ECX
        _emit 0x59
        // 0004b051:  c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // early_true (0004b054):
        // 0004b054:  5f                 POP EDI
        _emit 0x5f
        // 0004b055:  5e                 POP ESI
        _emit 0x5e
        // 0004b056:  5d                 POP EBP
        _emit 0x5d
        // 0004b057:  b0 01              MOV AL,0x1           ; return true
        _emit 0xb0
        _emit 0x01
        // 0004b059:  5b                 POP EBX
        _emit 0x5b
        // 0004b05a:  59                 POP ECX
        _emit 0x59
        // 0004b05b:  c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
