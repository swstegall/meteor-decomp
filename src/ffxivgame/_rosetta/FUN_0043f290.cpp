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
// FUNCTION: ffxivgame 0x0043f290 — __stdcall allocation helper (0x45 / 69 bytes)
//
// Calling convention: __stdcall (1 DWORD stack arg, `ret 4` epilogue).
// No frame pointer; 8-byte on-stack "space descriptor" buffer.
//
// Body:
//   1. Allocate an 8-byte buffer on the stack.
//   2. Call FUN_0040e2d0 (__thiscall, ECX=&buf, args: 0x10, DAT_00f57b04)
//      which stores {[buf]=0x10, [buf+4]=DAT_00f57b04} and returns &buf in EAX.
//      Save &buf in ESI.
//   3. Load global allocator pointer from DAT_01327fc0 into EAX.
//      If null, call FUN_0040e500 (__cdecl) to lazy-init it; EAX = allocator.
//   4. Compute stride = arg1 * 28 via: LEA ECX,[EDX*8], SUB ECX,EDX,
//      ADD ECX,ECX, ADD ECX,ECX  →  EDX*8 - EDX = EDX*7 → *2 → *4 = EDX*28.
//   5. Call FUN_0040e110 (__thiscall, ECX=allocator, args: stride, &buf).
//   6. Epilogue: pop ESI, reclaim 8 local bytes, ret 4.
//
// Reloc-bearing sites (compare.py masks 4-byte payloads):
//   +0x04  PUSH imm32  → DAT_00f57b04 (name-string address)
//   +0x09  CALL rel32  → FUN_0040e2d0
//   +0x0c  MOV EAX,[imm32] → DAT_01327fc0
//   +0x14  CALL rel32  → FUN_0040e500
//   +0x1f  CALL rel32  → FUN_0040e110

extern "C" {
void FUN_0040e2d0();
void FUN_0040e500();
void FUN_0040e110();
int DAT_01327fc0;
int DAT_00f57b04;
}

extern "C" __declspec(naked) void FUN_0043f290() {
    __asm {
        sub     esp, 8
        push    esi
        push    offset DAT_00f57b04
        push    10h
        lea     ecx, [esp + 0ch]
        call    FUN_0040e2d0
        mov     esi, eax
        mov     eax, dword ptr [DAT_01327fc0]
        test    eax, eax
        jnz     skip_init
        call    FUN_0040e500
    skip_init:
        mov     edx, dword ptr [esp + 10h]
        lea     ecx, [edx*8]
        sub     ecx, edx
        add     ecx, ecx
        add     ecx, ecx
        push    esi
        push    ecx
        mov     ecx, eax
        call    FUN_0040e110
        pop     esi
        add     esp, 8
        ret     4
    }
}
