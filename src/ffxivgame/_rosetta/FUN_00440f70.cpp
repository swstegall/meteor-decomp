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
// FUNCTION: ffxivgame 0x00040f70 — __thiscall wrapper: call FUN_00440ce0(this),
//                                   then call _free(this->field_0x4)
//                                   (17 B / 0x11)
//
// Calling convention: __thiscall (ECX = this, no stack args).
// ESI is callee-saved and used to hold `this` across the inner CALL so that
// [ESI+0x4] can be read back after FUN_00440ce0 returns and passed to _free.
// The function has no epilogue within its 17-byte range — execution falls
// through into shared epilogue code at 0x00440f81 after _free returns.
// An identical pattern appears at FUN_00440f90 (0x00040f90) which calls
// FUN_00440d40 instead of FUN_00440ce0 before the same _free tail.
//
// Asm (17 bytes @ orig RVA 0x00040f70):
//   56                   PUSH ESI                  ; save ESI
//   8b f1                MOV  ESI, ECX             ; ESI = this
//   e8 68 fd ff ff       CALL FUN_00440ce0         ; __thiscall, ECX still = this
//   8b 46 04             MOV  EAX, [ESI + 0x4]     ; EAX = this->field_0x4
//   50                   PUSH EAX                  ; push arg to _free
//   e8 96 0b 59 00       CALL _free                ; free(this->field_0x4)
//
// The two reloc-bearing sites (CALL rel32 + CALL rel32) are masked by
// compare.py and do not affect the diff outcome.

extern "C" void FUN_00440ce0();
extern "C" void _free(void*);

extern "C" __declspec(naked) void FUN_00440f70() {
    __asm {
        push esi
        mov  esi, ecx
        call FUN_00440ce0
        mov  eax, dword ptr [esi + 0x4]
        push eax
        call _free
    }
}
