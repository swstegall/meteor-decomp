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
// FUNCTION: ffxivgame 0x004061a0 — __thiscall checked-iterator
//                                  `operator+(difference_type)` for a
//                                  small "container + cursor" iterator
//                                  pair (51 bytes).
//
// Layout (inferred from the asm):
//   Iterator (this, ECX):
//     +0x00  Container *_Mycont
//     +0x04  T         *_Myptr
//   Container (whatever _Mycont points to):
//     +0x04  T *_Myfirst  (begin)
//     +0x08  T *_Mylast   (end / one-past)
//
// Source shape (MSVC 2005 _SECURE_SCL=1 / ITERATOR_DEBUG_LEVEL=1 idiom):
//
//   Iter Iter::operator+(difference_type _Off) const {
//       Iter _Tmp;
//       Container *cont = this->_Mycont;
//       T *ptr = this->_Myptr;
//       if (cont == 0)
//           _invalid_parameter_noinfo();
//       ptr += _Off;
//       if (ptr > cont->_Mylast || ptr < cont->_Myfirst)
//           _invalid_parameter_noinfo();
//       _Tmp._Mycont = cont;
//       _Tmp._Myptr  = ptr;
//       return _Tmp;
//   }
//
// Calling convention: __thiscall (ECX = this; two stack args = hidden
// sret out-pointer + difference_type; callee cleans 8 bytes via `ret 8`).
//
// Frame:
//   PUSH ESI ; PUSH EDI                  ; callee-saves
//   [no ESP adjustment]
//
// Quirks MSVC's scheduler picked here:
//   - Loads `_Mycont` into ESI, runs `TEST ESI, ESI` BEFORE pushing EDI;
//     the PUSH EDI / MOV EDI, [ECX+4] pair is hoisted between TEST and
//     the conditional JNZ because PUSH/MOV don't touch EFLAGS.
//   - Branch shape on the bounds check is "fail by falling through":
//     `JA fail` then `JNC pass`, with `CALL` sitting at `fail:`.
//     Equivalent to `if (ptr > _Mylast || ptr < _Myfirst) fail();`.
//   - Out-of-line CALL to `_invalid_parameter_noinfo` (the no-info CRT
//     iterator-debug-failure hook, defined in crt/InvalidParameter.cpp
//     at RVA 0x5d22b4 / VA 0x9d22b4).
//
// Naked __asm so the TEST/PUSH/MOV interleave, the short-form JNZ/JA/JAE
// branches, and the POP EDI between the two `[EAX]` stores are pinned
// to the orig encoding. The two REL32 callsites
// (`_invalid_parameter_noinfo`) are masked out of the byte diff by
// tools/compare.py.

extern "C" void _invalid_parameter_noinfo(void);

extern "C" __declspec(naked) void FUN_004061a0() {
    __asm {
        push    esi
        mov     esi, dword ptr [ecx]
        test    esi, esi
        push    edi
        mov     edi, dword ptr [ecx + 4]
        jnz     skip_null
        call    _invalid_parameter_noinfo
    skip_null:
        mov     eax, dword ptr [esp + 0x10]
        add     edi, eax
        cmp     edi, dword ptr [esi + 8]
        ja      bounds_fail
        cmp     edi, dword ptr [esi + 4]
        jae     bounds_ok
    bounds_fail:
        call    _invalid_parameter_noinfo
    bounds_ok:
        mov     eax, dword ptr [esp + 0xc]
        mov     dword ptr [eax + 4], edi
        pop     edi
        mov     dword ptr [eax], esi
        pop     esi
        ret     8
    }
}
