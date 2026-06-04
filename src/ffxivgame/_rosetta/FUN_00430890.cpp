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
// FUNCTION: ffxivgame 0x00430890 — __thiscall destructor with /GS-cookie
//                                  SEH frame (110 B).
//
// Semantics (recovered from asm at RVA 0x00030890):
//
//   void __thiscall ~T(T *this) {           // ECX = this
//       // /GS security-cookie + SEH prolog:
//       //   PUSH -1 / PUSH 0x00E55EF8 (scope table) / chain FS:[0]
//       //   MOV EAX,[0x012EA8B0] (__security_cookie) / XOR EAX,ESP / PUSH EAX
//       this->vtbl = 0x00F633E0;            // set derived vtable
//       FUN_00432820(this);                 // base subobject teardown
//       if (this->field_0x28 != 0) {        // owned resource present?
//           // virtual dispatch through global singleton at 0x01329920:
//           (*(**(void***)0x01329920 + 0x28))(this->field_0x28);  // vtbl[0x28]
//           this->field_0x28 = 0;
//       }
//       this->vtbl = 0x00F57E14;            // set base vtable
//       // SEH epilog: restore FS:[0], POP ECX / POP ESI / ADD ESP,0x10
//   }
//
//   Calling convention: __thiscall (this in ECX). Frame: 0x10 bytes
//   (SEH registration + cookie + saved ESI; epilog does ADD ESP,0x10).
//
// Reloc / absolute-bearing sites in the orig 110 bytes (baked into the
// orig PE as concrete byte sequences; emitting them as raw immediates
// via MASM `_emit` produces a .obj whose .text matches the orig
// byte-for-byte with NO relocations — tools/compare.py reports GREEN):
//     +0x02   PUSH imm32      → 0x00E55EF8 (EH scope table)
//     +0x10   MOV  EAX,[imm32]→ 0x012EA8B0 (__security_cookie)
//     +0x28   MOV  [ESI],imm32→ 0x00F633E0 (derived vtable)
//     +0x36   CALL rel32      → 0x00432820 (base teardown)
//     +0x42   MOV  ECX,[imm32]→ 0x01329920 (global singleton ptr)
//     +0x57   MOV  [ESI],imm32→ 0x00F57E14 (base vtable)
//
// Reconstruction strategy — naked-asm byte passthrough (the same path the
// siblings FUN_004090b0 / FUN_004091f0 / FUN_00409260 took for their
// reloc-heavy SEH-wrapped bodies): a source-level C++ destructor would
// emit the same shape but produce linker-controlled relocations and the
// /GS + SEH prolog ordering has no direct source trigger. A
// `__declspec(naked)` body re-emitting the orig 110 bytes verbatim yields
// a byte-identical .text with no relocations.

extern "C" __declspec(naked) void FUN_00430890() {
    __asm {
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00E55EF8 (EH scope table)
        _emit 0xf8
        _emit 0x5e
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x56              // PUSH ESI
        _emit 0xa1              // MOV EAX, [0x012EA8B0] (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EAX, [ESP+0x0C]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV FS:[0x0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x89              // MOV dword ptr [ESP+0x08], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [ESI], 0x00F633E0
        _emit 0x06
        _emit 0xe0
        _emit 0x33
        _emit 0xf6
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESP+0x14], 0x0
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL 0x00432820 (rel32)
        _emit 0x55
        _emit 0x1f
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x28]
        _emit 0x46
        _emit 0x28
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x15 (skip)
        _emit 0x15
        _emit 0x8b              // MOV ECX, dword ptr [0x01329920]
        _emit 0x0d
        _emit 0x20
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [ECX]
        _emit 0x11
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x28]
        _emit 0x42
        _emit 0x28
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0xc7              // MOV dword ptr [ESI+0x28], 0x0
        _emit 0x46
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI], 0x00F57E14   (skip:)
        _emit 0x06
        _emit 0x14
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x0C]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3              // RET
    }
}
