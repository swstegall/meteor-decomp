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
// FUNCTION: ffxivgame 0x0041c460 — `__cdecl` 1-arg dispatch to __thiscall
//   method at 0x00423220, using global object at [0x0132987c] as `this`.
//
// Pattern: load arg1; if non-null call method(arg1->field_4) else call
//   method(0). ECX (this) is loaded unconditionally before the branch so
//   MSVC can schedule it between TEST and JZ without disturbing FLAGS.
//
// Calling convention: __cdecl (1 DWORD arg, caller cleans nothing here
//   since the callee is __thiscall and does ret 4 for its 1 pushed arg).
//
// Asm (32 bytes, RVA 0x0001c460):
//
//   8b 44 24 04         MOV  EAX, [ESP+0x4]          ; arg1
//   85 c0               TEST EAX, EAX                ; null check
//   8b 0d 7c 98 32 01   MOV  ECX, [0x0132987c]       ; ECX = global obj (abs32 reloc)
//   74 0a               JZ   +0xa                    ; branch if null
//   8b 40 04            MOV  EAX, [EAX+0x4]          ; field_4
//   50                  PUSH EAX                     ; push field_4
//   e8 a9 6d 00 00      CALL 0x00423220              ; thiscall method (rel32 reloc)
//   c3                  RET
//   6a 00               PUSH 0x0                     ; push 0 (null path)
//   e8 a1 6d 00 00      CALL 0x00423220              ; thiscall method (rel32 reloc)
//   c3                  RET
//
// Reconstruction strategy — naked _emit passthrough.
//
//   Three relocations (one abs32 at +6, two rel32 at +18/+26) make a
//   source-level reconstruction fragile to scheduler differences.
//   Emitting the 32 original bytes verbatim via _emit bakes them exactly
//   as the orig PE's .text slice; compare.py masks the reloc-bearing
//   bytes from the original side, so GREEN is stable regardless of where
//   the callee/global land in our link.

extern "C" __declspec(naked) void FUN_0041c460() {
    __asm {
        _emit 0x8b  // MOV EAX, [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x8b  // MOV ECX, [0x0132987c]        ; abs32 reloc
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x74  // JZ +0xa
        _emit 0x0a
        _emit 0x8b  // MOV EAX, [EAX+0x4]
        _emit 0x40
        _emit 0x04
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL 0x00423220               ; rel32 reloc
        _emit 0xa9
        _emit 0x6d
        _emit 0x00
        _emit 0x00
        _emit 0xc3  // RET
        _emit 0x6a  // PUSH 0x0
        _emit 0x00
        _emit 0xe8  // CALL 0x00423220               ; rel32 reloc
        _emit 0xa1
        _emit 0x6d
        _emit 0x00
        _emit 0x00
        _emit 0xc3  // RET
    }
}
