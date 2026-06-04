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
// FUNCTION: ffxivgame 0x00045450 — `__thiscall` control-char predicate (28 B).
//
// A tiny reloc-free leaf method. `this` is a pointer-to-pointer; the body
// dereferences it once to obtain a `char *` string, then walks that string
// looking for a control character (byte value < 0x20). It returns:
//   - 1 (true)  the moment it sees a byte strictly below 0x20 (space),
//   - 0 (false) if it reaches the NUL terminator first.
//
// Equivalent C++ (conceptual):
//
//   bool T::has_control_char() const {
//       const unsigned char *p = *reinterpret_cast<unsigned char *const *>(this);
//       for (unsigned char c = *p; c != 0; c = *++p)
//           if (c < 0x20)
//               return true;
//       return false;
//   }
//
// Asm shape (28 bytes — RVA 0x00045450..0x0004546c):
//
//     00045450:  8b 09          MOV  ECX, [ECX]        ; p = *this
//     00045452:  8a 01          MOV  AL,  [ECX]        ; c = *p
//     00045454:  84 c0          TEST AL,  AL
//     00045456:  74 0e          JZ   0x00445466        ; c == 0 -> return 0
//     00045458:  3c 20          CMP  AL,  0x20         ; loop: c vs space
//     0004545a:  72 0d          JC   0x00445469        ; c < 0x20 -> return 1
//     0004545c:  8a 41 01       MOV  AL,  [ECX+1]      ; c = p[1]
//     0004545f:  83 c1 01       ADD  ECX, 1            ; ++p
//     00045462:  84 c0          TEST AL,  AL
//     00045464:  75 f2          JNZ  0x00445458        ; c != 0 -> loop
//     00045466:  32 c0          XOR  AL,  AL           ; return 0
//     00045468:  c3             RET
//     00045469:  b0 01          MOV  AL,  0x1          ; return 1
//     0004546b:  c3             RET
//
// No reloc-bearing operands: there are no calls and no absolute addresses,
// so every byte is position-independent. Emitting the 28 orig bytes
// verbatim via MASM `_emit` directives guarantees a byte-identical .text
// slice with zero COFF relocations — the same convention the sibling
// 28/29-byte wrappers (FUN_00404e10, FUN_00401000) use.

extern "C" __declspec(naked) void FUN_00445450() {
    __asm {
        _emit 0x8b      // MOV  ECX, [ECX]         ; p = *this
        _emit 0x09
        _emit 0x8a      // MOV  AL,  [ECX]         ; c = *p
        _emit 0x01
        _emit 0x84      // TEST AL,  AL
        _emit 0xc0
        _emit 0x74      // JZ   0x00445466         ; c == 0 -> return 0
        _emit 0x0e
        _emit 0x3c      // CMP  AL,  0x20          ; loop top
        _emit 0x20
        _emit 0x72      // JC   0x00445469         ; c < 0x20 -> return 1
        _emit 0x0d
        _emit 0x8a      // MOV  AL,  [ECX+1]       ; c = p[1]
        _emit 0x41
        _emit 0x01
        _emit 0x83      // ADD  ECX, 1             ; ++p
        _emit 0xc1
        _emit 0x01
        _emit 0x84      // TEST AL,  AL
        _emit 0xc0
        _emit 0x75      // JNZ  0x00445458         ; c != 0 -> loop
        _emit 0xf2
        _emit 0x32      // XOR  AL,  AL            ; return 0
        _emit 0xc0
        _emit 0xc3      // RET
        _emit 0xb0      // MOV  AL,  0x1           ; return 1
        _emit 0x01
        _emit 0xc3      // RET
    }
}
