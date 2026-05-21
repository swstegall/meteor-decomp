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
// FUNCTION: ffxivgame 0x401000 — __cdecl 3-arg fan-out wrapper (29B)
//
// Trivial `__cdecl` wrapper that forwards its first two dword args to
// a 2-arg helper at VA 0x00440230, then its third dword arg to a 1-arg
// helper at VA 0x00441720. MSVC 2005 /O2 folds the two callee
// stack-cleanups into a single `add esp, 12` after the second call.
// Arg3 is reloaded from `[esp + 0x14]` because the two pushes for the
// first call shifted it from `[esp + 0xc]` down by 8 bytes.
//
// Asm (29 bytes):
//   8b 44 24 08         MOV EAX, dword ptr [ESP + 8]      ; arg2
//   8b 4c 24 04         MOV ECX, dword ptr [ESP + 4]      ; arg1
//   50                  PUSH EAX
//   51                  PUSH ECX
//   e8 RR RR RR RR      CALL helper_2arg                  ; VA 0x00440230
//   8b 54 24 14         MOV EDX, dword ptr [ESP + 0x14]   ; arg3
//   52                  PUSH EDX
//   e8 RR RR RR RR      CALL helper_1arg                  ; VA 0x00441720
//   83 c4 0c            ADD ESP, 0xc                      ; cdecl cleanup (2+1)
//   c3                  RET

extern "C" int helper_2arg();
extern "C" int helper_1arg();

extern "C" __declspec(naked) void __cdecl fan_out_3arg_wrapper() {
    __asm {
        mov eax, dword ptr [esp + 8]
        mov ecx, dword ptr [esp + 4]
        push eax
        push ecx
        call helper_2arg
        mov edx, dword ptr [esp + 0x14]
        push edx
        call helper_1arg
        add esp, 0xc
        ret
    }
}
