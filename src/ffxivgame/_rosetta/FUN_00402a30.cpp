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
// FUNCTION: ffxivgame 0x00402a30 — Rapture main-window init (254 B)
//
// __thiscall member function. Layout of the body, read from the asm:
//
//   1. Save the two stack args (HINSTANCE + int) at this->+0x0c / +0x10.
//   2. Zero a stack-local WNDCLASSEXW (memset of bytes 0x04..0x2f after
//      seeding cbSize=0x30) and populate it with style=0xb (CS_DBLCLKS|
//      CS_VREDRAW|CS_HREDRAW), the static window proc at FUN_00401c20,
//      a class icon, the standard arrow cursor, a dark-gray brush
//      (GetStockObject(4)), the wide class name L"RAPTURE", and a
//      matching small icon.
//   3. RegisterClassExW(&local).
//   4. CreateWindowExA(0, "RAPTURE", <lpWindowName>, 0, CW_USEDEFAULT
//      x4, NULL, this->FUN_004023f0(), this->hInstance, NULL) — store
//      the HWND at this->+0x18.
//   5. Tail helpers: FUN_0090f800(&this->hInstance) (cdecl) and
//      this->FUN_00401820() (thiscall).
//   6. Return AL=1.
//
// Key MSVC-2005 idiom: the LoadIconA IAT slot is loaded into EBX once
// (mov ebx,[__imp__LoadIconA@8]) and reused for BOTH the class icon
// and small-icon loads — `call ebx` twice. A plain C++ source that
// just calls LoadIconA twice would emit two independent
// `call dword ptr [__imp__LoadIconA@8]` sites (no caching). Naked asm
// is the simplest way to lock in MSVC's exact register choice and
// instruction selection here.

extern "C" __declspec(dllimport) void *__stdcall LoadIconA(void *hInstance, const char *lpIconName);
extern "C" __declspec(dllimport) void *__stdcall LoadCursorA(void *hInstance, const char *lpCursorName);
extern "C" __declspec(dllimport) void *__stdcall GetStockObject(int i);
extern "C" __declspec(dllimport) unsigned short __stdcall RegisterClassExW(const void *lpwcx);
extern "C" __declspec(dllimport) void *__stdcall CreateWindowExA(
    unsigned long dwExStyle, const char *lpClassName, const char *lpWindowName,
    unsigned long dwStyle, int X, int Y, int nWidth, int nHeight,
    void *hWndParent, void *hMenu, void *hInstance, void *lpParam);

extern "C" void *__cdecl memset(void *dst, int value, unsigned int count);

// MSVC treats memset as an intrinsic — disable so the symbol is usable
// in inline-asm `call` operands without `C2420: illegal symbol`.
#pragma function(memset)

// Member fns and helpers (linker stubs — addresses don't need to match
// at link time since this .obj is only ever compared byte-wise, not
// linked into a full PE).
extern "C" int FUN_004023f0(void);              // 0x004023f0 — this->menu-ctor
extern "C" int FUN_0090f800(void *p);           // 0x0090f800 — cdecl helper
extern "C" int FUN_00401820(void);              // 0x00401820 — this->post-init
extern "C" long __stdcall FUN_00401c20(         // 0x00401c20 — wndproc
    void *hWnd, unsigned uMsg, unsigned wParam, long lParam);

// Globals (in .rdata / .data at fixed VAs in the original binary).
extern wchar_t s_RAPTURE_W[];                   // 0x00f541c0 — L"RAPTURE"
extern char s_RAPTURE_A[];                      // 0x00f541d0 — "RAPTURE"
extern char s_window_title[];                   // 0x013232c0 — lpWindowName

extern "C" __declspec(naked) void FUN_00402a30() {
    __asm {
        sub     esp, 0x30
        mov     eax, dword ptr [esp+0x34]
        push    ebx
        push    esi
        push    edi
        mov     esi, ecx
        mov     ecx, dword ptr [esp+0x44]
        push    0x2c
        lea     edx, [esp+0x14]
        push    0
        push    edx
        mov     dword ptr [esi+0xc], eax
        mov     dword ptr [esi+0x10], ecx
        mov     dword ptr [esp+0x18], 0x30
        call    memset
        mov     eax, dword ptr [esi+0x14]
        mov     ebx, dword ptr [LoadIconA]
        add     esp, 0xc
        lea     edi, [esi+0x14]
        push    0x8d
        push    eax
        mov     dword ptr [esp+0x18], 0xb
        mov     dword ptr [esp+0x1c], offset FUN_00401c20
        mov     dword ptr [esp+0x20], 0
        mov     dword ptr [esp+0x24], 0
        mov     dword ptr [esp+0x28], eax
        call    ebx
        push    0x7f00
        push    0
        mov     dword ptr [esp+0x2c], eax
        call    dword ptr [LoadCursorA]
        push    4
        mov     dword ptr [esp+0x2c], eax
        call    dword ptr [GetStockObject]
        mov     dword ptr [esp+0x2c], eax
        mov     eax, dword ptr [edi]
        push    0x8d
        push    eax
        mov     dword ptr [esp+0x3c], offset s_RAPTURE_W
        call    ebx
        lea     ecx, [esp+0xc]
        push    ecx
        mov     dword ptr [esp+0x3c], eax
        call    dword ptr [RegisterClassExW]
        mov     edx, dword ptr [edi]
        push    0
        push    edx
        mov     ecx, esi
        call    FUN_004023f0
        push    eax
        push    0
        push    0x80000000
        push    0x80000000
        push    0x80000000
        push    0x80000000
        push    0
        push    offset s_window_title
        push    offset s_RAPTURE_A
        push    0
        call    dword ptr [CreateWindowExA]
        push    edi
        mov     dword ptr [esi+0x18], eax
        call    FUN_0090f800
        add     esp, 4
        mov     ecx, esi
        call    FUN_00401820
        pop     edi
        pop     esi
        mov     al, 1
        pop     ebx
        add     esp, 0x30
        ret     8
    }
}
