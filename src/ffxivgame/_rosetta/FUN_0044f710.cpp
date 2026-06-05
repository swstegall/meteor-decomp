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
// FUNCTION: ffxivgame 0x0004f710 — 3-arg __cdecl wrapper that chains two
// calls to FUN_00445210 (passing arg2 in ECX each time) and feeds the
// result into FUN_0044f680.
//
// Calling convention: __cdecl (caller cleans — single ADD ESP,0xc clears
// all three pushed dwords). Frame: none. Both FUN_00445210 calls receive
// `this` in ECX (= [ESP+8], stable across the body because every push is
// cleaned only at the end), plus a stack arg.
//
// Asm (34 bytes @ orig RVA 0x0004f710):
//   8b 44 24 0c    MOV EAX, [ESP + 0x0c]   ; arg3
//   8b 4c 24 08    MOV ECX, [ESP + 0x08]   ; arg2 (this)
//   50             PUSH EAX                 ; arg3
//   e8 ?? ?? ?? ?? CALL FUN_00445210
//   8b 4c 24 08    MOV ECX, [ESP + 0x08]   ; arg2 (this) again
//   50             PUSH EAX                 ; result1
//   e8 ?? ?? ?? ?? CALL FUN_00445210
//   50             PUSH EAX                 ; result2
//   e8 ?? ?? ?? ?? CALL FUN_0044f680
//   83 c4 0c       ADD ESP, 0x0c            ; cdecl cleanup (3 dwords)
//   c3             RET
//
// CALL rel32 targets are reloc wildcards in compare.py, so the extern
// declarations below only need to exist — their addresses don't matter.

extern "C" int FUN_00445210();
extern "C" int FUN_0044f680();

extern "C" __declspec(naked) void FUN_0044f710() {
    __asm {
        mov eax, dword ptr [esp + 0x0c]
        mov ecx, dword ptr [esp + 0x08]
        push eax
        call FUN_00445210
        mov ecx, dword ptr [esp + 0x08]
        push eax
        call FUN_00445210
        push eax
        call FUN_0044f680
        add esp, 0x0c
        ret
    }
}
