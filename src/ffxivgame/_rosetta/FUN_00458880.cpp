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
// FUNCTION: ffxivgame 0x00058880 — __cdecl 2-arg wrapper that calls FUN_00458810
//                                  and returns its first argument (23 B)
//
// Asm (23 bytes @ orig RVA 0x00058880):
//   8b 44 24 08        MOV EAX, dword ptr [ESP+0x8]  ; load arg2 BEFORE PUSH ESI
//   56                 PUSH ESI                       ; save callee-saved ESI
//   8b 74 24 08        MOV ESI, dword ptr [ESP+0x8]  ; load arg1 (after PUSH, ESP+8 = arg1)
//   50                 PUSH EAX                       ; push arg2
//   56                 PUSH ESI                       ; push arg1
//   e8 xx xx xx xx     CALL FUN_00458810              ; rel32 reloc
//   83 c4 08           ADD ESP, 0x8                   ; __cdecl caller-cleanup
//   8b c6              MOV EAX, ESI                   ; return arg1
//   5e                 POP ESI
//   c3                 RET
//
// Calling convention: __cdecl (bare RET; caller cleans 8 bytes after the
//   inner CALL to FUN_00458810).
//
// Notable compiler artefact — load-before-PUSH optimisation:
//   arg2 is loaded into EAX at [ESP+0x8] BEFORE the PUSH ESI prologue so
//   that after the PUSH shifts ESP by 4, arg1 can be loaded from [ESP+0x8]
//   using the same offset.  Without this hoist MSVC would need to use
//   [ESP+0xC] for arg2 after the push.  MSVC 2005 /O2 consistently emits
//   this pattern when one argument must be kept alive across a CALL (here
//   arg1 is placed in callee-saved ESI and returned in EAX after the call).
//
// Source shape (inferred):
//   void* FUN_00458880(void* arg1, void* arg2) {
//       FUN_00458810(arg1, arg2);
//       return arg1;
//   }

extern "C" void FUN_00458810();

extern "C" __declspec(naked) void FUN_00458880() {
    __asm {
        mov  eax, dword ptr [esp + 0x8]
        push esi
        mov  esi, dword ptr [esp + 0x8]
        push eax
        push esi
        call FUN_00458810
        add  esp, 0x8
        mov  eax, esi
        pop  esi
        ret
    }
}
