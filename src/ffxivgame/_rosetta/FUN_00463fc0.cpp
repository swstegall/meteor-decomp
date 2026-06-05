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
// FUNCTION: ffxivgame 0x00063fc0 — sk_push: append element to end of stack (22 B)
//
// Thin two-argument wrapper around sk_insert (FUN_00463ec0).  Takes a pointer
// to an sk-style container (first field is an int count) and a value to push,
// then calls sk_insert with the current count as the insertion index so the
// element lands at the tail.
//
// Calling convention: __cdecl (caller cleans up; bare RET).
// No prologue — leaf wrapper with no callee-saved register usage.
//
// Asm (22 bytes @ orig RVA 0x00063fc0):
//   8b 44 24 04    MOV EAX, dword ptr [ESP+0x4]   ; EAX = sk (arg1)
//   8b 08          MOV ECX, dword ptr [EAX]        ; ECX = sk->num (count)
//   8b 54 24 08    MOV EDX, dword ptr [ESP+0x8]    ; EDX = data (arg2)
//   51             PUSH ECX                         ; 3rd arg to sk_insert
//   52             PUSH EDX                         ; 2nd arg to sk_insert
//   50             PUSH EAX                         ; 1st arg to sk_insert
//   e8 ee fe ff ff CALL FUN_00463ec0 (sk_insert)   ; rel32 reloc
//   83 c4 0c       ADD ESP, 0xc
//   c3             RET
//
// The CALL rel32 operand is a relocatable slot; compare.py masks it as a
// 4-byte wildcard (IMAGE_REL_I386_REL32), so the .obj matches byte-for-byte
// against the orig slice.

// sk_insert (FUN_00463ec0): inserts data at position 'where' in the stack,
// growing the backing array if needed.  Returns the new element count.
extern "C" int FUN_00463ec0();

extern "C" __declspec(naked) int FUN_00463fc0() {
    __asm {
        mov  eax, dword ptr [esp + 4]   // 8b 44 24 04 — EAX = sk
        mov  ecx, dword ptr [eax]        // 8b 08       — ECX = sk->num
        mov  edx, dword ptr [esp + 8]   // 8b 54 24 08 — EDX = data
        push ecx                          // 51          — 3rd arg (where)
        push edx                          // 52          — 2nd arg (data)
        push eax                          // 50          — 1st arg (sk)
        call FUN_00463ec0                 // e8 rel32    — sk_insert
        add  esp, 12                      // 83 c4 0c
        ret                               // c3
    }
}
