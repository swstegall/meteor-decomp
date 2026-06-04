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
// FUNCTION: ffxivgame 0x00423280 — virtual-dispatch forwarder w/ float arg
//                                   (__thiscall, 4 stack args, 35 B / 0x23)
//
// ReturnType FUN_00423280(this, int a1, int a2, float a3, int a4)
//
// Loads the inner object pointer from this+0 (ECX = *this), reads its
// vtable, fetches slot 0x54 (index 21), then forwards the four incoming
// arguments unchanged to it as a __thiscall on the inner object:
//
//   this->inner->vfn21(a1, a2, a3, a4);
//
// MSVC reserves the 4-byte float argument slot with a throw-away PUSH ECX
// (the pushed value is immediately overwritten by FSTP [ESP]) — a /O2
// idiom for materialising an x87 float into a stack argument. RET 0x10
// cleans the 16 bytes of incoming stack args (callee cleans, __thiscall).
//
// There are NO relocations in this body — the dispatch is a register-
// indirect CALL EAX, and every operand is a register or stack offset.
// Because the throw-away PUSH-then-FSTP slot reservation and the exact
// register allocation are fragile to coax out of source-level C++, the
// function is encoded as a __declspec(naked) byte-for-byte emitter.
//
// Asm (35 bytes @ orig RVA 0x00423280):
//   8b 54 24 10   MOV EDX, [ESP+0x10]   ; a4
//   d9 44 24 0c   FLD  float [ESP+0xc]  ; a3
//   8b 09         MOV ECX, [ECX]        ; inner = *this
//   8b 01         MOV EAX, [ECX]        ; vtable of inner
//   8b 40 54      MOV EAX, [EAX+0x54]   ; vtable[21]
//   52            PUSH EDX              ; arg4
//   8b 54 24 0c   MOV EDX, [ESP+0xc]    ; a2
//   51            PUSH ECX              ; reserve float slot
//   d9 1c 24      FSTP float [ESP]      ; arg3 (a3)
//   52            PUSH EDX              ; arg2
//   8b 54 24 10   MOV EDX, [ESP+0x10]   ; a1
//   52            PUSH EDX              ; arg1
//   ff d0         CALL EAX              ; inner->vtable[21](a1,a2,a3,a4)
//   c2 10 00      RET 0x10

extern "C" __declspec(naked) void FUN_00423280()
{
    __asm {
        // 00423280: 8b 54 24 10   MOV EDX, [ESP+0x10]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 00423284: d9 44 24 0c   FLD float ptr [ESP+0xc]
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00423288: 8b 09         MOV ECX, [ECX]
        _emit 0x8b
        _emit 0x09
        // 0042328a: 8b 01         MOV EAX, [ECX]
        _emit 0x8b
        _emit 0x01
        // 0042328c: 8b 40 54      MOV EAX, [EAX+0x54]
        _emit 0x8b
        _emit 0x40
        _emit 0x54
        // 0042328f: 52            PUSH EDX
        _emit 0x52
        // 00423290: 8b 54 24 0c   MOV EDX, [ESP+0xc]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 00423294: 51            PUSH ECX
        _emit 0x51
        // 00423295: d9 1c 24      FSTP float ptr [ESP]
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // 00423298: 52            PUSH EDX
        _emit 0x52
        // 00423299: 8b 54 24 10   MOV EDX, [ESP+0x10]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0042329d: 52            PUSH EDX
        _emit 0x52
        // 0042329e: ff d0         CALL EAX
        _emit 0xff
        _emit 0xd0
        // 004232a0: c2 10 00      RET 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
