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
// FUNCTION: ffxivgame 0x000153e0 — `__thiscall` VfxLogger depth-increment
//                                   with cap-at-0xf guard (19 B / 0x13).
//
// Reads the byte nesting-depth field at `this+0x80`, and increments it
// only if the current value is strictly less than 15 (0xf).  When the
// depth is already at the cap, the function returns with no side-effect.
//
// C++ equivalent:
//
//   void VfxLogger::PushDepth() {                 // ECX = this
//       unsigned char d = this->depth;            // MOV AL, [ECX+0x80]
//       if (d < 0x0f)                             // CMP AL, 0xf / JNC skip
//           this->depth = d + 1;                  // ADD AL, 0x1 / MOV [ECX+0x80], AL
//   }
//
// The `this->depth` member at offset +0x80 is the same indent-depth byte
// catalogued in decomp-notes/types/ffxivgame/0x000154f0.md (VfxLogger,
// range observed 0..7; this function caps writes at 0xf).
//
// Asm (19 bytes @ orig RVA 0x000153e0):
//
//   8a 81 80 00 00 00   MOV  AL, byte ptr [ECX + 0x80]  ; d = this->depth
//   3c 0f               CMP  AL, 0xf                    ; d >= 15?
//   73 08               JNC  +0x8  → 0x004153f2          ; yes → skip (no carry = unsigned >=)
//   04 01               ADD  AL, 0x1                    ; d + 1
//   88 81 80 00 00 00   MOV  byte ptr [ECX + 0x80], AL  ; this->depth = d + 1
//   c3                  RET
//
// Calling convention: __thiscall (ECX = this, no stack args, plain RET).
// No prologue, no callee-saves, no stack frame, no security cookie.
//
// Reloc-bearing sites: NONE.  The 19-byte slice is entirely reloc-free
// (all addressing is ECX-register-relative; no CALL, no absolute data
// reference).  Naked-asm passthrough is used — matching the established
// local idiom for this function cluster (see FUN_00415310, FUN_00415350,
// FUN_004153a0) — to guarantee byte-identical output without relying on
// MSVC 2005 /O2 choosing ADD AL,1 over INC AL for the increment.

extern "C" __declspec(naked) void FUN_004153e0() {
    __asm {
        // 000153e0: mov al, byte ptr [ecx + 0x80]   ; d = this->depth
        _emit 0x8a
        _emit 0x81
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000153e6: cmp al, 0xf                     ; d >= 15?
        _emit 0x3c
        _emit 0x0f
        // 000153e8: jnc +0x8  →  0x004153f2 (ret)   ; skip if unsigned >=
        _emit 0x73
        _emit 0x08
        // 000153ea: add al, 0x1
        _emit 0x04
        _emit 0x01
        // 000153ec: mov byte ptr [ecx + 0x80], al   ; this->depth = d + 1
        _emit 0x88
        _emit 0x81
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000153f2: ret
        _emit 0xc3
    }
}
