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
// FUNCTION: ffxivgame 0x00003cd0 — __stdcall single-arg thunk over the
// 2-arg __cdecl allocator FUN_00403b70.
//
// Asm (18 bytes @ 0x00003cd0):
//   8b 44 24 04         MOV  EAX, dword ptr [ESP + 0x4]   ; load arg
//   6a 00               PUSH 0                            ; flag = 0
//   50                  PUSH EAX                          ; count
//   e8 94 fe ff ff      CALL FUN_00403b70                 ; rel32 → 0x3b70
//   83 c4 08            ADD  ESP, 8                       ; cdecl cleanup
//   c2 04 00            RET  4                            ; stdcall ret
//
// Conventions:
//   - This function: __stdcall (RET 4 cleans the single inbound arg).
//   - Callee FUN_00403b70 is __cdecl (ADD ESP, 8 cleans 2 outbound args).
//   - Return value flows through EAX (FUN_00403b70 is an `operator new[]`
//     wrapper for a 0x54-byte element type; the cast to `void*` happens
//     implicitly by passing the EAX return through untouched).
//
// Sibling pattern: identical shape to FUN_00403c60 (over FUN_00401090).
// MSVC 2005 /O2 picks the "load arg to register, then push" form
// (MOV EAX, [ESP+4] / PUSH EAX) over the alternative "PUSH [ESP+8]"
// memory-operand form when the source names the argument as a local-like
// value.

extern "C" void * __cdecl FUN_00403b70(unsigned int count, int flag);

extern "C" void * __stdcall FUN_00403cd0(unsigned int count)
{
    return FUN_00403b70(count, 0);
}
