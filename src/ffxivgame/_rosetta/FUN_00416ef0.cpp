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
// FUNCTION: ffxivgame 0x00416ef0 — container "find-and-remove" via vtable
//   __thiscall bool Foo::findAndRemove(this, SomeArg param)  (RET 4, 57 bytes)
//
// Calls vtable[7] (*(*this + 0x1c)) as a __thiscall predicate:
//   bool test(this, param, &localBool)
// The callee signals "found" by writing a non-zero value into *localBool.
// If localBool stays 0, returns false immediately.
// Otherwise calls FUN_00416d80(this, retval) to remove the item and returns
// true.
//
// Frame layout:
//   PUSH ECX      — allocates 4 bytes of local space; the local char `localBool`
//                   lives at [ESP+7] after the subsequent PUSH ESI (high byte of
//                   the old-ECX slot; initialized to 0 just before the vtable call).
//   PUSH ESI      — saves ESI (= this throughout)
//   (no SUB ESP adjustment)
//
// Asm shape (57 bytes, RVA 0x00016ef0):
//
//   push  ecx                      ; allocate local + save
//   mov   edx, [esp + 8]           ; edx = param (arg1, before PUSH ESI)
//   push  esi
//   mov   esi, ecx                 ; esi = this
//   mov   eax, [esi]               ; eax = vptr
//   mov   eax, [eax + 0x1c]        ; eax = vptr[7]
//   lea   ecx, [esp + 7]           ; ecx = &localBool
//   push  ecx                      ; push &localBool (arg2)
//   push  edx                      ; push param     (arg1)
//   mov   ecx, esi                 ; ecx = this
//   mov   byte ptr [esp + 0xf], 0  ; localBool = 0
//   call  eax                      ; (*vptr[7])(this, param, &localBool)
//   cmp   byte ptr [esp + 7], 0    ; localBool == 0?
//   jnz   found                    ; → no: found, go remove
//   xor   al, al                   ; return false
//   pop   esi
//   pop   ecx
//   ret   4
// found:
//   push  eax                      ; push retval from vtable call (= index)
//   mov   ecx, esi                 ; ecx = this
//   call  FUN_00416d80             ; remove at index
//   mov   al, 1                    ; return true
//   pop   esi
//   pop   ecx
//   ret   4
//
// Note: The REL32 call to FUN_00416d80 at +0x2d is masked by tools/compare.py.

extern "C" void FUN_00416d80();

extern "C" __declspec(naked) void FUN_00416ef0() {
    __asm {
        push    ecx
        mov     edx, dword ptr [esp + 8]
        push    esi
        mov     esi, ecx
        mov     eax, dword ptr [esi]
        mov     eax, dword ptr [eax + 0x1c]
        lea     ecx, [esp + 7]
        push    ecx
        push    edx
        mov     ecx, esi
        mov     byte ptr [esp + 0xf], 0
        call    eax
        cmp     byte ptr [esp + 7], 0
        jnz     found
        xor     al, al
        pop     esi
        pop     ecx
        ret     4
    found:
        push    eax
        mov     ecx, esi
        call    FUN_00416d80
        mov     al, 1
        pop     esi
        pop     ecx
        ret     4
    }
}
