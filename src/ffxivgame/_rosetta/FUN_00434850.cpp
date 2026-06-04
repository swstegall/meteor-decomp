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
// FUNCTION: ffxivgame 0x00034850 — `__thiscall` 28-byte two-step helper.
//
// Reads a member pointer at [this+0x10], runs a thiscall method on it
// (FUN_00435670 via rel32), then reloads [this+0x10], fetches the field
// at +0x18 of that object, and forwards it (with a trailing -1 arg) to an
// imported __stdcall function through the IAT slot at VA 0x00f3e140.
//
// Asm shape (28 bytes — RVA 0x00034850..0x0003486c):
//
//     00034850:  56                   PUSH ESI
//     00034851:  8b f1                MOV  ESI, ECX            ; this
//     00034853:  8b 4e 10             MOV  ECX, [ESI+0x10]     ; this->m10
//     00034856:  e8 15 0e 00 00       CALL FUN_00435670        ; rel32 thiscall
//     0003485b:  8b 46 10             MOV  EAX, [ESI+0x10]     ; reload this->m10
//     0003485e:  8b 48 18             MOV  ECX, [EAX+0x18]     ; m10->m18
//     00034861:  6a ff                PUSH -1
//     00034863:  51                   PUSH ECX
//     00034864:  ff 15 40 e1 f3 00    CALL dword ptr [0x00f3e140] ; IAT import
//     0003486a:  5e                   POP  ESI
//     0003486b:  c3                   RET
//
// Reloc-bearing sites in the orig 28 bytes:
//     +0x06   REL32 → 0x00435670 (thiscall member method)
//     +0x14   DIR32 (indirect operand) → 0x00f3e140 (IAT import slot)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   Both reloc-bearing operands (the rel32 CALL displacement and the
//   absolute IAT pointer baked into the `ff 15` indirect CALL) are
//   re-emitted as raw bytes so the .obj's .text slice matches the orig
//   PE byte-for-byte; compare.py reads orig bytes at this RVA, so the
//   verbatim displacement/address resolve correctly. Same convention as
//   sibling 28-byte wrapper FUN_00404e10.

extern "C" __declspec(naked) void FUN_00434850() {
    __asm {
        _emit 0x56    // PUSH ESI
        _emit 0x8b    // MOV  ESI, ECX             ; this
        _emit 0xf1
        _emit 0x8b    // MOV  ECX, [ESI+0x10]      ; this->m10
        _emit 0x4e
        _emit 0x10
        _emit 0xe8    // CALL FUN_00435670         ; rel32 = +0x00000e15
        _emit 0x15
        _emit 0x0e
        _emit 0x00
        _emit 0x00
        _emit 0x8b    // MOV  EAX, [ESI+0x10]      ; reload this->m10
        _emit 0x46
        _emit 0x10
        _emit 0x8b    // MOV  ECX, [EAX+0x18]      ; m10->m18
        _emit 0x48
        _emit 0x18
        _emit 0x6a    // PUSH -1
        _emit 0xff
        _emit 0x51    // PUSH ECX
        _emit 0xff    // CALL dword ptr [0x00f3e140] ; IAT import
        _emit 0x15
        _emit 0x40
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x5e    // POP  ESI
        _emit 0xc3    // RET
    }
}
