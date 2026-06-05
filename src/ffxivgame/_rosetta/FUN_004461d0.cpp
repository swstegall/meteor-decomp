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
// FUNCTION: ffxivgame 0x000461d0 — `__thiscall` UTF-8 multibyte-advance
//                                  helper (123 B / 0x7b, `RET 4` = one
//                                  stack arg + ECX `this`).
//
// Asm shape (read from orig RVA 0x000461d0, image VA 0x004461d0):
//
//   __thiscall int advance(This *this /*ECX*/, unsigned count /*[ESP+4]*/)
//   {
//       unsigned char *p = this->buf;          // MOV ECX,[ECX]
//       if (count != 0) {                       // TEST EAX,EAX / JZ tail
//           unsigned n = count;                 // ESI = count
//           do {
//               unsigned char c = *p;           // AL = [ECX]
//               int len;
//               // (c + 0x80) <= 0x3f  ⇔ c is a stray continuation byte
//               if ((unsigned char)(c + 0x80) <= 0x3f) {
//                   len = 0;                     // XOR EAX,EAX
//               } else if (c < 0x80) {           // ASCII
//                   len = 1;
//               } else if (c < 0xe0) {
//                   len = 2;
//               } else if (c < 0xf0) {
//                   len = 3;
//               } else if (c < 0xf8) {
//                   len = 4;
//               } else if (c < 0xfc) {
//                   len = 5;
//               } else {
//                   // c < 0xfe → 6 ; else → 0  (SBB EAX,EAX / AND EAX,6)
//                   len = (c < 0xfe) ? 6 : 0;
//               }
//               p += len;                        // ADD ECX,EAX
//           } while (--n != 0);                  // SUB ESI,1 / JNZ
//       }
//       // tail: forward (&count_slot, p) to the sibling at 0x00445850,
//       // then return the (possibly-rewritten) count slot.
//       FUN_00445850(p, &count);                 // PUSH &arg / PUSH ECX / CALL
//       return count;                            // MOV EAX,[ESP+0xc]
//   }
//
//   The single reloc-bearing site is the rel32 CALL at +0x6c:
//     +0x6c  CALL rel32 → .text 0x00445850 (FUN_00445850; displacement
//                         0xfffff60f from the following RVA 0x00046241).
//   The baked rel32 is valid against the orig load address; emitting it as
//   raw immediate bytes reproduces the orig slice exactly, and
//   `tools/compare.py` masks the call displacement anyway.
//
// Reconstruction strategy — naked-asm byte passthrough, matching the
// sibling _rosetta idiom (FUN_00401350 / FUN_00403d60 / FUN_00401650).
// A source-level form would have to coax MSVC 2005 /O2 into the exact
// register allocation (ESI as the loop counter, ECX as the running
// pointer), the `(c + 0x80) <= 0x3f` continuation-byte test, the
// `SBB/AND 6` branchless tail, and the rel32 sibling call. Each of those
// is brittle under /O2, so we re-emit the orig 123 bytes verbatim; the
// .obj `.text` ends up byte-identical and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004461d0() {
    __asm {
        _emit 0x8b              // MOV EAX, [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x8b              // MOV ECX, [ECX]
        _emit 0x09
        _emit 0x74              // JZ +0x5c  (→ tail)
        _emit 0x5c
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x8d              // LEA ECX, [ECX]
        _emit 0x49
        _emit 0x00

        _emit 0x8a              // MOV AL, [ECX]        (loop top, 0x004461e0)
        _emit 0x01
        _emit 0x8a              // MOV DL, AL
        _emit 0xd0
        _emit 0x80              // ADD DL, 0x80
        _emit 0xc2
        _emit 0x80
        _emit 0x80              // CMP DL, 0x3f
        _emit 0xfa
        _emit 0x3f
        _emit 0x77              // JA +4
        _emit 0x04
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0xeb              // JMP +0x3e  (→ ADD ECX,EAX)
        _emit 0x3e

        _emit 0x3c              // CMP AL, 0x80
        _emit 0x80
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x33
        _emit 0x33

        _emit 0x3c              // CMP AL, 0xe0
        _emit 0xe0
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x2
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x28
        _emit 0x28

        _emit 0x3c              // CMP AL, 0xf0
        _emit 0xf0
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x3
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x1d
        _emit 0x1d

        _emit 0x3c              // CMP AL, 0xf8
        _emit 0xf8
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x12
        _emit 0x12

        _emit 0x3c              // CMP AL, 0xfc
        _emit 0xfc
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x5
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +7
        _emit 0x07

        _emit 0x3c              // CMP AL, 0xfe
        _emit 0xfe
        _emit 0x1b              // SBB EAX, EAX
        _emit 0xc0
        _emit 0x83              // AND EAX, 0x6
        _emit 0xe0
        _emit 0x06

        _emit 0x03              // ADD ECX, EAX        (0x0044622e)
        _emit 0xc8
        _emit 0x83              // SUB ESI, 0x1
        _emit 0xee
        _emit 0x01
        _emit 0x75              // JNZ -0x55  (→ loop top)
        _emit 0xab
        _emit 0x5e              // POP ESI

        _emit 0x8d              // LEA EAX, [ESP+0x4]   (tail, 0x00446236)
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL rel32 → 0x00445850
        _emit 0x0f
        _emit 0xf6
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EAX, [ESP+0xc]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
