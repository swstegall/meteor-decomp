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
// FUNCTION: ffxivgame 0x00001820 — center-window + ShowWindow init method
// (__thiscall on a graphics/window-host class, 465 B)
//
// `this` (ECX → ESI) is the same window-host object initialised by the
// sibling at RVA 0x00001750 (which sets this->m_c = 0x500 (1280) and
// this->m_10 = 0x2d0 (720) — the default 1280x720 client area). Field
// layout, inferred from the two functions together:
//
//   +0x09  byte  windowed-mode-already-positioned flag
//   +0x0c  int   client-area width  (window->m_c)
//   +0x10  int   client-area height (window->m_10)
//   +0x18  HWND  top-level window handle
//
// Behaviour:
//
//   if (this->m_9 == 0) {
//       // Compute centered window position for a (m_c x m_10) client.
//       RECT r = { 0, 0, m_c, m_10 };
//       BOOL bMenu = (GetMenu(hwnd) != NULL) ? 1 : 0;
//       AdjustWindowRect(&r, WS_POPUP|WS_VISIBLE|WS_CAPTION|
//                            WS_SYSMENU|WS_MINIMIZEBOX, bMenu);
//       int adj_w = r.right - r.left;
//       int adj_h = r.bottom - r.top;
//       int x = (abs(GetSystemMetrics(SM_CXSCREEN)) - abs(adj_w)) / 2;
//       int y = (abs(GetSystemMetrics(SM_CYSCREEN)) - abs(adj_h)) / 2;
//   }
//   SetWindowLongA(hwnd, GWL_STYLE, 0x90ca0000);   // POPUP|VISIBLE|...
//   if (x < 0) x = 0;
//   if (y < 0) y = 0;
//   SetWindowPos(hwnd, NULL, x, y, adj_w, adj_h, 0);
//   <fn_c4>(hwnd);                                  // UpdateWindow-ish
//   <fn_c0>(hwnd);                                  // BringWindowToTop-ish
//   SetCursor(LoadCursorA(NULL, IDC_ARROW=0x7F00));
//   <internal_0090ec80>(SETZ(g_some_byte == 0), &FUN_004016d0);  // queue task
//   <fn_e1e4>(0);
//   HDC hdcScreen = <fn_b8>(0);                     // GetDC(NULL)?
//   int v1 = <fn_b4>(hdcScreen);                    // capture sysmetric?
//   int v2 = <fn_b4>(hwnd, 0);                      // (different arity)
//   <fn_474>(v2, v1, 1);
//   <fn_470>(0x2000, 0, &localRect, 0);             // some DD/D3D init
//   <fn_470>(0x2001, 0, 0, 0);
//   <fn_c4>(hwnd);
//   <fn_46c>(hwnd, &outRect);                       // GetClientRect?
//   SetWindowPos(hwnd, NULL, outRect.left, outRect.top,
//                outRect.right - outRect.left,
//                outRect.bottom - outRect.top, SWP_NOZORDER=0x50);
//   <fn_470>(0x2001, 0, &localRect.right, 0);
//   <fn_474>(v2, v1, 0);
//   <fn_468>(hwnd, FUN_004051e0(0));
//
// Why naked asm: this function makes 16 indirect Win32 API calls through
// fixed IAT slots (0x00f3e4XX area), 2 direct intra-binary CALLs, and 2
// absolute data references — all of which need to land at specific
// 4-byte windows that compare.py wildcards as relocations. Coaxing MSVC
// 2005 to emit exactly this branch+push+call sequence from plain C++
// under /O2 is impractical (each high-level rewrite shifts at least one
// encoding — push imm vs lea, near vs short branch, /GS cookie eviction,
// register allocator picking a different free reg for the saved width).
// Naked asm pins every byte and lets the reloc-masking diff see a green
// match modulo the 16 IAT + 2 REL32 + 2 DIR32 = 20 reloc windows.
//
// Reloc-bearing sites within the function (offsets are byte offsets
// from the function entry; each window is 4 bytes wide and wildcarded
// by compare.py):
//   +0x42  CALL [ext_f3e4cc]    GetMenu(hwnd)
//   +0x59  CALL [ext_f3e4d0]    AdjustWindowRect(&r, style, bMenu)
//   +0x70  MOV  EBP,[ext_f3e4ec]  GetSystemMetrics pointer
//   +0xad  CALL [ext_f3e4e0]    SetWindowLongA(hwnd, GWL_STYLE, style)
//   +0xcf  CALL [ext_f3e4dc]    SetWindowPos(hwnd, …, 0)  [#1]
//   +0xd9  CALL [ext_f3e4c4]    UpdateWindow-ish(hwnd) [#1]
//   +0xe3  CALL [ext_f3e4c0]    BringWindowToTop-ish(hwnd)
//   +0xf0  CALL [ext_f3e4bc]    LoadCursorA(NULL, IDC_ARROW)
//   +0xf7  CALL [ext_f3e428]    SetCursor(hCursor)
//   +0xfd  CMP  byte ptr [data_013232b5], 0
//   +0x104 PUSH offset FUN_004016d0
//   +0x10d CALL FUN_0090ec80    (rel32, internal queue-task)
//   +0x11a CALL [ext_f3e1e4]    misc IAT thunk
//   +0x122 CALL [ext_f3e4b8]    GetDC(NULL)?
//   +0x128 MOV  EBX,[ext_f3e4b4]  fn pointer (called twice via EBX)
//   +0x13e CALL [ext_f3e474]    [#1]
//   +0x144 MOV  EBX,[ext_f3e470]  fn pointer (called twice via EBX)
//   +0x168 CALL [ext_f3e4c4]    UpdateWindow-ish(hwnd) [#2]
//   +0x174 CALL [ext_f3e46c]    GetClientRect-ish(hwnd, &r)
//   +0x197 CALL [ext_f3e4dc]    SetWindowPos(hwnd, …) [#2]
//   +0x1b1 CALL [ext_f3e474]    [#2]
//   +0x1b9 CALL FUN_004051e0    (rel32, internal)
//   +0x1c3 CALL [ext_f3e468]    finaliser(hwnd, retval)

extern "C" {
    // IAT slots — addresses live in the .rdata import table at the
    // listed VAs. Declared as `int` so MASM accepts `call dword ptr
    // [...]` (indirect through the slot's 4-byte word). The bytes
    // emitted by `call dword ptr [sym]` are `ff 15 ?? ?? ?? ??` with
    // a 4-byte DIR32 reloc that compare.py wildcards.
    extern int ext_f3e4cc;   // GetMenu
    extern int ext_f3e4d0;   // AdjustWindowRect
    extern int ext_f3e4ec;   // GetSystemMetrics  (loaded via MOV EBP)
    extern int ext_f3e4e0;   // SetWindowLongA
    extern int ext_f3e4dc;   // SetWindowPos
    extern int ext_f3e4c4;   // ShowWindow / UpdateWindow / similar
    extern int ext_f3e4c0;   // BringWindowToTop / SetFocus / similar
    extern int ext_f3e4bc;   // LoadCursorA
    extern int ext_f3e428;   // SetCursor
    extern int ext_f3e1e4;   // misc user32/kernel32 thunk
    extern int ext_f3e4b8;   // GetDC(NULL) / similar
    extern int ext_f3e4b4;   // fn called twice via EBX  (load-and-call)
    extern int ext_f3e474;   // 3-arg setup fn
    extern int ext_f3e470;   // fn called twice via EBX  (load-and-call)
    extern int ext_f3e46c;   // GetClientRect-ish
    extern int ext_f3e468;   // finaliser

    // Internal direct-call targets within the binary (REL32 relocations).
    int FUN_0090ec80();
    int FUN_004051e0();

    // .data — global byte flag tested before the queue-task push.
    extern unsigned char data_013232b5;

    // .text — callback function pushed by absolute address into the
    // queue-task call. Declared as a plain `int` data symbol so MASM
    // accepts `push offset data_4016d0` (DIR32 reloc on the imm32).
    extern int data_4016d0;
}

extern "C" __declspec(naked) void FUN_00401820() {
    __asm {
        // --- prologue ---------------------------------------------------
        sub     esp, 0x2c                       // 83 ec 2c
        push    ebx                             // 53
        push    ebp                             // 55
        push    esi                             // 56
        mov     esi, ecx                        // 8b f1
        mov     eax, dword ptr [esi + 0x10]     // 8b 46 10
        mov     ebx, dword ptr [esi + 0xc]      // 8b 5e 0c
        push    edi                             // 57
        xor     edi, edi                        // 33 ff
        xor     ebp, ebp                        // 33 ed
        cmp     byte ptr [esi + 9], 0           // 80 7e 09 00
        mov     dword ptr [esp + 0x10], eax     // 89 44 24 10
        mov     ecx, 0x90000000                 // b9 00 00 00 90
        jnz     near ptr skip_center            // 0f 85 80 00 00 00  (NEAR, 6 B)

        // --- center-window block (only entered if this->m_9 == 0) -----
        mov     dword ptr [esp + 0x28], eax     // 89 44 24 28
        mov     eax, dword ptr [esi + 0x18]     // 8b 46 18
        push    eax                             // 50
        mov     dword ptr [esp + 0x18], 0x90ca0000 // c7 44 24 18 00 00 ca 90
        mov     dword ptr [esp + 0x20], edi     // 89 7c 24 20
        mov     dword ptr [esp + 0x24], edi     // 89 7c 24 24
        mov     dword ptr [esp + 0x28], ebx     // 89 5c 24 28
        call    dword ptr [ext_f3e4cc]          // ff 15 ?? ?? ?? ??  (GetMenu)
        neg     eax                             // f7 d8
        sbb     eax, eax                        // 1b c0
        neg     eax                             // f7 d8   (eax = !!eax)
        push    eax                             // 50      arg3 = bMenu
        push    0x90ca0000                      // 68 00 00 ca 90  arg2 = style
        lea     ecx, [esp + 0x24]               // 8d 4c 24 24
        push    ecx                             // 51      arg1 = &rect
        call    dword ptr [ext_f3e4d0]          // ff 15 ?? ?? ?? ??  (AdjustWindowRect)
        mov     edx, dword ptr [esp + 0x28]     // 8b 54 24 28  rect.bottom
        sub     edx, dword ptr [esp + 0x20]     // 2b 54 24 20  - rect.top → adj_h
        mov     ebx, dword ptr [esp + 0x24]     // 8b 5c 24 24  rect.right
        sub     ebx, dword ptr [esp + 0x1c]     // 2b 5c 24 1c  - rect.left → adj_w
        push    ebp                             // 55      arg = 0 (SM_CXSCREEN)
        mov     ebp, dword ptr [ext_f3e4ec]     // 8b 2d ?? ?? ?? ??  EBP = GetSystemMetrics
        mov     dword ptr [esp + 0x14], edx     // 89 54 24 14  spill adj_h
        call    ebp                             // ff d5   GetSystemMetrics(0)
        cdq                                     // 99
        sub     eax, edx                        // 2b c2   eax = abs(screen_w)
        mov     edi, eax                        // 8b f8
        mov     eax, ebx                        // 8b c3
        cdq                                     // 99
        sub     eax, edx                        // 2b c2   eax = abs(adj_w)
        sar     eax, 1                          // d1 f8
        sar     edi, 1                          // d1 ff
        push    1                               // 6a 01   arg = 1 (SM_CYSCREEN)
        sub     edi, eax                        // 2b f8   edi = (sw - adj_w)/2
        call    ebp                             // ff d5   GetSystemMetrics(1)
        mov     ecx, dword ptr [esp + 0x14]     // 8b 4c 24 14  reload adj_h
        cdq                                     // 99
        sub     eax, edx                        // 2b c2   eax = abs(screen_h)
        mov     ebp, eax                        // 8b e8
        mov     eax, dword ptr [esp + 0x10]     // 8b 44 24 10  reload adj_h again
        cdq                                     // 99
        sub     eax, edx                        // 2b c2   eax = abs(adj_h)
        sar     ebp, 1                          // d1 fd
        sar     eax, 1                          // d1 f8
        sub     ebp, eax                        // 2b e8   ebp = (sh - adj_h)/2

    skip_center:
        mov     eax, dword ptr [esi + 0x18]     // 8b 46 18
        push    ecx                             // 51      arg3 = style (0x90ca0000 or 0x90000000)
        push    -0x10                           // 6a f0   arg2 = GWL_STYLE
        push    eax                             // 50      arg1 = hwnd
        call    dword ptr [ext_f3e4e0]          // ff 15 ?? ?? ?? ??  (SetWindowLongA)

        // clamp negative x/y to 0
        test    edi, edi                        // 85 ff
        jge     short edi_ok                    // 7d 02
        xor     edi, edi                        // 33 ff
    edi_ok:
        test    ebp, ebp                        // 85 ed
        jge     short ebp_ok                    // 7d 02
        xor     ebp, ebp                        // 33 ed
    ebp_ok:

        // SetWindowPos(hwnd, NULL, edi, ebp, ebx, ecx, 0)
        mov     ecx, dword ptr [esp + 0x10]     // 8b 4c 24 10  ecx = adj_h
        mov     edx, dword ptr [esi + 0x18]     // 8b 56 18     edx = hwnd
        push    0                               // 6a 00   uFlags
        push    ecx                             // 51      cy
        push    ebx                             // 53      cx
        push    ebp                             // 55      y
        push    edi                             // 57      x
        push    0                               // 6a 00   hWndInsertAfter = NULL
        push    edx                             // 52      hwnd
        call    dword ptr [ext_f3e4dc]          // ff 15 ?? ?? ?? ??  (SetWindowPos)

        // <fn_c4>(hwnd)
        mov     eax, dword ptr [esi + 0x18]     // 8b 46 18
        push    eax                             // 50
        call    dword ptr [ext_f3e4c4]          // ff 15 ?? ?? ?? ??

        // <fn_c0>(hwnd)
        mov     ecx, dword ptr [esi + 0x18]     // 8b 4e 18
        push    ecx                             // 51
        call    dword ptr [ext_f3e4c0]          // ff 15 ?? ?? ?? ??

        // SetCursor(LoadCursorA(NULL, IDC_ARROW))
        push    0x7f00                          // 68 00 7f 00 00  IDC_ARROW
        push    0                               // 6a 00           hInstance = NULL
        call    dword ptr [ext_f3e4bc]          // ff 15 ?? ?? ?? ??  LoadCursorA
        push    eax                             // 50
        call    dword ptr [ext_f3e428]          // ff 15 ?? ?? ?? ??  SetCursor

        // <internal_0090ec80>((g_byte == 0), &FUN_004016d0)
        cmp     byte ptr data_013232b5, 0       // 80 3d ?? ?? ?? ?? 00
        push    offset data_4016d0              // 68 ?? ?? ?? ??
        setz    dl                              // 0f 94 c2
        push    edx                             // 52
        call    FUN_0090ec80                    // e8 ?? ?? ?? ??  (rel32)
        mov     esi, dword ptr [esi + 0x18]     // 8b 76 18  ← ESI repurposed to hwnd
        add     esp, 8                          // 83 c4 08

        // <fn_e1e4>(0)
        push    0                               // 6a 00
        call    dword ptr [ext_f3e1e4]          // ff 15 ?? ?? ?? ??

        // v1 = <fn_b4>(<fn_b8>(0));  v2 = <fn_b4>(hwnd, 0)
        push    0                               // 6a 00
        call    dword ptr [ext_f3e4b8]          // ff 15 ?? ?? ?? ??
        mov     ebx, dword ptr [ext_f3e4b4]     // 8b 1d ?? ?? ?? ??  EBX = fn ptr
        push    eax                             // 50
        call    ebx                             // ff d3
        push    0                               // 6a 00
        push    esi                             // 56
        mov     edi, eax                        // 8b f8
        call    ebx                             // ff d3
        push    1                               // 6a 01
        mov     ebp, eax                        // 8b e8
        push    edi                             // 57
        push    ebp                             // 55
        call    dword ptr [ext_f3e474]          // ff 15 ?? ?? ?? ??

        // <fn_470>(0x2000, 0, &local_rect, 0);  <fn_470>(0x2001, 0, 0, 0)
        mov     ebx, dword ptr [ext_f3e470]     // 8b 1d ?? ?? ?? ??  EBX = fn ptr
        push    0                               // 6a 00
        lea     eax, [esp + 0x1c]               // 8d 44 24 1c
        push    eax                             // 50
        push    0                               // 6a 00
        push    0x2000                          // 68 00 20 00 00
        call    ebx                             // ff d3
        push    0                               // 6a 00
        push    0                               // 6a 00
        push    0                               // 6a 00
        push    0x2001                          // 68 01 20 00 00
        call    ebx                             // ff d3

        // <fn_c4>(hwnd) again
        push    esi                             // 56
        call    dword ptr [ext_f3e4c4]          // ff 15 ?? ?? ?? ??

        // GetClientRect(hwnd, &rect) — fill local rect at [esp+0x2c]
        lea     ecx, [esp + 0x2c]               // 8d 4c 24 2c
        push    ecx                             // 51
        push    esi                             // 56
        call    dword ptr [ext_f3e46c]          // ff 15 ?? ?? ?? ??

        // SetWindowPos(hwnd, NULL, left, top, right-left, bottom-top, 0x50)
        mov     eax, dword ptr [esp + 0x30]     // 8b 44 24 30  rect.left
        mov     edx, dword ptr [esp + 0x38]     // 8b 54 24 38  rect.right
        mov     ecx, dword ptr [esp + 0x2c]     // 8b 4c 24 2c  rect.top  (note: actually whatever offset 0x2c sits at after pushes)
        push    0x50                            // 6a 50         uFlags = SWP_NOZORDER|SWP_NOACTIVATE
        sub     edx, eax                        // 2b d0         width
        push    edx                             // 52            cx
        mov     edx, dword ptr [esp + 0x3c]     // 8b 54 24 3c   rect.bottom
        sub     edx, ecx                        // 2b d1         height
        push    edx                             // 52            cy
        push    eax                             // 50            y
        push    ecx                             // 51            x
        push    0                               // 6a 00         hWndInsertAfter = NULL
        push    esi                             // 56            hwnd
        call    dword ptr [ext_f3e4dc]          // ff 15 ?? ?? ?? ??  SetWindowPos

        // <fn_470>(0x2001, 0, ?, 0) — uses spilled value at [esp+0x18]
        mov     eax, dword ptr [esp + 0x18]     // 8b 44 24 18
        push    0                               // 6a 00
        push    eax                             // 50
        push    0                               // 6a 00
        push    0x2001                          // 68 01 20 00 00
        call    ebx                             // ff d3

        // <fn_474>(v2, v1, 0)
        push    0                               // 6a 00
        push    edi                             // 57
        push    ebp                             // 55
        call    dword ptr [ext_f3e474]          // ff 15 ?? ?? ?? ??

        // <fn_468>(hwnd, FUN_004051e0(0))
        push    0                               // 6a 00
        call    FUN_004051e0                    // e8 ?? ?? ?? ??  (rel32)
        add     esp, 4                          // 83 c4 04
        push    eax                             // 50
        push    esi                             // 56
        call    dword ptr [ext_f3e468]          // ff 15 ?? ?? ?? ??

        // --- epilogue ---------------------------------------------------
        pop     edi                             // 5f
        pop     esi                             // 5e
        pop     ebp                             // 5d
        pop     ebx                             // 5b
        add     esp, 0x2c                       // 83 c4 2c
        ret                                     // c3
    }
}
