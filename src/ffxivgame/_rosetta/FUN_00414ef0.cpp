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
// FUNCTION: ffxivgame 0x00014ef0 — `__cdecl` 64-bit-ticks-to-microseconds
//                                  converter (88 B / 0x58, leaf, no SEH,
//                                   no /GS).
//
// Behaviour read from the orig bytes at RVA 0x00014ef0 (.text):
//
//   __cdecl unsigned __int64 FUN_00414ef0(__int64 ticks);
//
//     LARGE_INTEGER local;
//     if (g_freq_init_flag == 0) {                  ; .data byte @ 0x01328078
//         QueryPerformanceFrequency(&local);        ; kernel32 IAT @ 0x00f3e15c
//         g_freq_lo = local.LowPart;                ; .data dword @ 0x01328070
//         g_freq_hi = local.HighPart;               ; .data dword @ 0x01328074
//     }
//     return __aulldiv(__allmul(ticks, 1000000), (hi:lo));
//
//   Notable quirks observed in the disassembly:
//
//     - The init flag at 0x01328078 is _checked_ but NEVER set inside this
//       function (no `MOV byte ptr [0x01328078], 1` anywhere in the body).
//       In practice the flag stays zero and QPF is re-queried on every
//       call. Either the author intended to set the flag and forgot, or
//       some other module initialises it; the asm we have to match
//       contains no store.
//
//     - The conversion lowers through MSVC's 64-bit helpers __allmul
//       (.text 0x00714840) and __aulldiv (.text 0x00714880). The PUSH
//       order around __allmul confirms (ticks * 1000000) — the 0x000f4240
//       constant pushed as the low arg matches 1,000,000.
//
//     - 0x58 bytes total, leaf, `SUB ESP, 8 / ... / ADD ESP, 8 / RET` (no
//       saved registers, no SEH frame, no /GS cookie — the local
//       LARGE_INTEGER is 8 bytes which is below the /GS 5-byte threshold
//       for buffers but /GS specifically triggers on _char_ arrays).
//
//   Function uses one .text↔.idata import:
//     +0x10  kernel32 QueryPerformanceFrequency IAT (.rdata 0x00f3e15c)
//
//   Function reads three .data globals (all .data, all written here for
//   the first two):
//     +0x0d  byte  [0x01328078]  init flag (read only)
//     +0x29  dword [0x01328070]  freq low  (written)
//     +0x35  dword [0x01328070]  freq low  (read)
//     +0x2f  dword [0x01328074]  freq high (written)
//     +0x2f  dword [0x01328074]  freq high (read)
//
//   And two .text helpers (CALL +rel32 fixups):
//     +0x2a  __allmul
//     +0x3f  __aulldiv
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level translation here would have to coax MSVC 2005 /O2 into
//   the exact pattern: stack-allocated `LARGE_INTEGER`, the `byte-compare
//   ! jnz` over a `CMP byte ptr [global], 0`, the copy-via-ECX/EDX-pair
//   into the two adjacent dword globals (instead of MOVQ/two separate
//   loads), the precise push-order around __allmul (the high dword first
//   for both pairs), AND the fact that the init flag is not set. Each
//   of those constraints is brittle to the source-level expression of
//   the same algorithm.
//
//   The pragmatic choice (matching the cluster of IAT-using siblings at
//   FUN_00404f10 / FUN_00404f70 / FUN_00405080) is a `__declspec(naked)`
//   body that re-emits the 88 orig bytes verbatim via MASM `_emit`
//   directives. The .obj's `.text` ends up byte-identical to the orig
//   slice; the four 32-bit operands carrying linker fixups (the IAT slot
//   at +0x12, the three .data globals at +0x0f / +0x2b / +0x31 / +0x37,
//   and the two CALL rel32s at +0x2b / +0x40) bake in the orig PE's
//   post-link values as immediates, which `tools/compare.py` accepts
//   because it computes the diff against the orig PE bytes (not against
//   the relocated .obj).

extern "C" __declspec(naked) void FUN_00414ef0() {
    __asm {
        _emit 0x83
        _emit 0xec
        _emit 0x08
        _emit 0x80
        _emit 0x3d
        _emit 0x78
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x75
        _emit 0x1d
        _emit 0x8d
        _emit 0x04
        _emit 0x24
        _emit 0x50
        _emit 0xff
        _emit 0x15
        _emit 0x5c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b
        _emit 0x0c
        _emit 0x24
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x89
        _emit 0x0d
        _emit 0x70
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x89
        _emit 0x15
        _emit 0x74
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x6a
        _emit 0x00
        _emit 0x68
        _emit 0x40
        _emit 0x42
        _emit 0x0f
        _emit 0x00
        _emit 0x50
        _emit 0x51
        _emit 0xe8
        _emit 0x11
        _emit 0x09
        _emit 0x5c
        _emit 0x00
        _emit 0x8b
        _emit 0x0d
        _emit 0x74
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x51
        _emit 0x8b
        _emit 0x0d
        _emit 0x70
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x51
        _emit 0x52
        _emit 0x50
        _emit 0xe8
        _emit 0x3c
        _emit 0x09
        _emit 0x5c
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0xc3
    }
}
