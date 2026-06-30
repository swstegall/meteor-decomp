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
// FUNCTION: ffxivgame 0x00469ea0 — _BIO_find_type (53 bytes / 0x35)
//
//   BIO *__cdecl BIO_find_type(BIO *bio, int type)
//
// Walks an OpenSSL BIO chain (singly-linked via bio->next_bio at offset
// +0x24) looking for a BIO whose method->type field satisfies the type
// predicate.
//
// Match logic (replicating OpenSSL's two-phase test):
//   mask = type & 0xFF;
//   if (mask == 0)  → match if (bio->method->type & type) != 0 (flag bits)
//   if (mask != 0)  → match if  bio->method->type == type   (exact)
//
// Returns the first matching BIO, or NULL if the chain is exhausted.
// Returns NULL immediately when bio is NULL on entry.
//
// Calling convention: __cdecl (caller cleans 8 bytes; single PUSH ESI
// callee-save; no ESP adjustment).
//
// Register allocation:
//   EAX = current BIO pointer (updated as we advance; returned as result)
//   ESI = type   (arg2, loaded after PUSH ESI from [ESP+0xC])
//   EDX = mask   (type & 0xFF, constant for the whole loop)
//   ECX = bio->method, then method->type (temporary each iteration)
//
// MSVC 2005 scheduling note: the compiler hoists MOV ECX,[ECX]
// (reading method->type) one slot before the JNZ that branches on
// EDX (the mask). Both reads are speculative-safe (no side effects),
// so MSVC moves the load up for pipeline overlap. A plain C source
// would produce this reordering with /O2.  We pin it with naked asm.
//
// Jump offsets (all short-form Jcc cb / JMP cb):
//   JNZ not_null  +0x01   (75 01)
//   JZ  advance   +0x10   (74 10)
//   JNZ exact_chk +0x06   (75 06)
//   JNZ exit      +0x0d   (75 0d)
//   JMP advance   +0x04   (eb 04)
//   JZ  exit      +0x07   (74 07)
//   JNZ loop_start -0x1d  (75 e3)
//
// AND EDX,0FFh uses the 6-byte 81/e2 form (81 e2 ff 00 00 00) because
// 0xFF cannot be sign-extended from 8 bits to give 0x000000FF; the
// assembler is forced to use the full 32-bit immediate.

extern "C" __declspec(naked) void FUN_00469ea0()
{
    __asm {
        mov     eax, dword ptr [esp + 0x4]
        test    eax, eax
        jnz     not_null
        ret
    not_null:
        push    esi
        mov     esi, dword ptr [esp + 0xc]
        mov     edx, esi
        and     edx, 0ffh
    loop_start:
        mov     ecx, dword ptr [eax]
        test    ecx, ecx
        jz      advance
        test    edx, edx
        mov     ecx, dword ptr [ecx]
        jnz     exact_chk
        test    ecx, esi
        jnz     exit
        jmp     advance
    exact_chk:
        cmp     ecx, esi
        jz      exit
    advance:
        mov     eax, dword ptr [eax + 0x24]
        test    eax, eax
        jnz     loop_start
    exit:
        pop     esi
        ret
    }
}
