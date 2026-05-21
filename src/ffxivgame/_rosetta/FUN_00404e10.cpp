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
// FUNCTION: ffxivgame 0x00404e10 — `__cdecl` 1-arg trampoline (28 B).
// Loads the caller's stack arg, calls `__thiscall FUN_00447450` with
// `ecx = &DAT_01323898` and that arg, then overwrites the stack arg
// slot with `&DAT_01323898` and tail-jumps to `__cdecl FUN_00452d00`.
// The `mov [esp+4], imm32` + `jmp` pair is MSVC 2005's classic tail-call
// optimisation that reuses the existing arg slot rather than push/pop a
// new one; the original return address at [esp+0] flows through unchanged.
//
// Pseudo-C:
//
//   void __cdecl FUN_00404e10(void *arg) {
//       FUN_00447450_method(&DAT_01323898, arg);   // ecx = this
//       FUN_00452d00(&DAT_01323898);                // tail call
//   }
//
// `DAT_01323898` is the global singleton instance whose address is
// returned by FUN_00404e30 (`return &DAT_01323898;`).
//
// Reloc-bearing positions in the resulting .obj (masked in the diff):
//   off 0x06   IMAGE_REL_I386_DIR32  → DAT_01323898 (mov ecx, imm32)
//   off 0x0b   IMAGE_REL_I386_REL32  → FUN_00447450 (call)
//   off 0x13   IMAGE_REL_I386_DIR32  → DAT_01323898 (mov m32, imm32)
//   off 0x18   IMAGE_REL_I386_REL32  → FUN_00452d00 (jmp)

extern "C" {

// Externals — declared so the inline-asm references produce relocations
// the COFF .obj can carry. The linker resolves them in the relink pass;
// for byte-level matching, compare.py masks the 4-byte reloc payloads.
int DAT_01323898;
int FUN_00447450();
int FUN_00452d00();

} // extern "C"

extern "C" __declspec(naked) void __cdecl FUN_00404e10() {
    __asm {
        mov     eax, dword ptr [esp + 4]            ; eax = arg
        push    eax                                  ; stack arg for thiscall
        mov     ecx, OFFSET DAT_01323898             ; ecx = this
        call    FUN_00447450                         ; thiscall, ret 4 cleans
        mov     dword ptr [esp + 4], OFFSET DAT_01323898  ; overwrite arg slot
        jmp     FUN_00452d00                         ; tail jmp (cdecl)
    }
}

// vim: ts=4 sts=4 sw=4 et
