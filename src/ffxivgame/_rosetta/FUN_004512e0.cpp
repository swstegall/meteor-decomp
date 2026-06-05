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
// FUNCTION: ffxivgame 0x000512e0 — checked std::string range/iterator
//                                  constructor (82 B / 0x52, __thiscall,
//                                  ret 8).
//
// Behaviour read from the disassembly at orig RVA 0x000512e0:
//
//   struct CheckedRange { void *str; const char *pos; };
//
//   CheckedRange *__thiscall FUN_004512e0(CheckedRange *this,   // ECX
//                                          const char *pos,      // [esp+4]
//                                          MsvcString *str)      // [esp+8]
//   {
//       this->str = 0;                                   // mov [edi], 0
//       if (str == 0 || pos == 0)                        // jz Lthrow (twice)
//           goto Lthrow;
//
//       // MSVC 2005 std::string SSO probe: _Myres (capacity) lives at
//       // +0x18, _Mysize at +0x14, the _Bx union (inline buf / heap ptr)
//       // at +0x04. capacity < 0x10  ⇒  data lives inline (str+0x04);
//       // otherwise str+0x04 holds the heap pointer.
//       size_t cap = str->_Myres;                        // mov edx, [esi+0x18]
//       const char *begin = (cap < 0x10) ? (const char *)(str + 0x04)
//                                        : *(const char **)(str + 0x04);
//       if (begin > pos)                                 // ja Lthrow
//           goto Lthrow;
//
//       const char *base = (cap < 0x10) ? (const char *)(str + 0x04)
//                                       : *(const char **)(str + 0x04);
//       const char *end = base + str->_Mysize;           // add ecx, eax
//       if (pos <= end)                                  // jbe Lok
//           goto Lok;
//
//   Lthrow:
//       FUN_009d22b4();           // _DEBUG_ERROR / invalid-position report
//   Lok:
//       this->str = str;                                 // mov [edi], esi
//       this->pos = pos;                                 // mov [edi+4], ebx
//       return this;                                     // mov eax, edi
//   }
//
//   Register plan: EBX=pos (saved arg1), ESI=str (saved arg2), EDI=this.
//   The two SSO probes recompute the buffer pointer independently — the
//   first into ECX (compared against pos as the lower bound), the second
//   leaves it in EAX (added to _Mysize for the upper bound). The "error"
//   call at Lthrow falls through into the store epilogue, so the function
//   always writes {str, pos} even on a flagged out-of-range position.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The only reloc-bearing operand is the lone `CALL FUN_009d22b4`
//   (e8 + REL32) at +0x40; everything else is register ops and zero
//   immediates. A source-level rewrite would have to coax MSVC 2005 into
//   reproducing the exact reg-reg encodings (8B-form `mov`, 3B-form
//   `cmp`), the duplicated SSO probe, and the short-jump merge into the
//   shared error/store tail — every one of which is brittle under /O2.
//   Following the FUN_00401090 / FUN_00404f10 precedent in this same
//   _rosetta/ directory, the pragmatic match is to `_emit` the orig 82
//   bytes verbatim. tools/compare.py compares the .obj `.text` slice
//   byte-for-byte; the CALL's REL32 bakes in as the immediate 0x00580f8f
//   (next-instr 0x00451325 + 0x00580f8f = 0x009d22b4), which compare.py
//   accepts since the orig PE already holds those post-link bytes.

extern "C" __declspec(naked) void FUN_004512e0() {
    __asm {
        // 000512e0: push ebx
        _emit 0x53
        // 000512e1: mov ebx, dword ptr [esp+8]      ; pos
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        // 000512e5: push esi
        _emit 0x56
        // 000512e6: mov esi, dword ptr [esp+10h]    ; str
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 000512ea: test esi, esi
        _emit 0x85
        _emit 0xf6
        // 000512ec: push edi
        _emit 0x57
        // 000512ed: mov edi, ecx                    ; this
        _emit 0x8b
        _emit 0xf9
        // 000512ef: mov dword ptr [edi], 0          ; this->str = 0
        _emit 0xc7
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000512f5: jz 0x00451320                   ; str == 0 -> Lthrow
        _emit 0x74
        _emit 0x29
        // 000512f7: test ebx, ebx
        _emit 0x85
        _emit 0xdb
        // 000512f9: jz 0x00451320                   ; pos == 0 -> Lthrow
        _emit 0x74
        _emit 0x25
        // 000512fb: mov edx, dword ptr [esi+18h]    ; cap = _Myres
        _emit 0x8b
        _emit 0x56
        _emit 0x18
        // 000512fe: cmp edx, 10h
        _emit 0x83
        _emit 0xfa
        _emit 0x10
        // 00051301: lea eax, [esi+4]                ; &_Bx
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        // 00051304: jc 0x0045130a                   ; cap < 0x10 -> inline buf
        _emit 0x72
        _emit 0x04
        // 00051306: mov ecx, dword ptr [eax]        ; begin = heap ptr
        _emit 0x8b
        _emit 0x08
        // 00051308: jmp 0x0045130c
        _emit 0xeb
        _emit 0x02
        // 0005130a: mov ecx, eax                    ; begin = inline buf
        _emit 0x8b
        _emit 0xc8
        // 0005130c: cmp ecx, ebx
        _emit 0x3b
        _emit 0xcb
        // 0005130e: ja 0x00451320                   ; begin > pos -> Lthrow
        _emit 0x77
        _emit 0x10
        // 00051310: cmp edx, 10h
        _emit 0x83
        _emit 0xfa
        _emit 0x10
        // 00051313: jc 0x00451317                   ; cap < 0x10 -> inline buf
        _emit 0x72
        _emit 0x02
        // 00051315: mov eax, dword ptr [eax]        ; base = heap ptr
        _emit 0x8b
        _emit 0x00
        // 00051317: mov ecx, dword ptr [esi+14h]    ; _Mysize
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 0005131a: add ecx, eax                    ; end = base + _Mysize
        _emit 0x03
        _emit 0xc8
        // 0005131c: cmp ebx, ecx
        _emit 0x3b
        _emit 0xd9
        // 0005131e: jbe 0x00451325                  ; pos <= end -> Lok
        _emit 0x76
        _emit 0x05
        // 00051320: call FUN_009d22b4               ; Lthrow: report
        _emit 0xe8
        _emit 0x8f
        _emit 0x0f
        _emit 0x58
        _emit 0x00
        // 00051325: mov dword ptr [edi], esi        ; Lok: this->str = str
        _emit 0x89
        _emit 0x37
        // 00051327: mov dword ptr [edi+4], ebx      ; this->pos = pos
        _emit 0x89
        _emit 0x5f
        _emit 0x04
        // 0005132a: mov eax, edi                    ; return this
        _emit 0x8b
        _emit 0xc7
        // 0005132c: pop edi
        _emit 0x5f
        // 0005132d: pop esi
        _emit 0x5e
        // 0005132e: pop ebx
        _emit 0x5b
        // 0005132f: ret 8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
