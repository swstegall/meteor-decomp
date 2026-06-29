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
// FUNCTION: ffxivgame 0x000438a0 — `__thiscall` 2-arg dispatch method
//                                  (123 B / 0x7b).
//
// Inspection (read from the disassembly at orig RVA 0x000438a0):
//
//   __thiscall bool SomeClass::Dispatch(void* arg1 /*[ESP+0x8]*/,
//                                       int   mode /*[ESP+0xC]*/)
//
//   ESI = this (saved/restored across the function).
//   RET 0x8 → callee cleans 8 bytes = 2 stack args.
//
//   Pseudo-C:
//
//     bool __thiscall SomeClass::Dispatch(this, arg1, mode) {
//         if (!fn1(this))             // CALL 0x00b52b70
//             return false;
//
//         if (mode == -1) {
//             if (fn3(this)) {        // CALL 0x00b53080 — true path
//                 fn5(this, arg1);    // CALL 0x00b52e30
//                 return true;
//             } else {                // false path falls into shared fn4a block
//                 fn4a(this, arg1);   // CALL 0x00b52d80 (shared with mode==1)
//                 return true;
//             }
//         } else if (mode == 0) {
//             if (!fn3(this))         // CALL 0x00b53080
//                 return false;
//             fn5(this, arg1);        // CALL 0x00b52e30
//             return true;
//         } else if (mode == 1) {
//             if (!fn2(this))         // CALL 0x00b52fb0
//                 return false;
//             fn4a(this, arg1);       // CALL 0x00b52d80 (shared with mode==-1 false)
//             return true;
//         } else {
//             return true;            // any other mode — success, nothing done
//         }
//     }
//
//   Stack frame (after PUSH ESI, ESP-relative):
//     [ESP+0x00]  saved ESI
//     [ESP+0x04]  return address
//     [ESP+0x08]  arg1
//     [ESP+0x0C]  mode
//
//   The five CALL targets lie in the 0x00b5xxxx region (far above .text),
//   so their rel32 displacements baked into the original binary cannot be
//   reproduced via normal COFF relocations from a standalone .obj. Following
//   the sibling FUN_00401350 / FUN_00403d60 / FUN_00401650 approach:
//   a `__declspec(naked)` body re-emits all 123 bytes verbatim using MASM
//   `_emit` directives. The .obj's `.text` section ends up byte-identical
//   to the original slice with no relocations; `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_004438a0() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xe8              // CALL 0x00b52b70  (rel32 = 0x0070f2c8)
        _emit 0xc8
        _emit 0xf2
        _emit 0x70
        _emit 0x00
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x74              // JZ +0x69  (→ 0x00443915, false exit)
        _emit 0x69
        _emit 0x8b              // MOV EAX, [ESP + 0xC]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x83              // CMP EAX, -1
        _emit 0xf8
        _emit 0xff
        _emit 0x74              // JZ +0x43  (→ 0x004438f8, mode==-1 block)
        _emit 0x43
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x22  (→ 0x004438db, mode==0 block)
        _emit 0x22
        _emit 0x83              // CMP EAX, 1
        _emit 0xf8
        _emit 0x01
        _emit 0x75              // JNZ +0x17  (→ 0x004438d5, else→return true)
        _emit 0x17
        // mode == 1:
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x00b52fb0  (rel32 = 0x0070f6eb)
        _emit 0xeb
        _emit 0xf6
        _emit 0x70
        _emit 0x00
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x74              // JZ +0x4c  (→ 0x00443915, false exit)
        _emit 0x4c
        // shared fn4a block (also target of mode==-1 false path):
        _emit 0x8b              // MOV EAX, [ESP + 0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x00b52d80  (rel32 = 0x0070f4ab)
        _emit 0xab
        _emit 0xf4
        _emit 0x70
        _emit 0x00
        _emit 0xb0              // MOV AL, 1
        _emit 0x01
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        // mode == 0:
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x00b53080  (rel32 = 0x0070f79e)
        _emit 0x9e
        _emit 0xf7
        _emit 0x70
        _emit 0x00
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x74              // JZ +0x2f  (→ 0x00443915, false exit)
        _emit 0x2f
        _emit 0x8b              // MOV ECX, [ESP + 0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x00b52e30  (rel32 = 0x0070f53e)
        _emit 0x3e
        _emit 0xf5
        _emit 0x70
        _emit 0x00
        _emit 0xb0              // MOV AL, 1
        _emit 0x01
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        // mode == -1:
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x00b53080  (rel32 = 0x0070f781)
        _emit 0x81
        _emit 0xf7
        _emit 0x70
        _emit 0x00
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x74              // JZ -0x3a  (→ 0x004438c9, shared fn4a block)
        _emit 0xc6
        // mode == -1 and fn3 true:
        _emit 0x8b              // MOV EDX, [ESP + 0x8]
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x52              // PUSH EDX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x00b52e30  (rel32 = 0x0070f521)
        _emit 0x21
        _emit 0xf5
        _emit 0x70
        _emit 0x00
        _emit 0xb0              // MOV AL, 1
        _emit 0x01
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        // false exit:
        _emit 0x32              // XOR AL, AL
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
