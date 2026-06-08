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
// FUNCTION: ffxivgame 0x0002d810 — 4-arg __cdecl thunk that forwards to
// FUN_00426190 with a derived 2nd argument (arg1 + 0x10).
//
// Calling convention: __cdecl — 4 args, void return, plain RET (caller cleans).
// Frame: none — no callee-saved registers, no locals.
//
// Asm (33 bytes @ RVA 0x0002d810):
//   8b 44 24 10  MOV EAX, [ESP + 0x10]    ; arg4
//   8b 4c 24 0c  MOV ECX, [ESP + 0x0c]    ; arg3
//   8b 54 24 08  MOV EDX, [ESP + 0x08]    ; arg2
//   50           PUSH EAX                  ; push arg4 (5th for callee)
//   8b 44 24 08  MOV EAX, [ESP + 0x08]    ; arg1 (after PUSH, offset shifts)
//   51           PUSH ECX                  ; push arg3
//   52           PUSH EDX                  ; push arg2
//   8d 48 10     LEA ECX, [EAX + 0x10]    ; compute arg1 + 0x10
//   51           PUSH ECX                  ; push arg1+0x10 (2nd for callee)
//   50           PUSH EAX                  ; push arg1 (1st for callee)
//   e8 ?? ?? ?? ?? CALL FUN_00426190       ; rel32 reloc
//   83 c4 14     ADD ESP, 0x14             ; cdecl cleanup (5 args * 4)
//   c3           RET

extern "C" void FUN_00426190();

extern "C" __declspec(naked) void FUN_0042d810() {
    __asm {
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x0c]
        mov edx, dword ptr [esp + 0x08]
        push eax
        mov eax, dword ptr [esp + 0x08]
        push ecx
        push edx
        lea ecx, [eax + 0x10]
        push ecx
        push eax
        call FUN_00426190
        add esp, 0x14
        ret
    }
}
