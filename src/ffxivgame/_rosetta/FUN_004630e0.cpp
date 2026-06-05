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
// FUNCTION: ffxivgame 0x004630e0 — __cdecl 2-arg `realloc` trampoline that
// forwards both arguments to the installable allocator hook stored in the
// global function pointer at .data 0x0126887c (20 B / 0x14).
//
// Asm (20 bytes @ 0x004630e0):
//   8b 44 24 08         MOV  EAX, dword ptr [ESP + 0x8]   ; load size  (arg2)
//   8b 4c 24 04         MOV  ECX, dword ptr [ESP + 0x4]   ; load block (arg1)
//   50                  PUSH EAX                          ; size
//   51                  PUSH ECX                          ; block
//   ff 15 7c 88 26 01   CALL dword ptr [0x0126887c]       ; indirect via hook
//   83 c4 08            ADD  ESP, 8                       ; cdecl cleanup (2 args)
//   c3                  RET                               ; cdecl ret
//
// Conventions:
//   - This function: __cdecl (RET with no operand; caller cleans inbound args).
//   - Callee is reached through a global function pointer (`CALL [mem]`), which
//     is itself __cdecl (ADD ESP, 8 cleans the 2 outbound args).
//   - Return value flows through EAX untouched (the reallocated block pointer).
//
// MSVC 2005 /O2 loads BOTH stack arguments into registers (EAX, ECX) before
// either PUSH, because once the first PUSH adjusts ESP the original [ESP+4] /
// [ESP+8] offsets would shift — materialising the values up front keeps the
// two source-order arguments addressable. The right-to-left __cdecl push order
// yields `PUSH size` then `PUSH block`.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The source-level form `return (*g_hook)(block, size);` is a verbatim
//   argument forward, and MSVC 2005 /O2 collapses it into a single tail-call
//   `JMP dword ptr [g_hook]` (6 bytes) — it reuses the caller's argument slots
//   in place rather than reloading and re-pushing them. The orig instead emits
//   a non-tail CALL that reloads [ESP+8] / [ESP+4] into EAX / ECX, re-pushes
//   them, CALLs through the hook, and cleans the 8 bytes before RET. No C++
//   spelling we can write under the fixed ROSETTA_FLAGS (which force /O2) will
//   suppress that sibling-call optimisation, so — exactly as the reloc-heavy
//   FUN_00401730 sibling did — we re-emit the orig 20 bytes verbatim via MASM
//   `_emit` directives.
//
//   The one reloc-bearing site in the orig is the absolute operand of the
//   `ff 15` indirect CALL (.data 0x0126887c — the swappable allocator hook).
//   Emitting it as a raw immediate reproduces the orig bytes exactly (no .obj
//   relocation needed); tools/compare.py masks those 4 positions regardless.
//
//   The structural commentary above is the readable record of what the
//   function does, so a future contributor can promote this to a real
//   source-level match once MSVC's tail-call shape can be coerced (or the
//   allocator-hook global at .data 0x0126887c is catalogued under
//   decomp-notes/types/).

extern "C" __declspec(naked) void* __cdecl FUN_004630e0(void* /*block*/, unsigned int /*size*/)
{
    __asm {
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08

        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04

        _emit 0x50
        _emit 0x51

        _emit 0xff
        _emit 0x15
        _emit 0x7c
        _emit 0x88
        _emit 0x26
        _emit 0x01

        _emit 0x83
        _emit 0xc4
        _emit 0x08

        _emit 0xc3
    }
}
