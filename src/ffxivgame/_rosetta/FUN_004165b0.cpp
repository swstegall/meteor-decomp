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
// FUNCTION: ffxivgame 0x004165b0 — bit-flag getter (low bit of byte at this+4)
//
// First match candidate from `tools/find_easy_wins.py`'s top queue
// for ffxivgame.exe. 8 bytes, score 107, no calls, no relocations.
// Picked blind (no Ghidra GUI assistance) to validate the find-
// easy-wins workflow.
//
// Behavior (read from the asm @ 0x004165b0, 8 bytes):
//   8a 41 04         MOV AL, byte ptr [ECX + 0x4]
//   24 01            AND AL, 0x1
//   c2 08 00         RET 0x8
//
// Standard `__thiscall` bool-getter idiom: returns the low bit of
// a byte at offset 4 within the implicit `this`. The `RET 0x8`
// (8-byte arg cleanup) is unusual for a zero-arg method — it
// implies the caller pushes 2 dwords beyond `this` (or possibly
// this is a virtual override whose signature includes a pad word
// for the multi-inheritance thunk adjustment).
//
// Wait — RET 0x8 with __thiscall (`this` in ECX, NOT on stack)
// means there IS a stack-passed argument of 8 bytes. So this is
// NOT a zero-arg method; it's a method with one 8-byte arg
// (typically a struct-value param, like `MyStruct s` passed by
// value, or two scalar args totalling 8 bytes). The body ignores
// it but the calling convention requires popping. Closest C++
// match: a method that takes a struct and ignores it.
//
// Note: there are MANY (~80+ in the binary) functions with these
// exact 8 bytes — different classes, same implementation pattern.
// Matching this one source landlords matches all of them in spirit
// (their bytes are identical), though each appears at its own
// RVA in the YAML and counts as a separate "matched" entry.

// Iteration #1 [10 B vs orig 8 B]: returned `bool`, cl.exe emitted
//   `MOVZX EAX, [ECX+4]; AND EAX, 1; RET 8` (10 B). MOVZX zero-
//   extends to 32 bits, which is correct for C++ bool (must produce
//   0 or 1 as int) but bloats by 2 bytes. The orig's 8-bit form is
//   what you get when the source returns `unsigned char` and the
//   compiler can leave the high bytes undefined.

class BitFlagGetter {
public:
    // The struct-value param is what the orig RET 0x8 cleans up;
    // without it, cl.exe would emit `RET` (no arg cleanup).
    unsigned char get_low_bit(int unused_a, int unused_b) const;
private:
    int padding;            // [this+0]
    unsigned char flags;    // [this+4] — low bit is the flag
};

unsigned char BitFlagGetter::get_low_bit(int, int) const {
    unsigned char r = flags;
    r &= 1;
    return r;
}
