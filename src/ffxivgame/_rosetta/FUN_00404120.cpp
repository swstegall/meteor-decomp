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
// FUNCTION: ffxivgame 0x00404120 — __thiscall
// basic_string::assign(const char *_Ptr, size_type _Count) — 193 bytes.
//
// Layout of the basic_string-like object (`this` aka esi):
//   +0x00 : (vtable or container header, unused here)
//   +0x04 : SSO inline buffer (16 bytes when capacity < 0x10) OR heap
//           pointer to the data block (when capacity >= 0x10)
//   +0x14 : `_Mysize` (current string length, not counting the null)
//   +0x18 : `_Myres`  (capacity)
//
// Shape (reconstructed from the asm; matches MSVC 2005's
//        basic_string<char>::assign(const_pointer, size_type)):
//
//   const char *data = (_Myres < 0x10) ? &this->buf : this->ptr;
//   if (_Ptr >= data && _Ptr < data + _Mysize) {
//       // _Ptr aliases our own buffer — re-route to the substring form.
//       return this->assign(*this, _Ptr - data, _Count);   // FUN_00404040
//   }
//   if (_Count == size_type(-1)) {                         // == npos
//       std::_Xlen();                                      // FUN_009d042e
//   }
//   if (_Myres < _Count) {
//       _Grow(_Count, _Mysize);                            // FUN_00403d60
//   }
//   if (_Count != 0) {
//       char *dst = (_Myres < 0x10) ? &this->buf : this->ptr;
//       _memcpy_s(dst, _Myres, _Ptr, _Count);              // _memcpy_s
//       this->_Mysize = _Count;
//       dst[_Count] = '\0';
//   } else {
//       char *dst = (_Myres < 0x10) ? &this->buf : this->ptr;
//       this->_Mysize = 0;
//       dst[0] = '\0';
//   }
//   return this;
//
// MSVC 2005 fuses the "skip copy when _Count == 0" check across the
// grow-needed and no-grow paths by chaining a `jnz` into the existing
// `jbe end_term` slot:
//
//   71: test edi, edi              ; after _Grow, edi = _Count
//   73: jbe end_term                ; share-point for both paths
//   ...                             ; copy path falls through
//   81: test edi, edi               ; no-grow entry
//   83: jnz 0x73                    ; reuse the jbe instruction as a
//                                   ;   "go to copy path" trampoline
//                                   ;   (jbe falls through when edi != 0
//                                   ;    because we just proved ZF == 0)
//
// Calling convention: __thiscall (ECX = this; args pushed in reverse on
// the C stack; callee cleans 8 bytes via `ret 8`). Returns `this` in EAX
// on the copy + zero paths but NOT on the substring-rerouted path —
// FUN_00404040 produces its own EAX (it's the substring overload that
// returns its `this`).
//
// Naked __asm so register allocation, SIB-form (`[ebx + edi]`), short
// vs near branch selection, and the jnz-into-jbe trampoline trick are
// pinned to the orig encoding. The three REL32 callsites
// (FUN_00404040, FUN_009d042e, FUN_00403d60) plus the _memcpy_s callsite
// are masked out of the byte diff by tools/compare.py.

extern "C" int FUN_00404040();   // basic_string::assign(const basic_string&, size_t, size_t)
extern "C" int FUN_009d042e();   // std::_Xlen — "string too long" length_error throw
extern "C" int FUN_00403d60();   // basic_string::_Grow(size_t, size_t) — reallocate
extern "C" int _memcpy_s();      // C runtime memcpy_s(dst, dst_size, src, count)

extern "C" __declspec(naked) void FUN_00404120() {
    __asm {
        push    ebx
        push    ebp
        push    esi
        mov     esi, ecx
        mov     ecx, dword ptr [esi + 0x18]
        cmp     ecx, 0x10
        lea     ebx, [esi + 4]
        jb      short_buf1
        mov     eax, dword ptr [ebx]
        jmp     after_buf1
    short_buf1:
        mov     eax, ebx
    after_buf1:
        mov     ebp, dword ptr [esp + 0x10]
        cmp     ebp, eax
        jb      outside
        cmp     ecx, 0x10
        jb      short_buf2
        mov     eax, dword ptr [ebx]
        jmp     after_buf2
    short_buf2:
        mov     eax, ebx
    after_buf2:
        mov     edx, dword ptr [esi + 0x14]
        add     edx, eax
        cmp     edx, ebp
        jbe     outside
        cmp     ecx, 0x10
        jb      short_buf3
        mov     ebx, dword ptr [ebx]
    short_buf3:
        mov     eax, dword ptr [esp + 0x14]
        push    eax
        sub     ebp, ebx
        push    ebp
        push    esi
        mov     ecx, esi
        call    FUN_00404040
        pop     esi
        pop     ebp
        pop     ebx
        ret     8
    outside:
        push    edi
        mov     edi, dword ptr [esp + 0x18]
        cmp     edi, -2
        jbe     no_xlen
        call    FUN_009d042e
    no_xlen:
        mov     eax, dword ptr [esi + 0x18]
        cmp     eax, edi
        jae     no_grow
        mov     ecx, dword ptr [esi + 0x14]
        push    ecx
        push    edi
        mov     ecx, esi
        call    FUN_00403d60
        test    edi, edi
    jbe_chain:
        jbe     end_term
        mov     ecx, dword ptr [esi + 0x18]
        cmp     ecx, 0x10
        jb      short_copy
        mov     eax, dword ptr [ebx]
        jmp     after_copy
    no_grow:
        test    edi, edi
        jnz     jbe_chain
        cmp     eax, 0x10
        mov     dword ptr [esi + 0x14], edi
        jb      zero_short
        mov     ebx, dword ptr [ebx]
    zero_short:
        pop     edi
        mov     eax, esi
        pop     esi
        pop     ebp
        mov     byte ptr [ebx], 0
        pop     ebx
        ret     8
    short_copy:
        mov     eax, ebx
    after_copy:
        push    edi
        push    ebp
        push    ecx
        push    eax
        call    _memcpy_s
        add     esp, 0x10
        cmp     dword ptr [esi + 0x18], 0x10
        mov     dword ptr [esi + 0x14], edi
        jb      term_short
        mov     ebx, dword ptr [ebx]
    term_short:
        mov     byte ptr [ebx + edi], 0
    end_term:
        pop     edi
        mov     eax, esi
        pop     esi
        pop     ebp
        pop     ebx
        ret     8
    }
}
