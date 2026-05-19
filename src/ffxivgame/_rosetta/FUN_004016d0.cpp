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
// FUNCTION: ffxivgame 0x004016d0 — `SetCursor` dispatcher.
//
// Single `bool` arg lives at `[esp + 4]`. If the bool is false (or any
// of the gating checks fail) we fall through to the tail block that
// calls `user32!SetCursor(NULL)` to restore the default arrow. If the
// bool is true AND the engine is running AND the UI root is alive AND
// the helper at 0x00403b20 finds the current cursor key in the
// 11-entry (string, value) lookup table at 0x01265018 AND the looked-
// up value is non-null, we dereference it (one extra indirection — the
// table holds `HCURSOR*`, not `HCURSOR`) and pass that to `SetCursor`.
//
// MSVC 2005 /O2 elides the frame entirely and reuses the parameter
// slot at `[esp + 4]` as the out-buffer that FUN_00403b20 writes the
// looked-up `HCURSOR*` into. The `LEA ECX,[ESP+4]; PUSH ECX` pair
// stashes that out-buffer pointer on the stack BEFORE setting up `this`
// for the FUN_0090ea30 thiscall — the pushed ECX is then consumed as
// FUN_00403b20's 4th argument after the intervening three pushes.
//
// Asm (84 bytes / 0x54):
//   80 7c 24 04 00       CMP byte ptr [ESP+4], 0
//   74 44                JZ  fail
//   e8 RR RR RR RR       CALL FUN_0090ed60     ; engine-running flag (al)
//   84 c0                TEST AL, AL
//   74 3b                JZ  fail
//   e8 RR RR RR RR       CALL FUN_0090ece0     ; ui root (eax = obj or 0)
//   85 c0                TEST EAX, EAX
//   74 32                JZ  fail
//   8d 4c 24 04          LEA  ECX, [ESP+4]     ; out-buffer = &slot
//   51                   PUSH ECX              ; will become 4th arg below
//   8b c8                MOV  ECX, EAX         ; this = ui-root
//   e8 RR RR RR RR       CALL FUN_0090ea30     ; returns this->field_0x228
//   50                   PUSH EAX              ; cursor key
//   6a 0b                PUSH 0x0B             ; table length
//   68 RR RR RR RR       PUSH offset g_cursor_lookup_table
//   e8 RR RR RR RR       CALL FUN_00403b20     ; (table, 11, key, &out)
//   83 c4 10             ADD  ESP, 0x10        ; pop 4 args (cdecl)
//   3c 01                CMP  AL, 1
//   75 12                JNZ  fail
//   8b 44 24 04          MOV  EAX, [ESP+4]     ; reload out-buffer slot
//   85 c0                TEST EAX, EAX
//   74 0a                JZ   fail
//   8b 10                MOV  EDX, [EAX]       ; HCURSOR = *HCURSOR*
//   52                   PUSH EDX
//   ff 15 RR RR RR RR    CALL [user32!SetCursor]
//   c3                   RET
// fail:
//   6a 00                PUSH 0
//   ff 15 RR RR RR RR    CALL [user32!SetCursor]
//   c3                   RET

extern "C" int FUN_0090ed60();
extern "C" int FUN_0090ece0();
extern "C" int FUN_0090ea30();
extern "C" int FUN_00403b20();
extern "C" int g_cursor_lookup_table;    // .data @ 0x01265018 — 11 (str, HCURSOR*) pairs
extern "C" int g_imp_SetCursor;          // .rdata IAT slot @ 0x00f3e428

extern "C" __declspec(naked) void FUN_004016d0() {
    __asm {
        cmp     byte ptr [esp + 4], 0
        jz      fail
        call    FUN_0090ed60
        test    al, al
        jz      fail
        call    FUN_0090ece0
        test    eax, eax
        jz      fail
        lea     ecx, [esp + 4]
        push    ecx
        mov     ecx, eax
        call    FUN_0090ea30
        push    eax
        push    0Bh
        push    offset g_cursor_lookup_table
        call    FUN_00403b20
        add     esp, 10h
        cmp     al, 1
        jnz     fail
        mov     eax, dword ptr [esp + 4]
        test    eax, eax
        jz      fail
        mov     edx, dword ptr [eax]
        push    edx
        call    dword ptr [g_imp_SetCursor]
        ret
    fail:
        push    0
        call    dword ptr [g_imp_SetCursor]
        ret
    }
}
