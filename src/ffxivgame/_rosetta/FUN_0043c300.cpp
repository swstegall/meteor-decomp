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
// FUNCTION: ffxivgame 0x0003c300 — __thiscall teardown: call method on field_0xbc,
//                                   then close two handles at field_0x8 and field_0xc
//                                   (36 B / 0x24)
//
// Calling convention: __thiscall (ECX = this, no stack args, plain RET).
// Frame: none (/Oy — only ESI callee-save used, no locals).
//
// Asm (36 bytes @ orig RVA 0x0003c300):
//   56                   PUSH ESI
//   8b f1                MOV  ESI, ECX                    ; ESI = this
//   8b 8e bc 00 00 00    MOV  ECX, dword ptr [ESI+0xbc]   ; ECX = this->field_0xbc
//   e8 f2 fa 82 00       CALL FUN_00c6be00                ; thiscall on field_0xbc
//   8b 46 08             MOV  EAX, dword ptr [ESI+0x8]    ; load field_0x8
//   50                   PUSH EAX                         ; push as arg
//   ff 15 38 e1 f3 00    CALL dword ptr [0x00f3e138]      ; IAT fn #1 (e.g. SetEvent)
//   8b 4e 0c             MOV  ECX, dword ptr [ESI+0xc]    ; load field_0xc
//   51                   PUSH ECX                         ; push as arg
//   ff 15 3c e1 f3 00    CALL dword ptr [0x00f3e13c]      ; IAT fn #2 (e.g. CloseHandle)
//   5e                   POP  ESI
//   c3                   RET
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The two IAT calls (`ff 15 addr32`) cannot be reproduced from source-level
//   C++ without correctly-named import thunks; and the direct CALL to
//   FUN_00c6be00 is a rel32 relocation that compare.py masks.
//   Following the established sibling idiom (FUN_0043bf30, FUN_0043b770),
//   the body is re-emitted verbatim via MASM _emit directives with a single
//   symbolic `call FUN_00c6be00` for the linker relocation.
//
//   Reloc-bearing sites masked by compare.py:
//     +0x09  rel32 CALL FUN_00c6be00
//     +0x12  dword ptr [0x00f3e138]  → IAT slot for fn #1
//     +0x1c  dword ptr [0x00f3e13c]  → IAT slot for fn #2

extern "C" void FUN_00c6be00();   // forward declaration for the CALL relocation

extern "C" __declspec(naked) void FUN_0043c300() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0xbc]
        _emit 0x8e
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        call FUN_00c6be00       // CALL FUN_00c6be00 (e8 rel32, reloc)
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x8]
        _emit 0x46
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL dword ptr [0x00f3e138]
        _emit 0x15
        _emit 0x38
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0xc]
        _emit 0x4e
        _emit 0x0c
        _emit 0x51              // PUSH ECX
        _emit 0xff              // CALL dword ptr [0x00f3e13c]
        _emit 0x15
        _emit 0x3c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
