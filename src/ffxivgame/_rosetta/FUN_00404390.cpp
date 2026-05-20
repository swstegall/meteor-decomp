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
// FUNCTION: ffxivgame 0x00004390 — std::length_error::length_error(const std::string &)
//                                  thunk over std::logic_error (25 B / 0x19)
//
// Shape (reconstructed from the orig .text slice at RVA 0x00004390):
//
//   __thiscall length_error * ctor(this, const string *msg) {
//       logic_error::logic_error(msg);          // FUN_00404320
//       *(void**)this = &length_error::vftable; // 0x00f54a38 (overrides
//                                               //   the logic_error vptr
//                                               //   just installed)
//       return this;
//   }
//
//   mov  eax, [esp+4]            ; load msg arg
//   push esi
//   push eax                     ; forward msg to parent ctor
//   mov  esi, ecx                ; save this
//   call FUN_00404320            ; std::logic_error ctor
//   mov  dword ptr [esi], offset length_error_vftable
//   mov  eax, esi                ; return this
//   pop  esi
//   ret  4                       ; __thiscall, callee cleans 1 stack arg
//
//   The CALL site at +0x08 carries a REL32 reloc to FUN_00404320 and
//   the MOV imm32 at +0x0F carries a DIR32 reloc to the vftable. Both
//   reloc-byte windows are masked by tools/compare.py, so the .obj
//   ends up byte-identical to the orig slice modulo those linker
//   fixups.
//
// Calling convention: __thiscall (ECX = this, one stack arg, RET 4).
// Stack frame: -4 (PUSH ESI / POP ESI bracket).

extern "C" {

// std::logic_error::logic_error(const string &) — __thiscall ctor at
// orig 0x00404320 (covered separately by src/ffxivgame/_rosetta/
// FUN_00404320.cpp). Declared as a function so MSVC inline asm accepts
// `call FUN_00404320`; the assembler emits an IMAGE_REL_I386_REL32
// reloc that the comparator masks.
void FUN_00404320();

// std::length_error::vftable at orig 0x00f54a38. Declared as `int` so
// `mov dword ptr [esi], offset length_error_vftable` produces a
// DIR32 reloc to the vftable address — the value is set by the linker
// and masked out of the byte diff.
int length_error_vftable;

}  // extern "C"

extern "C" __declspec(naked) void FUN_00404390() {
    __asm {
        mov eax, dword ptr [esp+4]
        push esi
        push eax
        mov esi, ecx
        call FUN_00404320
        mov dword ptr [esi], offset length_error_vftable
        mov eax, esi
        pop esi
        ret 4
    }
}
