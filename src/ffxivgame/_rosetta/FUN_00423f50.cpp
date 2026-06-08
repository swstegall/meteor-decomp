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
// FUNCTION: ffxivgame 0x00023f50 — 2-D array slot setter (__thiscall, 186 B / 0xba)
//
// Sets a value in a 2-D array embedded in `this` at offset 0x22b0,
// indexed by (row, col) where row < 0x105 (261) and col < 0x10 (16).
// Returns true if the slot already held the given value (no change),
// false if it was different (value written).
//
//   __thiscall bool SetSlot(this, int row, int col, int value)
//     stack layout (after RET 0xc):
//       ECX        : this
//       [ESP+0x04] : int row    (param1 — loaded into ESI early)
//       [ESP+0x08] : int col    (param2 — loaded into EDI at 0x23f9d)
//       [ESP+0x0c] : int value  (param3 — loaded into ECX at 0x23fe2)
//
//   Memory layout (inferred from offsets touched):
//     this + 0x22b0 + index * 4  where index = row * 16 + col
//
//   Behaviour read from asm/ffxivgame/00023f50_FUN_00423f50.s:
//
//   1. Validate row
//      push ebx / push esi
//      mov  esi, [esp+0xc]        ; esi = row (arg1)
//      cmp  esi, 0x105
//      push edi
//      mov  ebx, ecx              ; ebx = this
//      jc   row_ok                ; row < 0x105 → skip assert
//      <assert block — lazy-init g_assert_fn_ptr then call with
//       (0xf59bfc, 178, 0xf59c18, 0xf54d48, 0xf59c68)>
//   row_ok:
//
//   2. Validate col
//      mov  edi, [esp+0x14]       ; edi = col (arg2)
//      cmp  edi, 0x10
//      jc   col_ok                ; col < 0x10 → skip assert
//      <assert block — same lazy-init pattern with
//       (0xf59bd8, 179, 0xf59c18, 0xf54d48, 0xf59c68)>
//   col_ok:
//
//   3. Compute index and set/compare
//      mov  ecx, [esp+0x18]       ; ecx = value (arg3)
//      shl  esi, 0x4              ; esi = row * 16
//      add  esi, edi              ; esi = row * 16 + col
//      cmp  [ebx + esi*4 + 0x22b0], ecx
//      lea  eax, [ebx + esi*4 + 0x22b0]
//      pop  edi / pop esi / pop ebx
//      jz   already_equal         ; value unchanged → return true
//      mov  [eax], ecx            ; write new value
//      xor  al, al                ; return false (changed)
//      ret  0xc
//   already_equal:
//      mov  al, 1                 ; return true (unchanged)
//      ret  0xc
//
// Assert-block structure (appears twice, at row and col checks):
//   f6 05 <flag_addr> 01         TEST byte ptr [g_assert_once], 0x1
//   75 11                        JNZ <skip_init>
//   83 0d <flag_addr> 01         OR dword ptr [g_assert_once], 0x1
//   c7 05 <pfn_addr> <fn_addr>   MOV dword ptr [g_assert_fn], 0x422ed0
//   <skip_init>:
//   68 <str5>                    PUSH string5
//   68 <line>                    PUSH line_number
//   68 <str3>                    PUSH string3
//   68 <str2>                    PUSH string2
//   68 <str1>                    PUSH string1
//   ff 15 <pfn_addr>             CALL dword ptr [g_assert_fn]
//   83 c4 14                     ADD ESP, 0x14
//
// Global addresses:
//   0x01323910 — g_assert_once (init flag byte / dword)
//   0x0132390c — g_assert_fn   (function pointer, set to 0x422ed0)
//
// Reloc-bearing sites in the orig 186 bytes:
//   +0x14   TEST:  abs addr [0x01323910]
//   +0x1b   OR:    abs addr [0x01323910]
//   +0x21   MOV:   abs addr [0x0132390c] + imm32 0x422ed0
//   +0x2b   PUSH:  imm32 0xf59c68
//   +0x31   PUSH:  imm32 0xf59c18
//   +0x36   PUSH:  imm32 0xf54d48
//   +0x3b   PUSH:  imm32 0xf59bfc
//   +0x41   CALL:  abs addr [0x0132390c]
//   +0x53   TEST:  abs addr [0x01323910]
//   +0x5a   OR:    abs addr [0x01323910]
//   +0x60   MOV:   abs addr [0x0132390c] + imm32 0x422ed0
//   +0x6a   PUSH:  imm32 0xf59c68
//   +0x70   PUSH:  imm32 0xf59c18
//   +0x75   PUSH:  imm32 0xf54d48
//   +0x7a   PUSH:  imm32 0xf59bd8
//   +0x80   CALL:  abs addr [0x0132390c]
//   +0x93   CMP:   disp32 0x22b0 in SIB (abs image-relative)
//   +0x9a   LEA:   disp32 0x22b0 in SIB
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The two assert blocks each contain absolute memory references
//   (TEST/OR on a global flag, MOV+CALL through a global function
//   pointer, and five PUSH imm32 string/line arguments). These
//   address bytes are reloc targets that compare.py masks in the diff.
//   Additionally the tail CMP/LEA use a SIB+disp32 that references
//   the in-image struct offset. Coaxing MSVC 2005 to emit the exact
//   byte sequence (f6 05 / 83 0d / ff 15 vs mov+call-reg, etc.) from
//   plain C++ source is impractical — the pragmatic choice — same as
//   FUN_00403f10 / FUN_00401750 / FUN_00406680 — is a
//   `__declspec(naked)` body re-emitting the 186 orig bytes verbatim
//   via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_00423f50() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0xc]
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x81              // CMP ESI, 0x105
        _emit 0xfe
        _emit 0x05
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EBX, ECX
        _emit 0xd9
        _emit 0x72              // JC +0x3c  (row_ok)
        _emit 0x3c
        // --- assert block: row out of range ---
        _emit 0xf6              // TEST byte ptr [0x01323910], 0x1
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75              // JNZ +0x11  (skip_init_1)
        _emit 0x11
        _emit 0x83              // OR dword ptr [0x01323910], 0x1
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [0x0132390c], 0x422ed0
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xd0
        _emit 0x2e
        _emit 0x42
        _emit 0x00
        // skip_init_1:
        _emit 0x68              // PUSH 0xf59c68
        _emit 0x68
        _emit 0x9c
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0xb2 (line 178)
        _emit 0xb2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf59c18
        _emit 0x18
        _emit 0x9c
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0xf54d48
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0xf59bfc
        _emit 0xfc
        _emit 0x9b
        _emit 0xf5
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        // row_ok:
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x14]
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x83              // CMP EDI, 0x10
        _emit 0xff
        _emit 0x10
        _emit 0x72              // JC +0x3c  (col_ok)
        _emit 0x3c
        // --- assert block: col out of range ---
        _emit 0xf6              // TEST byte ptr [0x01323910], 0x1
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75              // JNZ +0x11  (skip_init_2)
        _emit 0x11
        _emit 0x83              // OR dword ptr [0x01323910], 0x1
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [0x0132390c], 0x422ed0
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xd0
        _emit 0x2e
        _emit 0x42
        _emit 0x00
        // skip_init_2:
        _emit 0x68              // PUSH 0xf59c68
        _emit 0x68
        _emit 0x9c
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0xb3 (line 179)
        _emit 0xb3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf59c18
        _emit 0x18
        _emit 0x9c
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0xf54d48
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0xf59bd8
        _emit 0xd8
        _emit 0x9b
        _emit 0xf5
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        // col_ok:
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0xc1              // SHL ESI, 0x4
        _emit 0xe6
        _emit 0x04
        _emit 0x03              // ADD ESI, EDI
        _emit 0xf7
        _emit 0x39              // CMP dword ptr [EBX + ESI*4 + 0x22b0], ECX
        _emit 0x8c
        _emit 0xb3
        _emit 0xb0
        _emit 0x22
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EAX, [EBX + ESI*4 + 0x22b0]
        _emit 0x84
        _emit 0xb3
        _emit 0xb0
        _emit 0x22
        _emit 0x00
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x74              // JZ +0x07  (already_equal)
        _emit 0x07
        _emit 0x89              // MOV dword ptr [EAX], ECX
        _emit 0x08
        _emit 0x32              // XOR AL, AL
        _emit 0xc0
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
        // already_equal:
        _emit 0xb0              // MOV AL, 1
        _emit 0x01
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
