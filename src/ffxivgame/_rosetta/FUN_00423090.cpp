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
// FUNCTION: ffxivgame 0x00423090 — __thiscall conditional virtual dispatch
//                                  (52 bytes / 0x34).
//
// Layout (inferred from the asm):
//   This (ECX):
//     +0x00  SomeBase  *m_base    — object whose vtable[4] is called on failure
//     +0x04  SomeHelper *m_helper — object whose FUN_00423b90 is called first
//
// Source shape:
//   The function calls m_helper->FUN_00423b90(arg1, arg2, arg3).
//   If that returns 0 (false), it falls through to a virtual dispatch:
//     m_base->vtable[4](arg1, arg3, arg2)   ← note arg2/arg3 are swapped
//   If the helper call returns non-zero, the virtual dispatch is skipped.
//
// Stack argument mapping (after the four callee-save pushes):
//   EBX ← arg2  (read at [ESP+0xc] after PUSH EBX — original [ESP+0x8])
//   EBP ← arg1  (read at [ESP+0xc] after PUSH EBP — original [ESP+0x4])
//   EDI ← arg3  (read at [ESP+0x1c] after all four pushes)
//
// First call (FUN_00423b90, __thiscall):
//   push EDI (arg3), save this→ESI, load this->field4→ECX,
//   push EBX (arg2), push EBP (arg1), CALL  →  (arg1, arg2, arg3)
//
// Virtual call (thiscall via CALL EDX):
//   ECX = *this = this->m_base,  EAX = vtable,  EDX = vtable[4]
//   push EBX (arg2), push EDI (arg3), push EBP (arg1)
//   → (arg1, arg3, arg2)   (arg2/arg3 order inverted vs. first call)
//
// Calling convention: __thiscall (ECX = this; three DWORD stack args;
//   callee cleans 0xc via `ret 0xc`).
//
// Frame:
//   PUSH EBX / PUSH EBP / PUSH ESI / PUSH EDI — no ESP adjustment.
//
// The REL32 operand of `call FUN_00423b90` is masked by tools/compare.py.

extern "C" void FUN_00423b90();  // __thiscall, 3 DWORD args, ret 0xc

extern "C" __declspec(naked) void FUN_00423090() {
    __asm {
        push    ebx
        mov     ebx, dword ptr [esp + 0xc]
        push    ebp
        mov     ebp, dword ptr [esp + 0xc]
        push    esi
        push    edi
        mov     edi, dword ptr [esp + 0x1c]
        push    edi
        mov     esi, ecx
        mov     ecx, dword ptr [esi + 4]
        push    ebx
        push    ebp
        call    FUN_00423b90
        test    al, al
        jnz     done
        mov     ecx, dword ptr [esi]
        mov     eax, dword ptr [ecx]
        mov     edx, dword ptr [eax + 0x10]
        push    ebx
        push    edi
        push    ebp
        call    edx
    done:
        pop     edi
        pop     esi
        pop     ebp
        pop     ebx
        ret     0xc
    }
}
