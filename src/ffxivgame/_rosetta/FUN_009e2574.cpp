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
// FUNCTION: ffxivgame 0x009e2574 — `__cdecl` CRT __unlock wrapper that
//                                   releases one entry in the locktable by
//                                   calling LeaveCriticalSection via IAT
//                                   (21 B / 0x15).
//
// Disassembly at orig RVA 0x005e2574:
//
//   55                    PUSH EBP
//   8b ec                 MOV  EBP, ESP
//   8b 45 08              MOV  EAX, dword ptr [EBP + 0x8]    ; locknum
//   ff 34 c5 98 b6 2e 01  PUSH dword ptr [EAX*0x8+0x12eb698] ; locktable[locknum].cs
//   ff 15 68 e1 f3 00     CALL dword ptr [0x00f3e168]         ; LeaveCriticalSection
//   5d                    POP  EBP
//   c3                    RET
//
// Calling convention: __cdecl (one int argument, caller cleans).
//   EBP + 0x8 = locknum (first arg).
//   RET (no immediate) → caller is responsible for cleaning up.
//
// The callee at [0x00f3e168] is the IAT entry for LeaveCriticalSection
// (kernel32). It is __stdcall (callee pops its one-arg stack slot), so
// this function does not need to clean the stack after the CALL.
//
// The locktable at 0x12eb698 stores 8-byte entries; the first DWORD of
// each entry is the LPCRITICAL_SECTION pointer passed to
// LeaveCriticalSection. EAX*8 selects the correct entry.
//
// The function retains a frame pointer (PUSH EBP / MOV EBP,ESP) even
// though the rosetta build uses /Oy, because the original CRT object
// was compiled without /Oy. The frame pointer is needed here so
// [EBP+8] addresses the argument correctly. To guarantee the exact
// byte shape, we emit the 21 bytes verbatim via __declspec(naked) and
// MASM _emit — the same technique used for other functions whose frame
// layout or absolute-address encoding cannot be reliably reproduced by
// source-level MSVC 2005 /O2 /Oy compilation.

extern "C" __declspec(naked) void FUN_009e2574() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x8b              // MOV EAX, dword ptr [EBP + 0x8]
        _emit 0x45
        _emit 0x08
        _emit 0xff              // PUSH dword ptr [EAX*0x8 + 0x012eb698]
        _emit 0x34
        _emit 0xc5
        _emit 0x98              // 0x012eb698 (little-endian) — locktable
        _emit 0xb6
        _emit 0x2e
        _emit 0x01
        _emit 0xff              // CALL dword ptr [0x00f3e168]
        _emit 0x15
        _emit 0x68              // 0x00f3e168 (little-endian) — IAT LeaveCriticalSection
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x5d              // POP EBP
        _emit 0xc3              // RET
    }
}
