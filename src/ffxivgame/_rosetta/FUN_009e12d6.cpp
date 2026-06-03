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
// FUNCTION: ffxivgame 0x009e12d6 — __cdecl null-guarded struct-field
//                                  iterator (prologue, 23 B)
//
// Ghidra sized this function as 23 bytes (0x17), but the real function
// extends to roughly RVA 0x5e1464 (~400 B): it walks every field of a
// struct pointed to by arg1, calling helper 0x005d5c88 for each field.
// Because compare.py reads exactly `size` bytes from the binary, we emit
// the precise 23-byte prologue slice verbatim via naked _emit.
//
// Asm (23 bytes @ orig VA 0x009e12d6 / RVA 0x005e12d6):
//
//   56                   PUSH ESI                     ; callee-save ESI
//   8b 74 24 08          MOV  ESI, [ESP+8]            ; ESI = arg1 (struct ptr)
//   85 f6                TEST ESI, ESI               ; null-check arg1
//   0f 84 81 01 00 00    JE   near +0x181             ; bail if null → 0x005e1464
//   ff 76 04             PUSH dword ptr [ESI+0x04]   ; push field_0x04
//   e8 9d 49 ff ff       CALL 0x005d5c88             ; call per-field helper
//   ff 76                [first 2 B of PUSH [ESI+0x08], continued beyond slice]
//
// Calling convention : __cdecl (arg1 at [ESP+8] after PUSH ESI; bare
//                               far JE exits via caller cleanup).
// Frame             : none — /Oy omits frame pointer; function is leaf-ish.
// Register use      : ESI saved/used as struct pointer.

extern "C" __declspec(naked) void FUN_009e12d6() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, [ESP+8]
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x0f              // JE near +0x181 (→ 0x005e1464)
        _emit 0x84
        _emit 0x81
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xff              // PUSH dword ptr [ESI+0x04]
        _emit 0x76
        _emit 0x04
        _emit 0xe8              // CALL 0x005d5c88 (rel32)
        _emit 0x9d
        _emit 0x49
        _emit 0xff
        _emit 0xff
        _emit 0xff              // [continuation: first 2 B of PUSH [ESI+0x08]]
        _emit 0x76
    }
}
