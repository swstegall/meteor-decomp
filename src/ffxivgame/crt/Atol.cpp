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
// MSVC 2005 CRT — `atol`. One-line wrapper over `strtol` with base-10
// fixed. Recovered 2026-05-02 from byte signature vs atol.c in the
// MSVC 2005 SP1 CRT source.

extern "C" long __cdecl strtol(const char *nptr, char **endptr, int base);

#pragma optimize("s", on)

// FUNCTION: ffxivgame 0x005d6fcc — atol (17 B)
//   6a 0a           PUSH 10
//   6a 00           PUSH 0
//   ff 74 24 0c     PUSH [ESP+0xc]
//   e8 ?? ?? ?? ??  CALL strtol
//   83 c4 0c        ADD ESP, 0xc
//   c3              RET

extern "C" long __cdecl atol(const char *nptr) {
    return strtol(nptr, 0, 10);
}

#pragma optimize("", on)
