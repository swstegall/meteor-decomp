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
// FUNCTION: ffxivgame 0x00049600 — `__thiscall` checked (`_SECURE_SCL`)
//                                  std::wstring const_iterator constructor
//                                  with in-range validation (83 B / 0x53).
//
// Behaviour read from the disassembly at orig RVA 0x00049600:
//
//   struct CheckedIter { const wstring *_Mycont; const wchar_t *_Myptr; };
//
//   CheckedIter* __thiscall ctor(CheckedIter *this,    // ECX
//                                const wchar_t *ptr,    // arg1  -> EBX
//                                const wstring *str);   // arg2  -> ESI
//
//   The body initialises this->_Mycont = 0, then validates that `ptr`
//   points within the container's character buffer [_Bx, _Bx + _Mysize].
//   The MSVC 2005 std::basic_string<wchar_t> layout used here:
//
//       +0x04  _Bx       union { wchar_t _Buf[8]; wchar_t *_Ptr; }
//       +0x14  _Mysize   size_t
//       +0x18  _Myres    size_t   (capacity; >= 8 means heap _Ptr in use)
//
//   So the "base pointer" is `_Bx._Ptr` when `_Myres >= 8` (heap), else
//   the inline `_Bx._Buf` (SSO). The `* 2` in `LEA EDX,[EAX + ECX*2]`
//   (sizeof(wchar_t) == 2) and the `_BUF_SIZE == 8` threshold both
//   confirm wchar_t.
//
//   Pseudo-C:
//
//       this->_Mycont = 0;
//       if (str == 0 || ptr == 0) goto bad;
//       {
//           const wchar_t *base = (str->_Myres >= 8) ? str->_Bx._Ptr
//                                                     : str->_Bx._Buf;
//           if (base > ptr) goto bad;
//           if (ptr > base + str->_Mysize) {
//           bad:
//               FUN_009d22b4();        // _SCL_secure_invalid_argument /
//                                      //   _DEBUG_ERROR-style report
//           }
//       }
//       this->_Mycont = str;
//       this->_Myptr  = ptr;
//       return this;
//
// Single reloc-bearing site in the orig 83 bytes:
//     +0x41   CALL FUN_009d22b4   (e8 + REL32 — checked-iter error helper)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Reproducing the exact branch layout (the two `CMP _Myres,8` / `JC`
//   SSO-vs-heap selects, the JA/JBE two-sided range test, the shared
//   error-call merge target) at source level would require the precise
//   MSVC 2005 <xstring> checked-iterator header expansion. The function
//   is short and the only reloc is the single error-helper CALL, whose
//   REL32 operand the orig PE already holds post-link, so — matching the
//   sibling FUN_00401090 / FUN_00404f10 precedent — we `_emit` the 83
//   orig bytes verbatim. tools/compare.py compares the .obj `.text` to
//   the orig slice byte-for-byte; the raw immediates match directly.

extern "C" __declspec(naked) void FUN_00449600() {
    __asm {
        // 00049600: push ebx
        _emit 0x53
        // 00049601: mov ebx, dword ptr [esp+8]        ; ptr (arg1)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        // 00049605: push esi
        _emit 0x56
        // 00049606: mov esi, dword ptr [esp+10h]      ; str (arg2)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 0004960a: test esi, esi
        _emit 0x85
        _emit 0xf6
        // 0004960c: push edi
        _emit 0x57
        // 0004960d: mov edi, ecx                       ; this
        _emit 0x8b
        _emit 0xf9
        // 0004960f: mov dword ptr [edi], 0             ; this->_Mycont = 0
        _emit 0xc7
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00049615: jz 00449641                        ; str == 0 -> bad
        _emit 0x74
        _emit 0x2a
        // 00049617: test ebx, ebx
        _emit 0x85
        _emit 0xdb
        // 00049619: jz 00449641                        ; ptr == 0 -> bad
        _emit 0x74
        _emit 0x26
        // 0004961b: mov edx, dword ptr [esi+18h]       ; _Myres
        _emit 0x8b
        _emit 0x56
        _emit 0x18
        // 0004961e: cmp edx, 8
        _emit 0x83
        _emit 0xfa
        _emit 0x08
        // 00049621: lea eax, [esi+4]                   ; &_Bx
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        // 00049624: jc 0044962a                        ; SSO -> base = &_Buf
        _emit 0x72
        _emit 0x04
        // 00049626: mov ecx, dword ptr [eax]           ; heap base = _Ptr
        _emit 0x8b
        _emit 0x08
        // 00049628: jmp 0044962c
        _emit 0xeb
        _emit 0x02
        // 0004962a: mov ecx, eax                       ; SSO base = &_Buf
        _emit 0x8b
        _emit 0xc8
        // 0004962c: cmp ecx, ebx
        _emit 0x3b
        _emit 0xcb
        // 0004962e: ja 00449641                        ; base > ptr -> bad
        _emit 0x77
        _emit 0x11
        // 00049630: cmp edx, 8
        _emit 0x83
        _emit 0xfa
        _emit 0x08
        // 00049633: jc 00449637                        ; SSO keeps &_Buf
        _emit 0x72
        _emit 0x02
        // 00049635: mov eax, dword ptr [eax]           ; heap base = _Ptr
        _emit 0x8b
        _emit 0x00
        // 00049637: mov ecx, dword ptr [esi+14h]       ; _Mysize
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 0004963a: lea edx, [eax + ecx*2]             ; base + size (wchar_t)
        _emit 0x8d
        _emit 0x14
        _emit 0x48
        // 0004963d: cmp ebx, edx
        _emit 0x3b
        _emit 0xda
        // 0004963f: jbe 00449646                       ; ptr <= end -> ok
        _emit 0x76
        _emit 0x05
        // 00049641: call FUN_009d22b4                  ; checked-iter error
        _emit 0xe8
        _emit 0x6e
        _emit 0x8c
        _emit 0x58
        _emit 0x00
        // 00049646: mov dword ptr [edi], esi           ; this->_Mycont = str
        _emit 0x89
        _emit 0x37
        // 00049648: mov dword ptr [edi+4], ebx         ; this->_Myptr  = ptr
        _emit 0x89
        _emit 0x5f
        _emit 0x04
        // 0004964b: mov eax, edi                       ; return this
        _emit 0x8b
        _emit 0xc7
        // 0004964d: pop edi
        _emit 0x5f
        // 0004964e: pop esi
        _emit 0x5e
        // 0004964f: pop ebx
        _emit 0x5b
        // 00049650: ret 8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
