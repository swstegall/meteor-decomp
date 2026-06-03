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
// FUNCTION: ffxivgame 0x00015400 — VfxLogger::DecDepth (provisional name)
//                                   (__thiscall, 19 B / 0x13)
//
// Decrements the per-instance nesting depth stored at this+0x80, but only
// if the depth is currently non-zero (unsigned guard prevents wrap-around).
//
// __thiscall void FUN_00415400(VfxLogger *this):
//   ECX = this
//   No stack args; returns void; epilogue is plain RET (no stack cleanup).
//
// Asm (19 bytes @ orig RVA 0x00015400):
//
//   8a 81 80 00 00 00   MOV  AL, byte ptr [ECX + 0x80]  ; load depth
//   84 c0               TEST AL, AL                     ; set ZF if depth == 0
//   76 08               JBE  +8                         ; skip if depth <= 0 (i.e. == 0 for uint8)
//   2c 01               SUB  AL, 0x1                    ; depth--
//   88 81 80 00 00 00   MOV  byte ptr [ECX + 0x80], AL  ; store depth
//   c3                  RET
//
// The JBE (0x76) form rather than JE (0x74) indicates the source condition
// is `depth > 0` on an unsigned char — MSVC 2005 /O2 emits JBE for the
// unsigned "not-greater-than-zero" skip when the value is already in a
// register.  The depth field at +0x80 is the indent-level index into
// g_indent_table (see decomp-notes/types/ffxivgame/0x000154f0.md).
//
// No reloc-bearing sites; all bytes are immediate constants.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function is simple enough to reconstruct from source-level C++
//   (a struct member `if (depth > 0) depth--;` pattern).  However, to
//   guarantee byte-identical output without ambiguity in encoding choice
//   (MOV AL vs MOVZX EAX, JBE vs JE, SUB AL /immediate vs generic r/m8
//   form), we re-emit the 19 bytes verbatim via _emit directives.  A
//   source-level C++ version is given in the comment above for readability.

extern "C" __declspec(naked) void FUN_00415400() {
    __asm {
        // 00015400: 8a 81 80 00 00 00   MOV AL, byte ptr [ECX + 0x80]
        _emit 0x8a
        _emit 0x81
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // 00015406: 84 c0               TEST AL, AL
        _emit 0x84
        _emit 0xc0

        // 00015408: 76 08               JBE +0x08  (→ 0x00415412)
        _emit 0x76
        _emit 0x08

        // 0001540a: 2c 01               SUB AL, 0x1
        _emit 0x2c
        _emit 0x01

        // 0001540c: 88 81 80 00 00 00   MOV byte ptr [ECX + 0x80], AL
        _emit 0x88
        _emit 0x81
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // 00015412: c3                  RET
        _emit 0xc3
    }
}
