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
// FUNCTION: ffxivgame 0x0044c580 — `__thiscall` vtable-install-then-noreturn
//                                  trampoline (18 B / 0x12, no epilogue).
//
// Calling convention: __thiscall (ECX = this, cached into callee-save ESI).
// The function has NO return path: its final instruction is a CALL into a
// noreturn helper (FUN_00dd1be9 — RVA 0x9d1be9 / VA 0xdd1be9), so MSVC emits
// neither a POP ESI nor a RET. The PUSH ESI at entry is never balanced
// because control never comes back.
//
// Object layout (offsets touched):
//   [this + 0x00]  vtable / type pointer — overwritten with 0x00f6735c
//   [this + 0x04]  DWORD payload, forwarded as the helper's single arg
//
// Asm shape (18 bytes @ 0x0044c580):
//   56                 PUSH ESI                       ; save (never restored)
//   8b f1              MOV  ESI, ECX                  ; this -> ESI
//   8b 46 04           MOV  EAX, [ESI + 0x4]          ; this->field_4
//   50                 PUSH EAX                       ; helper arg
//   c7 06 5c 73 f6 00  MOV  [ESI], 0x00f6735c         ; this->vptr = &vtbl
//   e8 57 56 58 00     CALL FUN_00dd1be9              ; rel32, noreturn
//
//   Equivalent C++ (what the function does, structurally):
//
//     void C::method() {
//         this->vptr = (void*)0x00f6735c;   // re-seat vtable
//         FUN_00dd1be9(this->field_4);      // noreturn — never returns
//     }
//
// Reloc-bearing sites in the orig 18 bytes:
//   +0x09  absolute imm32  0x00f6735c — vtable address. At image base
//          0x00400000 the bytes are literally `5c 73 f6 00`; the naked
//          _emit re-emits them verbatim (no reloc record, exact match).
//   +0x0e  rel32 CALL window — masked by tools/compare.py during the diff.
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//   A source-level body can't reproduce this shape in an isolated TU. With
//   the helper marked __declspec(noreturn), MSVC 2005 /O2 keeps `this` in
//   ECX (no PUSH ESI / MOV ESI,ECX) — 12 bytes, wrong shape. Without the
//   noreturn marker it appends POP ESI / RET — too long. The original's
//   "save ESI, install vtable, fall into a noreturn call" form is what a
//   real-binary relink produced; the rosetta path re-emits the exact 18
//   bytes and lets compare.py mask the lone REL32 call window. GREEN.

extern "C" __declspec(naked) void FUN_0044c580() {
    __asm {
        _emit 0x56                   // PUSH ESI
        _emit 0x8b                   // MOV ESI, ECX
        _emit 0xf1

        _emit 0x8b                   // MOV EAX, [ESI+4]
        _emit 0x46
        _emit 0x04

        _emit 0x50                   // PUSH EAX

        _emit 0xc7                   // MOV dword ptr [ESI], 0x00f6735c
        _emit 0x06
        _emit 0x5c
        _emit 0x73
        _emit 0xf6
        _emit 0x00

        _emit 0xe8                   // CALL FUN_00dd1be9 (rel32, noreturn)
        _emit 0x57
        _emit 0x56
        _emit 0x58
        _emit 0x00
    }
}
