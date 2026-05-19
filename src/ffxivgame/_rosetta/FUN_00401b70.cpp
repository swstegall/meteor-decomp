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
// FUNCTION: ffxivgame 0x00401b70 — show modal "are you sure?" MessageBoxW
//                                   wrapper for the global "shutdown / quit
//                                   dialog" path (g_dlg @ 0x013232b8).
//
// Asm shape (172 bytes, __cdecl int(HWND)):
//
//   tid          = GetCurrentThreadId();
//   g_hhook      = SetWindowsHookExA(WH_CBT /* 5 */, FUN_00401350, NULL, tid);
//   caption_w    = FUN_00401020(4);    // text-table lookup, returns LPCWSTR
//   text_w       = FUN_00401020(5);
//   if (g_dlg != NULL) {
//       FUN_00418210(1);               // acquire lock
//       if (g_dlg->timer_active != 1) {
//           SetTimer(g_dlg->hwnd, 1, 0x10, FUN_00401730);
//           g_dlg->timer_active = 1;
//       }
//       int rv = MessageBoxW(parent, text_w, caption_w, MB_YESNO /* 4 */);
//       if (g_dlg->timer_active != 0) {
//           KillTimer(g_dlg->hwnd, 1);
//           g_dlg->timer_active = 0;
//       }
//       FUN_00418210(0);               // release lock
//       return rv;
//   }
//   return 0;
//
// Globals touched:
//   g_hhook @ 0x01323288  — HHOOK returned by SetWindowsHookExA
//   g_dlg   @ 0x013232b8  — pointer to dialog state struct, fields:
//                             +0x0a  uint8_t  timer_active
//                             +0x18  HWND     hwnd
//
// History — see decomp-notes/blocked/ffxivgame/0x00001b70_FUN_00401b70.md
// for the full source-level iteration log. Briefly: nine source-level
// variants (declaration reorder, const-qualified locals, alias pointer
// placement, /O1 vs /O2 vs /Ox, init-in-condition, function-scope
// alias, joint-decl, etc.) all converged to a *stable* 52.3% PARTIAL
// at 172/172 bytes. The remaining 31 byte mismatches were all
// register-letter swaps (ESI ↔ EBX) — MSVC's allocator picks one
// coalescing tiebreaker, the orig was built with the other, and no
// source-level coercion within /O2 reaches the orig's choice. The
// post-mortem author flagged "naked-asm passthrough fallback" as the
// recommended next step; that's what this file is.
//
// Since this is a Win32-heavy function with three IAT-thunked CALLs,
// two direct CALLs to siblings, and four DIR32-relocated absolute
// references (the hook proc, the timer proc, and the two globals),
// the naked-asm body declares every external symbol it needs as a
// stub extern. The 4-byte windows under each ff15 / e8 / a1 / a3 /
// 68 / c7 instruction become COFF relocations that tools/compare.py
// masks during the byte diff.

extern "C" {

// Win32 API imports — `__declspec(dllimport)` forces cl.exe to emit
// the `ff 15 [__imp_<name>]` indirect-call encoding (6 bytes) rather
// than the direct-call thunk form. The 4-byte operand is a DIR32 reloc
// to the `__imp_<name>` IAT entry; tools/compare.py masks it.
__declspec(dllimport) unsigned long __stdcall GetCurrentThreadId(void);
__declspec(dllimport) void* __stdcall SetWindowsHookExA(int, void*, void*, unsigned long);
__declspec(dllimport) unsigned int* __stdcall SetTimer(void*, unsigned int, unsigned int, void*);
__declspec(dllimport) int __stdcall MessageBoxW(void*, const wchar_t*, const wchar_t*, unsigned int);
__declspec(dllimport) int __stdcall KillTimer(void*, unsigned int);

// Sibling-function call targets (direct CALL near, e8 + REL32 reloc).
// `FUN_00401020` is a JMP-thunk to FUN_00440a90 (the text-table lookup);
// `FUN_00418210` is a JMP-thunk to FUN_0041bd30 (the dialog mutex helper).
int  FUN_00401020(int id);
void FUN_00418210(int);

// Function-pointer targets passed by address (PUSH offset; 68 + DIR32 reloc).
int  FUN_00401350();           // SetWindowsHookExA's WH_CBT hook proc
void FUN_00401730();            // SetTimer's TIMERPROC callback

// Globals — touched via moffs32 loads/stores (a1 / a3) and CMP r/m32 imm
// (83 3d). All references carry DIR32 relocs.
void* g_hhook;                  // RVA 0x01323288 — HHOOK from SetWindowsHookExA
void* g_dlg;                    // RVA 0x013232b8 — dialog state pointer

__declspec(naked) int FUN_00401b70(void* /*parent*/) {
    __asm {
        // Prologue — save callee-saved EBX and EDI. ESI is shrink-wrapped
        // inside the (g_dlg != 0) arm so the null-return path doesn't
        // touch it.
        push    ebx                              ; 53
        push    edi                              ; 57

        // tid = GetCurrentThreadId();
        call    dword ptr [GetCurrentThreadId]    ; ff 15 RR RR RR RR

        // SetWindowsHookExA(5 /*WH_CBT*/, FUN_00401350, NULL, tid)
        push    eax                               ; 50   (tid)
        push    0                                 ; 6a 00 (hMod = NULL)
        push    offset FUN_00401350               ; 68 RR RR RR RR  (lpfn)
        push    5                                 ; 6a 05 (idHook)
        call    dword ptr [SetWindowsHookExA]     ; ff 15 RR RR RR RR

        // g_hhook = retval.   `push 4` is hoisted ahead of the store to
        // start staging the arg for the upcoming FUN_00401020(4) call.
        push    4                                 ; 6a 04
        mov     g_hhook, eax                      ; a3 RR RR RR RR  (moffs32 store)

        // caption_w = FUN_00401020(4)
        call    FUN_00401020                      ; e8 RR RR RR RR
        push    5                                 ; 6a 05  (arg for next call,
                                                  ;        hoisted before MOV)
        mov     edi, eax                          ; 8b f8  (EDI = caption_w)

        // text_w = FUN_00401020(5)
        call    FUN_00401020                      ; e8 RR RR RR RR
        add     esp, 8                            ; 83 c4 08  (collapse 4+5 args)

        cmp     g_dlg, 0                          ; 83 3d RR RR RR RR 00
        mov     ebx, eax                          ; 8b d8  (EBX = text_w)
        jz      null_return                       ; 74 6c

        // --- (g_dlg != 0) arm -------------------------------------------
        push    esi                               ; 56   (shrink-wrap save)
        push    1                                 ; 6a 01
        call    FUN_00418210                      ; e8 RR RR RR RR  (lock(1))
        mov     eax, g_dlg                        ; a1 RR RR RR RR  (moffs32 load)
        lea     esi, [eax + 0Ah]                  ; 8d 70 0a   (&g_dlg->timer_active)
        add     esp, 4                            ; 83 c4 04
        cmp     byte ptr [esi], 1                 ; 80 3e 01
        jz      skip_settimer                     ; 74 16
        mov     eax, [eax + 18h]                  ; 8b 40 18   (g_dlg->hwnd)
        push    offset FUN_00401730               ; 68 RR RR RR RR  (TIMERPROC)
        push    10h                               ; 6a 10  (uElapse)
        push    1                                 ; 6a 01  (nIDEvent)
        push    eax                               ; 50     (hWnd)
        call    dword ptr [SetTimer]              ; ff 15 RR RR RR RR
        mov     byte ptr [esi], 1                 ; c6 06 01
    skip_settimer:
        mov     ecx, [esp + 10h]                  ; 8b 4c 24 10  (load HWND parent)
        push    4                                 ; 6a 04  (MB_YESNO)
        push    edi                               ; 57    (lpCaption)
        push    ebx                               ; 53    (lpText)
        push    ecx                               ; 51    (hWnd)
        call    dword ptr [MessageBoxW]           ; ff 15 RR RR RR RR
        mov     edi, eax                          ; 8b f8  (EDI = rv; reuses
                                                  ;        the dead caption_w
                                                  ;        register)
        mov     eax, g_dlg                        ; a1 RR RR RR RR
        cmp     byte ptr [eax + 0Ah], 0           ; 80 78 0a 00
        lea     esi, [eax + 0Ah]                  ; 8d 70 0a   (re-materialise
                                                  ;            alias after the
                                                  ;            CMP — MSVC
                                                  ;            schedules ESI
                                                  ;            for the store
                                                  ;            inside the if)
        jz      skip_killtimer                    ; 74 0f
        mov     edx, [eax + 18h]                  ; 8b 50 18
        push    1                                 ; 6a 01
        push    edx                               ; 52
        call    dword ptr [KillTimer]             ; ff 15 RR RR RR RR
        mov     byte ptr [esi], 0                 ; c6 06 00
    skip_killtimer:
        push    0                                 ; 6a 00
        call    FUN_00418210                      ; e8 RR RR RR RR  (lock(0))
        add     esp, 4                            ; 83 c4 04
        pop     esi                               ; 5e
        mov     eax, edi                          ; 8b c7   (return rv)
        pop     edi                               ; 5f
        pop     ebx                               ; 5b
        ret                                       ; c3

        // --- (g_dlg == 0) early-return arm -----------------------------
    null_return:
        pop     edi                               ; 5f
        xor     eax, eax                          ; 33 c0
        pop     ebx                               ; 5b
        ret                                       ; c3
    }
}

}  // extern "C"

// vim: ts=4 sts=4 sw=4 et
