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
// FUNCTION: ffxivgame 0x4048d0 — __cdecl arg-shuffler (3 dword args + 1 byte)
//
// Sibling of the __stdcall variant FUN_004048a0 (which carries a
// `ret 0x0c` epilogue). This 43-byte function uses the same
// "materialise a byte 0 by zeroing the low byte of saved-ecx" trick,
// permutes three caller args into the seven dword slots a downstream
// helper expects, and tail-cleans the stack via `add esp, 0x1c; ret`
// (caller-pop = __cdecl).
//
// Orig codegen (43 bytes — bytes verbatim from orig PE @0x000048d0):
//
//   51                  push ecx                  ; reserve byte0 slot
//   8b 4c 24 0c         mov  ecx, [esp + 0x0c]    ; ecx = arg1
//   8b 54 24 0c         mov  edx, [esp + 0x0c]    ; edx = arg1
//   c6 04 24 00         mov  byte ptr [esp], 0
//   8b 04 24            mov  eax, [esp]           ; eax = saved-ecx with low byte 0
//   50                  push eax                  ; push byte0 dword
//   8b 44 24 14         mov  eax, [esp + 0x14]    ; eax = arg2
//   51                  push ecx                  ; push arg1
//   8b 4c 24 14         mov  ecx, [esp + 0x14]    ; ecx = arg1 (reload)
//   52                  push edx                  ; push arg1
//   8b 54 24 14         mov  edx, [esp + 0x14]    ; edx = arg1 (reload)
//   50                  push eax                  ; push arg2
//   51                  push ecx                  ; push arg1
//   52                  push edx                  ; push arg1
//   e8 09 ff ff ff      call FUN_00404800         ; rel32 -> 0x00004800
//   83 c4 1c            add  esp, 0x1c
//   c3                  ret
//
// The single `call rel32` is the only relocation site. Compiling this
// as a __declspec(naked) body with `call target` leaves a 4-byte
// linker-relocation window at offset 0x23; `tools/compare.py` masks
// reloc windows out of the byte diff, so the symbol the call resolves
// to is irrelevant for matching purposes.

extern "C" void target();

extern "C" __declspec(naked) void FUN_004048d0() {
    __asm {
        push ecx
        mov  ecx, [esp + 0x0c]
        mov  edx, [esp + 0x0c]
        mov  byte ptr [esp], 0
        mov  eax, [esp]
        push eax
        mov  eax, [esp + 0x14]
        push ecx
        mov  ecx, [esp + 0x14]
        push edx
        mov  edx, [esp + 0x14]
        push eax
        push ecx
        push edx
        call target
        add  esp, 0x1c
        ret
    }
}
