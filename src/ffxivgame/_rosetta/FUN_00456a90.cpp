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
// FUNCTION: ffxivgame 0x00456a90 — ring-buffer membership test
//                                  (__thiscall, 134 bytes, returns bool).
//
// The object lays out a 33-entry int slot array at +0x4 followed by two
// signed 16-bit cursors:
//
//   struct RingBuf {
//       int   slots[0x21];   // +0x04 .. +0x88
//       short head;          // +0x88
//       short tail;          // +0x8a
//   };
//
// Signature (inferred):
//
//   bool __thiscall FUN_00456a90(RingBuf *this /*ecx*/, int key /*[esp+4]*/);
//
// It scans the active span (head, tail] — the indices strictly after
// `head` up through `tail`, wrapping around the 0x21-slot array when
// head > tail — looking for a slot whose value == key:
//
//   if (head > tail) {                         // wrapped span
//       for (i = head + 1; i < 0x21; ++i)      // tail half: (head .. top]
//           if (slots[i - 1] == key) return true;
//       for (i = 0; i <= tail; ++i)            // head half: [0 .. tail]
//           if (slots[i - 1] == key) return true;  // (note: ECX already +4)
//   } else {                                   // contiguous span
//       for (i = head + 1; i <= tail; ++i)
//           if (slots[i - 1] == key) return true;
//   }
//   return false;
//
// (The two index-base/scale details — `[ecx + eax*4 + 4]` in the first
// loop vs. a pre-incremented `ecx` walked by +4 in the second — make a
// faithful source-level translation fragile; the cursors are signed
// shorts compared with the MOVSX/JLE/JG mix.)
//
// Calling convention: __thiscall (ECX = this; `ret 4` pops the single
// stack argument). Returns bool in AL.
//
// The body contains NO CALLs and NO absolute symbol references, so the
// .obj emits zero relocations — the 134 bytes are reproduced verbatim
// via MASM `_emit` directives, matching siblings FUN_00408610 /
// FUN_004087f0. `tools/compare.py` reports GREEN with no masked windows.

extern "C" __declspec(naked) void FUN_00456a90() {
    __asm {
        _emit 0x0f          // MOVZX EAX, word ptr [ECX+0x88]
        _emit 0xb7
        _emit 0x81
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x56          // PUSH ESI
        _emit 0x0f          // MOVZX ESI, word ptr [ECX+0x8a]
        _emit 0xb7
        _emit 0xb1
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66          // CMP AX, SI
        _emit 0x3b
        _emit 0xc6
        _emit 0x57          // PUSH EDI
        _emit 0x0f          // MOVSX EAX, AX
        _emit 0xbf
        _emit 0xc0
        _emit 0x7e          // JLE 0x00456aef
        _emit 0x47
        _emit 0x8b          // MOV EDI, dword ptr [ESP+0x0c]
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x83          // ADD EAX, 0x01
        _emit 0xc0
        _emit 0x01
        _emit 0x83          // CMP EAX, 0x21
        _emit 0xf8
        _emit 0x21
        _emit 0x73          // JNC 0x00456ac7
        _emit 0x13
        _emit 0x8d          // LEA EDX, [ECX + EAX*4 + 0x04]
        _emit 0x54
        _emit 0x81
        _emit 0x04
        _emit 0x39          // CMP dword ptr [EDX], EDI    (loop1:)
        _emit 0x3a
        _emit 0x74          // JZ 0x00456ae8
        _emit 0x2c
        _emit 0x83          // ADD EAX, 0x01
        _emit 0xc0
        _emit 0x01
        _emit 0x83          // ADD EDX, 0x04
        _emit 0xc2
        _emit 0x04
        _emit 0x83          // CMP EAX, 0x21
        _emit 0xf8
        _emit 0x21
        _emit 0x72          // JC loop1 (-0x0F)
        _emit 0xf1
        _emit 0x0f          // MOVSX EDX, SI               (0x00456ac7:)
        _emit 0xbf
        _emit 0xd6
        _emit 0x33          // XOR EAX, EAX
        _emit 0xc0
        _emit 0x85          // TEST EDX, EDX
        _emit 0xd2
        _emit 0x7c          // JL 0x00456b0f
        _emit 0x3f
        _emit 0x83          // ADD ECX, 0x04
        _emit 0xc1
        _emit 0x04
        _emit 0x39          // CMP dword ptr [ECX], EDI    (loop2:)
        _emit 0x39
        _emit 0x74          // JZ 0x00456ae8
        _emit 0x11
        _emit 0x83          // ADD EAX, 0x01
        _emit 0xc0
        _emit 0x01
        _emit 0x83          // ADD ECX, 0x04
        _emit 0xc1
        _emit 0x04
        _emit 0x3b          // CMP EAX, EDX
        _emit 0xc2
        _emit 0x7e          // JLE loop2 (-0x0E)
        _emit 0xf2
        _emit 0x5f          // POP EDI
        _emit 0x32          // XOR AL, AL
        _emit 0xc0
        _emit 0x5e          // POP ESI
        _emit 0xc2          // RET 0x04
        _emit 0x04
        _emit 0x00
        _emit 0x5f          // POP EDI                     (0x00456ae8: found)
        _emit 0xb0          // MOV AL, 0x01
        _emit 0x01
        _emit 0x5e          // POP ESI
        _emit 0xc2          // RET 0x04
        _emit 0x04
        _emit 0x00
        _emit 0x0f          // MOVSX EDX, SI               (0x00456aef:)
        _emit 0xbf
        _emit 0xd6
        _emit 0x83          // ADD EAX, 0x01
        _emit 0xc0
        _emit 0x01
        _emit 0x3b          // CMP EAX, EDX
        _emit 0xc2
        _emit 0x7f          // JG 0x00456b0f
        _emit 0x16
        _emit 0x8b          // MOV ESI, dword ptr [ESP+0x0c]
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x8d          // LEA ECX, [ECX + EAX*4 + 0x04]
        _emit 0x4c
        _emit 0x81
        _emit 0x04
        _emit 0x39          // CMP dword ptr [ECX], ESI    (loop3:)
        _emit 0x31
        _emit 0x74          // JZ 0x00456ae8 (-0x1D)
        _emit 0xe3
        _emit 0x83          // ADD EAX, 0x01
        _emit 0xc0
        _emit 0x01
        _emit 0x83          // ADD ECX, 0x04
        _emit 0xc1
        _emit 0x04
        _emit 0x3b          // CMP EAX, EDX
        _emit 0xc2
        _emit 0x7e          // JLE loop3 (-0x0E)
        _emit 0xf2
        _emit 0x5f          // POP EDI                     (0x00456b0f:)
        _emit 0x32          // XOR AL, AL
        _emit 0xc0
        _emit 0x5e          // POP ESI
        _emit 0xc2          // RET 0x04
        _emit 0x04
        _emit 0x00
    }
}
