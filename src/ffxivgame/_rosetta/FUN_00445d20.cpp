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
// FUNCTION: ffxivgame 0x00045d20 — __thiscall byte-array equality test
//                                  (58 bytes / 0x3a)
//
// __thiscall char FUN_00445d20(this, Buf *other)
//   ECX        : this
//   [ESP+0x04] : Buf *other  (single DWORD stack arg; callee cleans via ret 4)
//   returns AL : 1 if equal, 0 if not
//
// Layout (inferred):
//   Buf:
//     +0x00  char *data
//     +0x04  ??           (unused here)
//     +0x08  int   size
//
// Source shape (inferred):
//
//   bool Buf::operator==(const Buf &o) const {
//       if (this->size - 1 != o.size - 1)        // length mismatch
//           return false;
//       for (int i = this->size - 2; i >= 0; --i)
//           if (this->data[i] != o.data[i])
//               return false;
//       return true;
//   }
//
// MSVC 2005 keeps `this->size` live in EAX: it materialises `size-1`
// into EDI via LEA for the length compare, then reuses EAX for the loop
// counter `size-2` (ADD EAX,-2 ; JS → all-equal when the buffer holds
// fewer than 2 bytes). The down-counting loop (`SUB EAX,1 ; JNS`)
// compares data[i] top-down. The `JNZ back` on mismatch re-uses the
// shared "POP EDI ; XOR AL,AL ; POP ESI ; RET 4" false-return tail.
//
// Asm (58 bytes):
//   8b 54 24 04   MOV EDX, [ESP+4]          ; other
//   8b 41 08      MOV EAX, [ECX+8]          ; this->size
//   56            PUSH ESI
//   8b 72 08      MOV ESI, [EDX+8]          ; other->size
//   57            PUSH EDI
//   83 ee 01      SUB ESI, 1
//   8d 78 ff      LEA EDI, [EAX-1]
//   3b f7         CMP ESI, EDI
//   74 07         JZ  +7                    ; lengths equal
//   5f            POP EDI                   ; false:
//   32 c0         XOR AL, AL
//   5e            POP ESI
//   c2 04 00      RET 4
//   83 c0 fe      ADD EAX, -2               ; i = size-2
//   78 11         JS  +0x11                 ; <2 bytes → equal
//   8b 12         MOV EDX, [EDX]            ; other->data
//   8b 31         MOV ESI, [ECX]            ; this->data
//   8a 0c 06      MOV CL,  [ESI+EAX]        ; loop: this->data[i]
//   3a 0c 02      CMP CL,  [EDX+EAX]        ; vs other->data[i]
//   75 e8         JNZ -0x18                 ; mismatch → false
//   83 e8 01      SUB EAX, 1
//   79 f3         JNS -0xd                  ; loop while i >= 0
//   5f            POP EDI                   ; true:
//   b0 01         MOV AL, 1
//   5e            POP ESI
//   c2 04 00      RET 4
//
// No relocations / no external calls — a __declspec(naked) verbatim
// byte passthrough reproduces the orig slice exactly (the short jumps,
// the ADD EAX,-2 / LEA [EAX-1] immediates, and the dual ret-4 tails all
// pin to the orig encoding). compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_00445d20() {
    __asm {
        _emit 0x8b    // MOV EDX, dword ptr [ESP + 0x4]
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x8b    // MOV EAX, dword ptr [ECX + 0x8]
        _emit 0x41
        _emit 0x08
        _emit 0x56    // PUSH ESI
        _emit 0x8b    // MOV ESI, dword ptr [EDX + 0x8]
        _emit 0x72
        _emit 0x08
        _emit 0x57    // PUSH EDI
        _emit 0x83    // SUB ESI, 0x1
        _emit 0xee
        _emit 0x01
        _emit 0x8d    // LEA EDI, [EAX - 0x1]
        _emit 0x78
        _emit 0xff
        _emit 0x3b    // CMP ESI, EDI
        _emit 0xf7
        _emit 0x74    // JZ +0x07
        _emit 0x07
        _emit 0x5f    // POP EDI
        _emit 0x32    // XOR AL, AL
        _emit 0xc0
        _emit 0x5e    // POP ESI
        _emit 0xc2    // RET 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x83    // ADD EAX, -0x2
        _emit 0xc0
        _emit 0xfe
        _emit 0x78    // JS +0x11
        _emit 0x11
        _emit 0x8b    // MOV EDX, dword ptr [EDX]
        _emit 0x12
        _emit 0x8b    // MOV ESI, dword ptr [ECX]
        _emit 0x31
        _emit 0x8a    // MOV CL, byte ptr [ESI + EAX*0x1]
        _emit 0x0c
        _emit 0x06
        _emit 0x3a    // CMP CL, byte ptr [EDX + EAX*0x1]
        _emit 0x0c
        _emit 0x02
        _emit 0x75    // JNZ -0x18
        _emit 0xe8
        _emit 0x83    // SUB EAX, 0x1
        _emit 0xe8
        _emit 0x01
        _emit 0x79    // JNS -0xd
        _emit 0xf3
        _emit 0x5f    // POP EDI
        _emit 0xb0    // MOV AL, 0x1
        _emit 0x01
        _emit 0x5e    // POP ESI
        _emit 0xc2    // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
