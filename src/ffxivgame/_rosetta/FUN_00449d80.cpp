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
// FUNCTION: ffxivgame 0x00449d80 — __thiscall basic_string<DWORD>-style
// `_Tidy(_Built, _Newsize)` body that shrinks a DWORD-array string back
// into its 4-element (0x10-byte) SSO buffer.
//
// Shape (reconstructed from asm/ffxivgame/00049d80_FUN_00449d80.s):
//
//   void _Tidy(this, bool _Built, size_type _Newsize) {
//       if (_Built && this->_Myres >= 4) {             // currently heap-allocated
//           DWORD *_Src = *(DWORD **)(this + 4);       // saved heap pointer
//           if (_Newsize != 0) {
//               FUN_00449a40(this + 4, 4, _Src, _Newsize); // bounds-checked DWORD copy
//           }
//           FUN_0044d350(_Src, this->_Myres * 4 + 4, 0xc); // free(_Src, bytes, blocktype)
//       }
//       this->_Mysize = _Newsize;
//       this->_Myres  = 3;                             // SSO capacity (4 DWORDs - 1)
//       *(DWORD *)((char *)this + 4 + _Newsize * 4) = 0; // null terminator
//   }
//
// Layout of the DWORD-string object (this aka esi):
//   +0x00 : (header / vtable slot, not touched here)
//   +0x04 : SSO inline buffer (4 DWORD = 16 bytes) OR heap pointer when
//           _Myres >= 4
//   +0x14 : _Mysize  (current length in DWORDs, no null)
//   +0x18 : _Myres   (capacity in DWORDs - 1; >=4 means heap-allocated)
//
// This is structurally identical to FUN_00403eb0 (_Tidy for wchar_t) but
// uses 4-byte DWORD elements instead of 2-byte wchar_t:
//   - heap threshold: _Myres >= 4 (vs >= 8 for wchar_t)
//   - SSO reset value: _Myres = 3 (vs 7 for wchar_t)
//   - copy call: FUN_00449a40 (bounds-checked DWORD copy, 54 bytes at
//     VA 0x00449a40) with dst_size=4; not _memcpy_s
//   - dealloc size: LEA ECX,[EAX*4+4] = (_Myres+1)*4 bytes (vs EDX*2+2)
//   - null terminator: dword ptr [esi+edi*4+4] (vs word ptr for wchar_t)
//
// FUN_0044d350 is the 3-arg internal deallocator used by MSVC 2005's
// basic_string allocator path; the 0xc third argument is the CRT block-type
// tag (_CRT_BLOCK in debug, ignored in retail).
//
// Calling convention: __thiscall — `this` arrives in ECX, the two
// stack args occupy [esp+4] (1-byte `_Built` widened to 4 bytes) and
// [esp+8] (`_Newsize`); callee cleans 8 bytes via `ret 8`.
//
// Branch shape: a single forward jump from the dual guard
// (`if (_Built && _Myres >= 4)`) merges directly onto the tail block
// that writes `_Mysize`, `_Myres`, and the null terminator — which is
// why both branches share the `pop ebx` / `pop edi` / `pop esi` epilogue.
// Inside the copy arm a second short jump (`jbe`) gates the copy on
// `_Newsize != 0`.
//
// Naked __asm so:
//   - the CMP-then-PUSH-then-MOV prologue order is preserved (`cmp byte
//     ptr [esp+4], 0` reads the pre-push slot, so any source-level
//     formulation would force MSVC to spill differently);
//   - `lea ecx, [eax*4 + 4]` emits the 7-byte no-base SIB form
//     (8d 0c 85 04 00 00 00) — MSVC can't trivially derive this from
//     a C expression without additional casts;
//   - `mov dword ptr [esi + edi*4 + 4], 0` writes the null terminator
//     as a dword (not word);
//   - the two cross-RVA calls (FUN_00449a40 + FUN_0044d350) get REL32
//     relocs which tools/compare.py masks in the byte diff.

extern "C" void FUN_00449a40();   // bounds-checked DWORD block copy (dst, cap, src, cnt)
extern "C" int  FUN_0044d350();   // internal 3-arg deallocator (ptr, bytes, blocktype)

extern "C" __declspec(naked) void FUN_00449d80() {
    __asm {
        cmp     byte ptr [esp + 4], 0
        push    esi
        push    edi
        mov     edi, dword ptr [esp + 0x10]
        mov     esi, ecx
        jz      tail
        cmp     dword ptr [esi + 0x18], 4
        jb      tail
        test    edi, edi
        lea     eax, [esi + 4]
        push    ebx
        mov     ebx, dword ptr [eax]
        jbe     skip_copy
        push    edi
        push    ebx
        push    0x4
        push    eax
        call    FUN_00449a40
        add     esp, 0x10
    skip_copy:
        mov     eax, dword ptr [esi + 0x18]
        push    0xc
        lea     ecx, [eax * 4 + 4]
        push    ecx
        push    ebx
        call    FUN_0044d350
        add     esp, 0xc
        pop     ebx
    tail:
        mov     dword ptr [esi + 0x14], edi
        mov     dword ptr [esi + 0x18], 3
        mov     dword ptr [esi + edi * 4 + 4], 0
        pop     edi
        pop     esi
        ret     8
    }
}
