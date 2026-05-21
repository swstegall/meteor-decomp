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
// FUNCTION: ffxivgame 0x00403c80 — 4-arg __cdecl thunk over `memmove_s`
// that returns the destination pointer instead of the bounds-check
// `errno_t`. Same structural pattern as the inline wrappers MSVC
// emits when callers want a `memmove_s`-style copy but expect the
// classic `memmove`/`memcpy` return value (dest).
//
// Asm (33 bytes):
//   8b 44 24 10        MOV EAX, [ESP + 0x10]    ; arg4 = count
//   8b 4c 24 0c        MOV ECX, [ESP + 0x0c]    ; arg3 = src
//   8b 54 24 08        MOV EDX, [ESP + 0x08]    ; arg2 = destsz
//   56                 PUSH ESI                  ; save callee-save
//   8b 74 24 08        MOV ESI, [ESP + 0x08]    ; arg1 = dest (after PUSH)
//   50                 PUSH EAX                  ; push count
//   51                 PUSH ECX                  ; push src
//   52                 PUSH EDX                  ; push destsz
//   56                 PUSH ESI                  ; push dest
//   e8 ?? ?? ?? ??     CALL _memmove_s           ; rel32 reloc
//   83 c4 10           ADD ESP, 0x10             ; cdecl cleanup
//   8b c6              MOV EAX, ESI              ; return dest
//   5e                 POP ESI
//   c3                 RET
//
// Calling convention: __cdecl (caller cleans up). Stack frame: -4
// (PUSH ESI / POP ESI bracket only). The pre-PUSH reads of EAX/ECX/EDX
// pin the arg-load ordering MSVC emits for this idiom — load all
// non-dest args before perturbing ESP, then re-load dest via the
// shifted offset after PUSH ESI.

extern "C" int memmove_s();

extern "C" __declspec(naked) void FUN_00403c80() {
    __asm {
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x0c]
        mov edx, dword ptr [esp + 0x08]
        push esi
        mov esi, dword ptr [esp + 0x08]
        push eax
        push ecx
        push edx
        push esi
        call memmove_s
        add esp, 0x10
        mov eax, esi
        pop esi
        ret
    }
}
