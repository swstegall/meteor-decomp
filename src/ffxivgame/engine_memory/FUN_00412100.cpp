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
// FUNCTION: ffxivgame 0x00012100 — __thiscall, 1 stack arg, 76 bytes
//           Multi-virtual-dispatch sequence with sibling call FUN_004120b0.
//
// ESI = this (cached throughout).
// EDI = iVar2 (return value of piVar1->vfunc_01(), used as scratch).
//
// Sequence:
//   1. this->vfunc_11()                      [vtable +0x2c, ECX=this]
//   2. piVar1 = param_1->vfunc_05()          [vtable +0x14, ECX=param_1]
//   3. iVar2  = piVar1->vfunc_01()           [vtable +0x04, ECX=piVar1]
//   4. this->field_4->vfunc_04(*(iVar2+0x28))[vtable +0x10, ECX=field_4, 1 stack arg]
//   5. *(iVar2+0x28) = 0
//   6. this->FUN_004120b0(iVar2)             [__thiscall, 1 stack arg = iVar2]
//   7. this->vfunc_12()                      [vtable +0x30, ECX=this]
//
// Asm (76 bytes @ orig RVA 0x00012100):
//   56                   PUSH ESI
//   8b f1                MOV ESI, ECX
//   8b 06                MOV EAX, [ESI]
//   8b 50 2c             MOV EDX, [EAX+0x2c]
//   57                   PUSH EDI
//   ff d2                CALL EDX
//   8b 4c 24 0c          MOV ECX, [ESP+0xc]
//   8b 01                MOV EAX, [ECX]
//   8b 50 14             MOV EDX, [EAX+0x14]
//   ff d2                CALL EDX
//   8b 10                MOV EDX, [EAX]
//   8b c8                MOV ECX, EAX
//   8b 42 04             MOV EAX, [EDX+0x4]
//   ff d0                CALL EAX
//   8b 4e 04             MOV ECX, [ESI+0x4]
//   8b 11                MOV EDX, [ECX]
//   8b 52 10             MOV EDX, [EDX+0x10]
//   8b f8                MOV EDI, EAX
//   8b 47 28             MOV EAX, [EDI+0x28]
//   50                   PUSH EAX
//   ff d2                CALL EDX
//   57                   PUSH EDI
//   8b ce                MOV ECX, ESI
//   c7 47 28 00 00 00 00 MOV dword ptr [EDI+0x28], 0
//   e8 72 ff ff ff       CALL FUN_004120b0
//   8b 06                MOV EAX, [ESI]
//   8b 50 30             MOV EDX, [EAX+0x30]
//   8b ce                MOV ECX, ESI
//   ff d2                CALL EDX
//   5f                   POP EDI
//   5e                   POP ESI
//   c2 04 00             RET 4

// IObj_A: vtable with at least 13 slots; slots 11 (0x2c) and 12 (0x30) used
struct IObj_A_12100 {
    virtual void vfunc_00() = 0;
    virtual void vfunc_01() = 0;
    virtual void vfunc_02() = 0;
    virtual void vfunc_03() = 0;
    virtual void vfunc_04() = 0;
    virtual void vfunc_05() = 0;
    virtual void vfunc_06() = 0;
    virtual void vfunc_07() = 0;
    virtual void vfunc_08() = 0;
    virtual void vfunc_09() = 0;
    virtual void vfunc_0a() = 0;
    virtual void vfunc_0b() = 0;   // vtable +0x2c
    virtual void vfunc_0c() = 0;   // vtable +0x30
};

// IObj_B: vtable with at least 6 slots; slot 5 (0x14) returns IObj_C*
struct IObj_C_12100;
struct IObj_B_12100 {
    virtual void       vfunc_00() = 0;
    virtual void       vfunc_01() = 0;
    virtual void       vfunc_02() = 0;
    virtual void       vfunc_03() = 0;
    virtual void       vfunc_04() = 0;
    virtual IObj_C_12100 *vfunc_05() = 0; // vtable +0x14
};

// IObj_C: vtable with at least 2 slots; slot 1 (0x04) returns int*
struct IObj_D_12100;
struct IObj_C_12100 {
    virtual void         vfunc_00() = 0;
    virtual IObj_D_12100 *vfunc_01() = 0; // vtable +0x04
};

// IObj_D: plain int block; field at offset 0x28
struct IObj_D_12100 {
    char _pad[0x28];
    int  field_0x28;
};

// IObj_E: vtable with at least 5 slots; slot 4 (0x10) takes 1 int arg
struct IObj_E_12100 {
    virtual void vfunc_00() = 0;
    virtual void vfunc_01() = 0;
    virtual void vfunc_02() = 0;
    virtual void vfunc_03() = 0;
    virtual void vfunc_04(int val) = 0; // vtable +0x10
};

// `this` type: IObj_A at 0x00, IObj_E* at 0x04
struct FUN_00412100_C : public IObj_A_12100 {
    IObj_E_12100 *field_4; // at 0x04

    void FUN_00412100(IObj_B_12100 *param_1);
    void FUN_004120b0(IObj_D_12100 *p);
};

void FUN_00412100_C::FUN_00412100(IObj_B_12100 *param_1)
{
    vfunc_0b();
    IObj_C_12100 *piVar1 = param_1->vfunc_05();
    IObj_D_12100 *iVar2 = piVar1->vfunc_01();
    field_4->vfunc_04(iVar2->field_0x28);
    iVar2->field_0x28 = 0;
    FUN_004120b0(iVar2);
    vfunc_0c();
}
