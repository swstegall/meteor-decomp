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
// FUNCTION: ffxivgame 0x0006a5a0 — __cdecl vtable-slot-10 dispatcher (23 B)
//
// Takes a pointer to an object, reads its vtable, and tail-calls the
// function pointer stored at vtable offset 0x28 (slot 10). Returns -1
// if the function pointer is null.
//
// Asm (23 bytes @ orig RVA 0x0006a5a0):
//   8b 4c 24 04           MOV  ECX, dword ptr [ESP+0x4]   ; ECX = arg (obj ptr)
//   8b 01                 MOV  EAX, dword ptr [ECX]       ; EAX = vtable
//   8b 40 28              MOV  EAX, dword ptr [EAX+0x28]  ; EAX = vtable[10]
//   85 c0                 TEST EAX, EAX                   ; null check
//   74 06                 JZ   +6 → null_fp               ; if null, return -1
//   89 4c 24 04           MOV  dword ptr [ESP+0x4], ECX   ; restore arg for callee
//   ff e0                 JMP  EAX                        ; tail call
//   83 c8 ff              OR   EAX, 0xffffffff            ; EAX = -1
//   c3                    RET
//
// Calling convention: __cdecl (bare RET; caller cleans up).
// No stack frame — leaf with no saved registers.

extern "C" __declspec(naked) int FUN_0046a5a0() {
    __asm {
        mov ecx, dword ptr [esp + 4]
        mov eax, dword ptr [ecx]
        mov eax, dword ptr [eax + 0x28]
        test eax, eax
        jz null_fp
        mov dword ptr [esp + 4], ecx
        jmp eax
    null_fp:
        or eax, 0xffffffff
        ret
    }
}
