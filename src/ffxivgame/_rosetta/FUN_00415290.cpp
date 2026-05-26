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
// FUNCTION: ffxivgame 0x00415290 — five-function-pointer registration thunk
//                                   (__cdecl, no args, 34 B / 0x22)
//
// Source-level intent (Ghidra hint):
//   FUN_00416400(&LAB_004150c0, &LAB_00415140, &DAT_00414fb0,
//                &LAB_004150b0, &LAB_00415210);
//
// All five pushed operands are absolute addresses of code labels inside
// the same .text section (cdecl right-to-left ordering):
//   arg1 (top of stack at CALL): 0x004150c0
//   arg2                       : 0x00415140
//   arg3                       : 0x00414fb0  (one-instruction RET stub)
//   arg4                       : 0x004150b0  (XOR EAX,EAX / RET — returns 0)
//   arg5 (pushed first)        : 0x00415210
// Target of the CALL:            0x00416400 (rel32 offset 0x1152 from
//                                 0x004152ae, i.e. fall-through after CALL)
//
// Each PUSH carries a 4-byte DIR32 relocation against the corresponding
// LAB/DAT symbol; the CALL carries a 4-byte REL32 relocation against
// FUN_00416400. compare.py masks those nine reloc windows, so the .obj's
// .text matches orig byte-for-byte: 5x `68 RR RR RR RR`, one
// `e8 RR RR RR RR`, then the literal `83 c4 14 c3` epilogue.
//
// Reproduced via __declspec(naked) — the function has no prologue, no
// locals, no frame pointer. A naive C++ rewrite of the form
//   FUN_00416400(FUN_004150c0, FUN_00415140, ...)
// would compile to the identical byte sequence under /O2, but the naked
// form guarantees the relocation layout and avoids dragging in C++
// function-pointer-type plumbing for symbols that are pure label
// addresses on the wire.
//
// Asm (34 bytes @ orig RVA 0x00015290):
//   68 c0 50 41 00     PUSH offset FUN_004150c0      ; arg1
//   68 40 51 41 00     PUSH offset FUN_00415140      ; arg2
//   68 b0 4f 41 00     PUSH offset FUN_00414fb0      ; arg3
//   68 b0 50 41 00     PUSH offset FUN_004150b0      ; arg4
//   68 10 52 41 00     PUSH offset FUN_00415210      ; arg5
//   e8 52 11 00 00     CALL FUN_00416400             ; rel32 reloc
//   83 c4 14           ADD ESP, 0x14                 ; 5 * 4 cleanup
//   c3                 RET

extern "C" void FUN_004150c0();
extern "C" void FUN_00415140();
extern "C" void FUN_00414fb0();
extern "C" void FUN_004150b0();
extern "C" void FUN_00415210();
extern "C" void FUN_00416400();

extern "C" __declspec(naked) void __cdecl FUN_00415290() {
    __asm {
        push offset FUN_00415210
        push offset FUN_004150b0
        push offset FUN_00414fb0
        push offset FUN_00415140
        push offset FUN_004150c0
        call FUN_00416400
        add esp, 14h
        ret
    }
}
