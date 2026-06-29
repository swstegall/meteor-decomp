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
// FUNCTION: ffxivgame 0x0043f6d0 — string _Tidy/__stdcall clear (49 B / 0x31)
//
// Resets a string-like object's buffer to the "small" (inline) state.
// If capacity (at +0x18) is >= 16 and the heap-buffer pointer (at +0x04)
// is non-null, calls FUN_0040df70 (__thiscall; ECX = *(ptr-4), arg = ptr)
// to release the allocation, then resets:
//   [param+0x18] = 0x0f  (capacity back to small-string inline limit)
//   [param+0x14] = 0x00  (size = 0)
//   byte [param+0x04] = 0x00  (null-terminate inline buffer)
//
// Calling convention: __stdcall (callee cleans 4 bytes, `ret 4`).
// Stack layout (after PUSH ESI):
//   [ESP+0x04]  return address
//   [ESP+0x08]  param1 — pointer to the string object
// Frame: PUSH ESI / POP ESI only (no sub ESP).
//
// Asm (49 bytes @ orig RVA 0x0003f6d0):
//   56                       PUSH ESI
//   8b 74 24 08              MOV ESI, [ESP+0x8]
//   83 7e 18 10              CMP dword ptr [ESI+0x18], 0x10
//   72 10                    JC  +0x10   → skip (small-string, no free)
//   8b 46 04                 MOV EAX, [ESI+0x4]
//   85 c0                    TEST EAX, EAX
//   74 09                    JZ  +0x09   → skip (null ptr, no free)
//   8b 48 fc                 MOV ECX, [EAX-0x4]   ; owner = *(ptr-4)
//   50                       PUSH EAX              ; arg = ptr
//   e8 85 e8 fc ff           CALL FUN_0040df70     ; free the buffer
// skip:
//   c7 46 18 0f 00 00 00     MOV dword ptr [ESI+0x18], 0x0f
//   c7 46 14 00 00 00 00     MOV dword ptr [ESI+0x14], 0x0
//   c6 46 04 00              MOV byte ptr  [ESI+0x4],  0x0
//   5e                       POP ESI
//   c2 04 00                 RET 4
//
// Reconstruction: naked asm (same idiom as FUN_0043bc60 / FUN_004381c0).
// The CALL FUN_0040df70 emits a REL32 reloc masked by tools/compare.py.

extern "C" void FUN_0040df70();

extern "C" __declspec(naked) void FUN_0043f6d0()
{
    __asm {
        push    esi
        mov     esi, dword ptr [esp + 0x8]
        cmp     dword ptr [esi + 0x18], 0x10
        jc      skip
        mov     eax, dword ptr [esi + 0x4]
        test    eax, eax
        jz      skip
        mov     ecx, dword ptr [eax - 0x4]
        push    eax
        call    FUN_0040df70
    skip:
        mov     dword ptr [esi + 0x18], 0x0f
        mov     dword ptr [esi + 0x14], 0x0
        mov     byte ptr [esi + 0x4], 0x0
        pop     esi
        ret     4
    }
}
