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
// FUNCTION: ffxivgame 0x00010400 — SQEX::CDev::Engine::Memory::Alternative::IBlock
//                                  scalar deleting destructor (vtable-reset variant)
//
// Asm (31 bytes):
//   f6 44 24 04 01        TEST byte ptr [ESP+0x4], 0x1    ; test delete flag before PUSH
//   56                    PUSH ESI
//   8b f1                 MOV ESI, ECX                    ; cache `this`
//   c7 06 40 67 f5 00     MOV dword ptr [ESI], vftable    ; reset to IBlock::vftable
//   74 09                 JZ no_delete                    ; skip free if flag bit 0 == 0
//   56                    PUSH ESI                        ; arg: this
//   e8 XX XX XX XX        CALL IBlock_free_fn             ; __cdecl free
//   83 c4 04              ADD ESP, 4                      ; caller cleanup (__cdecl)
// no_delete:
//   8b c6                 MOV EAX, ESI                    ; return this
//   5e                    POP ESI
//   c2 04 00              RET 0x4                         ; __thiscall, 1 stack arg
//
// This is the simplified scalar-deleting-destructor pattern for IBlock: there
// is no separate real_dtor call — the destructor body is just the vtable reset.
// MSVC 2005 hoists the TEST above the PUSH ESI (evaluates the delete flag before
// saving the callee-saved register). The free function at 0x009d1b17 (VA) is
// __cdecl — the caller cleans the argument with ADD ESP, 4 after the CALL.
// Note: the ASM dump omitted the ADD ESP, 4 at 0x10416; compare.py's size
// override of 31 bytes (vs symbols.json's 28) reveals the 3 hidden bytes.

extern "C" void *IBlock_vftable;  // SQEX::CDev::Engine::Memory::Alternative::IBlock::vftable — VA 0x00f56740

extern "C" void IBlock_free_fn(void *p);  // VA 0x009d1b17 (__cdecl)

extern "C" __declspec(naked) void FUN_00410400()
{
    __asm {
        test byte ptr [esp+4], 1
        push esi
        mov esi, ecx
        mov dword ptr [esi], offset IBlock_vftable
        jz no_delete
        push esi
        call IBlock_free_fn
        add esp, 4
    no_delete:
        mov eax, esi
        pop esi
        ret 4
    }
}
