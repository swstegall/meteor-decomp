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
// FUNCTION: ffxivgame 0x0000f6b0 — __thiscall: virtual call through inner->field_0x4c,
//                                   store result in field_0xc, return &field_0x8 (24 bytes)
//
// Asm (24 bytes @ orig RVA 0x0000f6b0):
//   56          PUSH ESI
//   8b f1       MOV ESI, ECX                  ; cache this
//   8b 46 04    MOV EAX, [ESI + 0x4]          ; EAX = this->inner
//   8b 48 4c    MOV ECX, [EAX + 0x4c]         ; ECX = inner->obj (field at 0x4c)
//   8b 11       MOV EDX, [ECX]                ; EDX = obj vtable
//   8b 42 04    MOV EAX, [EDX + 0x4]          ; EAX = vtable[1]
//   ff d0       CALL EAX                       ; call obj->method1() — ECX = obj (__thiscall)
//   89 46 0c    MOV [ESI + 0xc], EAX          ; this->fieldC = return value
//   8d 46 08    LEA EAX, [ESI + 0x8]          ; EAX = &this->field8
//   5e          POP ESI
//   c3          RET                            ; __thiscall, no stack args
//
// Same Outer/Inner struct relationship as FUN_0040f630: this->field_0x4 is a
// pointer to an Inner object; Inner::field_0x4c is a pointer to a vtable-backed
// object whose virtual slot 1 is called here.

struct FUN_0040f6b0_VBase {
    virtual int method0();
    virtual int method1();
};

struct FUN_0040f6b0_Inner {
    char _pad[0x4c];
    FUN_0040f6b0_VBase *obj;
};

struct FUN_0040f6b0_C {
    char _pad0[0x4];
    FUN_0040f6b0_Inner *inner;
    int field8;
    int fieldC;

    int *FUN_0040f6b0();
};

int *FUN_0040f6b0_C::FUN_0040f6b0()
{
    fieldC = inner->obj->method1();
    return &field8;
}
