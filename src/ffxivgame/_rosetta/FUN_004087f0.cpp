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
// FUNCTION: ffxivgame 0x004087f0 — MSVC 2005 STL `std::_Med3` (median-
//                                  of-three swap) instantiated for an
//                                  iterator over a 64-byte char[] record
//                                  (284 B / 0x11c).
//
// The 64-byte record is `strcmp`-compared (so the records are
// null-terminated strings padded out to 64 bytes — probably fixed-width
// path/name buffers). The companion `std::_Median` (FUN_004089f0)
// calls this four times to build a Tukey ninther pivot — see that file
// for the surrounding `<algorithm>` template instantiation.
//
//   template<class _RanIt>
//   void _Med3(_RanIt _First, _RanIt _Mid, _RanIt _Last)
//   {
//       if (_DEBUG_LT(*_Mid,   *_First)) swap(*_First, *_Mid);
//       if (_DEBUG_LT(*_Last,  *_Mid))   swap(*_Mid,   *_Last);
//       if (_DEBUG_LT(*_Mid,   *_First)) swap(*_First, *_Mid);
//   }
//
// With `_DEBUG_LT` inlining to `strcmp(b, a) < 0` and `swap` inlining
// to a 3-step `memcpy` round through a 64-byte stack temp. MSVC 2005's
// `/Oi` (implied by `/O2`) inlines both intrinsics: strcmp into the
// canonical byte-pair compare loop with SBB-based sign normalisation,
// and memcpy of a constant 64-byte length into `mov ecx, 16; rep
// movsd`. The outer prologue allocates exactly the temp buffer (`sub
// esp, 0x40`) and saves EBX/EBP/ESI/EDI; EBX caches `_First` /
// `_Last` / `_First` across the three comparisons (re-read from the
// stack between blocks), EBP holds `_Mid` throughout.
//
// `/GS` does not fire on the temp buffer because we declare it as
// `unsigned int tmp[16]` — MSVC 2005's stack-cookie heuristic only
// triggers on 1/2-byte element arrays of ≥5 bytes (the "string buffer"
// classification). A `char tmp[64]` here would force a `__security_cookie`
// prologue that orig doesn't have.

#include <string.h>

#pragma intrinsic(strcmp, memcpy)

extern "C" void FUN_004087f0(char *a, char *b, char *c) {
    unsigned int tmp[16];

    if (strcmp(b, a) < 0) {
        memcpy(tmp, b, 64);
        memcpy(b, a, 64);
        memcpy(a, tmp, 64);
    }

    if (strcmp(c, b) < 0) {
        memcpy(tmp, c, 64);
        memcpy(c, b, 64);
        memcpy(b, tmp, 64);
    }

    if (strcmp(b, a) < 0) {
        memcpy(tmp, b, 64);
        memcpy(b, a, 64);
        memcpy(a, tmp, 64);
    }
}
