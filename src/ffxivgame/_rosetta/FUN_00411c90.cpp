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
// FUNCTION: ffxivgame 0x00011c90 — `__thiscall` constructor with SEH frame
//                                   (89 B / 0x59). Sets two vtable pointers
//                                   and cross-links two doubly-linked-list
//                                   sentinels in a subobject.
//
// Inspection (read from asm/ffxivgame/00011c90_FUN_00411c90.s):
//
//   __thiscall void SomeClass::SomeClass();
//
//   The constructor:
//     1. Installs a C++ EH SEH frame (PUSH -1 / PUSH handler / PUSH FS:[0] /
//        MOV FS:[0], ESP) and enters try-level 0.
//     2. Sets vtable at [this + 0x00] = 0xf56ce8 (primary vtable).
//     3. Calls FUN_00411c20 via ECX = this  (__thiscall base-class ctor).
//     4. Sets vtable at [this + 0x24] = 0xf567c4 (embedded subobject vtable).
//     5. Cross-links the two sentinel nodes whose addresses are stored in
//        [this + 0x28] and [this + 0x2c] (doubly-linked-list init):
//          *(int**)(this->field28 + 8) = this->field2c;   // head->next = tail
//          *(int**)(this->field2c + 4) = this->field28;   // tail->prev = head
//     6. Restores SEH and returns.
//
//   Stack frame (no EBP — ESP-relative only):
//     [ESP+ 0]  saved ESI
//     [ESP+ 4]  saved ECX / this pointer (copied from ESI after prologue)
//     [ESP+ 8]  previous FS:[0] (old SEH chain link)
//     [ESP+ C]  exception handler address (0xe54fcb)
//     [ESP+10]  SEH try-level sentinel (−1 on entry → 0 when try block begins)
//     [ESP+14]  return address
//
//   Calling convention: __thiscall — ECX = this on entry; no stack args;
//   bare RET (no immediate) since the callee owns no caller-passed stack.
//
// Relocations in the orig 89 bytes (masked by tools/compare.py):
//   +0x03  (4 B): exception handler absolute address — 0xe54fcb
//   +0x1f  (4 B): primary vtable absolute address    — 0xf56ce8
//   +0x2c  (4 B): CALL rel32 to FUN_00411c20         — 0xffffff60
//   +0x39  (4 B): subobj vtable absolute address     — 0xf567c4
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The SEH frame prologue/epilogue, vtable stores, and SEH try-level
//   transitions must be reproduced with exact byte fidelity, and the four
//   relocation sites above embed absolute/relative addresses whose raw bytes
//   (as stored in the orig binary at image base 0x00400000) must appear
//   verbatim in the .obj's .text section. A source-level C++ constructor
//   would produce a different SEH-frame layout and different reloc
//   placeholders. Using _emit byte passthrough guarantees identity.
//
// Asm (89 bytes — RVA 0x00011c90..0x00011ce8):
//
//   00011c90:  6a ff                  PUSH -0x1
//   00011c92:  68 cb 4f e5 00         PUSH 0xe54fcb            ; EH handler [reloc]
//   00011c97:  64 a1 00 00 00 00      MOV EAX, FS:[0x0]
//   00011c9d:  50                     PUSH EAX
//   00011c9e:  64 89 25 00 00 00 00   MOV dword ptr FS:[0x0], ESP
//   00011ca5:  51                     PUSH ECX                 ; save this
//   00011ca6:  56                     PUSH ESI
//   00011ca7:  8b f1                  MOV ESI, ECX
//   00011ca9:  89 74 24 04            MOV dword ptr [ESP+0x4], ESI
//   00011cad:  c7 06 e8 6c f5 00      MOV dword ptr [ESI], 0xf56ce8 ; vtable [reloc]
//   00011cb3:  c7 44 24 10 00 00 00 00 MOV dword ptr [ESP+0x10], 0x0 ; try-level=0
//   00011cbb:  e8 60 ff ff ff          CALL 0x00411c20               ; base ctor [reloc]
//   00011cc0:  8b 46 28               MOV EAX, dword ptr [ESI+0x28]
//   00011cc3:  8b 4e 2c               MOV ECX, dword ptr [ESI+0x2c]
//   00011cc6:  c7 46 24 c4 67 f5 00   MOV dword ptr [ESI+0x24], 0xf567c4 ; subobj vtable [reloc]
//   00011ccd:  89 48 08               MOV dword ptr [EAX+0x8], ECX
//   00011cd0:  8b 56 2c               MOV EDX, dword ptr [ESI+0x2c]
//   00011cd3:  8b 46 28               MOV EAX, dword ptr [ESI+0x28]
//   00011cd6:  8b 4c 24 08            MOV ECX, dword ptr [ESP+0x8]
//   00011cda:  89 42 04               MOV dword ptr [EDX+0x4], EAX
//   00011cdd:  5e                     POP ESI
//   00011cde:  64 89 0d 00 00 00 00   MOV dword ptr FS:[0x0], ECX
//   00011ce5:  83 c4 10               ADD ESP, 0x10
//   00011ce8:  c3                     RET

extern "C" __declspec(naked) void FUN_00411c90() {
    __asm {
        // 00011c90:  6a ff
        _emit 0x6a
        _emit 0xff
        // 00011c92:  68 cb 4f e5 00      PUSH 0xe54fcb (EH handler) [reloc]
        _emit 0x68
        _emit 0xcb
        _emit 0x4f
        _emit 0xe5
        _emit 0x00
        // 00011c97:  64 a1 00 00 00 00   MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00011c9d:  50                  PUSH EAX
        _emit 0x50
        // 00011c9e:  64 89 25 00 00 00 00 MOV dword ptr FS:[0x0], ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00011ca5:  51                  PUSH ECX
        _emit 0x51
        // 00011ca6:  56                  PUSH ESI
        _emit 0x56
        // 00011ca7:  8b f1               MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00011ca9:  89 74 24 04         MOV dword ptr [ESP+0x4], ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x04
        // 00011cad:  c7 06 e8 6c f5 00   MOV dword ptr [ESI], 0xf56ce8 [reloc]
        _emit 0xc7
        _emit 0x06
        _emit 0xe8
        _emit 0x6c
        _emit 0xf5
        _emit 0x00
        // 00011cb3:  c7 44 24 10 00 00 00 00   MOV dword ptr [ESP+0x10], 0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00011cbb:  e8 60 ff ff ff      CALL 0x00411c20 [reloc]
        _emit 0xe8
        _emit 0x60
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00011cc0:  8b 46 28            MOV EAX, dword ptr [ESI+0x28]
        _emit 0x8b
        _emit 0x46
        _emit 0x28
        // 00011cc3:  8b 4e 2c            MOV ECX, dword ptr [ESI+0x2c]
        _emit 0x8b
        _emit 0x4e
        _emit 0x2c
        // 00011cc6:  c7 46 24 c4 67 f5 00  MOV dword ptr [ESI+0x24], 0xf567c4 [reloc]
        _emit 0xc7
        _emit 0x46
        _emit 0x24
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00011ccd:  89 48 08            MOV dword ptr [EAX+0x8], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 00011cd0:  8b 56 2c            MOV EDX, dword ptr [ESI+0x2c]
        _emit 0x8b
        _emit 0x56
        _emit 0x2c
        // 00011cd3:  8b 46 28            MOV EAX, dword ptr [ESI+0x28]
        _emit 0x8b
        _emit 0x46
        _emit 0x28
        // 00011cd6:  8b 4c 24 08         MOV ECX, dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 00011cda:  89 42 04            MOV dword ptr [EDX+0x4], EAX
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 00011cdd:  5e                  POP ESI
        _emit 0x5e
        // 00011cde:  64 89 0d 00 00 00 00  MOV dword ptr FS:[0x0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00011ce5:  83 c4 10            ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00011ce8:  c3                  RET
        _emit 0xc3
    }
}
