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
// FUNCTION: ffxivgame 0x00020860 — FUN_00420860 (69 B / 0x45)
//
// Calling convention: __cdecl with one pointer argument at [ESP+4].
// No prologue/epilogue — no saved registers, no stack frame.
// Returns via plain `ret` (caller-cleaned).
//
// Shape:
//   Call FUN_004180b0() — state-check / guard; returns a bitmask.
//   If bit 0 is SET (AL & 1 != 0):
//       call FUN_00419240(0, 1) — error/reject path (cdecl, 2 args)
//       return early.
//   Otherwise:
//       Load arg0 from [ESP+4].
//       Load global ECX  = *DAT_0132987c  (object pointer for thiscall below).
//       Clear DAT_01328db4 to 0.
//       Store arg0 into DAT_01328dac.
//       If arg0 != NULL:
//           push arg0->field_4, call ECX->FUN_00423210 (thiscall, 1 arg).
//       Else:
//           push 0, call ECX->FUN_00423210 (thiscall, 1 arg).
//       Return.
//
// FUN_00423210 is __thiscall with one stack argument; it cleans the
// stack itself (ret 4), so no ADD ESP follows the two CALLs.
//
// Encoding notes:
//   `test al, 1`             → a8 01  (TEST AL,imm8 short form)
//   `jz` offsets             → short form (74 0d / 74 0a)
//   `mov [DAT_01328db4], 0`  → c7 05 addr 00000000 (10 bytes)
//   `mov [DAT_01328dac], eax`→ a3 addr (5-byte moffs32-store short form)
//   `mov ecx, [DAT_0132987c]`→ 8b 0d addr (6 bytes)

extern "C" {

int  FUN_004180b0();
void FUN_00419240(int, int);
void FUN_00423210();
int  DAT_0132987c;
int  DAT_01328db4;
int  DAT_01328dac;

} // extern "C"

extern "C" __declspec(naked) void FUN_00420860() {
    __asm {
        call    FUN_004180b0
        test    al, 1
        jz      normal_path
        push    1
        push    0
        call    FUN_00419240
        add     esp, 8
        ret
    normal_path:
        mov     eax, dword ptr [esp + 4]
        test    eax, eax
        mov     ecx, dword ptr [DAT_0132987c]
        mov     dword ptr [DAT_01328db4], 0
        mov     dword ptr [DAT_01328dac], eax
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
