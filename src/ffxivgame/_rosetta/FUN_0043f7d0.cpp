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
// FUNCTION: ffxivgame 0x0043f7d0 — __thiscall SSO basic_string<char>
//                                  init-and-assign from const char* (59 B / 0x3b)
//
// Calling convention: __thiscall (ECX = this); one DWORD stack arg
//   (const char* src); callee-cleans 4 bytes via `ret 4`.
//   Returns `this` in EAX.
//
// Object layout (MSVC 2005 SSO basic_string<char>, `this` = ESI):
//   +0x00  [vtable / container header, unused in this fragment]
//   +0x04  union { char _Buf[16]; char *_Ptr; } _Bx   (inline SSO buffer)
//   +0x14  size_type _Mysize   (current length)
//   +0x18  size_type _Myres    (capacity; < 0x10 → using inline _Buf)
//
// Behaviour:
//   1. Initialize the string to empty:
//        _Myres  = 15   (SSO capacity — inline buffer fits 15 chars + NUL)
//        _Mysize = 0
//        _Buf[0] = '\0'
//   2. Compute strlen(src) via an inline scan loop (NOP-aligned to 0x20).
//   3. Call FUN_0043f590 (basic_string::assign(const char*, size_t))
//      with `this` in ECX and (src, len) on the stack.
//   4. Return `this`.
//
// The NOP at offset 0x1f aligns the inner scan loop to a 16-byte boundary
// (offset 0x20 from function entry in .text), which MSVC 2005 /O2 emits
// automatically when exactly 31 prologue/init bytes precede the loop.
//
// The inner loop uses `ADD EAX, 1` (83 C0 01) rather than `INC EAX` (FF C0) —
// the form MSVC 2005 emits for its strlen intrinsic expansion.
//
// REL32 callsite to FUN_0043f590 is masked by tools/compare.py.

extern "C" int FUN_0043f590();  // basic_string::assign(const char*, size_t)

extern "C" __declspec(naked) void FUN_0043f7d0() {
    __asm {
        mov     edx, dword ptr [esp + 4]    // load src before ESP shifts
        push    esi
        mov     esi, ecx                    // esi = this
        mov     eax, edx                    // eax = src (scan pointer)
        push    edi
        mov     dword ptr [esi + 0x18], 15  // _Myres  = 0xf
        mov     dword ptr [esi + 0x14], 0   // _Mysize = 0
        mov     byte ptr [esi + 4], 0       // _Buf[0] = '\0'
        lea     edi, [eax + 1]              // edi = src + 1 (strlen base)
        nop                                 // loop-alignment pad → loop at +0x20
    strloop:
        mov     cl, byte ptr [eax]
        add     eax, 1
        test    cl, cl
        jnz     strloop
        sub     eax, edi                    // len = (post-NUL ptr) - (src+1)
        push    eax                         // push len
        push    edx                         // push src
        mov     ecx, esi
        call    FUN_0043f590
        pop     edi
        mov     eax, esi                    // return this
        pop     esi
        ret     4
    }
}
