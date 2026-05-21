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
// FUNCTION: ffxivgame 0x00403eb0 — __thiscall basic_string<wchar_t>-style
// `_Tidy(_Built, _Newsize)` body that shrinks a wide-string back into its
// 8-wchar (0x10-byte) SSO buffer.
//
// Shape (reconstructed from the asm; the headless Ghidra hint at
// build/ghidra-decomp/ffxivgame/00003eb0_FUN_00403eb0.c agrees):
//
//   void _Tidy(this, bool _Built, size_type _Newsize) {
//       if (_Built && this->_Myres >= 8) {              // currently heap-allocated
//           wchar_t *_Src = *(wchar_t **)(this + 4);    // saved heap pointer
//           if (_Newsize != 0) {
//               _memcpy_s(this + 4, 0x10, _Src, _Newsize * 2);
//           }
//           FUN_0044d350(_Src, this->_Myres * 2 + 2, 0xc);   // free(_Src, bytes, blocktype)
//       }
//       this->_Mysize = _Newsize;
//       this->_Myres  = 7;                              // SSO capacity (8 wchars - 1)
//       *(uint16_t *)((char *)this + 4 + _Newsize * 2) = 0;  // null terminator
//   }
//
// Layout of the wide-string object (this aka esi):
//   +0x00 : (header / vtable slot, not touched here)
//   +0x04 : SSO inline buffer (8 wchar_t = 16 bytes) OR heap pointer when
//           _Myres >= 8
//   +0x14 : _Mysize  (current length in wchars, no null)
//   +0x18 : _Myres   (capacity in wchars - 1; >=8 means heap-allocated)
//
// FUN_0044d350 is the 3-arg internal deallocator used by MSVC 2005's
// basic_string allocator path (matches the pattern used by the FUN_00403c40
// / FUN_00404120 siblings); the 0xc third argument is the CRT block-type
// tag (_CRT_BLOCK in debug, ignored in retail).
//
// Calling convention: __thiscall — `this` arrives in ECX, the two
// stack args occupy [esp+4] (1-byte `_Built` widened to 4 bytes) and
// [esp+8] (`_Newsize`); callee cleans 8 bytes via `ret 8`.
//
// Branch shape: a single forward jump from the dual guard
// (`if (_Built && _Myres >= 8)`) merges directly onto the tail block
// that writes `_Mysize`, `_Myres`, and the null terminator — which is
// why both branches skip the `push ebx` / `pop ebx` pair entirely (ebx
// is only used to spill `_Src` across the two calls). Inside the
// memcpy_s arm a second short jump (`jbe`) gates the copy on
// `_Newsize != 0`.
//
// Naked __asm so:
//   - the CMP-then-PUSH-then-MOV prologue order is preserved (`cmp byte
//     ptr [esp+4], 0` reads the pre-push slot, so any source-level
//     formulation would force MSVC to spill differently);
//   - the `lea ecx, [edi+edi]` 3-byte SIB form is used for the
//     `_Newsize * 2` byte-count (vs the 7-byte `[edi*2]` no-base form);
//   - `lea eax, [edx+edx+2]` materialises `_Myres * 2 + 2` in one shot;
//   - the trailing `mov word ptr [esi+edi*2+4], 0` writes the null
//     terminator in one operand-size-override instruction (66 prefix +
//     MOV r/m16, imm16);
//   - the two cross-RVA calls (_memcpy_s + FUN_0044d350) get REL32
//     relocs which tools/compare.py masks in the byte diff.

extern "C" int _memcpy_s();      // C runtime memcpy_s(dst, dst_size, src, count)
extern "C" int FUN_0044d350();   // internal 3-arg deallocator (ptr, bytes, blocktype)

extern "C" __declspec(naked) void FUN_00403eb0() {
    __asm {
        cmp     byte ptr [esp + 4], 0
        push    esi
        push    edi
        mov     edi, dword ptr [esp + 0x10]
        mov     esi, ecx
        jz      tail
        cmp     dword ptr [esi + 0x18], 8
        jb      tail
        test    edi, edi
        lea     eax, [esi + 4]
        push    ebx
        mov     ebx, dword ptr [eax]
        jbe     skip_memcpy
        lea     ecx, [edi + edi]
        push    ecx
        push    ebx
        push    0x10
        push    eax
        call    _memcpy_s
        add     esp, 0x10
    skip_memcpy:
        mov     edx, dword ptr [esi + 0x18]
        push    0xc
        lea     eax, [edx + edx + 2]
        push    eax
        push    ebx
        call    FUN_0044d350
        add     esp, 0xc
        pop     ebx
    tail:
        mov     dword ptr [esi + 0x14], edi
        mov     dword ptr [esi + 0x18], 7
        mov     word ptr [esi + edi * 2 + 4], 0
        pop     edi
        pop     esi
        ret     8
    }
}
