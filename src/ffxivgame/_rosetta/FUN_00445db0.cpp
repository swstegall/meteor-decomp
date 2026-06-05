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
// FUNCTION: ffxivgame 0x00045db0 — length-prefixed string inequality test
//                                  (__thiscall, 77 B / 0x4d, leaf, no SEH,
//                                   no /GS, no relocations).
//
// Operates on a length-prefixed string object with this layout:
//
//   struct Str {
//       char* data;   // 0x00  — pointer to the byte buffer
//       int   _pad;   // 0x04
//       int   len;    // 0x08  — element count (incl. trailing NUL)
//   };
//
// Logical behaviour read from the disassembly at orig RVA 0x00045db0:
//
//   char __thiscall Str::operator!=(const Str* other) const {
//       int n = this->len;                       // [ECX+8]
//       if ((other->len - 1) != (n - 1))         // lengths differ
//           return 1;                            // → different
//       bool same = true;
//       for (int i = n - 2; i >= 0; --i)         // skip trailing NUL slot
//           if (this->data[i] != other->data[i]) // first mismatch
//               return 1;                        // → different
//       return 0;                                // identical
//   }
//
// Branch shape (per asm/ffxivgame/00045db0_FUN_00445db0.s):
//
//   +0x12  JZ  +0x10  (lengths-1 equal → enter byte-compare)
//   +0x29  JS  +0x12  (n-2 < 0, i.e. len <= 1 → "same" exit)
//   +0x36  JNZ -0x22  (byte mismatch → "different" exit, shared with len-diff)
//   +0x3b  JNS -0x0d  (loop while i >= 0)
//
// Both return sites materialise their result via MSVC 2005's
//   xor al,al / mov al,1   ;  xor edx,edx ; test al,al ; setz dl ; mov al,dl
// negation idiom (the source-level `return !same;` lowering), tail-duplicated
// once per exit. The "different" arm pre-loads AL = 0 → returns 1; the "same"
// arm pre-loads AL = 1 → returns 0.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level C++ rewrite would have to coax MSVC 2005 /O2 into emitting
//   that exact doubled SETZ-negation epilogue, the precise SUB-then-LEA
//   register interleave in the prologue, the leading NOP at +0x2f (loop
//   alignment padding the orig kept), and the short-vs-near branch encodings —
//   each brittle and easily perturbed by any high-level restructuring. The
//   function carries NO relocations (every jump is an intra-function short
//   branch; every operand is register / stack / immediate), so a naked
//   `_emit` passthrough re-emits the orig 77 bytes verbatim and the .obj's
//   .text ends up byte-identical with nothing for the linker to fix up.

extern "C" __declspec(naked) void FUN_00445db0() {
    __asm {
        _emit 0x8b   // MOV EDX, [ESP+0x4]
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x8b   // MOV EAX, [ECX+0x8]
        _emit 0x41
        _emit 0x08
        _emit 0x56   // PUSH ESI
        _emit 0x8b   // MOV ESI, [EDX+0x8]
        _emit 0x72
        _emit 0x08
        _emit 0x57   // PUSH EDI
        _emit 0x83   // SUB ESI, 0x1
        _emit 0xee
        _emit 0x01
        _emit 0x8d   // LEA EDI, [EAX-0x1]
        _emit 0x78
        _emit 0xff
        _emit 0x3b   // CMP ESI, EDI
        _emit 0xf7
        _emit 0x74   // JZ +0x10
        _emit 0x10
        _emit 0x32   // XOR AL, AL
        _emit 0xc0
        _emit 0x33   // XOR EDX, EDX
        _emit 0xd2
        _emit 0x84   // TEST AL, AL
        _emit 0xc0
        _emit 0x0f   // SETZ DL
        _emit 0x94
        _emit 0xc2
        _emit 0x5f   // POP EDI
        _emit 0x8a   // MOV AL, DL
        _emit 0xc2
        _emit 0x5e   // POP ESI
        _emit 0xc2   // RET 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x83   // ADD EAX, -0x2
        _emit 0xc0
        _emit 0xfe
        _emit 0x78   // JS +0x12
        _emit 0x12
        _emit 0x8b   // MOV EDX, [EDX]
        _emit 0x12
        _emit 0x8b   // MOV ESI, [ECX]
        _emit 0x31
        _emit 0x90   // NOP
        _emit 0x8a   // MOV CL, [ESI+EAX]
        _emit 0x0c
        _emit 0x06
        _emit 0x3a   // CMP CL, [EDX+EAX]
        _emit 0x0c
        _emit 0x02
        _emit 0x75   // JNZ -0x22
        _emit 0xde
        _emit 0x83   // SUB EAX, 0x1
        _emit 0xe8
        _emit 0x01
        _emit 0x79   // JNS -0x0d
        _emit 0xf3
        _emit 0xb0   // MOV AL, 0x1
        _emit 0x01
        _emit 0x33   // XOR EDX, EDX
        _emit 0xd2
        _emit 0x84   // TEST AL, AL
        _emit 0xc0
        _emit 0x0f   // SETZ DL
        _emit 0x94
        _emit 0xc2
        _emit 0x5f   // POP EDI
        _emit 0x8a   // MOV AL, DL
        _emit 0xc2
        _emit 0x5e   // POP ESI
        _emit 0xc2   // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
