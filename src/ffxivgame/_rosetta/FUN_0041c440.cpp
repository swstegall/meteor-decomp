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
// FUNCTION: ffxivgame 0x0001c440 — FUN_0041c440 (32 B / 0x20)
//
// Calling convention: __cdecl, one pointer argument at [ESP+4].
// No prologue/epilogue — no saved registers, no stack frame.
// Returns via plain `ret` (caller-cleaned).
//
// Shape:
//   Load arg0 from [ESP+4].
//   Load global ECX = *DAT_0132987c (object pointer for thiscall callee).
//   If arg0 != NULL:
//       push arg0->field_4, call ECX->FUN_00423210 (__thiscall, 1 stack arg).
//   Else:
//       push 0, call ECX->FUN_00423210 (__thiscall, 1 stack arg).
//
// FUN_00423210 is __thiscall with one stack argument; it cleans the
// stack itself (ret 4), so no ADD ESP follows either CALL.
//
// Asm (32 bytes):
//   8b 44 24 04        MOV  EAX, [ESP+4]         ; arg0
//   85 c0              TEST EAX, EAX
//   8b 0d 7c 98 32 01  MOV  ECX, [DAT_0132987c]  ; thiscall receiver
//   74 0a              JZ   null_case             ; +0x0a
//   8b 40 04           MOV  EAX, [EAX+4]         ; arg0->field_4
//   50                 PUSH EAX
//   e8 b9 6d 00 00     CALL FUN_00423210
//   c3                 RET
//   6a 00              PUSH 0
//   e8 b1 6d 00 00     CALL FUN_00423210
//   c3                 RET

extern "C" {

void FUN_00423210();
int  DAT_0132987c;

} // extern "C"

extern "C" __declspec(naked) void FUN_0041c440() {
    __asm {
        mov     eax, dword ptr [esp + 4]
        test    eax, eax
        mov     ecx, dword ptr [DAT_0132987c]
        jz      null_case
        mov     eax, dword ptr [eax + 4]
        push    eax
        call    FUN_00423210
        ret
    null_case:
        push    0
        call    FUN_00423210
        ret
    }
}
