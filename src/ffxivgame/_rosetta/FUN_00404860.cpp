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
// FUNCTION: ffxivgame 0x00004860 — `__stdcall` 3-arg wrapper around the
//                                  SEH-protected array-construct helper at
//                                  0x00404570 (60 B / 0x3c)
//
// Inspection (read from the orig .text slice at RVA 0x00004860):
//
//   __stdcall int wrapper(p1, p2, p3) — `RET 0xC`, 3 dword args.
//
//   Body (matches asm flow byte-for-byte):
//
//     push ecx                       ; allocate 4-byte local at [esp]
//     mov  edx, [esp+0x10]           ; edx = p3
//     push esi
//     mov  esi, [esp+0x10]           ; esi = p2
//     push edi
//     mov  edi, [esp+0x10]           ; edi = p1
//     mov  byte ptr [esp+0x8], 0     ; touch only the low byte of local
//     mov  eax, [esp+0x8]            ; reload that local (3 upper bytes
//                                    ;   are the scratch ecx-at-entry, but
//                                    ;   MSVC promoted the byte store to a
//                                    ;   dword reload anyway)
//     push eax                       ; arg6 = local
//     mov  eax, [esp+0x1c]           ; eax = p3 (independent reload)
//     push edx                       ; arg5 = p3
//     push ecx                       ; arg4 = scratch ecx-at-entry
//     push eax                       ; arg3 = p3
//     push esi                       ; arg2 = p2
//     push edi                       ; arg1 = p1
//     call FUN_00404570              ; 6-arg __cdecl callee
//     add  esp, 0x18                 ; clean 6*4 = 0x18 bytes
//     lea  ecx, [esi*8]              ; (7-byte form: 8d 0c f5 00 00 00 00)
//     sub  ecx, esi                  ; ecx = esi * 7
//     lea  eax, [edi+ecx*4]          ; eax = edi + esi*28
//     pop  edi
//     pop  esi
//     pop  ecx
//     ret  0xC                       ; __stdcall — return p1 + p2 * 0x1c
//
//   Headless-Ghidra hint:
//     int FUN_00404860(int p1,int p2,undefined4 p3) {
//       FUN_00404570(p1,p2,p3); return p1 + p2 * 0x1c;
//     }
//
//   The wrapper is a thin shim that builds a 6-arg call frame around a
//   __cdecl callee whose declared signature is only the 3 args the
//   caller threads through — the extra three pushed words are scratch
//   slots that the SEH-aware callee at 0x00404570 either ignores or
//   consumes via tail-stack patterns the compiler emitted for the
//   __try/__finally frame. Reconstructing this at the C++ source level
//   would force MSVC to materialise a different prologue (different
//   register allocation for the three captured args, no uninitialised-
//   ecx pass, no byte-store-then-dword-reload pattern). The pragmatic
//   match is a `__declspec(naked)` body so the byte sequence is exact.
//
//   The CALL at +0x22 carries a REL32 relocation to FUN_00404570 which
//   tools/compare.py masks out of the diff; everything else is raw
//   bytes that MASM's inline assembler reproduces from the mnemonics
//   below (verified short/long encodings — `mov reg, [esp+disp8]`,
//   `lea ecx, [esi*8]` 7-byte form, `lea eax, [edi+ecx*4]`, `ret imm16`).

extern "C" void FUN_00404570();

extern "C" __declspec(naked) void FUN_00404860() {
    __asm {
        push ecx
        mov  edx, [esp+0x10]
        push esi
        mov  esi, [esp+0x10]
        push edi
        mov  edi, [esp+0x10]
        mov  byte ptr [esp+0x8], 0
        mov  eax, [esp+0x8]
        push eax
        mov  eax, [esp+0x1c]
        push edx
        push ecx
        push eax
        push esi
        push edi
        call FUN_00404570
        add  esp, 0x18
        lea  ecx, [esi*8]
        sub  ecx, esi
        lea  eax, [edi+ecx*4]
        pop  edi
        pop  esi
        pop  ecx
        ret  0x0c
    }
}
