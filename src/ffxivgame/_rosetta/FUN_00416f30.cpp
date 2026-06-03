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
// FUNCTION: ffxivgame 0x00416f30 — remove-if loop over a counted array
//   __thiscall int Foo::removeIf(this, SomeArg param)  (RET 4, 96 bytes)
//
// Iterates elements count-1 down to 0.  For each element i the function:
//   1. Computes an element offset:
//        stride = byte[this+0x14] + byte[this+0x15] + byte[this+0x16]
//        offset = stride * i + this->field_04 + byte[this+0x14]
//   2. Calls virtual method vptr[9] (= [*vptr + 0x24]) as
//        __thiscall bool test(this, param, offset)
//   3. If test returns true, calls FUN_00416d80(this, i) to remove the
//      element and increments the match counter.
// Returns the total count of removed elements.
//
// Asm shape (96 bytes, RVA 0x00016f30):
//
//   push esi
//   mov  esi, ecx                  ; this
//   mov  eax, [esi+0xc]            ; count
//   test eax, eax
//   jnz  has_count
//   pop  esi
//   ret  4                         ; early-out (count == 0)
//
//   has_count:
//   push ebx
//   push edi
//   lea  edi, [eax - 1]            ; i = count - 1
//   xor  ebx, ebx                  ; result = 0
//   test edi, edi
//   jl   loop_done                 ; skip if count-1 < 0 (signed)
//   push ebp
//   mov  ebp, [esp+0x14]           ; EBP = param (arg1)
//   mov  edi, edi                  ; 2-byte NOP for loop-head alignment
//
//   loop_top:
//   movzx ecx, byte [esi+0x14]
//   movzx edx, byte [esi+0x16]
//   movzx eax, byte [esi+0x15]
//   add  edx, ecx                  ; edx = b16+b14
//   add  eax, edx                  ; eax = stride
//   mov  edx, [esi]                ; edx = vptr
//   imul eax, edi                  ; eax = stride * i
//   add  eax, [esi+0x4]
//   add  eax, ecx                  ; += b14
//   push eax                       ; push offset
//   mov  eax, [edx+0x24]           ; eax = vptr[9]
//   push ebp                       ; push param
//   mov  ecx, esi
//   call eax                       ; bool test(this, param, offset)
//   test al, al
//   jz   skip_remove
//   push edi
//   mov  ecx, esi
//   call FUN_00416d80              ; removeAt(this, i)
//   add  ebx, 1
//
//   skip_remove:
//   sub  edi, 1
//   jns  loop_top
//   pop  ebp
//
//   loop_done:
//   pop  edi
//   mov  eax, ebx                  ; return match count
//   pop  ebx
//   pop  esi
//   ret  4

extern "C" void FUN_00416d80();

extern "C" __declspec(naked) void FUN_00416f30() {
    __asm {
        push esi
        mov  esi, ecx
        mov  eax, dword ptr [esi + 0xc]
        test eax, eax
        jnz  has_count
        pop  esi
        ret  4
    has_count:
        push ebx
        push edi
        lea  edi, [eax - 1]
        xor  ebx, ebx
        test edi, edi
        jl   loop_done
        push ebp
        mov  ebp, dword ptr [esp + 0x14]
        mov  edi, edi
    loop_top:
        movzx ecx, byte ptr [esi + 0x14]
        movzx edx, byte ptr [esi + 0x16]
        movzx eax, byte ptr [esi + 0x15]
        add  edx, ecx
        add  eax, edx
        mov  edx, dword ptr [esi]
        imul eax, edi
        add  eax, dword ptr [esi + 0x4]
        add  eax, ecx
        push eax
        mov  eax, dword ptr [edx + 0x24]
        push ebp
        mov  ecx, esi
        call eax
        test al, al
        jz   skip_remove
        push edi
        mov  ecx, esi
        call FUN_00416d80
        add  ebx, 1
    skip_remove:
        sub  edi, 1
        jns  loop_top
        pop  ebp
    loop_done:
        pop  edi
        mov  eax, ebx
        pop  ebx
        pop  esi
        ret  4
    }
}
