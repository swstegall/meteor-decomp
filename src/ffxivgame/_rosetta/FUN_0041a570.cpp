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
// FUNCTION: ffxivgame 0x0041a570 — __thiscall write-and-notify helper (95 bytes)
//
// bool __thiscall FUN_0041a570(int base, int count, int arg2, void *src)
//   [RET 0x10 — callee cleans 4 DWORD stack args]
//
// Behaviour:
//   1. If `count` (arg1) == 0, compute it as this->field_14 - base.
//   2. Call FUN_00419d60(this, base, count, arg2) — returns a destination ptr.
//   3. memcpy(dst, src, count).
//   4. If this->field_20 == 0  OR  this->field_24 == this->field_20,
//      call the out-of-range notifier FUN_009d22b4.
//   5. Call FUN_00423350(global, this->field_18, this->field_20).
//   6. Return true (AL = 1).
//
// Calling convention details:
//   ECX = this (saved to EDI in prologue)
//   [ESP+4]  = base   (arg0, loaded into EAX before first PUSH)
//   [ESP+8]  = count  (arg1, loaded into ESI after first PUSH → [ESP+0xc])
//   [ESP+c]  = arg2   (loaded into ECX after two PUSHes → [ESP+0x14])
//   [ESP+10] = src    (arg3, loaded into EDX after inner call returns)
//
// Reloc-bearing CALL/MOV sites (compare.py masks the 4-byte operand):
//   +0x1e   CALL rel32  → FUN_00419d60 (0x00419d60)
//   +0x27   CALL rel32  → FUN_009d4600 (0x009d4600, CRT _memcpy)
//   +0x40   CALL rel32  → FUN_009d22b4 (0x009d22b4, out-of-range notifier)
//   +0x53   CALL rel32  → FUN_00423350 (0x00423350)
//   +0x4b   MOV  [imm32]→ DAT_0132987c (0x0132987c, global object ptr)

extern "C" {
    void FUN_00419d60();
    void FUN_009d4600();   // CRT _memcpy
    void FUN_009d22b4();   // out-of-range / sanity notifier
    void FUN_00423350();
    extern int DAT_0132987c;
}

extern "C" __declspec(naked) void FUN_0041a570() {
    __asm {
        mov  eax, dword ptr [esp + 0x4]
        push esi
        mov  esi, dword ptr [esp + 0xc]
        test esi, esi
        push edi
        mov  edi, ecx
        jnz  skip_default
        mov  esi, dword ptr [edi + 0x14]
        sub  esi, eax
    skip_default:
        mov  ecx, dword ptr [esp + 0x14]
        push ecx
        push esi
        push eax
        mov  ecx, edi
        call FUN_00419d60
        mov  edx, dword ptr [esp + 0x18]
        push esi
        push edx
        push eax
        call FUN_009d4600
        mov  eax, dword ptr [edi + 0x20]
        add  esp, 0xc
        test eax, eax
        jz   call_notify
        mov  ecx, dword ptr [edi + 0x24]
        sub  ecx, eax
        jnz  skip_notify
    call_notify:
        call FUN_009d22b4
    skip_notify:
        mov  edx, dword ptr [edi + 0x20]
        mov  eax, dword ptr [edi + 0x18]
        mov  ecx, dword ptr [DAT_0132987c]
        push edx
        push eax
        call FUN_00423350
        pop  edi
        mov  al, 0x1
        pop  esi
        ret  0x10
    }
}
