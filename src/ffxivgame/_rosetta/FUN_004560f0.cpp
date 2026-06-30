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
// FUNCTION: ffxivgame 0x004560f0 — 2-arg __cdecl double-push wrapper that
//                                  calls FUN_00456060 twice, once for each
//                                  argument (24 B / 0x18).
//
// Calling convention: __cdecl, two arguments; bare RET (caller cleans).
// No stack frame (no PUSH EBP / MOV EBP,ESP prologue).
//
// Behaviour:
//   1. Load arg2 into EAX from [ESP+0x8], push it, call FUN_00456060(arg2).
//   2. Load arg1 into ECX from [ESP+0x8] — after the first PUSH, ESP is
//      ESP0-4, so [ESP+0x8] now equals the original [ESP+0x4] = arg1.
//   3. Push ECX, call FUN_00456060(arg1).
//   4. ADD ESP,8 (batch-cleanup both pushes), RET.
//
// The batched ADD ESP,8 (no intermediate cleanup between the two calls) is
// an MSVC 2005 /O2 call-site optimisation.  The register sequence EAX first,
// ECX second is exactly what the compiler produces for a two-call sequence
// with no frame pointer.
//
// Asm (24 bytes):
//   8b 44 24 08     MOV EAX, [ESP+0x8]     ; arg2
//   50              PUSH EAX
//   e8 RR RR RR RR  CALL FUN_00456060      ; rel32 reloc
//   8b 4c 24 08     MOV ECX, [ESP+0x8]     ; arg1 (ESP is now ESP0-4)
//   51              PUSH ECX
//   e8 RR RR RR RR  CALL FUN_00456060      ; rel32 reloc
//   83 c4 08        ADD ESP, 0x8
//   c3              RET

extern "C" void FUN_00456060();

extern "C" __declspec(naked) void FUN_004560f0() {
    __asm {
        mov  eax, dword ptr [esp + 8]
        push eax
        call FUN_00456060
        mov  ecx, dword ptr [esp + 8]
        push ecx
        call FUN_00456060
        add  esp, 8
        ret
    }
}
