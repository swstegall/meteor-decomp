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
// FUNCTION: ffxivgame 0x00420760 — FUN_00420760 (__thiscall, 33 B)
//
// Conditional release: if this->field_0x10 is non-null, calls vtable[3]
// on the global singleton at [0x01329920] passing field_0x10 as the
// argument, then zeroes field_0x10.
//
// Calling convention: __thiscall (ECX = this, void return, plain RET).
// Frame: -4 (PUSH ESI / POP ESI only — saves this to callee-save ESI
// so ECX is free for the virtual dispatch).
//
// Asm (33 bytes @ RVA 0x00020760):
//   56                       PUSH ESI
//   8b f1                    MOV ESI, ECX              ; ESI = this
//   8b 46 10                 MOV EAX, [ESI+0x10]       ; load field_0x10
//   85 c0                    TEST EAX, EAX
//   74 15                    JZ .done                   ; null → skip
//   8b 0d <g_singleton>      MOV ECX, [g_singleton]    ; load object ptr
//   8b 11                    MOV EDX, [ECX]             ; load vtable
//   50                       PUSH EAX                   ; arg = field_0x10
//   8b 42 0c                 MOV EAX, [EDX+0xc]        ; vtable[3]
//   ff d0                    CALL EAX
//   c7 46 10 00 00 00 00     MOV [ESI+0x10], 0          ; zero field
// .done:
//   5e                       POP ESI
//   c3                       RET

// Global singleton pointer stored at [0x01329920].
// Virtual method at vtable slot 3 (offset 0x0c) takes one int argument.
struct UNK_01329920 {
    virtual void vfunc_0();
    virtual void vfunc_1();
    virtual void vfunc_2();
    virtual void vfunc_3(int handle);
};

extern "C" UNK_01329920* g_FUN_00420760_singleton; // dword at 0x01329920

// Owner class — field_0x10 is a handle/id at offset 0x10.
struct FUN_00420760_state {
    int m_pad0;   // 0x00
    int m_pad1;   // 0x04
    int m_pad2;   // 0x08
    int m_pad3;   // 0x0C
    int m_handle; // 0x10

    void FUN_00420760();
};

void FUN_00420760_state::FUN_00420760() {
    if (m_handle) {
        g_FUN_00420760_singleton->vfunc_3(m_handle);
        m_handle = 0;
    }
}
