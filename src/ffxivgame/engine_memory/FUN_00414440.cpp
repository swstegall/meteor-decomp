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
// FUNCTION: ffxivgame 0x00014440 — member-adjusted vtable tail-jmp (slot 0xc)
//                                  (14 bytes)
//
// Asm (14 bytes @ orig RVA 0x00014440):
//   8b 49 04    MOV ECX, dword ptr [ECX + 0x4]   ; ecx = this->field_0x4 (sub-object ptr)
//   8b 41 04    MOV EAX, dword ptr [ECX + 0x4]   ; eax = sub_obj->field_0x4 (vtable)
//   8b 50 0c    MOV EDX, dword ptr [EAX + 0xc]   ; edx = vtable[3] (slot 0xc)
//   83 c1 04    ADD ECX, 0x4                     ; this-adjustor: bump ecx past field_0x4
//   ff e2      JMP EDX                           ; tail-call into slot 3
//
// Adjustor-thunk shape: load the embedded sub-object pointer at
// this+0x4, fetch *its* vtable (also at offset +0x4), tail-call the
// vtable slot at offset 0xc (the 4th function pointer) after bumping
// ECX past the sub-object's "outer header" field. Sibling templates
// FUN_0040f5c0 (slot 0x4) and FUN_0040f5d0 (slot 0x8) share the byte
// shape modulo the 1-byte slot index — this is the slot-0xc member of
// the same cluster.

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
// The real implementation is the MSVC __declspec(naked) + __asm block below,
// which clang cannot parse. Production builds always use cl.exe (MSVC 2005).
extern "C" void FUN_00414440() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_00414440() {
    __asm {
        mov ecx, [ecx + 4]
        mov eax, [ecx + 4]
        mov edx, [eax + 0xc]
        add ecx, 4
        jmp edx
    }
}
#endif
