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
// FUNCTION: ffxivgame 0x00006b70 — __thiscall 2-int pair-setter returning *this.
//
// Asm (25 bytes @ 0x00006b70):
//   8b 54 24 08         MOV EDX, [ESP + 0x8]    ; load arg b
//   8b c1               MOV EAX, ECX             ; preserve `this` as return value
//   8b 4c 24 04         MOV ECX, [ESP + 0x4]    ; load arg a (clobbers this)
//   89 0d ?? ?? ?? ??   MOV [g_pair_a], ECX     ; g_pair_a = a      (DIR32 reloc)
//   89 15 ?? ?? ?? ??   MOV [g_pair_b], EDX     ; g_pair_b = b      (DIR32 reloc)
//   c2 08 00            RET 0x8                  ; __thiscall: callee cleans 8 bytes of args
//
// The two writes target consecutive 4-byte globals at .data
// 0x01327a00 / 0x01327a04. Sibling at 0x00006ba0 follows the identical
// shape against globals at 0x01327a10 / 0x01327a14 — looks like one
// of several "remember-last-pair" stashes the engine keeps for a
// configuration / hit-test / cursor cache.

class C {
public:
    C *set_pair(int a, int b);
};

extern "C" {
    int g_pair_a;
    int g_pair_b;
}

C *C::set_pair(int a, int b)
{
    g_pair_a = a;
    g_pair_b = b;
    return this;
}
