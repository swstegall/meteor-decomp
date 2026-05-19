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
// FUNCTION: ffxivgame 0x004014b0 — main-window message pump tick (307 B)
//
// __thiscall on a window-pump-style class. ESI holds `this` throughout
// the body. Wraps a try-block (inline SEH prolog + cookie) around the
// PeekMessageW/IME-fixup/Translate/Dispatch loop.
//
// Class fields used:
//   +0x08  char   m_should_stop      (early-exit bit; if 1, return 0)
//   +0x0b  char   m_pump_counter     (++ each tick, reset at 8)
//   +0x18  HWND   m_main_hwnd        (PeekMessageW filter)
//   +0x30  ImeFilter m_ime           (FUN_004b3820 receiver — gates the
//                                     "replace wParam with virtual key"
//                                     behaviour for VK_PROCESSKEY)
//   +0x34  Handler* m_handler        (vtable[1] is the "tick" predicate
//                                     called when there are no messages
//                                     OR when the counter has wrapped)
//
// Behaviour:
//   if (m_should_stop != 1) {
//     m_pump_counter++;
//     if (m_pump_counter < 8) {
//       if (PeekMessageW(&msg, m_main_hwnd, 0, 0, PM_REMOVE)) {
//         // window message → optional IME virtual-key fixup
//         if ((msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) &&
//             msg.wParam == VK_PROCESSKEY) {
//           if (m_ime.IsActive())
//             msg.wParam = ImmGetVirtualKey(msg.hwnd);
//           else if (ImmGetVirtualKey(msg.hwnd) == VK_TAB)
//             msg.wParam = VK_TAB;
//         }
//         TranslateMessage(&msg); DispatchMessageW(&msg);
//         return 1;
//       }
//       if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
//         TranslateMessage(&msg); DispatchMessageW(&msg);
//         return 1;
//       }
//       // No messages — run the tick predicate.
//       return m_handler->vtable[1]();
//     }
//     // Counter wrapped — run tick and reset.
//     if (m_handler->vtable[1]()) { m_pump_counter = 0; return 1; }
//   }
//   return 0;
//
// Implemented as __declspec(naked) inline asm because the orig uses the
// inline (non-helper) SEH+cookie prolog. Reproducing that exact byte
// sequence from C++ source is fragile — the prolog choice depends on
// /Os vs /Ot heuristics that don't have a direct source trigger. The
// asm reproduces every byte; reloc-bearing positions are masked by the
// diff tool.

// __security_cookie — global at .data 0x012ea8b0, supplied by CRT cookie.c.
extern "C" unsigned __security_cookie;

// IAT slots (.idata). Pre-bound dword pointers to the user32 imports.
extern "C" int g_imp_PeekMessageW;     // .idata @ 0x00f3e420
extern "C" int g_imp_TranslateMessage; // .idata @ 0x00f3e41c
extern "C" int g_imp_DispatchMessageW; // .idata @ 0x00f3e3e8

// Direct-call targets in .text.
extern "C" int FUN_004b3820();         // ImeFilter::IsActive
extern "C" int FUN_009d00a2();         // ImmGetVirtualKey near-jump thunk

// Per-function SEH scope table emitted by the compiler at .text
// 0x00e54322. For matching, only the 4-byte relocation placeholder
// matters — the diff masks reloc bytes.
extern "C" int __ehhandler$FUN_004014b0;

extern "C" __declspec(naked) char FUN_004014b0() {
    __asm {
        push    ebp
        mov     ebp, esp
        push    -1
        push    offset __ehhandler$FUN_004014b0
        mov     eax, fs:[0]
        push    eax
        sub     esp, 24h
        push    ebx
        push    esi
        push    edi
        mov     eax, __security_cookie
        xor     eax, ebp
        push    eax
        lea     eax, [ebp - 0Ch]
        mov     fs:[0], eax
        mov     [ebp - 10h], esp
        mov     esi, ecx
        mov     [ebp - 14h], esi
        mov     ebx, 1
        cmp     [esi + 8], bl
        mov     dword ptr [ebp - 4], 0
        jz      ret_zero
        add     [esi + 0Bh], bl
        cmp     byte ptr [esi + 0Bh], 8
        jae     counter_wrapped
        mov     eax, [esi + 18h]
        mov     edi, dword ptr [g_imp_PeekMessageW]
        push    ebx
        push    0
        push    0
        push    eax
        lea     ecx, [ebp - 30h]
        push    ecx
        call    edi
        test    eax, eax
        jz      no_window_msg
        mov     eax, [ebp - 2Ch]
        cmp     eax, 100h
        jz      check_proc_key
        cmp     eax, 101h
        jnz     translate_dispatch
    check_proc_key:
        cmp     dword ptr [ebp - 28h], 0E5h
        jnz     translate_dispatch
        lea     ecx, [esi + 30h]
        call    FUN_004b3820
        test    al, al
        jz      ime_get_tab
        mov     edx, [ebp - 30h]
        push    edx
        call    FUN_009d00a2
        mov     [ebp - 28h], eax
        jmp     translate_dispatch
    ime_get_tab:
        mov     eax, [ebp - 30h]
        push    eax
        call    FUN_009d00a2
        cmp     eax, 9
        jnz     translate_dispatch
        mov     [ebp - 28h], eax
    translate_dispatch:
        lea     ecx, [ebp - 30h]
        push    ecx
        call    dword ptr [g_imp_TranslateMessage]
        lea     edx, [ebp - 30h]
        push    edx
        call    dword ptr [g_imp_DispatchMessageW]
        mov     al, bl
        mov     ecx, [ebp - 0Ch]
        mov     fs:[0], ecx
        pop     ecx
        pop     edi
        pop     esi
        pop     ebx
        mov     esp, ebp
        pop     ebp
        ret
    no_window_msg:
        push    ebx
        push    0
        push    0
        push    0
        lea     eax, [ebp - 30h]
        push    eax
        call    edi
        test    eax, eax
        jnz     translate_dispatch
        mov     eax, [esi + 34h]
        mov     edx, [eax + 4]
        lea     ecx, [esi + 34h]
        call    edx
        test    al, al
        jnz     ret_one
    ret_zero:
        xor     al, al
        mov     ecx, [ebp - 0Ch]
        mov     fs:[0], ecx
        pop     ecx
        pop     edi
        pop     esi
        pop     ebx
        mov     esp, ebp
        pop     ebp
        ret
    counter_wrapped:
        mov     eax, [esi + 34h]
        mov     edx, [eax + 4]
        lea     ecx, [esi + 34h]
        call    edx
        test    al, al
        jz      ret_zero
        mov     byte ptr [esi + 0Bh], 0
    ret_one:
        mov     al, bl
        mov     ecx, [ebp - 0Ch]
        mov     fs:[0], ecx
        pop     ecx
        pop     edi
        pop     esi
        pop     ebx
        mov     esp, ebp
        pop     ebp
        ret
    }
}
