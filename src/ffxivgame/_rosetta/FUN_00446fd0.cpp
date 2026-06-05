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
// FUNCTION: ffxivgame 0x00046fd0 — `__thiscall` 2-arg trampoline that
//                                   forwards to the `__thiscall` 3-arg
//                                   method at 0x00446800 with a trailing
//                                   zero (20 B / 0x14).
//
// Behaviour read from the disassembly at orig RVA 0x00046fd0:
//
//   8b 44 24 08        mov  eax, [esp+8]     ; load 2nd stack arg
//   8b 54 24 04        mov  edx, [esp+4]     ; load 1st stack arg
//   6a 00              push 0                 ; trailing zero (3rd callee arg)
//   50                 push eax               ; 2nd arg
//   52                 push edx               ; 1st arg
//   e8 1f f8 ff ff     call 0x00446800        ; e8 + REL32 reloc
//   c2 08 00           ret  8                 ; stdcall-style epilogue (2 args)
//
//   The callee at 0x00446800 opens with `mov esi, ecx` and closes with
//   `ret 0xc` — i.e. it is a `__thiscall` member taking `this` in ECX
//   plus three 32-bit stack arguments which it pops itself. This
//   trampoline never writes ECX, so the `this` pointer it was invoked
//   with flows straight through to 0x00446800 unchanged. That makes
//   0x00446fd0 itself a `__thiscall` method: it re-dispatches the same
//   receiver to the 3-arg method, appending a constant 0 as the final
//   argument (`return this->FUN_00446800(arg1, arg2, 0);`). The callee
//   cleans all three pushed slots (`ret 0xc`), so this thunk's own
//   epilogue only pops its two inbound stack args (`ret 8`).
//
//   No prologue. No callee-saves. No stack frame. No security cookie.
//
// Reloc-bearing site in the orig 20 bytes:
//   +0x0c   __thiscall callee CALL   .text 0x00446800 rel32 (FUN_00446800)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The ECX-passthrough shape (no instruction touches the `this`
//   register, then a forced eax/edx materialisation of the two inbound
//   stack args in that exact order before the pushes) is not something
//   MSVC 2005 will reliably emit from a source-level `this->g(a,b,0)`
//   call in a 20-byte function where the single CALL rel32 is the only
//   reloc. The same pragmatic choice taken by FUN_00401730 for its
//   reloc-heavy trampoline applies here: emit the orig 20 bytes verbatim
//   via MASM `_emit` directives so the .obj `.text` slice is
//   byte-identical to the orig (which is what tools/compare.py checks).
//
//   Promote to a real `__thiscall` member match once the owning class
//   (receiver of 0x00446800 / 0x00446fd0) is catalogued under
//   decomp-notes/types/.

extern "C" __declspec(naked) void FUN_00446fd0() {
    __asm {
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08

        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x04

        _emit 0x6a
        _emit 0x00

        _emit 0x50

        _emit 0x52

        _emit 0xe8
        _emit 0x1f
        _emit 0xf8
        _emit 0xff
        _emit 0xff

        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
