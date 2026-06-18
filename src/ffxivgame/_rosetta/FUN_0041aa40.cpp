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
// FUNCTION: ffxivgame 0x0041aa40 — __thiscall conditional release (33 B)
//
// If this->field_0x18 is non-null, calls vtable[8] (offset 0x20) on the
// global singleton at [0x01329920] passing field_0x18 as the argument,
// then zeroes field_0x18.  Structurally identical to FUN_00420760 (same
// 33-byte shape) but with field offset 0x18 and vtable slot 8 instead of
// 0x10 / slot 3.
//
// Calling convention: __thiscall (ECX = this, void return, plain RET).
// Frame: -4 (PUSH ESI / POP ESI only — saves this to callee-save ESI
// so ECX is free for the virtual dispatch).
//
// Asm (33 bytes @ RVA 0x0001aa40):
//   56                       PUSH ESI
//   8b f1                    MOV ESI, ECX              ; ESI = this
//   8b 46 18                 MOV EAX, [ESI+0x18]       ; load field_0x18
//   85 c0                    TEST EAX, EAX
//   74 15                    JZ .done                   ; null → skip
//   8b 0d <g_singleton>      MOV ECX, [g_singleton]    ; load object ptr
//   8b 11                    MOV EDX, [ECX]             ; load vtable
//   50                       PUSH EAX                   ; arg = field_0x18
//   8b 42 20                 MOV EAX, [EDX+0x20]       ; vtable[8]
//   ff d0                    CALL EAX
//   c7 46 18 00 00 00 00     MOV [ESI+0x18], 0          ; zero field
// .done:
//   5e                       POP ESI
//   c3                       RET

// Global singleton pointer stored at [0x01329920].
// Virtual method at vtable slot 8 (offset 0x20) takes one int argument.
struct UNK_0001aa40_mgr {
    virtual void vfunc_0();
    virtual void vfunc_1();
    virtual void vfunc_2();
    virtual void vfunc_3();
    virtual void vfunc_4();
    virtual void vfunc_5();
    virtual void vfunc_6();
    virtual void vfunc_7();
    virtual void vfunc_8(int handle);
};

extern "C" UNK_0001aa40_mgr* g_FUN_0041aa40_singleton; // dword at 0x01329920

// Owner class — field_0x18 is a handle/id at offset 0x18.
struct FUN_0041aa40_owner {
    int m_pad0;   // 0x00
    int m_pad1;   // 0x04
    int m_pad2;   // 0x08
    int m_pad3;   // 0x0C
    int m_pad4;   // 0x10
    int m_pad5;   // 0x14
    int m_handle; // 0x18

    void FUN_0041aa40();
};

void FUN_0041aa40_owner::FUN_0041aa40() {
    if (m_handle) {
        g_FUN_0041aa40_singleton->vfunc_8(m_handle);
        m_handle = 0;
    }
}
