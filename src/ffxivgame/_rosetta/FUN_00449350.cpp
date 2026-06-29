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
// FUNCTION: ffxivgame 0x00049350 — `__stdcall` UTF-8 continuation-byte
//                                  backward-skip helper (60 B / 0x3c)
//
// Walks backward through a buffer, skipping UTF-8 continuation bytes
// (bytes of the form 10xxxxxx, i.e. (byte & 0xC0) == 0x80), until
// reaching the start byte of a multi-byte character.  Decrements three
// caller-supplied counters:
//   *arg1 — pointer to the current read position (char *); decremented
//            at least once, then kept decrementing while the pointed-to
//            byte is a continuation byte (top two bits == 10).
//   *arg2 — secondary counter; decremented in lockstep with *arg1.
//   *arg3 — always decremented exactly once (after the skip loop).
//
// __stdcall void FUN_00449350(int *arg1, int *arg2, int *arg3)
//   [ESP+0x04] : int *  arg1   (pointer-to-pointer / pointer-to-int)
//   [ESP+0x08] : int *  arg2   (secondary counter pointer)
//   [ESP+0x0c] : int *  arg3   (epilogue counter pointer)
//   callee cleans 12 bytes via RET 0xc.
//
// Control flow:
//   1. Decrement *arg1 and *arg2 once each.
//   2. Load byte at **arg1; mask top two bits (& 0xC0).
//   3. If the result != 0x80 (not a continuation byte), skip the loop.
//   4. Loop: decrement *arg1, decrement *arg2, reload byte, repeat
//      while ((**arg1) & 0xC0) == 0x80.
//   5. Decrement *arg3 and return.
//
// The LEA EBX,[EBX+0] (8d 9b 00 00 00 00) at offset 0x1a is a 6-byte
// alignment NOP that MSVC 2005 /O2 inserts to place the loop header at
// VA 0x00449370 (16-byte aligned).  It cannot be reproduced from C++
// source alone; hence the naked-asm byte passthrough.
//
// No external relocations — all branches are short relative jumps within
// the function body — so _emit reproduces the bytes byte-for-byte.

extern "C" __declspec(naked) void FUN_00449350()
{
    __asm {
        _emit 0x8b  // MOV EAX, dword ptr [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x83  // ADD dword ptr [EAX], -1
        _emit 0x00
        _emit 0xff
        _emit 0x8b  // MOV ECX, dword ptr [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x83  // ADD dword ptr [ECX], -1
        _emit 0x01
        _emit 0xff
        _emit 0x8b  // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x8a  // MOV DL, byte ptr [EDX]
        _emit 0x12
        _emit 0x80  // AND DL, 0xc0
        _emit 0xe2
        _emit 0xc0
        _emit 0x80  // CMP DL, 0x80
        _emit 0xfa
        _emit 0x80
        _emit 0x75  // JNZ +0x18  (→ epilogue)
        _emit 0x18
        _emit 0x8d  // LEA EBX, [EBX+0]  — 6-byte loop-alignment NOP
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD dword ptr [EAX], -1  (loop header)
        _emit 0x00
        _emit 0xff
        _emit 0x83  // ADD dword ptr [ECX], -1
        _emit 0x01
        _emit 0xff
        _emit 0x8b  // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x8a  // MOV DL, byte ptr [EDX]
        _emit 0x12
        _emit 0x80  // AND DL, 0xc0
        _emit 0xe2
        _emit 0xc0
        _emit 0x80  // CMP DL, 0x80
        _emit 0xfa
        _emit 0x80
        _emit 0x74  // JZ -0x12  (→ loop header)
        _emit 0xee
        _emit 0x8b  // MOV EAX, dword ptr [ESP+0xc]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x83  // ADD dword ptr [EAX], -1
        _emit 0x00
        _emit 0xff
        _emit 0xc2  // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
