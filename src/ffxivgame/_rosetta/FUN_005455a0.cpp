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
// FUNCTION: ffxivgame 0x005455a0 — multi-inheritance `this`-adjustor
// thunk (SUB ECX, 8; JMP rel32). MSVC compiler-emits these for
// virtual overrides where the override class is reached via a
// secondary base — the adjustor adjusts `this` back from the
// secondary base subobject to the derived class before jumping
// into the actual override.
//
// Asm (8 bytes):
//   83 e9 08         SUB ECX, 0x8        ; this-pointer adjustment
//   e9 ?? ?? ?? ??   JMP target          ; reloc — tail-call to override
//
// We approximate with `__declspec(naked)` + inline asm — produces
// exactly the same bytes as the compiler-emitted thunk, with the
// JMP target being a regular extern C function.

extern "C" int target();

extern "C" __declspec(naked) void adjustor_thunk_8() {
    __asm {
        sub ecx, 8
        jmp target
    }
}
