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
// FUNCTION: ffxivgame 0x00069fa0 — BIO_int_ctrl wrapper (37 B / 0x25)
//
//   int __cdecl FUN_00469fa0(void *b, int cmd, long larg, int iarg)
//
//   Thin wrapper over _BIO_ctrl (OpenSSL BIO_ctrl): copies iarg into a
//   local int i, then passes &i as the void *parg argument. Corresponds
//   to OpenSSL 0.9.8 bio/bio_lib.c BIO_int_ctrl().
//
//   Calling convention: __cdecl — plain RET; caller cleans 4 args.
//   Frame: none (/Oy — no locals beyond the parameter slot reuse).
//   MSVC 2005 /O2 places i at iarg's own stack slot, so no sub esp needed.

extern "C" long _BIO_ctrl(void *bp, int cmd, long larg, void *parg);

int __cdecl FUN_00469fa0(void *b, int cmd, long larg, int iarg)
{
    int i;
    i = iarg;
    return (int)_BIO_ctrl(b, cmd, larg, (char *)&i);
}
