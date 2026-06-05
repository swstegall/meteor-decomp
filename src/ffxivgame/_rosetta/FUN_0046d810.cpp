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
// FUNCTION: ffxivgame 0x0006d810 — conditional CRYPTO_add_lock dispatcher
//           __cdecl int(int *arg1, int arg2, OuterStruct *arg3)
//           79 bytes, no frame, no saved registers.
//
// Validates arg3->kind (must be 1 or 6), arg3->inner (non-null, flag bit 0
// set), then either stores 1 at inner->base + *arg1 (arg2==0 path) or
// delegates to _CRYPTO_add_lock (arg2!=0 path).
//
// The `return 1` in the null-arg2 path is emitted as `LEA EAX, [EDX+1]`
// (3 B) rather than `MOV EAX, 1` (5 B): MSVC 2005 /O2 reuses the
// known-zero EDX register via LEA as a peephole.  C++ source cannot reliably
// coerce this choice, so the function is implemented as naked asm.
//
// Reloc-bearing sites (4-byte windows, wildcarded by compare.py):
//   +0x3c  PUSH offset data_00f79540  (dir32, OpenSSL __FILE__ string in .rdata)
//   +0x44  CALL FUN_00466000          (rel32, _CRYPTO_add_lock)

extern "C" int FUN_00466000(int *, int, int, const char *, int);
extern "C" const char data_00f79540[];

extern "C" __declspec(naked) void FUN_0046d810() {
    __asm {
        // +0x00  8b 4c 24 0c  MOV ECX, [ESP+0xc]  — arg3
        mov     ecx, dword ptr [esp + 0x0c]
        // +0x04  8a 01        MOV AL, [ECX]         — kind
        mov     al, byte ptr [ecx]
        // +0x06  3c 01        CMP AL, 1
        cmp     al, 1
        // +0x08  74 04        JZ loc_d81e           — kind==1 → body
        jz      loc_d81e
        // +0x0a  3c 06        CMP AL, 6
        cmp     al, 6
        // +0x0c  75 3e        JNZ loc_d85c          — kind!=6 → fail
        jnz     loc_d85c

    loc_d81e:
        // +0x0e  8b 49 10     MOV ECX, [ECX+0x10]  — inner struct ptr
        mov     ecx, dword ptr [ecx + 0x10]
        // +0x11  85 c9        TEST ECX, ECX
        test    ecx, ecx
        // +0x13  74 37        JZ loc_d85c           — inner==null → fail
        jz      loc_d85c
        // +0x15  f6 41 04 01  TEST byte [ECX+4], 1 — flags bit 0
        test    byte ptr [ecx + 0x4], 1
        // +0x19  74 31        JZ loc_d85c           — flag clear → fail
        jz      loc_d85c

        // +0x1b  8b 54 24 04  MOV EDX, [ESP+4]     — arg1 (pointer)
        mov     edx, dword ptr [esp + 0x4]
        // +0x1f  8b 41 08     MOV EAX, [ECX+8]     — inner->base
        mov     eax, dword ptr [ecx + 0x8]
        // +0x22  03 02        ADD EAX, [EDX]        — + *arg1 → addr
        add     eax, dword ptr [edx]
        // +0x24  8b 54 24 08  MOV EDX, [ESP+8]     — arg2 (amount)
        mov     edx, dword ptr [esp + 0x8]
        // +0x28  85 d2        TEST EDX, EDX
        test    edx, edx
        // +0x2a  75 0a        JNZ loc_d846          — arg2!=0 → call
        jnz     loc_d846

        // +0x2c  c7 00 01 00 00 00  MOV [EAX], 1   — *addr = 1
        mov     dword ptr [eax], 1
        // +0x32  8d 42 01     LEA EAX, [EDX+1]     — return 1 (EDX==0)
        lea     eax, [edx + 1]
        // +0x35  c3           RET
        ret

    loc_d846:
        // +0x36  8b 49 0c     MOV ECX, [ECX+0xc]   — inner->lock_type
        mov     ecx, dword ptr [ecx + 0xc]
        // +0x39  6a 75        PUSH 0x75            — line
        push    0x75
        // +0x3b  68 ?? ?? ?? ?? PUSH offset str    — file (DIR32 reloc)
        push    offset data_00f79540
        // +0x40  51           PUSH ECX             — lock_type
        push    ecx
        // +0x41  52           PUSH EDX             — amount (arg2)
        push    edx
        // +0x42  50           PUSH EAX             — addr
        push    eax
        // +0x43  e8 ?? ?? ?? ?? CALL               — (REL32 reloc)
        call    FUN_00466000
        // +0x48  83 c4 14     ADD ESP, 0x14        — clean 5 args
        add     esp, 0x14
        // +0x4b  c3           RET
        ret

    loc_d85c:
        // +0x4c  33 c0        XOR EAX, EAX         — return 0
        xor     eax, eax
        // +0x4e  c3           RET
        ret
    }
}
