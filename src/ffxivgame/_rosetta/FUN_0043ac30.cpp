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
// FUNCTION: ffxivgame 0x0003ac30 — __thiscall single-field container
//                                  initialiser (22 B / 0x16)
//
// Asm (22 bytes @ orig RVA 0x0003ac30):
//   68 b0 ab 43 00        PUSH 0x43abb0             ; arg4: default string ptr
//   6a 06                 PUSH 0x6                  ; arg3: flags
//   68 70 01 00 00        PUSH 0x170                ; arg2: capacity
//   83 c1 18              ADD  ECX,0x18              ; ECX = &this->field_18
//   51                    PUSH ECX                   ; arg1: buffer
//   e8 07 70 59 00        CALL FUN_009d1c4c           ; __stdcall, 4 args, callee-cleans
//   c3                    RET
//
// Calling convention: __thiscall (ECX = this, no stack args, RET 0 →
// plain `ret`).
//
// FUN_009d1c4c is the same __stdcall(buffer, capacity, flags, string)
// container-field init helper seen in FUN_004381c0 and FUN_00422b20 —
// this function is the minimal single-field-init variant: it just
// biases `this` by 0x18 to reach the target member and forwards to
// the shared initialiser with capacity 0x170 / flags 6 / default
// string literal at VA 0x0043abb0.
//
// Reconstruction strategy — direct MASM translation. The PUSH imm32
// operands (0x43abb0, 0x170) are baked verbatim as plain immediates
// (matches the sibling idiom in FUN_0046c140); FUN_009d1c4c is declared
// `extern "C"` so MASM emits a proper COFF R_386_REL32 relocation for
// the CALL, which tools/compare.py masks automatically.

extern "C" void FUN_009d1c4c();  // __stdcall, 4 args, callee-cleans

extern "C" __declspec(naked) void FUN_0043ac30() {
    __asm {
        push    0x43abb0
        push    6
        push    0x170
        add     ecx, 0x18
        push    ecx
        call    FUN_009d1c4c
        ret
    }
}
