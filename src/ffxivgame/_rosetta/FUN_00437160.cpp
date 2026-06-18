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
// FUNCTION: ffxivgame 0x00437160 — allocate-and-dispatch member fn (__thiscall, 86 B).
//
// Two-argument sibling of FUN_004374a0: reads the same global manager pointer
// at 0x01328d90, indexes into its element table via the same
// `(index*8 - index)*4` = 28-byte stride, then asks that element to allocate
// a 12-byte object via FUN_00417ab0(this=element, size=12).
//
// On success the new object is given vtable 0x00f64940 and both stack
// arguments are stashed in its second and third slots, then the object is
// handed to FUN_0043c2d0(this=this->field_0x8, obj). On allocation failure
// the same dispatch is invoked with a null pointer.
//
// Pseudo-source (logical structure):
//
//   void __thiscall FUN_00437160(int arg1, int arg2) {
//       Tbl* t = *(Tbl**)0x01328d90;
//       Elem* e = (Elem*)(t->base + (t->index * 7) * 4);
//       Obj* o = e->Allocate(12);
//       if (o) {
//           o->vtbl = 0x00f64940;
//           o->arg1 = arg1;
//           o->arg2 = arg2;
//           this->field_0x8->Dispatch(o);
//       } else {
//           this->field_0x8->Dispatch(0);
//       }
//   }
//
// Naked asm: two DIR32 (global ptr + vtable literal) plus two REL32 calls
// and an `lea edx,[eax*8]` scaled-index-with-zero-disp — MSVC 2005's exact
// /O2 schedule. compare.py reloc-masks the 4-byte windows.
//
// Reloc-bearing sites:
//   +0x05  MOV ECX,[data_01328d90]    (dir32, manager global)
//   +0x1e  CALL FUN_00417ab0          (rel32)
//   +0x30  MOV [EAX], offset 0xf64940 (dir32, vtable)
//   +0x3f  CALL FUN_0043c2d0          (rel32)
//   +0x4e  CALL FUN_0043c2d0          (rel32)

extern "C" {
    // .text — RVA 0x00417ab0. Element allocator, __thiscall(size) -> void*.
    int FUN_00417ab0();
    // .text — RVA 0x0043c2d0. Dispatch, __thiscall(obj).
    int FUN_0043c2d0();
    // .data — manager table pointer.
    extern int data_01328d90;
    // .rdata — vtable literal stored into the new object.
    extern int data_00f64940;
}

extern "C" __declspec(naked) void FUN_00437160() {
    __asm {
        push    esi
        mov     esi, ecx
        mov     ecx, dword ptr [data_01328d90]
        movzx   eax, byte ptr [ecx]
        lea     edx, [eax*8]
        sub     edx, eax
        mov     eax, dword ptr [ecx + 4]
        lea     ecx, [eax + edx*4]
        push    0xc
        call    FUN_00417ab0
        test    eax, eax
        jz      zero_path
        mov     ecx, dword ptr [esp + 8]
        mov     edx, dword ptr [esp + 0xc]
        mov     dword ptr [eax], offset data_00f64940
        mov     dword ptr [eax + 4], ecx
        mov     dword ptr [eax + 8], edx
        mov     ecx, dword ptr [esi + 8]
        push    eax
        call    FUN_0043c2d0
        pop     esi
        ret     8
    zero_path:
        mov     ecx, dword ptr [esi + 8]
        xor     eax, eax
        push    eax
        call    FUN_0043c2d0
        pop     esi
        ret     8
    }
}
