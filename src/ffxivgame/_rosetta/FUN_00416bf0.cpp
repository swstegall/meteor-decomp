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
// FUNCTION: ffxivgame 0x00416bf0 — __thiscall member predicate with vtable
//                                  dispatch and byte-field arithmetic (95 bytes)
//
// Calling convention: __thiscall, 2 stack args, returns bool in AL.
// RET 0x8 confirms the callee cleans 2 DWORDs from the stack.
//
// Behaviour:
//   Calls vtable[7] (offset 0x1c / 4 = slot 7) with arg1 and a stack-allocated
//   bool output parameter.  If the vtable function reports failure (out-param
//   byte == 0), returns false.
//
//   On success, uses the vtable return value (EAX) together with three byte
//   fields at this+0x14, this+0x15, this+0x16 to compute a stride offset:
//
//       offset = (unk14 + unk15 + unk16) * vtable_ret + field_4 + unk14
//
//   Then calls FUN_00416810 via ECX = arg2, EDX = unk15 >> 2, and pushes
//   the computed offset as a stack argument.  Returns true (AL = 1).
//
//   The local bool lives at [ESP+7] — the high byte of the PUSH ECX slot —
//   which MSVC 2005 reaches via an indirect `LEA ECX, [ESP+7]` / `PUSH ECX`
//   / `MOV [ESP+0xF], 0` sequence (the initialisation happens after the two
//   additional PUSHes shift the byte from +7 to +0xF).

extern "C" void FUN_00416810();

extern "C" __declspec(naked) void FUN_00416bf0() {
    __asm {
        push    ecx
        mov     edx, dword ptr [esp + 0x8]
        push    esi
        mov     esi, ecx
        mov     eax, dword ptr [esi]
        mov     eax, dword ptr [eax + 0x1c]
        lea     ecx, [esp + 0x7]
        push    ecx
        push    edx
        mov     ecx, esi
        mov     byte ptr [esp + 0xf], 0x0
        call    eax
        cmp     byte ptr [esp + 0x7], 0x0
        jnz     success
        xor     al, al
        pop     esi
        pop     ecx
        ret     0x8
    success:
        movzx   ecx, byte ptr [esi + 0x15]
        push    ebx
        movzx   ebx, byte ptr [esi + 0x16]
        push    edi
        movzx   edi, byte ptr [esi + 0x14]
        add     ebx, edi
        add     ebx, ecx
        imul    ebx, eax
        add     ebx, dword ptr [esi + 0x4]
        mov     edx, ecx
        mov     ecx, dword ptr [esp + 0x18]
        add     ebx, edi
        shr     edx, 0x2
        push    ebx
        call    FUN_00416810
        add     esp, 0x4
        pop     edi
        pop     ebx
        mov     al, 0x1
        pop     esi
        pop     ecx
        ret     0x8
    }
}
