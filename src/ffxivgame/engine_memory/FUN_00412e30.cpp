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
// FUNCTION: ffxivgame 0x00012e30 — scalar deleting destructor for a class with
//           dual inheritance from IDebugSpace (primary, offset 0) and
//           IDebugBlock (secondary, offset 8). (38 bytes)
//
// Ghidra pseudo-C shows void return, but EAX holds `this` via ESI at exit —
// same pattern as IBlock's scalar deleting destructor (FUN_00410400).
// MSVC 2005 hoists the TEST before PUSH ESI (evaluates the delete flag first,
// then caches `this` in a callee-saved register).
//
// Note: symbols.json reported 35 bytes but size_overrides.json corrects this
// to 38 bytes (reason: 'RET imm16 (04 00)' — Ghidra's flow analysis missed the
// ADD ESP, 4 + MOV EAX, ESI + POP ESI at the end).
//
// Asm (38 bytes @ RVA 0x00012e30):
//   F6 44 24 04 01      TEST byte ptr [ESP+4], 1              ; hoist delete flag
//   56                  PUSH ESI
//   8B F1               MOV ESI, ECX                          ; cache `this`
//   C7 46 08 xx xx xx xx  MOV dword ptr [ESI+8], IDebugBlock_vftable ; DIR32 reloc
//   C7 06 xx xx xx xx     MOV dword ptr [ESI],   IDebugSpace_vftable ; DIR32 reloc
//   74 09               JZ  no_delete                         ; skip 9 bytes
//   56                  PUSH ESI                              ; arg: this
//   E8 xx xx xx xx      CALL free_fn                          ; REL32 reloc (__cdecl)
//   83 C4 04            ADD ESP, 4                            ; caller cleanup (__cdecl)
//  no_delete:
//   8B C6               MOV EAX, ESI                          ; return this
//   5E                  POP ESI
//   C2 04 00            RET 4
//
// Vtable references:
//   IDebugBlock::vftable at RVA 0xb56788
//   IDebugSpace::vftable at RVA 0xb567b4

extern "C" void *IDebugBlock_vftable;  // SQEX::CDev::Engine::Memory::Alternative::IDebugBlock::vftable
extern "C" void *IDebugSpace_vftable;  // SQEX::CDev::Engine::Memory::Alternative::IDebugSpace::vftable

extern "C" void FUN_00412e30_free_fn(void *p);  // __cdecl

extern "C" __declspec(naked) void FUN_00412e30()
{
    __asm {
        test byte ptr [esp+4], 1
        push esi
        mov esi, ecx
        mov dword ptr [esi+8], offset IDebugBlock_vftable
        mov dword ptr [esi], offset IDebugSpace_vftable
        jz no_delete
        push esi
        call FUN_00412e30_free_fn
        add esp, 4
    no_delete:
        mov eax, esi
        pop esi
        ret 4
    }
}
