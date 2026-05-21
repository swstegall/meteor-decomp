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
// FUNCTION: ffxivgame 0x00403cf0 — 1-arg __stdcall forwarder that calls
// FUN_00403bd0(arg, 0). Same structural pattern as the sibling
// FUN_00403cd0 (which forwards to FUN_00403b70 the same way): the
// wrapper supplies a default second argument (0) so callers that only
// pass one parameter can reach the inner helper without pushing the
// extra zero themselves.
//
// Asm (18 bytes):
//   8b 44 24 04        MOV EAX, [ESP + 4]       ; load incoming arg
//   6a 00              PUSH 0                   ; second arg = 0
//   50                 PUSH EAX                 ; first arg = original
//   e8 ?? ?? ?? ??     CALL FUN_00403bd0        ; reloc — __cdecl callee
//   83 c4 08           ADD ESP, 8               ; cdecl-style cleanup
//   c2 04 00           RET 4                    ; __stdcall pop
//
// No prologue (frame pointer omitted by /Oy). Caller cleans up the
// inner CALL's two stack args; this routine cleans up its own one
// stack arg via RET 4. Naked-asm keeps the byte layout pinned —
// MSVC's high-level lowering of an equivalent `__stdcall` wrapper
// shifts the load/push ordering depending on inliner heuristics.

extern "C" void FUN_00403bd0();

extern "C" __declspec(naked) void FUN_00403cf0() {
    __asm {
        mov eax, dword ptr [esp + 4]
        push 0
        push eax
        call FUN_00403bd0
        add esp, 8
        ret 4
    }
}
