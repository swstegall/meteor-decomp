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
// FUNCTION: ffxivgame 0x000248f0 — scalar deleting destructor variant (40 B)
//
// __thiscall; one stack parameter (unsigned int freeMem, RET 4).
//
// Sets the vtable pointer to 0x00F5BE40, calls the destructor body
// FUN_004247d0 with `this` as a __cdecl stack argument, then if
// (freeMem & 1) calls _free/operator_delete on `this`.  Returns `this`.
//
// The asm dump listed 37 bytes (size 0x25); the size_overrides file
// corrects this to 40 bytes — the three missing bytes at RVA 0x0002490f
// are the ADD ESP,4 cdecl-cleanup after the second CALL, confirmed by
// the JZ +9 offset that spans exactly 9 bytes (PUSH + CALL5 + ADD3).
//
// Asm (40 bytes, orig RVA 0x000248f0):
//   56                   PUSH ESI
//   8b f1                MOV ESI, ECX               ; this = ECX (__thiscall)
//   56                   PUSH ESI                   ; arg: this
//   c7 06 40 be f5 00    MOV [ESI], 0x00F5BE40      ; restore vtable
//   e8 RR RR RR RR       CALL FUN_004247d0          ; dtor body (__cdecl, arg on stack)
//   83 c4 04             ADD ESP, 4                 ; caller cleanup
//   f6 44 24 08 01       TEST byte ptr [ESP+8], 1   ; test freeMem flag
//   74 09                JZ +9  → MOV EAX, ESI     ; skip delete if flag==0
//   56                   PUSH ESI                   ; arg: this
//   e8 RR RR RR RR       CALL FUN_009d1b17          ; _free / operator delete
//   83 c4 04             ADD ESP, 4                 ; caller cleanup
//   8b c6                MOV EAX, ESI               ; return this
//   5e                   POP ESI
//   c2 04 00             RET 4                      ; callee pops freeMem

extern "C" void FUN_004247d0();
extern "C" void FUN_009d1b17();

extern "C" __declspec(naked) void FUN_004248f0()
{
    __asm {
        // 56
        push esi
        // 8b f1
        mov esi, ecx
        // 56
        push esi
        // c7 06 40 be f5 00
        mov dword ptr [esi], 0x00F5BE40
        // e8 RR RR RR RR
        call FUN_004247d0
        // 83 c4 04
        add esp, 4
        // f6 44 24 08 01
        test byte ptr [esp+8], 1
        // 74 09
        jz done
        // 56
        push esi
        // e8 RR RR RR RR
        call FUN_009d1b17
        // 83 c4 04
        add esp, 4
    done:
        // 8b c6
        mov eax, esi
        // 5e
        pop esi
        // c2 04 00
        ret 4
    }
}
