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
// FUNCTION: ffxivgame 0x00023240 — guarded virtual dispatch (36 B / 0x24)
//
// Calling convention: __thiscall (ECX = this); one stack arg `param_1` at
//   [ESP+4]. Returns void. Callee cleans 4 bytes (`ret 4`).
//
// Object layout (offsets touched, all in `this`):
//   [this + 0x00]  IBase_00423240*   field_0x0  — vtable-bearing object;
//                                                  slot 0x4c (index 19) called
//   [this + 0x04]  Cache_00423240*   field_0x4  — cache; FUN_00423830 at fixed addr
//
// Behaviour:
//   1. Call Cache::FUN_00423830(param_1) with this = field_0x4.
//   2. If it returns false (cache out of sync), dispatch through
//      field_0x0's vtable slot 0x4c (index 19) with param_1.
//
// Register allocation (MSVC 2005 /O2 /Oy):
//   ESI = this      (loaded from ECX)
//   EDI = param_1   (loaded from [ESP+0xc] after the two callee-saves)
//   ECX = scratch   (thiscall `this` for sub-calls)

struct Cache_00423240 {
    bool FUN_00423830(void* src);
};

struct IBase_00423240 {
    virtual void vfunc0(void*);
    virtual void vfunc1(void*);
    virtual void vfunc2(void*);
    virtual void vfunc3(void*);
    virtual void vfunc4(void*);
    virtual void vfunc5(void*);
    virtual void vfunc6(void*);
    virtual void vfunc7(void*);
    virtual void vfunc8(void*);
    virtual void vfunc9(void*);
    virtual void vfunc10(void*);
    virtual void vfunc11(void*);
    virtual void vfunc12(void*);
    virtual void vfunc13(void*);
    virtual void vfunc14(void*);
    virtual void vfunc15(void*);
    virtual void vfunc16(void*);
    virtual void vfunc17(void*);
    virtual void vfunc18(void*);
    virtual void vfunc_4c(void*); // vtable slot 19 = byte offset 0x4c
};

struct FUN_00423240_obj {
    IBase_00423240*  field_0x0;  // offset 0x00
    Cache_00423240*  field_0x4;  // offset 0x04

    void FUN_00423240(void* param_1);
};

void FUN_00423240_obj::FUN_00423240(void* param_1)
{
    if (!this->field_0x4->FUN_00423830(param_1)) {
        this->field_0x0->vfunc_4c(param_1);
    }
}
