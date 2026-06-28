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
// FUNCTION: ffxivgame 0x0043f480 — __thiscall string _Tidy() /
//                                   clear-and-reset helper (45 B / 0x2D).
//
// Object layout (inferred from offsets accessed):
//   [this + 0x04]  char    _Buf[16]  / char * _Ptr  (SSO union)
//   [this + 0x14]  DWORD   _Mysize   — current string length
//   [this + 0x18]  DWORD   _Myres    — current capacity
//
// Source shape (inferred from asm):
//
//   void Str::_Tidy() {
//       if (this->_Myres >= 0x10) {      // heap-allocated?
//           char *ptr = (char*)this->_Ptr;
//           if (ptr) {
//               // ECX = *(ptr - 4)  (allocator header / owner word)
//               // arg  = ptr
//               FUN_0040df70(ptr);        // string free (__thiscall)
//           }
//       }
//       this->_Myres = 0xf;              // reset capacity to SSO limit
//       this->_Mysize = 0;               // reset size to 0
//       this->_Buf[0] = '\0';            // null-terminate SSO buffer
//   }
//
// Calling convention: __thiscall (ECX = this), no stack args, void return.
// Epilogue: POP ESI / RET.
//
// Reloc-bearing sites (masked by tools/compare.py):
//   +0x14  CALL  rel32  → FUN_0040df70  (REL32)
//
// Reconstruction strategy — naked asm with MASM mnemonics (same pattern
// as sibling FUN_0043bc60). The single REL32 callsite is masked by
// compare.py; all other encodings are deterministic.

extern "C" {

void FUN_0040df70();

__declspec(naked) void FUN_0043f480()
{
    __asm {
        // 0003f480: 56
        push    esi
        // 0003f481: 8b f1
        mov     esi, ecx
        // 0003f483: 83 7e 18 10
        cmp     dword ptr [esi + 0x18], 0x10
        // 0003f487: 72 10
        jc      done
        // 0003f489: 8b 46 04
        mov     eax, dword ptr [esi + 0x4]
        // 0003f48c: 85 c0
        test    eax, eax
        // 0003f48e: 74 09
        jz      done
        // 0003f490: 8b 48 fc
        mov     ecx, dword ptr [eax - 0x4]
        // 0003f493: 50
        push    eax
        // 0003f494: e8 d7 ea fc ff
        call    FUN_0040df70
    done:
        // 0003f499: c7 46 18 0f 00 00 00
        mov     dword ptr [esi + 0x18], 0xf
        // 0003f4a0: c7 46 14 00 00 00 00
        mov     dword ptr [esi + 0x14], 0
        // 0003f4a7: c6 46 04 00
        mov     byte ptr [esi + 0x4], 0
        // 0003f4ab: 5e
        pop     esi
        // 0003f4ac: c3
        ret
    }
}

}  // extern "C"
