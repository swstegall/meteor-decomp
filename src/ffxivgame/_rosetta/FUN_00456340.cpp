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
// FUNCTION: ffxivgame 0x00056340 — handle-gated lazy-init stub
//                                  (__cdecl, 1 stack arg, 33 B / 0x21)
//
// Checks the global handle at [0x0126701c] against -1.  If the handle is
// already valid (not -1), the JNZ falls through directly into FUN_00456361
// (the immediately following function in .text) — no explicit RET is taken
// on this path.  If the handle is -1 (uninitialised), the function loads
// the caller's argument from [ESP+4], pushes it, sets ECX = 0x0132d0e0
// (the singleton object), calls FUN_00457270 (__thiscall, 1 stack arg),
// moves the result into ECX, calls FUN_00456a90 (__thiscall, no args),
// then returns via the RET at +0x20.
//
// Cross-function tail detail:
//   The JNZ +0x17 at +0x08 targets RVA 0x00056361 (VA 0x00456361), which
//   is one byte past this function's own RET at +0x20.  That address is
//   the entry point of FUN_00456361.  This is the same code-layout
//   optimisation documented in FUN_00455ef0 / FUN_00456060: the "already
//   initialised" fast path bypasses the stub body entirely and falls into
//   the sibling function rather than executing a redundant RET.
//
// Calling convention: __cdecl (no PUSH of callee-saved registers;
// epilogue is bare RET at +0x20; caller cleans).
//
// Reloc-bearing sites (4-byte windows, wildcarded by compare.py):
//   +0x01  MOV EAX, [0x0126701c]    dir32  (.data handle)
//   +0x10  MOV ECX, 0x0132d0e0      dir32  (.data singleton imm)
//   +0x15  CALL FUN_00457270        rel32
//   +0x1c  CALL FUN_00456a90        rel32
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The JNZ at +0x08 jumps PAST this function's own RET (+0x20) into the
//   following function.  A high-level C if/else would produce a JNZ to
//   the RET (offset +0x16) rather than past it (offset +0x17), so source-
//   level C cannot reproduce the exact encoding.  The established local
//   idiom (FUN_00456060, FUN_00456800, FUN_00456361) is a __declspec(naked)
//   body that re-emits all bytes verbatim via _emit directives.

extern "C" __declspec(naked) void FUN_00456340() {
    __asm {
        // 00056340: a1 1c 70 26 01  MOV EAX,[0x0126701c]
        _emit 0xa1
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        // 00056345: 83 f8 ff        CMP EAX,-0x1
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        // 00056348: 75 17           JNZ +0x17  (→ 0x00456361, next fn)
        _emit 0x75
        _emit 0x17
        // 0005634a: 8b 44 24 04     MOV EAX,dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0005634e: 50              PUSH EAX
        _emit 0x50
        // 0005634f: b9 e0 d0 32 01  MOV ECX,0x132d0e0  (singleton imm32)
        _emit 0xb9
        _emit 0xe0
        _emit 0xd0
        _emit 0x32
        _emit 0x01
        // 00056354: e8 17 0f 00 00  CALL 0x00457270
        _emit 0xe8
        _emit 0x17
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        // 00056359: 8b c8           MOV ECX,EAX
        _emit 0x8b
        _emit 0xc8
        // 0005635b: e8 30 07 00 00  CALL 0x00456a90
        _emit 0xe8
        _emit 0x30
        _emit 0x07
        _emit 0x00
        _emit 0x00
        // 00056360: c3              RET
        _emit 0xc3
    }
}
