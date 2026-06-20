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
// FUNCTION: ffxivgame 0x00054db0 — wstring::assign(wchar_t* first, wchar_t* last)
//                                  range-assign for an MSVC 2005 SSO wide-string
//                                  class (__thiscall, RET 0x8, 215 B / 0xd7).
//
// Layout of the wide-string object (this = ESI throughout):
//   [this+0x00] — vtable (not accessed)
//   [this+0x04] — SSO data union: if capacity < 8, inline buffer; else pointer
//   [this+0x14] — current length (in wchar_t units)
//   [this+0x18] — capacity (in wchar_t units); SSO threshold = 8
//
// Signature (recovered from caller patterns and RET 0x8):
//
//   wstring* __thiscall FUN_00454db0(wstring *this,
//                                    wchar_t *first,   // [ESP+0x10] after 3 callee saves
//                                    wchar_t *last);   // [ESP+0x14] after 3 callee saves
//
// Body (logical sketch):
//
//   wchar_t *buf  = (cap >= 8) ? this->ptr : (wchar_t*)&this->inline_buf;
//
//   if (first >= buf && first < buf + this->len) {
//       // Source range aliases our own buffer — delegate to the
//       // aliased-range overload at 0x00454cc0 which handles overlap.
//       // Position of first (in wchar_t) computed by (first - buf) >> 1.
//       this->FUN_00454cc0(this, pos_of_first, last);  // __thiscall, RET 0xc
//       return this;
//   }
//
//   count = last - first;          // nwchar = new length
//   if (count > 0xFFFFFFFE)        // npos-like sentinel
//       __throw_length_error();    // FUN at 0x009d042e
//
//   if (this->cap < count) {
//       // Need to grow — call reserve-like function at 0x00449760.
//       this->FUN_00449760(count, this->len);
//   }
//
//   if (count > 0) {
//       // Get write pointer (SSO-aware again after potential realloc).
//       wchar_t *dst = (this->cap >= 8) ? this->ptr : (wchar_t*)&this->inline_buf;
//       // Copy count wchar_t units from [first..last) into dst.
//       // Call to 0x009d17f3 = memmove-variant(__dst, old_size_bytes, src, count_bytes).
//       FUN_009d17f3(dst, this->len * 2, first, count * 2);  // __cdecl, ADD ESP 0x10
//       this->len = count;
//       // SSO-aware pointer reload then NUL-terminate.
//       wchar_t *end_ptr = (this->cap >= 8) ? this->ptr : (wchar_t*)&this->inline_buf;
//       end_ptr[count] = L'\0';
//   } else {
//       // count == 0 path: just set len=0, NUL-terminate.
//       this->len = 0;
//       wchar_t *p = (this->cap >= 8) ? this->ptr : (wchar_t*)&this->inline_buf;
//       p[0] = L'\0';
//   }
//   return this;
//
// Relocation sites (rel32 or dir32 bytes baked in the _emit stream;
// compare.py masks these windows against the orig slice):
//   +0x4a  rel32 → 0x00454cc0   (FUN_00454cc0, aliased-range helper)
//   +0x5e  rel32 → 0x009d042e   (throw-length-error / _Xlen)
//   +0x71  rel32 → 0x00449760   (FUN_00449760, reserve/grow helper)
//   +0xb4  rel32 → 0x009d17f3   (memmove-like CRT internal)
//
// Reconstruction strategy — naked-asm byte passthrough.
//
//   The function contains four inter-module CALL sites, three SSO
//   capacity-branch pairs, a wchar_t memmove dispatch, and a non-trivial
//   stack layout (EBP reused as data-pointer after its callee-save role).
//   Source-level C++ at /O2 would need to reproduce the exact register
//   scheduling (EBP holding both the saved frame pointer AND the SSO
//   data pointer, EDI carrying the new length across multiple branches)
//   plus the precise branch encodings (short JC/JBE/JNC). The safest
//   and most reliable path — consistent with FUN_00408910 / FUN_004011b0
//   / FUN_00404e40 in this _rosetta set — is the _emit byte passthrough.

extern "C" __declspec(naked) void FUN_00454db0() {
    __asm {
        // 00054db0
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI,ECX
        _emit 0xf1
        _emit 0x8b              // MOV EDX,[ESI+0x18]
        _emit 0x56
        _emit 0x18
        _emit 0x83              // CMP EDX,0x8
        _emit 0xfa
        _emit 0x08
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA EBP,[ESI+0x4]
        _emit 0x6e
        _emit 0x04
        _emit 0x72              // JC 0x00454dc5
        _emit 0x05
        // 00054dc0
        _emit 0x8b              // MOV ECX,[EBP]
        _emit 0x4d
        _emit 0x00
        _emit 0xeb              // JMP 0x00454dc7
        _emit 0x02
        _emit 0x8b              // MOV ECX,EBP
        _emit 0xcd
        // 00054dc7
        _emit 0x8b              // MOV EAX,[ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x3b              // CMP EAX,ECX
        _emit 0xc1
        _emit 0x72              // JC 0x00454e05
        _emit 0x36
        // 00054dcf
        _emit 0x83              // CMP EDX,0x8
        _emit 0xfa
        _emit 0x08
        _emit 0x72              // JC 0x00454dd9
        _emit 0x05
        _emit 0x8b              // MOV ECX,[EBP]
        _emit 0x4d
        _emit 0x00
        _emit 0xeb              // JMP 0x00454ddb
        _emit 0x02
        _emit 0x8b              // MOV ECX,EBP
        _emit 0xcd
        // 00054ddb
        _emit 0x8b              // MOV EDI,[ESI+0x14]
        _emit 0x7e
        _emit 0x14
        _emit 0x8d              // LEA ECX,[ECX+EDI*2]
        _emit 0x0c
        _emit 0x79
        _emit 0x3b              // CMP ECX,EAX
        _emit 0xc8
        _emit 0x76              // JBE 0x00454e05
        _emit 0x20
        // 00054de5
        _emit 0x83              // CMP EDX,0x8
        _emit 0xfa
        _emit 0x08
        _emit 0x72              // JC 0x00454ded
        _emit 0x03
        _emit 0x8b              // MOV EBP,[EBP]
        _emit 0x6d
        _emit 0x00
        // 00054ded
        _emit 0x8b              // MOV EDX,[ESP+0x14]
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x2b              // SUB EAX,EBP
        _emit 0xc5
        _emit 0x52              // PUSH EDX
        _emit 0xd1              // SAR EAX,1
        _emit 0xf8
        _emit 0x50              // PUSH EAX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ECX,ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x00454cc0   [reloc +0x4b]
        _emit 0xc1
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 00054dff
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        // 00054e05
        _emit 0x8b              // MOV EDI,[ESP+0x14]
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x83              // CMP EDI,-0x2
        _emit 0xff
        _emit 0xfe
        _emit 0x76              // JBE 0x00454e13
        _emit 0x05
        _emit 0xe8              // CALL 0x009d042e   [reloc +0x5f]
        _emit 0x1b
        _emit 0xb6
        _emit 0x57
        _emit 0x00
        // 00054e13
        _emit 0x8b              // MOV EAX,[ESI+0x18]
        _emit 0x46
        _emit 0x18
        _emit 0x3b              // CMP EAX,EDI
        _emit 0xc7
        _emit 0x73              // JNC 0x00454e37
        _emit 0x1d
        // 00054e1a
        _emit 0x8b              // MOV EAX,[ESI+0x14]
        _emit 0x46
        _emit 0x14
        _emit 0x50              // PUSH EAX
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV ECX,ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x00449760   [reloc +0x72]
        _emit 0x3a
        _emit 0x49
        _emit 0xff
        _emit 0xff
        // 00054e26
        _emit 0x85              // TEST EDI,EDI
        _emit 0xff
        _emit 0x76              // JBE 0x00454e7f
        _emit 0x55
        // 00054e2a
        _emit 0x8b              // MOV ECX,[ESI+0x18]
        _emit 0x4e
        _emit 0x18
        _emit 0x83              // CMP ECX,0x8
        _emit 0xf9
        _emit 0x08
        _emit 0x72              // JC 0x00454e54
        _emit 0x22
        _emit 0x8b              // MOV EAX,[EBP]
        _emit 0x45
        _emit 0x00
        _emit 0xeb              // JMP 0x00454e56
        _emit 0x1f
        // 00054e37
        _emit 0x85              // TEST EDI,EDI
        _emit 0xff
        _emit 0x75              // JNZ 0x00454e28
        _emit 0xed
        // 00054e3b
        _emit 0x83              // CMP EAX,0x8
        _emit 0xf8
        _emit 0x08
        _emit 0x89              // MOV [ESI+0x14],EDI
        _emit 0x7e
        _emit 0x14
        _emit 0x72              // JC 0x00454e46
        _emit 0x03
        _emit 0x8b              // MOV EBP,[EBP]
        _emit 0x6d
        _emit 0x00
        // 00054e46
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX,ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x66              // MOV word ptr [EBP],0x0
        _emit 0xc7
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        // 00054e54
        _emit 0x8b              // MOV EAX,EBP
        _emit 0xc5
        // 00054e56
        _emit 0x8b              // MOV EDX,[ESP+0x10]
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x53              // PUSH EBX
        _emit 0x8d              // LEA EBX,[EDI+EDI*1]
        _emit 0x1c
        _emit 0x3f
        _emit 0x53              // PUSH EBX
        _emit 0x52              // PUSH EDX
        _emit 0x03              // ADD ECX,ECX
        _emit 0xc9
        _emit 0x51              // PUSH ECX
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x009d17f3   [reloc +0xb5]
        _emit 0x8a
        _emit 0xc9
        _emit 0x57
        _emit 0x00
        // 00054e69
        _emit 0x83              // ADD ESP,0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x83              // CMP [ESI+0x18],0x8
        _emit 0x7e
        _emit 0x18
        _emit 0x08
        _emit 0x89              // MOV [ESI+0x14],EDI
        _emit 0x7e
        _emit 0x14
        _emit 0x72              // JC 0x00454e78
        _emit 0x03
        _emit 0x8b              // MOV EBP,[EBP]
        _emit 0x6d
        _emit 0x00
        // 00054e78
        _emit 0x66              // MOV word ptr [EBX+EBP*1],0x0
        _emit 0xc7
        _emit 0x04
        _emit 0x2b
        _emit 0x00
        _emit 0x00
        _emit 0x5b              // POP EBX
        // 00054e7f
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX,ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
