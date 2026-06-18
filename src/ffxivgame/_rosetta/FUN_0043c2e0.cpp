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
// FUNCTION: ffxivgame 0x0003c2e0 — __thiscall forwarding stub that calls
//                                  FUN_00c6be00 on two member pointers (23 B)
//
// Asm (23 bytes @ orig RVA 0x0003c2e0):
//   56                         PUSH ESI
//   8b f1                      MOV  ESI, ECX              ; ESI = this
//   8b 4e 24                   MOV  ECX, [ESI + 0x24]    ; ECX = this->field_0x24
//   e8 RR RR RR RR             CALL FUN_00c6be00          ; call on field_0x24
//   8b 8e bc 00 00 00          MOV  ECX, [ESI + 0xbc]    ; ECX = this->field_0xbc
//   5e                         POP  ESI
//   e9 RR RR RR RR             JMP  FUN_00c6be00          ; tail-call on field_0xbc
//
// Calling convention: __thiscall (ECX = this on entry; ESI callee-saved).
// No local variables. Return value from first CALL is discarded; the
// tail-call JMP inherits the caller's return address so the second
// invocation of FUN_00c6be00 returns directly to the caller.

extern "C" void FUN_00c6be00();

extern "C" __declspec(naked) void FUN_0043c2e0() {
    __asm {
        push esi
        mov  esi, ecx
        mov  ecx, dword ptr [esi + 0x24]
        call FUN_00c6be00
        mov  ecx, dword ptr [esi + 0xbc]
        pop  esi
        jmp  FUN_00c6be00
    }
}
