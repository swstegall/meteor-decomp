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
// FUNCTION: ffxivgame 0x0005d450 — __cdecl 1-arg wrapper calling two
//                                  sub-functions with the same argument (22 B)
//
// Asm (22 bytes @ orig RVA 0x0005d450):
//   56                   PUSH ESI
//   8b 74 24 08          MOV  ESI, dword ptr [ESP+0x8]  ; ESI = arg1
//   56                   PUSH ESI                       ; arg for call 1
//   e8 RR RR RR RR       CALL FUN_0045d160              ; rel32 reloc
//   56                   PUSH ESI                       ; arg for call 2
//   e8 RR RR RR RR       CALL FUN_004632f0              ; rel32 reloc
//   83 c4 08             ADD  ESP, 0x8                  ; deferred cleanup (2 pushes)
//   5e                   POP  ESI
//   c3                   RET
//
// Calling convention: __cdecl (bare RET; caller cleans up).
// No frame pointer: /Oy leaf-style frame.
//
// ESI is saved as a callee-saved register to hold arg1 across both
// __cdecl calls.  Stack cleanup for the two single-arg pushes is
// deferred to a single ADD ESP, 8 after the second call — MSVC 2005
// /O2 idiom for consecutive same-arg calls.

extern "C" void FUN_0045d160(void*);
extern "C" void FUN_004632f0(void*);

extern "C" __declspec(naked) void FUN_0045d450() {
    __asm {
        push    esi
        mov     esi, dword ptr [esp + 8]
        push    esi
        call    FUN_0045d160
        push    esi
        call    FUN_004632f0
        add     esp, 8
        pop     esi
        ret
    }
}
