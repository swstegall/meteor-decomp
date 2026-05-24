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
// FUNCTION: ffxivgame 0x00013530 — __thiscall: bool getter via inner-pointer
//                                   field-nonzero test (14 bytes)
//
// Asm (14 bytes @ orig RVA 0x00013530):
//   8b 41 04    MOV EAX, dword ptr [ECX + 0x4]   ; EAX = this->field_0x4 (a pointer)
//   33 c9       XOR ECX, ECX                     ; ECX = 0
//   39 48 30    CMP dword ptr [EAX + 0x30], ECX  ; flags = (field_0x4->field_0x30 cmp 0)
//   0f 95 c1    SETNE CL                         ; CL = (field_0x30 != 0)
//   8a c1       MOV AL, CL                       ; AL = CL (return low byte)
//   c3          RET                              ; __thiscall, no stack args
//
// Returns whether the dword at offset 0x30 inside the pointer cached at
// this+0x4 is non-zero. Classic MSVC 2005 /O2 `xor reg, reg; cmp [m], reg;
// setne reg8; mov al, reg8` lowering of a `bool` returning `!= 0` test —
// the high 24 bits of EAX carry the dirty upper bytes of the previously
// loaded pointer, but bool only inspects AL.

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
// The real implementation is the MSVC __declspec(naked) + __asm block below,
// which clang cannot parse. Production builds always use cl.exe (MSVC 2005).
extern "C" int FUN_00413530() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) int FUN_00413530() {
    __asm {
        mov eax, dword ptr [ecx + 0x4]
        xor ecx, ecx
        cmp dword ptr [eax + 0x30], ecx
        setne cl
        mov al, cl
        ret
    }
}
#endif
