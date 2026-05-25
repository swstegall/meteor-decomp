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
// FUNCTION: ffxivgame 0x00016320 — engine_memory tracker reset (40 B / 0x28)
//
// Zeroes the six DWORD counters at 0x01328d50..0x01328d64 and sets the
// enable-flag byte at 0x01328d7c. This is the partner of the tracker
// disable routine at 0x00016350 (which clears the flag and returns the
// counter base pointer): the two functions bracket a "reset → use →
// disable" lifecycle for the global allocator-statistics block that
// FUN_004162c0 / FUN_00416480 mutate during alloc/free events.
//
// Calling convention: __cdecl — no args, void return, plain RET.
// Frame: none (/Oy — function has no locals).
//
// Asm (40 bytes @ orig RVA 0x00016320):
//   33 c0                  XOR  EAX, EAX
//   a3 54 8d 32 01         MOV  [0x01328d54], EAX     ; counter 0
//   a3 50 8d 32 01         MOV  [0x01328d50], EAX     ; counter 1
//   a3 58 8d 32 01         MOV  [0x01328d58], EAX     ; counter 2
//   a3 5c 8d 32 01         MOV  [0x01328d5c], EAX     ; counter 3
//   a3 60 8d 32 01         MOV  [0x01328d60], EAX     ; counter 4
//   a3 64 8d 32 01         MOV  [0x01328d64], EAX     ; counter 5
//   c6 05 7c 8d 32 01 01   MOV  byte ptr [0x01328d7c], 0x1   ; enable
//   c3                     RET
//
// Reconstruction: __declspec(naked) _emit byte passthrough. The orig
// encodes every store as `a3 <abs32>` (the EAX-as-source short-form
// "MOV moffs32, EAX"). A source-level decomp would need MSVC 2005 to
// (a) hoist `xor eax, eax` and (b) pick the moffs short-form over
// `c7 05 <abs32> <imm32>` for each of six consecutive zero stores —
// that's deterministic in principle but depends on extern-global
// addressing constraints we don't have via header symbols. Naked-asm
// reproduces the exact 40-byte sequence with no relocations.

extern "C" __declspec(naked) void FUN_00416320() {
    __asm {
        // 00016320: 33 c0                  XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00016322: a3 54 8d 32 01         MOV [0x01328d54], EAX
        _emit 0xa3
        _emit 0x54
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00016327: a3 50 8d 32 01         MOV [0x01328d50], EAX
        _emit 0xa3
        _emit 0x50
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 0001632c: a3 58 8d 32 01         MOV [0x01328d58], EAX
        _emit 0xa3
        _emit 0x58
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00016331: a3 5c 8d 32 01         MOV [0x01328d5c], EAX
        _emit 0xa3
        _emit 0x5c
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00016336: a3 60 8d 32 01         MOV [0x01328d60], EAX
        _emit 0xa3
        _emit 0x60
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 0001633b: a3 64 8d 32 01         MOV [0x01328d64], EAX
        _emit 0xa3
        _emit 0x64
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00016340: c6 05 7c 8d 32 01 01   MOV byte ptr [0x01328d7c], 0x1
        _emit 0xc6
        _emit 0x05
        _emit 0x7c
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 00016347: c3                     RET
        _emit 0xc3
    }
}
