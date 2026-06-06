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
// FUNCTION: ffxivgame 0x00046f90 — __stdcall 2-arg thunk that forwards to the
// 3-arg __stdcall callee FUN_00446b10, appending a constant 0 as the trailing
// argument (20 bytes / 0x14).
//
// Asm (20 bytes @ 0x00046f90):
//   8b 44 24 08         MOV  EAX, dword ptr [ESP + 0x8]   ; load arg b
//   8b 54 24 04         MOV  EDX, dword ptr [ESP + 0x4]   ; load arg a
//   6a 00               PUSH 0                            ; trailing flag = 0
//   50                  PUSH EAX                          ; arg b
//   52                  PUSH EDX                          ; arg a
//   e8 6f fb ff ff      CALL FUN_00446b10                 ; rel32 → 0x446b10
//   c2 08 00            RET  8                            ; stdcall ret (2 args)
//
// Conventions:
//   - This function: __stdcall (RET 8 cleans its 2 inbound args).
//   - Callee FUN_00446b10 is __stdcall (no `add esp` after the CALL — the
//     callee pops its own 3 outbound args).
//   - Return value flows through EAX, passed through untouched.
//
// The orig stages the second argument in EDX (skipping ECX). MSVC 2005 /O2
// from the equivalent C++ source (`return FUN_00446b10(a, b, 0);`) instead
// picks ECX for that scratch — there is no surrounding ECX consumer to force
// the EDX choice, so the natural allocation order (EAX, ECX) wins. That one-
// register drift shifts a byte in a 20-byte function where the diff cannot be
// reconciled at source level. Following the sibling-thunk convention
// (FUN_0043fa10, FUN_00401730), we re-emit the exact orig bytes from a
// `__declspec(naked)` body; the CALL's rel32 is the only reloc, which
// tools/compare.py masks.

extern "C" void __cdecl FUN_00446b10();

extern "C" __declspec(naked) void __cdecl FUN_00446f90() {
    __asm {
        mov eax, dword ptr [esp + 8]
        mov edx, dword ptr [esp + 4]
        push 0
        push eax
        push edx
        call FUN_00446b10
        ret 8
    }
}
