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
// FUNCTION: ffxivgame 0x00449a80 — __thiscall wide-string resize/tidy helper
//                                  on std::basic_string<wchar_t>
//                                  (146 B / 0x92, ret 8).
//
// Calling convention: __thiscall (ECX = this); two stack args:
//   [esp+4]  unsigned int n   — new size (count of wchar_t)
//   [esp+8]  byte flag        — non-zero means allow shrink-to-SSO (_Tidy)
//
// Object layout (std::basic_string<wchar_t>, MSVC 2005, SSO buffer = 8):
//   this + 0x04   wchar_t* / SSO buf — heap pointer when _Myres >= 8;
//                                      else inline 8-wchar SSO buffer
//   this + 0x14   unsigned int _Mysize  — current length (in wchar_t)
//   this + 0x18   unsigned int _Myres   — current capacity (in wchar_t)
//
// Behaviour (recovered from asm @ 0x00049a80):
//
//   1. Overflow guard: if n > 0xFFFFFFFE → throw via FUN_009d042e
//   2. If _Myres < n: call FUN_00449760(this, n, _Mysize) to grow capacity
//   3. Else if flag != 0 && n < 8:
//        call FUN_00403eb0(this, 1, min(n, _Mysize)) to shrink into SSO
//   4. Else if n == 0:
//        set _Mysize = 0; null-terminate at buf[0]:
//          if _Myres >= 8: dereference this->_ptr and write 0 as word
//          else (SSO):     write 0 as word to this+4 inline buffer
//   5. Return n != 0 ? 1 : 0.
//
// The SBB+NEG pattern `xor ecx,ecx / cmp ecx,edi / sbb eax,eax / neg eax`
// computes EAX = (n > 0) ? 1 : 0 (bool "string is non-empty").
//
// CALL targets (REL32, wildcarded by tools/compare.py):
//   +0x0D  CALL FUN_009d042e — length_error throw helper (noreturn)
//   +0x20  CALL FUN_00449760 — wide-string grow/reserve
//   +0x4C  CALL FUN_00403eb0 — _Tidy(_Built=1, _Newsize)
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//   The function is structurally straightforward but has multiple early
//   returns with identical SBB+NEG epilogue blocks that MSVC 2005
//   does not collapse. Using naked asm reproduces the exact byte layout
//   including the short-jump encodings.

extern "C" {
    int FUN_009d042e();    // length_error throw helper (noreturn)
    int FUN_00449760();    // wide-string grow/reserve (thiscall, 2 stack args)
    int FUN_00403eb0();    // _Tidy: shrink-to-SSO (thiscall, 2 stack args)
}

extern "C" __declspec(naked) void FUN_00449a80() {
    __asm {
        push    esi                                     // 56
        push    edi                                     // 57
        mov     edi, dword ptr [esp + 0xc]              // 8b 7c 24 0c   n (first stack arg)
        cmp     edi, -2                                 // 83 ff fe      (cmp with 0xFFFFFFFE)
        mov     esi, ecx                                // 8b f1         this
        jbe     short size_ok                           // 76 05
        call    FUN_009d042e                            // e8 ...        throw length_error

    size_ok:
        mov     eax, dword ptr [esi + 0x18]             // 8b 46 18      _Myres
        cmp     eax, edi                                // 3b c7         _Myres vs n
        jnc     short cap_ok                            // 73 19         jump if _Myres >= n
        mov     eax, dword ptr [esi + 0x14]             // 8b 46 14      _Mysize
        push    eax                                     // 50
        push    edi                                     // 57            n
        mov     ecx, esi                                // 8b ce
        call    FUN_00449760                            // e8 ...        grow(this, n, _Mysize)
        xor     ecx, ecx                                // 33 c9
        cmp     ecx, edi                                // 3b cf         0 vs n
        sbb     eax, eax                                // 1b c0
        pop     edi                                     // 5f
        neg     eax                                     // f7 d8         eax = n != 0 ? 1 : 0
        pop     esi                                     // 5e
        ret     8                                       // c2 08 00

    cap_ok:
        cmp     byte ptr [esp + 0x10], 0                // 80 7c 24 10 00  flag
        jz      short no_compact                        // 74 25
        cmp     edi, 8                                  // 83 ff 08      n vs SSO threshold
        jnc     short no_compact                        // 73 20         skip if n >= 8
        mov     eax, dword ptr [esi + 0x14]             // 8b 46 14      _Mysize
        cmp     edi, eax                                // 3b f8         n vs _Mysize
        jnc     short use_eax                           // 73 02         skip if n >= _Mysize
        mov     eax, edi                                // 8b c7         eax = min(n, _Mysize)

    use_eax:
        push    eax                                     // 50            min(n, _Mysize)
        push    1                                       // 6a 01         _Built = true
        mov     ecx, esi                                // 8b ce
        call    FUN_00403eb0                            // e8 ...        _Tidy(this, 1, min)
        xor     ecx, ecx                                // 33 c9
        cmp     ecx, edi                                // 3b cf         0 vs n
        sbb     eax, eax                                // 1b c0
        pop     edi                                     // 5f
        neg     eax                                     // f7 d8         eax = n != 0 ? 1 : 0
        pop     esi                                     // 5e
        ret     8                                       // c2 08 00

    no_compact:
        test    edi, edi                                // 85 ff         test n
        jnz     short done                              // 75 23         if n != 0, skip to done
        cmp     eax, 8                                  // 83 f8 08      _Myres vs 8
        mov     dword ptr [esi + 0x14], edi             // 89 7e 14      _Mysize = 0
        jc      short inline_buf                        // 72 13         if _Myres < 8, SSO path
        mov     esi, dword ptr [esi + 0x4]              // 8b 76 04      deref heap ptr
        xor     ecx, ecx                                // 33 c9
        cmp     ecx, edi                                // 3b cf         0 vs 0
        mov     word ptr [esi], di                      // 66 89 3e      null-terminate (word)
        sbb     eax, eax                                // 1b c0
        pop     edi                                     // 5f
        neg     eax                                     // f7 d8         eax = 0
        pop     esi                                     // 5e
        ret     8                                       // c2 08 00

    inline_buf:
        add     esi, 4                                  // 83 c6 04      esi = this+4 (SSO buf)
        mov     word ptr [esi], 0                       // 66 c7 06 00 00  null-terminate

    done:
        xor     ecx, ecx                                // 33 c9
        cmp     ecx, edi                                // 3b cf
        sbb     eax, eax                                // 1b c0
        pop     edi                                     // 5f
        neg     eax                                     // f7 d8
        pop     esi                                     // 5e
        ret     8                                       // c2 08 00
    }
}
