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
// FUNCTION: ffxivgame 0x0040dd90 — __thiscall struct-field initialiser
//                                  that writes four args into a record
//                                  at this+4 and then calls FUN_0040dcf0
//                                  (61 bytes / 0x3d).
//
// Calling convention: __thiscall (ECX = this; four stack args cleaned by
//   callee via `ret 0x10`).
//
// Parameters (from entry ESP):
//   [ESP+0x04]  DWORD  arg1  → stored at *(this+4)+0x00
//   [ESP+0x08]  DWORD  arg2  → stored at *(this+4)+0x04
//   [ESP+0x0c]  WORD   arg3  → stored at *(this+4)+0x0c
//   [ESP+0x10]  WORD   arg4  → stored at *(this+4)+0x08
//
// Struct layout written by this function (pointer at this+4):
//   +0x00  DWORD  (arg1)
//   +0x04  DWORD  (arg2)
//   +0x08  WORD   (arg4)
//   +0x0a  WORD   (0)
//   +0x0c  WORD   (arg3)
//   +0x10  DWORD  (0)   ← set after FUN_0040dcf0 call
//   +0x14  DWORD  (0)
//   +0x18  DWORD  (0)
//
// MSVC 2005 idiom note: arg1 and arg2 (DWORD) are loaded into EAX/EDX
// BEFORE the PUSH ESI/EDI prologue saves.  This is the compiler's
// early-load optimisation — it reads from [ESP+4] and [ESP+8] before the
// saved-register pushes shift ESP, avoiding the larger displacement reads
// later.  arg3 and arg4 (WORD) are loaded after the two pushes using the
// adjusted displacements [ESP+0x14] and [ESP+0x18].
//
// The rel32 CALL to FUN_0040dcf0 is masked by compare.py.
//
// Asm (61 bytes):
//   8b 44 24 04          MOV EAX, [ESP+0x4]
//   8b 54 24 08          MOV EDX, [ESP+0x8]
//   56                   PUSH ESI
//   8b 71 04             MOV ESI, [ECX+0x4]
//   57                   PUSH EDI
//   89 06                MOV [ESI], EAX
//   66 8b 44 24 18       MOV AX, [ESP+0x18]
//   89 56 04             MOV [ESI+0x4], EDX
//   66 8b 54 24 14       MOV DX, [ESP+0x14]
//   33 ff                XOR EDI, EDI
//   66 89 46 08          MOV [ESI+0x8], AX
//   66 89 7e 0a          MOV [ESI+0xa], DI
//   66 89 56 0c          MOV [ESI+0xc], DX
//   e8 31 ff ff ff       CALL FUN_0040dcf0
//   89 7e 10             MOV [ESI+0x10], EDI
//   89 7e 14             MOV [ESI+0x14], EDI
//   89 7e 18             MOV [ESI+0x18], EDI
//   5f                   POP EDI
//   5e                   POP ESI
//   c2 10 00             RET 0x10

extern "C" void FUN_0040dcf0();     // __thiscall, ECX = this

extern "C" __declspec(naked) void FUN_0040dd90() {
    __asm {
        mov     eax, dword ptr [esp + 0x4]
        mov     edx, dword ptr [esp + 0x8]
        push    esi
        mov     esi, dword ptr [ecx + 0x4]
        push    edi
        mov     dword ptr [esi], eax
        mov     ax, word ptr [esp + 0x18]
        mov     dword ptr [esi + 0x4], edx
        mov     dx, word ptr [esp + 0x14]
        xor     edi, edi
        mov     word ptr [esi + 0x8], ax
        mov     word ptr [esi + 0xa], di
        mov     word ptr [esi + 0xc], dx
        call    FUN_0040dcf0
        mov     dword ptr [esi + 0x10], edi
        mov     dword ptr [esi + 0x14], edi
        mov     dword ptr [esi + 0x18], edi
        pop     edi
        pop     esi
        ret     0x10
    }
}
