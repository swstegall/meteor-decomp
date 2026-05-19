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
// FUNCTION: ffxivgame 0x00401460 — App-window teardown for a class with an
// inline polymorphic subobject at offset 0x34, an HWND at 0x18, and a
// mutex HANDLE at 0x960. Invoked while the host class is being torn
// down (likely from its destructor): forwards a virtual cleanup to the
// subobject, destroys the owned window, then releases + closes + nulls
// the mutex if one was created.
//
// Calling convention: `__thiscall` (member function, no stack args; `c3`
// epilogue with `push esi`/`mov esi, ecx` cache).
//
// Asm (66 bytes):
//   56                          push esi
//   8b f1                       mov  esi, ecx           ; cache `this`
//   8b 46 34                    mov  eax, [esi+0x34]    ; m_sub.vptr
//   8b 50 08                    mov  edx, [eax+8]       ; vtable slot 2
//   8d 4e 34                    lea  ecx, [esi+0x34]    ; &m_sub (this)
//   ff d2                       call edx                ; m_sub.vfunc()
//   8b 46 18                    mov  eax, [esi+0x18]    ; m_hwnd
//   50                          push eax
//   ff 15 7c e4 f3 00           call [user32!DestroyWindow]
//   8b 86 60 09 00 00           mov  eax, [esi+0x960]   ; m_mutex
//   85 c0                       test eax, eax
//   74 1e                       jz   short epilog
//   50                          push eax
//   ff 15 e8 e1 f3 00           call [kernel32!ReleaseMutex]
//   8b 8e 60 09 00 00           mov  ecx, [esi+0x960]   ; reload m_mutex
//   51                          push ecx
//   ff 15 ec e1 f3 00           call [kernel32!CloseHandle]
//   c7 86 60 09 00 00 00 00 00 00  mov dword ptr [esi+0x960], 0
//   epilog:
//   5e                          pop  esi
//   c3                          ret

#include <windows.h>

namespace {

class CSub {
public:
    virtual void v0();
    virtual void v1();
    virtual void v2();   // vtable slot 2 (offset 8) — the one called here
};

class C {
public:
    void Cleanup();
private:
    char   m_pad0[0x18];     // 0x000..0x017
    HWND   m_hwnd;           // 0x018
    char   m_pad1[0x18];     // 0x01c..0x033
    CSub   m_sub;            // 0x034 (vptr-only inline subobject)
    char   m_pad2[0x928];    // 0x038..0x95f
    HANDLE m_mutex;          // 0x960
};

void C::Cleanup() {
    m_sub.v2();
    DestroyWindow(m_hwnd);
    if (m_mutex != NULL) {
        ReleaseMutex(m_mutex);
        CloseHandle(m_mutex);
        m_mutex = NULL;
    }
}

} // namespace
