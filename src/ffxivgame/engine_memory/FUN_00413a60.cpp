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
// FUNCTION: ffxivgame 0x00013a60 — multi-inheritance `this`-adjustor (SUB ECX, 0x4; JMP)
//           (8 bytes)
//
// Asm (8 bytes @ orig RVA 0x00013a60):
//   83 e9 04          SUB ECX, 0x4          ; adjust `this` pointer down by 4 bytes
//   e9 RR RR RR RR   JMP FUN_00413f80      ; tail-call to inner method (reloc)
//
// MSVC compiler-generated adjustor thunk for multiple inheritance.
// Subtracts 4 from ECX (the `this` pointer) to reach the correct base-class
// sub-object, then unconditionally jumps to FUN_00413f80.

extern "C" void FUN_00413f80();

extern "C" __declspec(naked) void FUN_00413a60()
{
    __asm {
        sub ecx, 4
        jmp FUN_00413f80
    }
}
