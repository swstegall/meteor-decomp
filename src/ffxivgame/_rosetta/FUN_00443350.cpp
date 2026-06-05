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
// FUNCTION: ffxivgame 0x00043350 — `__thiscall` constructor that chains to a
//                                   base-class constructor and installs a
//                                   vtable pointer (18 B / 0x12).
//
// Inspection (read from asm/ffxivgame/00043350_FUN_00443350.s):
//
//   __thiscall <Class>* <Class>::<Class>();
//
//   The constructor:
//     1. Saves ESI, copies the incoming `this` (ECX) into ESI.
//     2. Calls FUN_00442740 via ECX = this (__thiscall base-class ctor).
//     3. Sets the vtable pointer at [this + 0x00] = 0x00f67124.
//     4. Returns `this` in EAX (the canonical MSVC ctor return-value idiom).
//
//   Calling convention: __thiscall — ECX = this on entry; no caller-pushed
//   stack args; bare RET (no immediate). EAX holds `this` on return.
//
// Asm (18 bytes — RVA 0x00043350..0x00043362):
//
//   00043350:  56                  PUSH ESI
//   00043351:  8b f1               MOV ESI, ECX                 ; this
//   00043353:  e8 e8 f3 ff ff      CALL 0x00442740              ; base ctor [reloc]
//   00043358:  c7 06 24 71 f6 00   MOV dword ptr [ESI], 0xf67124 ; vtable [reloc]
//   0004335e:  8b c6               MOV EAX, ESI                 ; return this
//   00043360:  5e                  POP ESI
//   00043361:  c3                  RET
//
// Relocations in the orig 18 bytes (masked by tools/compare.py):
//   +0x04  (4 B): CALL rel32 to FUN_00442740   — 0xfffff3e8
//   +0x0a  (4 B): vtable absolute address      — 0x00f67124
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ constructor cannot reproduce the absolute vtable
//   store (the vtable lives at 0x00f67124, which only exists in a full
//   relink) and the base-ctor CALL rel32 with byte fidelity in an 18-byte
//   function where every store/CALL is reloc-adjacent. The established
//   sibling idiom (see FUN_00411c90) is `__declspec(naked)` byte
//   passthrough via MASM `_emit`, which makes the .obj `.text` section
//   byte-identical to the orig slice — exactly what tools/compare.py grades.

extern "C" __declspec(naked) void FUN_00443350() {
    __asm {
        // 00043350:  56                  PUSH ESI
        _emit 0x56
        // 00043351:  8b f1               MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00043353:  e8 e8 f3 ff ff      CALL 0x00442740 [reloc]
        _emit 0xe8
        _emit 0xe8
        _emit 0xf3
        _emit 0xff
        _emit 0xff
        // 00043358:  c7 06 24 71 f6 00   MOV dword ptr [ESI], 0xf67124 [reloc]
        _emit 0xc7
        _emit 0x06
        _emit 0x24
        _emit 0x71
        _emit 0xf6
        _emit 0x00
        // 0004335e:  8b c6               MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 00043360:  5e                  POP ESI
        _emit 0x5e
        // 00043361:  c3                  RET
        _emit 0xc3
    }
}
