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
// FUNCTION: ffxivgame 0x00451430 — __thiscall basic_string data-pointer
//                                  forwarder (52 bytes).
//
// Layout (MSVC 2005 std::basic_string SSO idiom, this = ECX):
//     +0x04  union { char _Buf[16]; char *_Ptr; } _Bx
//     +0x14  size_type _Mysize
//     +0x18  size_type _Myres     (capacity)
//
//   When _Myres >= 16 the string is heap-allocated and the data lives at
//   *(char**)(this + 4); otherwise the data is the inline buffer at
//   (char*)(this + 4).
//
// Source shape (inferred):
//
//   T *Foo::method(T *result) {
//       const char *data = (this->_Myres < 16)
//                              ? this->_Bx._Buf       // LEA [ecx+4]
//                              : this->_Bx._Ptr;      // MOV [ecx+4]
//       result->ctor(data, this);                     // FUN_004512e0
//       return result;
//   }
//
// The `< 16` test is emitted as `cmp [ecx+0x18], 0x10 ; jc small_buf`,
// so the heap-pointer arm (MOV EAX, [ECX+4]) falls through and the
// inline-buffer arm (LEA EAX, [ECX+4]) is the taken branch.
//
// Both arms then call the __thiscall helper FUN_004512e0 on the sret
// out-pointer (ESI, loaded from the single stack arg) with two pushed
// args (this, data) and return ESI.
//
// Calling convention: __thiscall (ECX = this; one DWORD stack arg = sret
// out-pointer; callee cleans 4 via `ret 4`).
//
// Naked __asm so the SSO branch split, the short-form `jc`, the imm8
// `cmp`, and the duplicated tail in each arm pin to the orig encoding.
// The two REL32 callsites (FUN_004512e0) are masked by tools/compare.py.

extern "C" void FUN_004512e0();                 // __thiscall helper

extern "C" __declspec(naked) void FUN_00451430() {
    __asm {
        cmp     dword ptr [ecx + 0x18], 0x10
        jc      small_buf
        mov     eax, dword ptr [ecx + 4]
        push    esi
        mov     esi, dword ptr [esp + 8]
        push    ecx
        push    eax
        mov     ecx, esi
        call    FUN_004512e0
        mov     eax, esi
        pop     esi
        ret     4
    small_buf:
        push    esi
        mov     esi, dword ptr [esp + 8]
        lea     eax, [ecx + 4]
        push    ecx
        push    eax
        mov     ecx, esi
        call    FUN_004512e0
        mov     eax, esi
        pop     esi
        ret     4
    }
}
