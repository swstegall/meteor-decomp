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
// FUNCTION: ffxivgame 0x00403c40 — `std::basic_string<char>::_Eos(size_t)`
//                                  the canonical MSVC 2005 "set new length
//                                  + write NUL terminator" inline body
//                                  (31 B, __thiscall void(size_t)).
//
// Asm shape (read from the orig PE slice at RVA 0x00003c40 — 31 bytes,
// no relocations, no IAT touches, no SEH):
//
//   __thiscall void _Eos(MyString* this, size_t n) {  // ECX = this
//       // CMP first so the JB at the bottom can branch on the cached
//       // flags — the size store between CMP and JB is MSVC's typical
//       // out-of-order scheduling (MOVs don't touch the flag register).
//       this->_Mysize = n;                         // [this+0x14] = n
//       if (this->_Myres >= 0x10 /*_BUF_SIZE*/) {  // heap mode
//           char* p = this->_Bx._Ptr;              // [this+0x04]
//           p[n] = '\0';                           // terminate
//       } else {                                   // SSO inline buffer
//           char* p = (char*)&this->_Bx._Buf;      // (char*)this + 4
//           p[n] = '\0';                           // terminate
//       }
//   }
//
//   The string layout matches the one already documented in the sibling
//   FUN_00403e07 (`std::basic_string::assign` body chunk) — `_Bx` union
//   at offset 0x04, `_Mysize` at 0x14, `_Myres` at 0x18, with the SSO
//   threshold at `_Myres < 0x10`.
//
// Disassembly (verbatim):
//
//   00003c40:  83 79 18 10           cmp     dword ptr [ecx+0x18], 0x10
//   00003c44:  8b 44 24 04           mov     eax, [esp+0x4]            ; n
//   00003c48:  89 41 14              mov     [ecx+0x14], eax           ; _Mysize = n
//   00003c4b:  72 0a                 jb      sso_term                  ; cap < 0x10
//   00003c4d:  8b 49 04              mov     ecx, [ecx+0x4]            ; heap ptr
//   00003c50:  c6 04 01 00           mov     byte ptr [ecx+eax], 0x00  ; *(p+n) = 0
//   00003c54:  c2 04 00              ret     0x4
//   sso_term:
//   00003c57:  c6 44 01 04 00        mov     byte ptr [ecx+eax+0x4], 0 ; (this+4)[n] = 0
//   00003c5c:  c2 04 00              ret     0x4
//
// Reconstruction strategy — naked-asm body:
//
//   The function takes ECX (this) + a single 4-byte stack arg, has no
//   relocations, no IAT touches, no callees, and returns via `ret 4`.
//   A `__declspec(naked)` body with literal mnemonics reproduces the
//   31 bytes one-for-one — the readable form preferred over `_emit`
//   bytes when no reloc-bearing operand needs symbol resolution (cf.
//   the sibling FUN_00401460 / FUN_00401b70).
//
//   A source-level reconstruction would also have to coax MSVC into the
//   exact CMP→MOV→MOV→JB scheduling above, which depends on the
//   register-pressure picture of the surrounding template-instantiated
//   body — fragile across compiler builds. The naked form gives a
//   stable byte-identical match without that scheduling risk.

extern "C" __declspec(naked) void FUN_00403c40() {
    __asm {
        cmp     dword ptr [ecx+0x18], 0x10        ; 83 79 18 10
        mov     eax, dword ptr [esp+0x4]          ; 8b 44 24 04   (n)
        mov     dword ptr [ecx+0x14], eax         ; 89 41 14      (_Mysize = n)
        jb      sso_term                          ; 72 0a

        mov     ecx, dword ptr [ecx+0x4]          ; 8b 49 04      (heap ptr)
        mov     byte ptr [ecx+eax], 0             ; c6 04 01 00   (*(p+n) = 0)
        ret     4                                 ; c2 04 00

    sso_term:
        mov     byte ptr [ecx+eax+0x4], 0         ; c6 44 01 04 00 ((this+4)[n] = 0)
        ret     4                                 ; c2 04 00
    }
}
