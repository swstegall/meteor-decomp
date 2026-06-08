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
// FUNCTION: ffxivgame 0x000340c0 — allocate-and-dispatch member fn (__thiscall, 79 B).
//
// Sibling of FUN_004374a0 (identical structure, same pool/vtable/dispatch chain)
// but dispatches through this->field_0x0c rather than field_0x08.
//
// Reads a global manager pointer at 0x01328d90, indexes into its element table
// (tbl->base[ tbl->index * 7 ] — a 28-byte stride formed by (index*8 - index)*4),
// and asks that element to allocate an 8-byte object via FUN_00417ab0(this=element, 8).
//
// On success the new object is given vtable 0x00f64988 and stashes the function's
// single stack argument in its second slot, then is handed to
// FUN_0043c2d0(this=this->field_0x0c, obj). On allocation failure the same dispatch
// is invoked with a null pointer.
//
// Pseudo-source (logical structure):
//
//   void __thiscall FUN_004340c0(int arg) {
//       Tbl* t = *(Tbl**)0x01328d90;
//       Elem* e = (Elem*)(t->base + (t->index * 7) * 4);
//       Obj* o = e->Allocate(8);
//       if (o) {
//           o->vtbl = 0x00f64988;
//           o->arg  = arg;
//           this->field_0x0c->Dispatch(o);
//       } else {
//           this->field_0x0c->Dispatch(0);
//       }
//   }
//
// Orig codegen (79 bytes — read from asm/ffxivgame/000340c0_FUN_004340c0.s):
//
//   56                   push esi
//   8b f1                mov  esi, ecx                  ; esi = this
//   8b 0d 90 8d 32 01    mov  ecx, [0x01328d90]         ; ecx = g_tbl (DIR32)
//   0f b6 01             movzx eax, byte ptr [ecx]      ; eax = g_tbl->index
//   8d 14 c5 00 00 00 00 lea  edx, [eax*8 + 0]
//   2b d0                sub  edx, eax                  ; edx = idx*7
//   8b 41 04             mov  eax, [ecx+4]              ; eax = g_tbl->base
//   8d 0c 90             lea  ecx, [eax + edx*4]        ; ecx = base + idx*28
//   6a 08                push 8
//   e8 ce 39 fe ff       call FUN_00417ab0              ; rel32 → 0x00417ab0
//   85 c0                test eax, eax
//   74 1a                jz   else_branch
//   8b 4c 24 08          mov  ecx, [esp+8]              ; ecx = arg
//   c7 00 88 49 f6 00    mov  dword ptr [eax], 0xf64988 ; vtable stamp (DIR32)
//   89 48 04             mov  [eax+4], ecx              ; o->arg = arg
//   8b 4e 0c             mov  ecx, [esi+0xc]            ; ecx = this->field_0x0c
//   50                   push eax
//   e8 d4 81 00 00       call FUN_0043c2d0              ; rel32 → 0x0043c2d0
//   5e                   pop  esi
//   c2 04 00             ret  4
// else_branch:
//   8b 4e 0c             mov  ecx, [esi+0xc]            ; ecx = this->field_0x0c
//   33 c0                xor  eax, eax
//   50                   push eax
//   e8 c5 81 00 00       call FUN_0043c2d0              ; rel32 → 0x0043c2d0
//   5e                   pop  esi
//   c2 04 00             ret  4
//
// Reloc-bearing sites:
//   +0x03  MOV ECX,[data_01328d90]    (dir32, manager global)
//   +0x1d  CALL FUN_00417ab0          (rel32)
//   +0x2a  MOV [EAX], offset data_00f64988 (dir32, vtable)
//   +0x37  CALL FUN_0043c2d0          (rel32)
//   +0x46  CALL FUN_0043c2d0          (rel32)

extern "C" {
    // .text — RVA 0x00417ab0. Element allocator, __thiscall(size) -> void*.
    int FUN_00417ab0();
    // .text — RVA 0x0043c2d0. Dispatch, __thiscall(obj).
    int FUN_0043c2d0();
    // .data — manager table pointer.
    extern int data_01328d90;
    // .rdata — vtable literal stored into the new object.
    extern int data_00f64988;
}

extern "C" __declspec(naked) void FUN_004340c0() {
    __asm {
        push    esi
        mov     esi, ecx
        mov     ecx, dword ptr [data_01328d90]
        movzx   eax, byte ptr [ecx]
        lea     edx, [eax*8]
        sub     edx, eax
        mov     eax, dword ptr [ecx + 4]
        lea     ecx, [eax + edx*4]
        push    8
        call    FUN_00417ab0
        test    eax, eax
        jz      zero_path
        mov     ecx, dword ptr [esp + 8]
        mov     dword ptr [eax], offset data_00f64988
        mov     dword ptr [eax + 4], ecx
        mov     ecx, dword ptr [esi + 0x0c]
        push    eax
        call    FUN_0043c2d0
        pop     esi
        ret     4
    zero_path:
        mov     ecx, dword ptr [esi + 0x0c]
        xor     eax, eax
        push    eax
        call    FUN_0043c2d0
        pop     esi
        ret     4
    }
}
