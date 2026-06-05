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
// FUNCTION: ffxivgame 0x00045470 — strchr-on-member-buffer (index-or-(-1))
//                                   (__thiscall, 1 byte arg, 43 B / 0x2B)
//
// int __thiscall FUN_00445470(this, char needle)
//   stack layout (after PUSH ESI):
//       [ESP+0x08] : char needle      (the byte being searched for)
//   this->field_0 : char *            (NUL-terminated buffer base)
//
// Walks the C string at *this looking for the first byte equal to `needle`.
// Returns the zero-based index of the match (EAX - base), or -1 if the
// scan hits the terminating NUL without a hit. Classic strchr-returns-
// offset idiom: ESI holds the buffer base for the final SUB, EAX is the
// running cursor.
//
// Asm (43 bytes @ orig RVA 0x00045470):
//   56              PUSH ESI
//   8b 31           MOV  ESI, [ECX]            ; base = this->field_0
//   8b c6           MOV  EAX, ESI              ; cursor = base
//   8a 08           MOV  CL, [EAX]             ; CL = *cursor
//   84 c9           TEST CL, CL
//   74 13           JZ   not_found             ; empty string → -1
//   8a 54 24 08     MOV  DL, [ESP+8]           ; DL = needle
//   90              NOP
// loop:
//   3a ca           CMP  CL, DL
//   74 11           JZ   found
//   8a 48 01        MOV  CL, [EAX+1]
//   83 c0 01        ADD  EAX, 1
//   84 c9           TEST CL, CL
//   75 f2           JNZ  loop
// not_found:
//   83 c8 ff        OR   EAX, 0xffffffff       ; EAX = -1
//   5e              POP  ESI
//   c2 04 00        RET  4
// found:
//   2b c6           SUB  EAX, ESI              ; index = cursor - base
//   5e              POP  ESI
//   c2 04 00        RET  4
//
// Calling convention: __thiscall (ECX = this, one byte arg on the stack,
// callee cleans 4 via RET 4). No relocations — the function is entirely
// self-contained (no external CALLs, no absolute-address loads), so the
// naked _emit byte passthrough reproduces the orig 43-byte slice exactly.

extern "C" __declspec(naked) void FUN_00445470() {
    __asm {
        // 00045470: 56            PUSH ESI
        _emit 0x56
        // 00045471: 8b 31         MOV ESI, [ECX]
        _emit 0x8b
        _emit 0x31
        // 00045473: 8b c6         MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 00045475: 8a 08         MOV CL, [EAX]
        _emit 0x8a
        _emit 0x08
        // 00045477: 84 c9         TEST CL, CL
        _emit 0x84
        _emit 0xc9
        // 00045479: 74 13         JZ 0x0044548e
        _emit 0x74
        _emit 0x13
        // 0004547b: 8a 54 24 08   MOV DL, [ESP+8]
        _emit 0x8a
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 0004547f: 90            NOP
        _emit 0x90
        // 00045480: 3a ca         CMP CL, DL
        _emit 0x3a
        _emit 0xca
        // 00045482: 74 11         JZ 0x00445495
        _emit 0x74
        _emit 0x11
        // 00045484: 8a 48 01      MOV CL, [EAX+1]
        _emit 0x8a
        _emit 0x48
        _emit 0x01
        // 00045487: 83 c0 01      ADD EAX, 1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 0004548a: 84 c9         TEST CL, CL
        _emit 0x84
        _emit 0xc9
        // 0004548c: 75 f2         JNZ 0x00445480
        _emit 0x75
        _emit 0xf2
        // 0004548e: 83 c8 ff      OR EAX, 0xffffffff
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // 00045491: 5e            POP ESI
        _emit 0x5e
        // 00045492: c2 04 00      RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 00045495: 2b c6         SUB EAX, ESI
        _emit 0x2b
        _emit 0xc6
        // 00045497: 5e            POP ESI
        _emit 0x5e
        // 00045498: c2 04 00      RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
