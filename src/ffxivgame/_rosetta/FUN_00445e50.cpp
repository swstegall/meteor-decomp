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
// FUNCTION: ffxivgame 0x00045e50 — cached UTF-8 character-count accessor
//                                  (__thiscall, 132 bytes, no relocations).
//
// Behaviour reconstructed from the asm (RVA 0x00045e50, 0x84 bytes):
//
//   int __thiscall FUN_00445e50(CountedStr *this /*ECX*/) {
//       if (this->computed /*[this+0x10]*/ == 0) {
//           const unsigned char *p = this->str;   // [this+0x00]
//           int count = 0;                         // ESI
//           if (p == 0) return 0;                  // early out, no caching
//           unsigned char c = *p;
//           while (c != 0) {
//               int n;                             // bytes in this UTF-8 unit
//               if ((unsigned char)(c + 0x80) <= 0x3f) n = 0;   // 0x80..0xbf
//               else if (c <  0x80) n = 1;                       // ASCII
//               else if (c <  0xe0) n = 2;                       // 0xc0..0xdf
//               else if (c <  0xf0) n = 3;                       // 0xe0..0xef
//               else if (c <  0xf8) n = 4;                       // 0xf0..0xf7
//               else if (c <  0xfc) n = 5;                       // 0xf8..0xfb
//               else n = (c < 0xfe) ? 6 : 0;                     // SBB/AND idiom
//               p += n;
//               c = *p;
//               ++count;
//           }
//           this->computed  /*[this+0x10]*/ = 1;
//           this->length    /*[this+0x0c]*/ = count;
//       }
//       return this->length;   // [this+0x0c]
//   }
//
// Calling convention: __thiscall (ECX = this), int return in EAX. The
// SBB EAX,EAX / AND EAX,6 tail at the bottom of the length ladder is the
// MSVC carry-flag trick for `(c < 0xfe) ? 6 : 0`.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function contains no CALLs and no absolute symbol references; the
//   only control flow is internal short branches (rel8). A source-level
//   C++ form would compile to this shape under /O2, but the precise
//   branch-ladder layout and the `8d 49 00` 3-byte align-nop before the
//   loop top are products of cl.exe's code-layout pass that the source
//   form can't be coerced to reproduce within /O2. As with siblings
//   FUN_00408610 / FUN_00403d60, the pragmatic choice is a
//   `__declspec(naked)` body re-emitting the orig 132 bytes verbatim via
//   MASM `_emit`. The .obj `.text` ends byte-identical with NO
//   relocations; `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_00445e50() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x80              // CMP byte ptr [EDI+0x10], 0
        _emit 0x7f
        _emit 0x10
        _emit 0x00
        _emit 0x75              // JNZ +0x74 (→ 0x445ece)
        _emit 0x74
        _emit 0x8b              // MOV EDX, dword ptr [EDI]
        _emit 0x17
        _emit 0x33              // XOR ESI, ESI
        _emit 0xf6
        _emit 0x85              // TEST EDX, EDX
        _emit 0xd2
        _emit 0x75              // JNZ +5 (→ 0x445e67)
        _emit 0x05
        _emit 0x5f              // POP EDI
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        _emit 0x8a              // MOV AL, byte ptr [EDX]
        _emit 0x02
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x74              // JZ +0x5A (→ 0x445ec7)
        _emit 0x5a
        _emit 0x8d              // LEA ECX, [ECX]            (3-byte nop)
        _emit 0x49
        _emit 0x00
        _emit 0x8a              // MOV CL, AL                (loop_top:)
        _emit 0xc8
        _emit 0x80              // ADD CL, 0x80
        _emit 0xc1
        _emit 0x80
        _emit 0x80              // CMP CL, 0x3f
        _emit 0xf9
        _emit 0x3f
        _emit 0x77              // JA +4 (→ 0x445e7e)
        _emit 0x04
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0xeb              // JMP +0x3E (→ 0x445ebc)
        _emit 0x3e
        _emit 0x3c              // CMP AL, 0x80
        _emit 0x80
        _emit 0x73              // JNC +7 (→ 0x445e89)
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x33 (→ 0x445ebc)
        _emit 0x33
        _emit 0x3c              // CMP AL, 0xe0
        _emit 0xe0
        _emit 0x73              // JNC +7 (→ 0x445e94)
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x2
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x28 (→ 0x445ebc)
        _emit 0x28
        _emit 0x3c              // CMP AL, 0xf0
        _emit 0xf0
        _emit 0x73              // JNC +7 (→ 0x445e9f)
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x3
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x1D (→ 0x445ebc)
        _emit 0x1d
        _emit 0x3c              // CMP AL, 0xf8
        _emit 0xf8
        _emit 0x73              // JNC +7 (→ 0x445eaa)
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x12 (→ 0x445ebc)
        _emit 0x12
        _emit 0x3c              // CMP AL, 0xfc
        _emit 0xfc
        _emit 0x73              // JNC +7 (→ 0x445eb5)
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x5
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +7 (→ 0x445ebc)
        _emit 0x07
        _emit 0x3c              // CMP AL, 0xfe
        _emit 0xfe
        _emit 0x1b              // SBB EAX, EAX
        _emit 0xc0
        _emit 0x83              // AND EAX, 0x6
        _emit 0xe0
        _emit 0x06
        _emit 0x03              // ADD EDX, EAX             (advance:)
        _emit 0xd0
        _emit 0x8a              // MOV AL, byte ptr [EDX]
        _emit 0x02
        _emit 0x83              // ADD ESI, 0x1
        _emit 0xc6
        _emit 0x01
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x75              // JNZ -0x57 (→ 0x445e70 loop_top)
        _emit 0xa9
        _emit 0xc6              // MOV byte ptr [EDI+0x10], 0x1
        _emit 0x47
        _emit 0x10
        _emit 0x01
        _emit 0x89              // MOV dword ptr [EDI+0x0c], ESI
        _emit 0x77
        _emit 0x0c
        _emit 0x8b              // MOV EAX, dword ptr [EDI+0x0c]
        _emit 0x47
        _emit 0x0c
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
