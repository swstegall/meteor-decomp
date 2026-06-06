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
// FUNCTION: ffxivgame 0x0044e1b0 — __stdcall single-arg thunk over the
// 2-arg __cdecl allocator FUN_0096bef0.
//
// Asm (18 bytes @ 0x0004e1b0):
//   8b 44 24 04         MOV  EAX, dword ptr [ESP + 0x4]   ; load arg
//   6a 00               PUSH 0                            ; flag = 0
//   50                  PUSH EAX                          ; count
//   e8 34 dd 51 00      CALL FUN_0096bef0                 ; rel32 → 0x56bef0
//   83 c4 08            ADD  ESP, 8                       ; cdecl cleanup
//   c2 04 00            RET  4                            ; stdcall ret
//
// Conventions:
//   - This function: __stdcall (RET 4 cleans the single inbound arg).
//   - Callee FUN_0096bef0 is __cdecl (ADD ESP, 8 cleans 2 outbound args);
//     it loads [ESP+4] (count) and ignores [ESP+8] (the discarded type/flag
//     slot). It is an overflow-checking allocation helper that ultimately
//     forwards to operator new[].
//   - Return value flows through EAX (the callee's return is passed through
//     untouched; the implicit cast to void* happens at the call boundary).
//
// Sibling pattern: identical shape to FUN_00403cd0 (over FUN_00403b70) and
// FUN_00403c60 (over FUN_00401090). MSVC 2005 /O2 picks the "load arg to
// register, then push" form (MOV EAX, [ESP+4] / PUSH EAX) over the
// alternative "PUSH [ESP+8]" memory-operand form when the source names the
// argument as a local-like value.

extern "C" void * __cdecl FUN_0096bef0(unsigned int count, int flag);

extern "C" void * __stdcall FUN_0044e1b0(unsigned int count)
{
    return FUN_0096bef0(count, 0);
}
