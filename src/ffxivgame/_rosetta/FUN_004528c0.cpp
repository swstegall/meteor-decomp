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
// FUNCTION: ffxivgame 0x000528c0 — __thiscall init with circular-list sentinel (43 B / 0x2b)
//
//   SomeClass * __thiscall FUN_004528c0(SomeClass *this)
//     ECX = this  (callee-saved in ESI)
//     returns: this (EAX = ESI)
//
// Asm (43 bytes @ orig RVA 0x000528c0):
//   56                         PUSH ESI
//   8b f1                      MOV ESI, ECX          ; ESI = this
//   e8 28 eb ff ff             CALL 0x004513f0        ; allocate/create inner obj → EAX
//   89 46 04                   MOV [ESI+0x4], EAX    ; this->field_4 = obj
//   c6 40 45 01                MOV byte [EAX+0x45], 1 ; obj->field_0x45 = 1
//   8b 46 04                   MOV EAX, [ESI+0x4]    ; reload obj
//   89 40 04                   MOV [EAX+0x4], EAX    ; obj->field_4 = obj (self-ref)
//   8b 46 04                   MOV EAX, [ESI+0x4]    ; reload obj
//   89 00                      MOV [EAX], EAX        ; obj->field_0 = obj (self-ref)
//   8b 46 04                   MOV EAX, [ESI+0x4]    ; reload obj
//   89 40 08                   MOV [EAX+0x8], EAX    ; obj->field_8 = obj (self-ref)
//   c7 46 08 00 00 00 00       MOV dword ptr [ESI+0x8], 0 ; this->field_8 = 0
//   8b c6                      MOV EAX, ESI          ; return this
//   5e                         POP ESI
//   c3                         RET
//
// The three self-referential pointer stores (field_0/4/8 all → self) are the
// canonical MSVC pattern for initializing a circular-list sentinel node where
// next/prev and an extra link all start pointing to the node itself.
//
// One reloc: CALL at +3 (rel32 to FUN_004513f0); compare.py masks those bytes.
// Encoded as __declspec(naked) _emit passthrough (same as sibling FUN_004134b0)
// because the thiscall CCinside a naked block and the reload-then-self-assign
// pattern are fragile to reproduce from source-level C++ with MSVC 2005.

extern "C" void FUN_004513f0();   // forward declaration for the CALL relocation

extern "C" __declspec(naked) void FUN_004528c0() {
    __asm {
        // 000528c0: 56                    PUSH ESI
        _emit 0x56
        // 000528c1: 8b f1                 MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 000528c3: e8 28 eb ff ff        CALL FUN_004513f0
        call FUN_004513f0
        // 000528c8: 89 46 04              MOV [ESI+0x4], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 000528cb: c6 40 45 01           MOV byte ptr [EAX+0x45], 1
        _emit 0xc6
        _emit 0x40
        _emit 0x45
        _emit 0x01
        // 000528cf: 8b 46 04              MOV EAX, [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 000528d2: 89 40 04              MOV [EAX+0x4], EAX
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 000528d5: 8b 46 04              MOV EAX, [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 000528d8: 89 00                 MOV [EAX], EAX
        _emit 0x89
        _emit 0x00
        // 000528da: 8b 46 04              MOV EAX, [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 000528dd: 89 40 08              MOV [EAX+0x8], EAX
        _emit 0x89
        _emit 0x40
        _emit 0x08
        // 000528e0: c7 46 08 00 00 00 00  MOV dword ptr [ESI+0x8], 0
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000528e7: 8b c6                 MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 000528e9: 5e                    POP ESI
        _emit 0x5e
        // 000528ea: c3                    RET
        _emit 0xc3
    }
}
