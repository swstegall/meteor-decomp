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
// FUNCTION: ffxivgame 0x00063fe0 — sk_pop: remove and return last element (31 B)
//
// Pops the top (last) element from an OpenSSL-style sk container by
// delegating to sk_delete (FUN_00463f70) with index (num-1).  Returns
// NULL if the stack pointer is NULL or the stack is empty.
//
// Calling convention: __cdecl (caller cleans up; bare RET).
// No prologue — leaf wrapper with no callee-saved register usage.
//
// Asm (31 bytes @ orig RVA 0x00063fe0):
//   8b 4c 24 04       MOV ECX, dword ptr [ESP+0x4]  ; ECX = st (arg1)
//   85 c9             TEST ECX, ECX                  ; is st NULL?
//   75 03             JNZ +3 (→00463feb)             ; if not NULL, continue
//   33 c0             XOR EAX, EAX                   ; return NULL
//   c3                RET
//   8b 01             MOV EAX, dword ptr [ECX]       ; EAX = st->num
//   85 c0             TEST EAX, EAX                  ; is num <= 0?
//   7e f7             JLE -9 (→00463fe8)             ; if so, return NULL
//   83 c0 ff          ADD EAX, -0x1                  ; EAX = num - 1
//   50                PUSH EAX                       ; 2nd arg: index
//   51                PUSH ECX                       ; 1st arg: st
//   e8 75 ff ff ff    CALL FUN_00463f70 (sk_delete)  ; rel32 reloc
//   83 c4 08          ADD ESP, 0x8
//   c3                RET
//
// The JLE at +0xf jumps BACKWARD to the shared XOR EAX / RET epilogue
// at +0x8 — MSVC merges the two NULL-return paths into one block.
// The CALL rel32 operand is a relocatable slot; compare.py masks it as a
// 4-byte wildcard (IMAGE_REL_I386_REL32), so the .obj matches byte-for-byte
// against the orig slice.

// sk_delete (FUN_00463f70): removes element at index 'where', shifts remaining
// elements down, decrements count, and returns the removed element.
extern "C" int FUN_00463f70();

extern "C" __declspec(naked) void* FUN_00463fe0() {
    __asm {
        mov  ecx, dword ptr [esp + 4]    // 8b 4c 24 04  — ECX = st (arg1)
        test ecx, ecx                     // 85 c9        — is st NULL?
        jnz  notnull                      // 75 03        — if not NULL, skip
    retnull:
        xor  eax, eax                     // 33 c0        — return NULL
        ret                               // c3
    notnull:
        mov  eax, dword ptr [ecx]         // 8b 01        — EAX = st->num
        test eax, eax                     // 85 c0        — is num <= 0?
        jle  retnull                      // 7e f7        — if so, return NULL
        add  eax, -1                      // 83 c0 ff     — EAX = num - 1
        push eax                          // 50           — push index (num-1)
        push ecx                          // 51           — push st
        call FUN_00463f70                 // e8 rel32     — sk_delete(st, num-1)
        add  esp, 8                       // 83 c4 08
        ret                               // c3
    }
}
