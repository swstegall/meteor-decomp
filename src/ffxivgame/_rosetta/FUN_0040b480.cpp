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
// FUNCTION: ffxivgame 0x0000b480 — __thiscall trampoline: call FUN_0040b360(this),
//                                   then tail-call FUN_0040d790(this->field_0x5c)
//                                   (17 B / 0x11)
//
// Calling convention: __thiscall (ECX = this, no stack args, tail-calls callee).
// No prologue locals — only ESI is callee-saved (used to hold `this` across the
// inner CALL so [ESI+0x5c] can be read back after FUN_0040b360 returns).
//
// Asm (17 bytes @ orig RVA 0x0000b480):
//   56                   PUSH ESI                  ; save ESI
//   8b f1                MOV  ESI, ECX             ; ESI = this
//   e8 d8 fe ff ff       CALL FUN_0040b360         ; __thiscall, ECX still = this
//   8b 4e 5c             MOV  ECX, [ESI + 0x5c]    ; ECX = this->field_0x5c
//   5e                   POP  ESI                  ; restore ESI
//   e9 ff 22 00 00       JMP  FUN_0040d790         ; tail-call, ECX = new this
//
// The two reloc-bearing sites (CALL rel32 + JMP rel32) are masked by
// compare.py and do not affect the diff outcome.

extern "C" void FUN_0040b360();
extern "C" void FUN_0040d790();

extern "C" __declspec(naked) void FUN_0040b480() {
    __asm {
        push esi
        mov  esi, ecx
        call FUN_0040b360
        mov  ecx, dword ptr [esi + 0x5c]
        pop  esi
        jmp  FUN_0040d790
    }
}
