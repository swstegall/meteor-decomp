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
// FUNCTION: ffxivgame 0x0003a6b0 — `__thiscall` constructor that installs the
//                                   vtable at 0x00f66390 and zero-initialises
//                                   two members (17 B / 0x11).
//
// Behaviour read from the disassembly at orig RVA 0x0003a6b0:
//
//   void* __thiscall ctor(C* this) {
//       this->vftable    = (void*)0x00f66390;   // vtable pointer @ offset 0
//       this->field_0x14 = 0;
//       this->field_0x18 = 0;
//       return this;                            // ctor returns `this` in EAX
//   }
//
//   Calling convention: `__thiscall` (ECX = this, no stack args, plain `ret`).
//   No prologue, no stack frame, no callee-saves. The leading `mov eax, ecx`
//   stages `this` into EAX both as the addressing base for the three stores
//   and as the constructor's `this` return value; `xor ecx, ecx` materialises
//   the zero reused by the two `mov [eax+disp], ecx` member stores.
//
// Asm (17 bytes @ orig RVA 0x0003a6b0):
//   8b c1                MOV  EAX, ECX                  ; EAX = this (and retval)
//   33 c9                XOR  ECX, ECX                  ; ECX = 0
//   c7 00 90 63 f6 00    MOV  dword ptr [EAX], 0xf66390 ; this->vftable = &vtable
//   89 48 14             MOV  dword ptr [EAX+0x14], ECX ; this->field_0x14 = 0
//   89 48 18             MOV  dword ptr [EAX+0x18], ECX ; this->field_0x18 = 0
//   c3                   RET
//
// The vtable immediate (0x00f66390, .rdata) is a base-relocation site in the
// orig; emitted here as a raw constant, `tools/compare.py` masks that 4-byte
// window during the diff. Naked-asm passthrough keeps the 17-byte slice
// byte-identical without depending on MSVC's ctor codegen choices.

extern "C" __declspec(naked) void FUN_0043a6b0() {
    __asm {
        mov  eax, ecx
        xor  ecx, ecx
        mov  dword ptr [eax], 0x00f66390
        mov  dword ptr [eax + 0x14], ecx
        mov  dword ptr [eax + 0x18], ecx
        ret
    }
}
