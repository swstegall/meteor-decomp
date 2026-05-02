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
// MSVC 2005 CRT helper — `_invalid_parameter_noinfo`. Statically
// linked into ffxivgame.exe via `/MT` (the static CRT). Match
// recovered 2026-05-02 by reading the byte pattern at RVA
// 0x005d22b4 directly:
//
//   33 c0           XOR EAX, EAX           ; eax = NULL
//   50 50 50 50 50  PUSH EAX × 5           ; push five NULLs
//   e8 ?? ?? ?? ??  CALL _invalid_parameter
//   83 c4 14        ADD ESP, 0x14          ; cdecl cleanup
//   c3              RET
//
// This is the canonical Microsoft CRT signature for the no-info
// variant. The CRT source `<crt>/src/invarg.c` defines it as a
// one-line wrapper around `_invalid_parameter`. Called from
// `InstallUnpacker::Unpack` at offset 0x113 — likely a /GS or
// invalid-handle integrity check in the chunk-extraction loop.

extern "C" void __cdecl _invalid_parameter(
    void *expr,
    void *func,
    void *file,
    unsigned line,
    unsigned reserved);

// `optimize("s")` — favour size over speed for this function. Without
// it MSVC /O2 emits `PUSH 0 × 5` (10 B) for the five-NULL push;
// with it, MSVC sees the all-zero push and emits the shorter
// `XOR EAX, EAX; PUSH EAX × 5` (7 B). The MSVC CRT compiles invarg.c
// with size-priority for the same reason.
#pragma optimize("s", on)
extern "C" void __cdecl _invalid_parameter_noinfo(void) {
    _invalid_parameter(0, 0, 0, 0, 0);
}
#pragma optimize("", on)
