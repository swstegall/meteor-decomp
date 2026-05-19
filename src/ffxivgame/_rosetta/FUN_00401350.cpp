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
// FUNCTION: ffxivgame 0x00401350 — WH_CBT hook proc for localised MessageBoxW
//
// FUN_00401b70 installs this proc via SetWindowsHookExA(WH_CBT=5, …) right
// before calling MessageBoxW(), so that on the dialog's HCBT_ACTIVATE
// notification the four standard button labels (IDOK=1, IDCANCEL=2,
// IDYES=6, IDNO=7) get replaced with localised strings fetched from the
// engine's resource pool via thunk_FUN_00440a90(id). After patching the
// labels the hook tears itself down via UnhookWindowsHookEx(); every
// invocation (including nCode != HCBT_ACTIVATE ones) still chains to
// CallNextHookEx() so the hook stays well-behaved.
//
// Calling convention: __stdcall (HOOKPROC — ret 0xc).
// Stack frame: PUSH EBX + PUSH ESI; PUSH EDI only inside the if-branch.
//
// Register notes:
//   ebx = nCode  (saved across the body for the trailing CallNextHookEx)
//   esi = wParam (the dialog HWND — held in a callee-saved reg because
//                 SetDlgItemTextW is called four times on it)
//   edi = address of SetDlgItemTextW (cached out of the IAT once and
//                 reused for the four label patches — MSVC 2005 /O2
//                 hoists the import load into edi when the same
//                 dllimport is called 3+ times in one block)

#include <windows.h>

extern "C" const wchar_t * __cdecl thunk_FUN_00440a90(int id);

static HHOOK DAT_01323288;

extern "C" LRESULT CALLBACK FUN_00401350(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == 5) {
        HWND hwnd = (HWND)wParam;
        SetDlgItemTextW(hwnd, 1, thunk_FUN_00440a90(9));
        SetDlgItemTextW(hwnd, 2, thunk_FUN_00440a90(11));
        SetDlgItemTextW(hwnd, 6, thunk_FUN_00440a90(8));
        SetDlgItemTextW(hwnd, 7, thunk_FUN_00440a90(10));
        UnhookWindowsHookEx(DAT_01323288);
    }
    return CallNextHookEx(DAT_01323288, nCode, wParam, lParam);
}
