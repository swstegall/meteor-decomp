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
// FUNCTION: ffxivgame 0x004492f0 — small-buffer-optimised container push
//                                  (__thiscall, 1 explicit stack arg, 86 B)
//
// __thiscall void* FUN_004492f0(Container *param)
//   ECX  : this  (some owner object; only its vtable pointer at [this+0] is used)
//   [ESP+4] (before saves → [ESP+0x10] after 3 callee-save pushes) : Container *param
//
// Behaviour (from asm at RVA 0x000492f0):
//
//   1. Calls FUN_00445e50() — returns a count/size in EAX.
//   2. Loads param->field14 (EDI) and doubles EAX.
//   3. If param->field14 > 2*size  OR  param->field18 == 2*size → skip to tail.
//   4. Calls FUN_0044a5e0(param, 2*size, 1) (__thiscall, 2 stack args).
//      If it returns false → skip to tail.
//   5. If param->field18 >= 4: use the heap pointer at param->field4.
//      Otherwise (SBO): use &param->field4 directly.
//      Either way: slot[old_field14] = 0.
//   6. Tail: call FUN_00449210(*this, param) and return param in EAX.
//
// Container layout (inferred):
//   +0x04  union { T *heap_ptr; T inline_buf[3]; }
//   +0x14  unsigned int  size   (number of filled slots)
//   +0x18  unsigned int  capacity (SBO threshold = 4)
//
// Calling-convention notes:
//   FUN_00445e50  — no visible args / ECX clobbered; likely __cdecl or __thiscall
//                   called with no explicit this; EAX = result.
//   FUN_0044a5e0  — __thiscall, 2 explicit stack args; callee cleans RET 0x8.
//   FUN_00449210  — __cdecl, 2 args; caller cleans ADD ESP, 0x8.
//
// Reloc-bearing sites (CALL rel32; compare.py masks the 4-byte offset):
//   +0x05  CALL rel32 → FUN_00445e50  (VA 0x00445e50)
//   +0x21  CALL rel32 → FUN_0044a5e0  (VA 0x0044a5e0)
//   +0x46  CALL rel32 → FUN_00449210  (VA 0x00449210)
//
// Reconstruction strategy — naked __asm with forward-declared symbols:
//   All non-reloc bytes are structurally fixed; MASM selects short-form
//   Jcc/JMP for all intra-block branches (all offsets fit in one signed byte),
//   matching the original encodings. The three CALL rel32 operands are
//   relocated by the assembler → masked by compare.py → GREEN.

extern "C" {

void FUN_00445e50();
void FUN_0044a5e0();
void FUN_00449210();

__declspec(naked) void FUN_004492f0()
{
    __asm {
        push    ebx
        push    esi
        push    edi
        mov     ebx, ecx
        call    FUN_00445e50
        mov     esi, dword ptr [esp+0x10]
        mov     edi, dword ptr [esi+0x14]
        add     eax, eax
        cmp     edi, eax
        ja      tail
        cmp     dword ptr [esi+0x18], eax
        jz      tail
        push    1
        push    eax
        mov     ecx, esi
        call    FUN_0044a5e0
        test    al, al
        jz      tail
        cmp     dword ptr [esi+0x18], 4
        mov     dword ptr [esi+0x14], edi
        jc      small_buf
        mov     eax, dword ptr [esi+0x4]
        jmp     store_zero
    small_buf:
        lea     eax, [esi+0x4]
    store_zero:
        mov     dword ptr [eax+edi*4], 0
    tail:
        mov     eax, dword ptr [ebx]
        push    esi
        push    eax
        call    FUN_00449210
        add     esp, 8
        pop     edi
        mov     eax, esi
        pop     esi
        pop     ebx
        ret     4
    }
}

} // extern "C"
