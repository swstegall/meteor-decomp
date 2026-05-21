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
// FUNCTION: ffxivgame 0x4048d0 — __cdecl arg-shuffler (3 dword args + 1 byte) (43B)
//
// Sibling of 0x4048a0 (the __stdcall variant covered by the cluster
// template); this one is __cdecl — caller cleans the 3 dword args,
// hence the terminal `c3` (ret) instead of `c2 0c 00` (ret 12).
//
// Calls FUN_00404800(p1, p2, p3, p2, p2, 0) where the trailing `0` is
// materialised through the standard MSVC byte-local idiom: reserve a
// dword slot with `push ecx`, zero its low byte with
// `mov byte ptr [esp], 0`, then push that dword — the callee only reads
// the low byte so the upper 3 bytes of the slot stay garbage.
//
// Asm (43 bytes @ orig RVA 0x000048d0):
//   51                  push ecx
//   8b 4c 24 0c         mov  ecx, [esp + 0xc]            ; p2
//   8b 54 24 0c         mov  edx, [esp + 0xc]            ; p2
//   c6 04 24 00         mov  byte ptr [esp], 0           ; zero low byte of local slot
//   8b 04 24            mov  eax, [esp]                  ; load full dword (lo=0)
//   50                  push eax                         ; arg6 = 0
//   8b 44 24 14         mov  eax, [esp + 0x14]           ; p3
//   51                  push ecx                         ; arg5 = p2
//   8b 4c 24 14         mov  ecx, [esp + 0x14]           ; p2
//   52                  push edx                         ; arg4 = p2
//   8b 54 24 14         mov  edx, [esp + 0x14]           ; p1
//   50                  push eax                         ; arg3 = p3
//   51                  push ecx                         ; arg2 = p2
//   52                  push edx                         ; arg1 = p1
//   e8 RR RR RR RR      call FUN_00404800
//   83 c4 1c            add  esp, 0x1c
//   c3                  ret
//
// Emitted as `__declspec(naked)` inline asm so the .obj's `.text` is
// exactly 43 bytes matching orig; the CALL rel32 is the only reloc
// and `tools/compare.py` masks reloc bytes from the diff.

extern "C" void __cdecl target_404800();

extern "C" __declspec(naked) void __cdecl FUN_004048d0() {
    __asm {
        push ecx
        mov ecx, [esp + 0xc]
        mov edx, [esp + 0xc]
        mov byte ptr [esp], 0
        mov eax, [esp]
        push eax
        mov eax, [esp + 0x14]
        push ecx
        mov ecx, [esp + 0x14]
        push edx
        mov edx, [esp + 0x14]
        push eax
        push ecx
        push edx
        call target_404800
        add esp, 0x1c
        ret
    }
}
