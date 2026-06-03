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
// FUNCTION: ffxivgame 0x00418350 — 2-arg __cdecl interceptor/thunk (33 B)
//
// Stores arg2 into a global table at slot arg1*48, then tail-jumps to
// FUN_0041ccc0 with the same two args still on the stack.
//
// Calling convention: __cdecl (2 args, no frame, plain JMP at end).
//
// Asm (33 bytes @ RVA 0x00018350):
//   8b 44 24 04          MOV EAX, [ESP+4]              ; arg1 = index
//   8b 4c 24 08          MOV ECX, [ESP+8]              ; arg2 = value
//   8d 14 40             LEA EDX, [EAX + EAX*2]        ; EDX = EAX*3
//   c1 e2 04             SHL EDX, 4                    ; EDX = EAX*48
//   89 8a cc 90 32 01    MOV [EDX + 0x13290CC], ECX    ; g_table[arg1*48] = arg2
//   89 4c 24 08          MOV [ESP+8], ECX              ; write-back (redundant)
//   89 44 24 04          MOV [ESP+4], EAX              ; write-back (redundant)
//   e9 4f 49 00 00       JMP FUN_0041ccc0              ; tail call

extern "C" void FUN_0041ccc0();

extern "C" __declspec(naked) void FUN_00418350()
{
    __asm {
        mov     eax, dword ptr [esp + 0x4]
        mov     ecx, dword ptr [esp + 0x8]
        lea     edx, [eax + eax*2]
        shl     edx, 4
        mov     dword ptr [edx + 0x13290cc], ecx
        mov     dword ptr [esp + 0x8], ecx
        mov     dword ptr [esp + 0x4], eax
        jmp     FUN_0041ccc0
    }
}
