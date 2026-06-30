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
// FUNCTION: ffxivgame 0x00058920 — scalar deleting destructor variant (41 B)
//
// __thiscall; one stack parameter (unsigned int flags, RET 4).
//
// Sets the vtable pointer at [this+0] to 0x00F67878, then calls a __stdcall
// sub-object destructor via the global function pointer at [0x00f3e170] with
// &this->field_4 as its argument.  If (flags & 1), calls operator delete
// (FUN_009d1b17, __cdecl) on `this`.  Returns `this` in EAX.
//
// The asm dump listed 38 bytes (size 0x26 / end 0x00058946); the
// size_overrides file corrects this to 41 bytes — the three bytes at
// 0x00058940-0x00058942 are the ADD ESP,4 cdecl-cleanup after the
// FUN_009d1b17 CALL, and the RET 4 (c2 04 00) at 0x00058946 is included
// in the corrected count.  The JZ +9 offset spans exactly 9 bytes
// (PUSH 1 + CALL5 + ADD3) confirming the three hidden bytes.
//
// Asm (41 bytes, orig RVA 0x00058920):
//   56                      PUSH ESI
//   8b f1                   MOV ESI, ECX               ; this = ECX (__thiscall)
//   8d 46 04                LEA EAX, [ESI+4]           ; EAX = &this->field_4
//   50                      PUSH EAX                   ; arg for __stdcall sub-obj dtor
//   c7 06 78 78 f6 00       MOV [ESI], 0x00F67878      ; restore vtable
//   ff 15 70 e1 f3 00       CALL dword ptr [0x00f3e170]; __stdcall: destroy field_4
//   f6 44 24 08 01          TEST byte ptr [ESP+8], 1   ; test flags
//   74 09                   JZ +9  → MOV EAX, ESI     ; skip delete if flag==0
//   56                      PUSH ESI                   ; arg: this
//   e8 RR RR RR RR          CALL FUN_009d1b17          ; _free / operator delete
//   83 c4 04                ADD ESP, 4                 ; caller cleanup
//   8b c6                   MOV EAX, ESI               ; return this
//   5e                      POP ESI
//   c2 04 00                RET 4                      ; callee pops flags

extern "C" void FUN_009d1b17();

extern "C" __declspec(naked) void FUN_00458920()
{
    __asm {
        // 56
        push esi
        // 8b f1
        mov esi, ecx
        // 8d 46 04
        lea eax, [esi+4]
        // 50
        push eax
        // c7 06 78 78 f6 00
        mov dword ptr [esi], 0x00F67878
        // ff 15 70 e1 f3 00
        _emit 0xff
        _emit 0x15
        _emit 0x70
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
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
