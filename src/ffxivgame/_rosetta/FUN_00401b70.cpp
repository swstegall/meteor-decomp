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
// FUNCTION: ffxivgame 0x00001b70 — modal Yes/No prompt with WH_CBT hook
// (__cdecl int FUN_00401b70(HWND parent), 172 B).
//
// Behaviour (recovered from asm @ 0x00001b70):
//
//   int FUN_00401b70(HWND parent) {
//       DWORD tid          = GetCurrentThreadId();
//       g_mouse_hook       = SetWindowsHookExA(WH_CBT /* 5 */,
//                                              (HOOKPROC) FUN_00401350,
//                                              NULL, tid);
//       const wchar_t *cap = (const wchar_t *) FUN_00401020(4); // load_text
//       const wchar_t *txt = (const wchar_t *) FUN_00401020(5);
//       if (g_dlg) {
//           FUN_00418210(1);                 // with_lock(1)
//           if (g_dlg->timer_active != 1) {
//               SetTimer(g_dlg->hwnd, 1, 0x10, (TIMERPROC) FUN_00401730);
//               g_dlg->timer_active = 1;
//           }
//           int result = MessageBoxW(parent, txt, cap, MB_YESNO);
//           if (g_dlg->timer_active != 0) {
//               KillTimer(g_dlg->hwnd, 1);
//               g_dlg->timer_active = 0;
//           }
//           FUN_00418210(0);                 // with_lock(0)
//           return result;
//       }
//       return 0;
//   }
//
// Globals touched:
//   g_mouse_hook  @ .data 0x01323288  — HHOOK (return of SetWindowsHookExA)
//   g_dlg         @ .data 0x013232b8  — struct {
//                                            ... ;
//                                            u8   timer_active; // +0x0a
//                                            ... ;
//                                            HWND hwnd;         // +0x18
//                                       } *
//
// Why naked asm: an earlier worker exhausted the source-level loop on
// this function (see
// `decomp-notes/blocked/ffxivgame/0x00001b70_FUN_00401b70.md`). Plain
// C++ under /O2 reproduces every structural detail of the orig
// (branches, frame, IAT call sequence, /GS pattern) but lands a stable
// PARTIAL at 52.3% raw bytes because MSVC's register allocator picks
// EBX ↔ ESI for the three callee-saved slots differently than orig.
// Nine iterations of declaration-order and form variations on the
// source loop failed to flip the tiebreaker. The orig's allocator
// state in the full-binary build is unrecoverable from an isolated TU,
// so we fall back to byte-pinned naked asm (the path suggested by hint
// #4 of the post-mortem). compare.py wildcards relocation windows, so
// the seven external addresses (4 IAT slots + 1 PUSH offset of a
// HOOKPROC + 1 PUSH offset of a TIMERPROC + the 4 internal rel32
// CALLs into FUN_00401020/_01350/_01730/_18210 + the 4 DIR32 globals)
// don't need to land at orig addresses — only the opcode/modrm bytes
// around them do.
//
// Reloc-bearing sites (offsets within the function — each 4-byte
// window is wildcarded by compare.py against orig):
//   +0x04   CALL [ext_f3e1dc]    GetCurrentThreadId
//   +0x0d   PUSH offset FUN_00401350   (HOOKPROC for WH_CBT)
//   +0x14   CALL [ext_f3e460]    SetWindowsHookExA
//   +0x1b   MOV  [data_01323288], EAX
//   +0x20   CALL FUN_00401020    (load_text, JMP-thunk)
//   +0x29   CALL FUN_00401020    (load_text, JMP-thunk)
//   +0x32   CMP  [data_013232b8], 0
//   +0x3f   CALL FUN_00418210    (with_lock(1), JMP-thunk)
//   +0x44   MOV  EAX, [data_013232b8]
//   +0x57   PUSH offset FUN_00401730   (TIMERPROC for SetTimer)
//   +0x62   CALL [ext_f3e464]    SetTimer
//   +0x74   CALL [ext_f3e478]    MessageBoxW
//   +0x7b   MOV  EAX, [data_013232b8]
//   +0x90   CALL [ext_f3e4c8]    KillTimer
//   +0x9a   CALL FUN_00418210    (with_lock(0), JMP-thunk)

extern "C" {
    // .idata — IAT slots. Declared as plain `int` so MASM accepts
    // `call dword ptr [ext_f3eXXX]` — the assembler emits
    // `ff 15 ?? ?? ?? ??` with a DIR32 reloc on the imm32 slot address.
    extern int ext_f3e1dc;   // kernel32!GetCurrentThreadId
    extern int ext_f3e460;   // user32!SetWindowsHookExA
    extern int ext_f3e464;   // user32!SetTimer
    extern int ext_f3e478;   // user32!MessageBoxW
    extern int ext_f3e4c8;   // user32!KillTimer

    // .text — internal direct-call targets within the binary (REL32).
    // `FUN_00401020` and `FUN_00418210` are JMP-thunks (size 5,
    // `jmp rel32`) into FUN_00440a90 / FUN_0041bd30 respectively.
    int FUN_00401020();
    int FUN_00401350();   // pushed as offset — HOOKPROC for WH_CBT
    int FUN_00401730();   // pushed as offset — TIMERPROC for SetTimer
    int FUN_00418210();

    // .data — global byte/dword slots.
    extern int data_01323288;   // HHOOK g_mouse_hook
    extern int data_013232b8;   // struct dlg *g_dlg
}

extern "C" __declspec(naked) void FUN_00401b70() {
    __asm {
        // --- prologue ---------------------------------------------------
        push    ebx                                 // 53
        push    edi                                 // 57

        // tid = GetCurrentThreadId()
        call    dword ptr [ext_f3e1dc]              // ff 15 ?? ?? ?? ??

        // g_mouse_hook = SetWindowsHookExA(WH_CBT, FUN_00401350, NULL, tid)
        push    eax                                 // 50   dwThreadId
        push    0                                   // 6a 00  hMod
        push    offset FUN_00401350                 // 68 ?? ?? ?? ??  lpfn
        push    5                                   // 6a 05  WH_CBT
        call    dword ptr [ext_f3e460]              // ff 15 ?? ?? ?? ??

        // cap = FUN_00401020(4); txt = FUN_00401020(5)
        push    4                                   // 6a 04
        mov     [data_01323288], eax                // a3 ?? ?? ?? ??
        call    FUN_00401020                        // e8 ?? ?? ?? ??
        push    5                                   // 6a 05
        mov     edi, eax                            // 8b f8  EDI = cap
        call    FUN_00401020                        // e8 ?? ?? ?? ??
        add     esp, 8                              // 83 c4 08

        // if (g_dlg == 0) goto null_return
        cmp     dword ptr [data_013232b8], 0        // 83 3d ?? ?? ?? ?? 00
        mov     ebx, eax                            // 8b d8  EBX = txt
        jz      short null_return                   // 74 6c

        // FUN_00418210(1)
        push    esi                                 // 56  shrink-wrap save
        push    1                                   // 6a 01
        call    FUN_00418210                        // e8 ?? ?? ?? ??

        mov     eax, [data_013232b8]                // a1 ?? ?? ?? ??
        lea     esi, [eax + 0xa]                    // 8d 70 0a  ESI = &g_dlg->timer_active
        add     esp, 4                              // 83 c4 04
        cmp     byte ptr [esi], 1                   // 80 3e 01
        jz      short after_settimer                // 74 16

        // SetTimer(g_dlg->hwnd, 1, 0x10, FUN_00401730)
        mov     eax, dword ptr [eax + 0x18]         // 8b 40 18  EAX = g_dlg->hwnd
        push    offset FUN_00401730                 // 68 ?? ?? ?? ??
        push    0x10                                // 6a 10
        push    1                                   // 6a 01
        push    eax                                 // 50
        call    dword ptr [ext_f3e464]              // ff 15 ?? ?? ?? ??
        mov     byte ptr [esi], 1                   // c6 06 01

    after_settimer:
        // result = MessageBoxW(parent, txt, cap, MB_YESNO)
        mov     ecx, dword ptr [esp + 0x10]         // 8b 4c 24 10  ECX = parent (arg)
        push    4                                   // 6a 04  MB_YESNO
        push    edi                                 // 57     lpCaption
        push    ebx                                 // 53     lpText
        push    ecx                                 // 51     hWnd
        call    dword ptr [ext_f3e478]              // ff 15 ?? ?? ?? ??
        mov     edi, eax                            // 8b f8  EDI = result

        mov     eax, [data_013232b8]                // a1 ?? ?? ?? ??
        cmp     byte ptr [eax + 0xa], 0             // 80 78 0a 00
        lea     esi, [eax + 0xa]                    // 8d 70 0a
        jz      short after_killtimer               // 74 0f

        // KillTimer(g_dlg->hwnd, 1)
        mov     edx, dword ptr [eax + 0x18]         // 8b 50 18
        push    1                                   // 6a 01
        push    edx                                 // 52
        call    dword ptr [ext_f3e4c8]              // ff 15 ?? ?? ?? ??
        mov     byte ptr [esi], 0                   // c6 06 00

    after_killtimer:
        // FUN_00418210(0)
        push    0                                   // 6a 00
        call    FUN_00418210                        // e8 ?? ?? ?? ??
        add     esp, 4                              // 83 c4 04

        // return result
        pop     esi                                 // 5e
        mov     eax, edi                            // 8b c7
        pop     edi                                 // 5f
        pop     ebx                                 // 5b
        ret                                         // c3

    null_return:
        pop     edi                                 // 5f
        xor     eax, eax                            // 33 c0
        pop     ebx                                 // 5b
        ret                                         // c3
    }
}
