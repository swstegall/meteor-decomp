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
// FUNCTION: ffxivgame 0x00020d60 — derived exception class constructor
//                                  (__thiscall, 25 B / 0x19)
//
// Structural twin of std::bad_alloc::bad_alloc @ 0x00401030 and
// std::logic_error @ 0x00404390, but for a class one level further
// derived: calls FUN_00404320 (the std::logic_error-shaped base ctor
// that stamps vtable 0x00f54a2c and populates the embedded Utf8String),
// then overrides the vtable with the derived class's own vftable at
// VA 0x00f6702c.
//
// Calling convention: __thiscall (ECX = this; 1 explicit arg = pointer
// to Utf8String; callee cleans 4 bytes with `ret 4`).
//
// Asm (25 bytes @ RVA 0x00020d60):
//
//   8b 44 24 04         MOV  EAX, [ESP+4]              ; load msg pointer
//   56                  PUSH ESI
//   50                  PUSH EAX                       ; pass msg to base ctor
//   8b f1               MOV  ESI, ECX                  ; ESI = this
//   e8 b3 35 fe ff      CALL FUN_00404320              ; __thiscall base ctor
//                                                      ;   (ECX still = this)
//   c7 06 2c 70 f6 00   MOV  dword ptr [ESI], 0xf6702c ; stamp derived vtable
//   8b c6               MOV  EAX, ESI                  ; return this
//   5e                  POP  ESI
//   c2 04 00            RET  4
//
// Note: unlike the bad_alloc ctor at 0x00401030 which uses `lea eax,[esp+8]`
// (after PUSH ESI) to pass a *reference* to the argument, here the MOV
// is done BEFORE PUSH ESI so the argument value is loaded from [esp+4]
// and pushed by value — the base ctor receives the pointer directly, not
// a reference to it.
//
// Reloc-masked windows in compare.py output:
//   +0x08  CALL rel32  → FUN_00404320 (IMAGE_REL_I386_REL32)
//   +0x0D  MOV  imm32  → 0x00f6702c  (IMAGE_REL_I386_DIR32, vtable)

extern "C" void FUN_00404320();
extern "C" int  vtable_FUN_00420d60;

extern "C" __declspec(naked) void FUN_00420d60() {
    __asm {
        mov  eax, dword ptr [esp+4]
        push esi
        push eax
        mov  esi, ecx
        call FUN_00404320
        mov  dword ptr [esi], offset vtable_FUN_00420d60
        mov  eax, esi
        pop  esi
        ret  4
    }
}
