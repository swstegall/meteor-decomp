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
// MSVC 2005 CRT — `fopen`. Wraps `_fsopen` with `_SH_DENYNO` (0x40)
// share mode. Recovered 2026-05-02 from byte signature vs fopen.c.

struct _iobuf;
typedef struct _iobuf FILE;
extern "C" FILE * __cdecl _fsopen(const char *filename, const char *mode, int shflag);

#pragma optimize("s", on)

// FUNCTION: ffxivgame 0x005d6b52 — fopen (19 B)
//   6a 40           PUSH 0x40           ; _SH_DENYNO
//   ff 74 24 0c     PUSH [ESP+0xc]      ; mode
//   ff 74 24 0c     PUSH [ESP+0xc]      ; filename
//   e8 ?? ?? ?? ??  CALL _fsopen
//   83 c4 0c        ADD ESP, 0xc
//   c3              RET

extern "C" FILE * __cdecl fopen(const char *filename, const char *mode) {
    return _fsopen(filename, mode, 0x40);
}

#pragma optimize("", on)
