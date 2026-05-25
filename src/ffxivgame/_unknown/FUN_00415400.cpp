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
// FUNCTION: ffxivgame 0x00015400 — saturating-at-zero byte countdown
//                                   (__thiscall, 19 B / 0x13)
//
// Decrements the unsigned byte counter at `this + 0x80` by 1, clamped
// at zero. Used as a per-tick "cooldown" / "linger" countdown on the
// owning class.
//
// Pseudo-C:
//   void method() {
//       uint8_t v = this->counter;        // [this + 0x80]
//       if (v != 0) {
//           this->counter = (uint8_t)(v - 1);
//       }
//   }
//
// Calling convention: __thiscall (ECX = this, no stack args, bare RET).
// Frame: none (leaf, no callee-saved usage).
//
// The branch is the `JBE` form (not `JE`) — MSVC 2005 picked the
// unsigned-below-or-equal mnemonic to skip the SUB/MOV pair when AL
// is zero, since the immediately-preceding TEST leaves CF=0 so
// JBE collapses to JE in semantics but uses a distinct opcode (0x76)
// from JE (0x74). Likewise `SUB AL, 1` (2c 01) is selected over the
// shorter `DEC AL` (fe c8) — again an MSVC 2005 codegen choice that
// only the exact byte stream pins down. Encoded as `__declspec(naked)`
// + `_emit` to guarantee the byte layout.
//
// Asm (19 bytes @ orig RVA 0x00015400):
//   8a 81 80 00 00 00      MOV  AL, byte ptr [ECX + 0x80]
//   84 c0                  TEST AL, AL
//   76 08                  JBE  +0x08              (→ 0x00415412, the RET)
//   2c 01                  SUB  AL, 0x1
//   88 81 80 00 00 00      MOV  byte ptr [ECX + 0x80], AL
//   c3                     RET

extern "C" __declspec(naked) void FUN_00415400()
{
    __asm {
        // 00015400:  8a 81 80 00 00 00   MOV AL, byte ptr [ECX + 0x80]
        _emit 0x8a
        _emit 0x81
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00015406:  84 c0               TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // 00015408:  76 08               JBE +0x08  (→ 0x00015412)
        _emit 0x76
        _emit 0x08
        // 0001540a:  2c 01               SUB AL, 0x1
        _emit 0x2c
        _emit 0x01
        // 0001540c:  88 81 80 00 00 00   MOV byte ptr [ECX + 0x80], AL
        _emit 0x88
        _emit 0x81
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00015412:  c3                  RET
        _emit 0xc3
    }
}
