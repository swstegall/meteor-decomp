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
// FUNCTION: ffxivgame 0x00012e10 — SQEX::CDev::Engine::Memory::Alternative::IDebugBlock
//                                  scalar deleting destructor (vtable-reset variant)
//
// Asm (31 bytes):
//   f6 44 24 04 01        TEST byte ptr [ESP+0x4], 0x1    ; test delete flag before PUSH
//   56                    PUSH ESI
//   8b f1                 MOV ESI, ECX                    ; cache `this`
//   c7 06 88 67 f5 00     MOV dword ptr [ESI], vftable    ; reset to IDebugBlock::vftable
//   74 09                 JZ no_delete                    ; skip free if flag bit 0 == 0
//   56                    PUSH ESI                        ; arg: this
//   e8 XX XX XX XX        CALL IDebugBlock_free_fn        ; __cdecl free
//   83 c4 04              ADD ESP, 4                      ; caller cleanup (__cdecl)
// no_delete:
//   8b c6                 MOV EAX, ESI                    ; return this
//   5e                    POP ESI
//   c2 04 00              RET 0x4                         ; __thiscall, 1 stack arg
//
// Identical pattern to IBlock (FUN_00410400) — simplified scalar-deleting-destructor
// with no separate real_dtor call; the destructor body is just the vtable reset.
// MSVC 2005 hoists the TEST above the PUSH ESI (evaluates the delete flag before
// saving the callee-saved register). The free function is __cdecl; the caller cleans
// the argument with ADD ESP, 4 after the CALL.
//
// Note: compare.py reports size_override of 31 B (symbols.json had 28 B;
// RET imm16 (04 00) accounts for the 3-byte difference in encoding).
//
// Relocations (masked by compare.py):
//   DIR32: IDebugBlock::vftable (0xf56788) at func+9
//   REL32: IDebugBlock_free_fn at func+16

extern "C" void *IDebugBlock_vftable;      // SQEX::CDev::Engine::Memory::Alternative::IDebugBlock::vftable — VA 0xf56788
extern "C" void IDebugBlock_free_fn(void *p);  // __cdecl free-like function

extern "C" __declspec(naked) void FUN_00412e10()
{
    __asm {
        test byte ptr [esp+4], 1
        push esi
        mov esi, ecx
        mov dword ptr [esi], offset IDebugBlock_vftable
        jz no_delete
        push esi
        call IDebugBlock_free_fn
        add esp, 4
    no_delete:
        mov eax, esi
        pop esi
        ret 4
    }
}
