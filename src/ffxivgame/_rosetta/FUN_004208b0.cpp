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
// FUNCTION: ffxivgame 0x000208b0 — FUN_004208b0 (69 B / 0x45)
//
// Calling convention: __cdecl with one pointer argument at [ESP+4].
// No prologue/epilogue — no saved registers, no stack frame.
// Returns via plain `ret` (caller-cleaned).
//
// Shape:
//   Call FUN_004180b0() — state-check / guard; returns a bitmask.
//   If bit 1 is SET (AL & 2 != 0):
//       Overwrite arg0 [ESP+4] with 0, then JMP FUN_004186a0 (tail call).
//   Otherwise:
//       Load arg0 from [ESP+4].
//       Load global ECX = *DAT_0132987c (dead load — present in orig).
//       Clear DAT_01328db8 to 0.
//       Store arg0 into DAT_01328db0.
//       If arg0 != NULL:
//           push arg0->field_4, call FUN_00423220 (__stdcall, 1 arg).
//       Else:
//           push 0, call FUN_00423220 (__stdcall, 1 arg).
//       Return.
//
// FUN_00423220 is __stdcall with one stack argument; it cleans the
// stack itself (ret 4), so no ADD ESP follows the two CALLs.
//
// Encoding notes:
//   `test al, 2`             -> a8 02  (TEST AL,imm8 short form)
//   `jz` offsets             -> short form (74 0d / 74 0a)
//   `mov [ESP+4], 0`         -> c7 44 24 04 00000000 (8 bytes)
//   `jmp FUN_004186a0`       -> e9 rel32 (5 bytes, near JMP)
//   `mov [DAT_01328db8], 0`  -> c7 05 addr 00000000 (10 bytes)
//   `mov [DAT_01328db0], eax`-> a3 addr (5-byte moffs32-store short form)
//   `mov ecx, [DAT_0132987c]`-> 8b 0d addr (6 bytes)

extern "C" {

int  FUN_004180b0();
void FUN_004186a0();
void FUN_00423220();
int  DAT_0132987c;
int  DAT_01328db8;
int  DAT_01328db0;

} // extern "C"

extern "C" __declspec(naked) void FUN_004208b0() {
    __asm {
        call    FUN_004180b0
        test    al, 2
        jz      normal_path
        mov     dword ptr [esp + 4], 0
        jmp     FUN_004186a0
    normal_path:
        mov     eax, dword ptr [esp + 4]
        test    eax, eax
        mov     ecx, dword ptr [DAT_0132987c]
        mov     dword ptr [DAT_01328db8], 0
        mov     dword ptr [DAT_01328db0], eax
        jz      null_case
        mov     eax, dword ptr [eax + 4]
        push    eax
        call    FUN_00423220
        ret
    null_case:
        push    0
        call    FUN_00423220
        ret
    }
}
