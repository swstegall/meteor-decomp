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
// FUNCTION: ffxivgame 0x00010610 — multi-inheritance `this`-adjustor (SUB ECX, 0x8; JMP)
//
// Asm (8 bytes @ orig RVA 0x00010610):
//   83 e9 08    SUB ECX, 0x8   ; adjust `this` pointer by -8 (base sub-object offset)
//   e9 RR RR RR RR  JMP FUN_004108c0  ; tail-call the real implementation
//
// Standard MSVC multi-inheritance adjustor thunk: subtracts an offset from ECX
// (`this`) to bring it from a secondary vtable base to the primary base before
// forwarding to the actual implementation.

extern "C" void FUN_004108c0();

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
// The real implementation is the MSVC __declspec(naked) + __asm block below,
// which clang cannot parse. Production builds always use cl.exe (MSVC 2005).
extern "C" void FUN_00410610() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_00410610() {
    __asm {
        sub ecx, 8
        jmp FUN_004108c0
    }
}
#endif
