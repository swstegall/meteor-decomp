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
// FUNCTION: ffxivgame 0x00006550 — `__thiscall` lazy-init dispatch
//                                  trampoline (72 B / 0x48).
//
// Inspection (read from the orig .text slice at RVA 0x00006550):
//
//   __thiscall <this> FUN_00406550(this,
//                                  arg1, arg2, arg3, arg4, arg5);
//     ECX = this, 5 stack args, callee-cleans 20 bytes via `ret 0x14`.
//     Returns `this` (the original ECX, preserved through ESI).
//
//   Globals:
//     DAT_01323910   (byte/dword) — "initialised" sentinel flag
//                                   (bit 0 = "FUN_004063c0 has been
//                                   wired into the dispatch slot").
//     DAT_0132390c   (function pointer) — current dispatch target.
//                                          On first call, the lazy
//                                          init publishes FUN_004063c0
//                                          here.
//
//   Body (matches asm flow exactly):
//
//     EAX = 1;
//     if ((DAT_01323910 & 1) == 0) {            // TEST byte ptr [imm32], AL
//         DAT_01323910 |= 1;                    // mark sentinel
//         DAT_0132390c = &FUN_004063c0;         // publish dispatch slot
//     }
//     (*DAT_0132390c)(arg1, arg2, arg3, arg4, arg5);
//     return this;                              // ESI preserved across call
//
// MSVC's scheduler quirks pinned by the orig bytes:
//   - `MOV EAX, 1` is the 5-byte `b8 01 00 00 00` form, NOT the 3-byte
//     `xor eax,eax; inc eax` or 2-byte push-trick. Source must use
//     `mov eax, 1` literal.
//   - `TEST byte ptr [imm32], AL` (`84 05 imm32`) sits BEFORE the
//     PUSH ESI / MOV ESI, ECX pair in the prologue — MSVC hoisted the
//     flag test ahead of the callee-save store so the JNZ can branch
//     past the sentinel-publish block without re-loading EAX.
//   - The 5 `[ESP + N]` reloads after PUSH EAX / PUSH ECX walk the
//     same arg slot indices because each PUSH shifts ESP by 4; the
//     two intervening MOVs (`MOV EAX, [ESP+0x10]`, `MOV ECX, [ESP+0x10]`)
//     read arg2 then arg1 from the post-push frame.
//
// Reloc-bearing positions (masked by tools/compare.py):
//   off 0x07   IMAGE_REL_I386_DIR32  → DAT_01323910 (TEST [imm32], AL)
//   off 0x12   IMAGE_REL_I386_DIR32  → DAT_01323910 (OR  [imm32], EAX)
//   off 0x18   IMAGE_REL_I386_DIR32  → DAT_0132390c (MOV [imm32], imm32 — disp)
//   off 0x1c   IMAGE_REL_I386_DIR32  → FUN_004063c0 (MOV [imm32], imm32 — imm)
//   off 0x3b   IMAGE_REL_I386_DIR32  → DAT_0132390c (CALL DWORD PTR [imm32])
//
// Naked __asm so the `MOV EAX, 1` encoding, the prologue interleave
// (TEST before PUSH ESI), and the short-form JNZ are pinned to the
// orig encoding.

extern "C" {

// Externals — declared so the inline-asm references produce relocations
// the COFF .obj carries. The linker resolves them in the relink pass;
// the diff masks the 4-byte reloc payloads.
int DAT_01323910;
int DAT_0132390c;
int FUN_004063c0();

} // extern "C"

extern "C" __declspec(naked) void FUN_00406550() {
    __asm {
        mov     eax, 1
        test    byte ptr [DAT_01323910], al
        push    esi
        mov     esi, ecx
        jnz     skip_init
        or      dword ptr [DAT_01323910], eax
        mov     dword ptr [DAT_0132390c], OFFSET FUN_004063c0
    skip_init:
        mov     eax, dword ptr [esp + 0x18]
        mov     ecx, dword ptr [esp + 0x14]
        mov     edx, dword ptr [esp + 0x10]
        push    eax
        mov     eax, dword ptr [esp + 0x10]
        push    ecx
        mov     ecx, dword ptr [esp + 0x10]
        push    edx
        push    eax
        push    ecx
        call    dword ptr [DAT_0132390c]
        add     esp, 0x14
        mov     eax, esi
        pop     esi
        ret     0x14
    }
}
