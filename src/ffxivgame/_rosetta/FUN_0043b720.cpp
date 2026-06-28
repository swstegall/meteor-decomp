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
// FUNCTION: ffxivgame 0x0003b720 — __thiscall thread-safe indexed store
//                                  into a fixed-capacity int array, guarded
//                                  by an inline CRITICAL_SECTION (69 B / 0x45).
//
// Layout inferred from asm (ECX = this = ESI):
//   +0x0c  int*   pBegin  — base pointer of element array
//   +0x10  int*   pEnd    — one-past-end pointer (capacity boundary)
//   +0x20  int    index   — current insertion position / logical size
//   +0x24  CRITICAL_SECTION cs
//
// One __stdcall stack argument: the int value to store (RET 0x4).
//
// Body (matches asm flow exactly):
//   EnterCriticalSection(&this->cs);           // lock
//   ECX = this->pBegin;
//   EDI = this->index;
//   // bounds check: pBegin must be non-null AND index < capacity
//   if (ECX == NULL  ||  EDI >= (this->pEnd - this->pBegin) / 4)
//       FUN_009d22b4();                         // out-of-range / assert handler
//   this->pBegin[EDI] = arg1;                  // store value at index
//   ++this->index;
//   LeaveCriticalSection(&this->cs);           // unlock
//   // RET 0x4 — one DWORD arg cleaned from caller stack
//
// IAT entries confirmed by FUN_00406ff0 which uses the same addresses:
//   0x00f3e16c → kernel32!EnterCriticalSection
//   0x00f3e168 → kernel32!LeaveCriticalSection
//
// FUN_009d22b4 is the standard MSVC 2005 _invalid_parameter_noinfo /
// std::_Xran throw helper (also referenced by FUN_00444e40, FUN_004494d0,
// FUN_00451bf0, FUN_004328a0).
//
// Reconstruction strategy — naked __asm, same as FUN_00406ff0:
//   The two IAT indirect-call sites (`ff 15 RR RR RR RR`) and the one
//   REL32 to FUN_009d22b4 (`e8 RR RR RR RR`) are masked by
//   tools/compare.py as relocation wildcards.
//   JZ / JC are short-form (2 bytes each) because their targets are
//   within ±127 bytes of the next instruction.

extern "C" int g_imp_EnterCriticalSection;     // kernel32 IAT @ 0x00f3e16c
extern "C" int g_imp_LeaveCriticalSection;     // kernel32 IAT @ 0x00f3e168
extern "C" void FUN_009d22b4();

extern "C" __declspec(naked) void FUN_0043b720() {
    __asm {
        push    ebx
        push    esi
        mov     esi, ecx
        push    edi
        lea     ebx, [esi + 0x24]
        push    ebx
        call    dword ptr [g_imp_EnterCriticalSection]
        mov     ecx, dword ptr [esi + 0xc]
        test    ecx, ecx
        mov     edi, dword ptr [esi + 0x20]
        jz      range_fail
        mov     eax, dword ptr [esi + 0x10]
        sub     eax, ecx
        sar     eax, 0x2
        cmp     edi, eax
        jc      do_store
    range_fail:
        call    FUN_009d22b4
    do_store:
        mov     eax, dword ptr [esi + 0xc]
        mov     ecx, dword ptr [esp + 0x10]
        mov     dword ptr [eax + edi*4], ecx
        add     dword ptr [esi + 0x20], 0x1
        push    ebx
        call    dword ptr [g_imp_LeaveCriticalSection]
        pop     edi
        pop     esi
        pop     ebx
        ret     0x4
    }
}
