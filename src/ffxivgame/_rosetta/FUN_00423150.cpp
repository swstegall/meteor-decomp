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
// FUNCTION: ffxivgame 0x00423150 — __thiscall dispatch method (52 bytes)
//
//   Calling convention : __thiscall (ECX = this; three DWORD stack args;
//                        callee cleans 0xc via `ret 0xc`).
//
//   Registers at function entry (after the interleaved prologue):
//     ESI = this           (from ECX)
//     EBP = arg1           (loaded from [ESP+0x04] pre-saves)
//     EBX = arg2           (loaded from [ESP+0x08] pre-saves)
//     EDI = arg3           (loaded from [ESP+0x0c] pre-saves)
//
//   Prologue interleave (MSVC scheduler):
//     PUSH EBX ; MOV EBX,[ESP+0xc]  → save then load arg2
//     PUSH EBP ; MOV EBP,[ESP+0xc]  → save then load arg1
//     PUSH ESI ; PUSH EDI
//     MOV  EDI,[ESP+0x1c]           → load arg3 after all four saves
//
//   Body:
//     1. PUSH EDI,EBX,EBP (arg3,arg2,arg1 → right-to-left = (arg1,arg2,arg3))
//        MOV ESI,ECX ; MOV ECX,[ESI+4]
//        CALL FUN_00423770          → this->field4->method(arg1,arg2,arg3)
//     2. TEST AL,AL ; JNZ done      → if non-zero, skip
//     3. MOV ECX,[ESI] ; MOV EAX,[ECX] ; MOV EDX,[EAX+0x1c]
//        PUSH EBX,EDI,EBP (arg2,arg3,arg1 → right-to-left = (arg1,arg3,arg2))
//        CALL EDX                   → this->field0->vtable[7](arg1,arg3,arg2)
//
//   Naked __asm to pin the unusual interleaved prologue and the two
//   different argument orderings at the two call sites.  The REL32 to
//   FUN_00423770 and the indirect CALL EDX are both masked by compare.py.

extern "C" int FUN_00423770();   // __thiscall at RVA 0x00023770

extern "C" __declspec(naked) void FUN_00423150() {
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
        mov     ecx, dword ptr [esi + 0x4]
        push    ebx
        push    ebp
        call    FUN_00423770
        test    al, al
        jnz     done
        mov     ecx, dword ptr [esi]
        mov     eax, dword ptr [ecx]
        mov     edx, dword ptr [eax + 0x1c]
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
