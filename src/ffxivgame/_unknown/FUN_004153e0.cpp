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
// FUNCTION: ffxivgame 0x000153e0 — __thiscall saturating-increment of a
//                                   byte counter at this->field_0x80,
//                                   capped at 0x0F (19 B)
//
// Calling convention: __thiscall (ECX = this, no stack args, RET).
// No prologue — /Oy omits frame pointer; no locals. Loads the counter
// byte into AL once, compares it against the cap (0x0F), and only
// writes the incremented value back when the cap has not yet been
// reached. The load-once / branch-over-store shape avoids re-reading
// the field when the cap is already hit.
//
// Asm (19 bytes @ orig RVA 0x000153e0):
//   8a 81 80 00 00 00     MOV AL, byte ptr [ECX + 0x80]   ; AL = this->field_0x80
//   3c 0f                 CMP AL, 0x0F                    ; cap check
//   73 08                 JAE skip                        ; branch over inc+store
//   04 01                 ADD AL, 0x1                     ; ++AL
//   88 81 80 00 00 00     MOV byte ptr [ECX + 0x80], AL   ; this->field_0x80 = AL
// skip:
//   c3                    RET

extern "C" __declspec(naked) void FUN_004153e0() {
    __asm {
        mov al, byte ptr [ecx + 0x80]
        cmp al, 0x0F
        jae skip
        add al, 1
        mov byte ptr [ecx + 0x80], al
    skip:
        ret
    }
}
