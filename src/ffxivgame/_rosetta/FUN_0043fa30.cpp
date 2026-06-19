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
// FUNCTION: ffxivgame 0x0003fa30 — __thiscall wrapper: initialise `count`
//                                  0x1C-byte string-objects starting at
//                                  `begin` via FUN_0043f870, then return
//                                  begin + count (60 B / 0x3C).
//
// Calling convention: __thiscall (ECX = this, three DWORD stack args;
//                                 callee cleans via RET 0xC).
// Stack args (right-to-left push order):
//   arg1 — SomeStruct* begin   (base of 0x1C-stride object array)
//   arg2 — int         count   (number of elements to initialise)
//   arg3 — const char* str     (string data forwarded to the initialiser)
//
// Returns: pointer = begin + count * 0x1C  (past-the-end of the range).
//
// Body shape:
//   PUSH ECX allocates 4 bytes of stack as a bool local (false).
//   PUSH ESI / PUSH EDI save callee-preserved registers.
//   The three stack args are loaded as EDX (arg3), ESI (arg2), EDI (arg1).
//   The bool slot at [ESP+0x8] is then zeroed (MOV byte ptr, 0) and
//   pushed as the sixth (last) argument to FUN_0043f870.
//   FUN_0043f870 is called as __cdecl with 6 args:
//     (begin, count, str, this, str, false)
//   After cleaning up the 6 args (ADD ESP, 0x18) the return value is
//   computed inline:
//     ECX = ESI * 7   (via LEA [ESI*8] then SUB ESI)
//     EAX = EDI + ECX * 4   =  begin + count * 28

extern "C" void FUN_0043f870();   // __cdecl: (ptr, count, str, ctx, str2, flag)

extern "C" __declspec(naked) void FUN_0043fa30() {
    __asm {
        push    ecx
        mov     edx, dword ptr [esp + 0x10]
        push    esi
        mov     esi, dword ptr [esp + 0x10]
        push    edi
        mov     edi, dword ptr [esp + 0x10]
        mov     byte ptr [esp + 0x8], 0x0
        mov     eax, dword ptr [esp + 0x8]
        push    eax
        mov     eax, dword ptr [esp + 0x1c]
        push    edx
        push    ecx
        push    eax
        push    esi
        push    edi
        call    FUN_0043f870
        add     esp, 0x18
        lea     ecx, [esi*8]
        sub     ecx, esi
        lea     eax, [edi + ecx*4]
        pop     edi
        pop     esi
        pop     ecx
        ret     0xc
    }
}
