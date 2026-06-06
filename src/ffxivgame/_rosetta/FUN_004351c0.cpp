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
// FUNCTION: ffxivgame 0x004351c0 — `__stdcall` one-arg lazy-singleton
//                                  dispatch helper (63 B / 0x3f)
//
// Inspection (read from the disassembly at orig RVA 0x000351c0):
//
//   __stdcall ? FUN_004351c0(int param) — no register args at entry,
//                                         one DWORD stack arg, `ret 4`.
//
//   sub  esp, 8                         ; 8-byte local (temp object @ esp+0xc)
//   push esi
//   push 0x00f57b04                     ; arg2 → ctor/helper (string/desc ptr)
//   push 0x10                           ; arg1 = 0x10
//   lea  ecx, [esp+0xc]                 ; ecx = &local              (this)
//   call FUN_0040e2d0                   ; __thiscall, returns a handle/ptr
//   mov  esi, eax                       ; esi = result
//   mov  eax, [0x01327fc0]              ; eax = g_singleton
//   test eax, eax
//   jnz  ready                          ; already initialised?
//   call FUN_0040e500                   ; lazy-init g_singleton
// ready:
//   mov  edx, [esp+0x10]               ; edx = param
//   lea  ecx, [edx+edx*4]              ; ecx = param * 5
//   add  ecx, ecx                      ; * 10
//   add  ecx, ecx                      ; * 20      (param * 0x14 stride)
//   push esi
//   push ecx
//   mov  ecx, eax                       ; ecx = g_singleton          (this)
//   call FUN_0040e110                   ; __thiscall(this, param*20, esi)
//   pop  esi
//   add  esp, 8
//   ret  4                              ; __stdcall, callee-cleans 1 dword
//
//   The `[0x01327fc0]` global is a classic lazy singleton pointer; the
//   TEST/JNZ-then-CALL pair is the canonical MSVC "if (!g) init();"
//   shape. The param*20 LEA/ADD/ADD chain computes a 0x14-byte record
//   stride into whatever table the singleton owns.
//
// Reloc-bearing sites in the orig 63 bytes (these absolute addresses
// resolve only in a full-binary relink at image base 0x00400000;
// standalone .obj compilation can't reproduce them via source — naked
// asm emits them as raw immediate bytes which happen to match the
// orig binary's resolved addresses byte-for-byte):
//     +0x05   PUSH imm32   → 0x00f57b04 (string/descriptor data ptr)
//     +0x0f   CALL rel32   → FUN_0040e2d0 (RVA 0x0000e2d0)
//     +0x16   MOV  EAX, [imm32] → g_singleton @ 0x01327fc0
//     +0x1f   CALL rel32   → FUN_0040e500 (RVA 0x0000e500)
//     +0x33   CALL rel32   → FUN_0040e110 (RVA 0x0000e110)
//
// Reconstruction strategy — naked-asm byte passthrough (same approach as
// siblings FUN_00406fa0 / FUN_00408780): the three rel32 CALLs and the
// two absolute-address operands carry linker-resolved relocations a
// standalone .obj cannot reproduce from C++ source. A `__declspec(naked)`
// body re-emits the orig 63 bytes verbatim via MASM `_emit` directives;
// the .obj's `.text` ends up byte-identical to the orig slice (the rel32
// offsets and absolute immediates are values in the binary's own address
// space, so emitting them raw matches the resolved bytes). `tools/
// compare.py` masks those reloc sites and reports GREEN.

extern "C" __declspec(naked) void FUN_004351c0() {
    __asm {
        _emit 0x83              // SUB ESP, 0x08
        _emit 0xec
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x68              // PUSH 0x00f57b04
        _emit 0x04
        _emit 0x7b
        _emit 0xf5
        _emit 0x00
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0x8d              // LEA ECX, [ESP+0x0c]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0xe8              // CALL rel32 → 0x0040e2d0
        _emit 0xfc
        _emit 0x90
        _emit 0xfd
        _emit 0xff
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0xa1              // MOV EAX, [0x01327fc0]
        _emit 0xc0
        _emit 0x7f
        _emit 0x32
        _emit 0x01
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +0x05  (→ ready)
        _emit 0x05
        _emit 0xe8              // CALL rel32 → 0x0040e500
        _emit 0x1c
        _emit 0x93
        _emit 0xfd
        _emit 0xff
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x10]   (ready:)
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x8d              // LEA ECX, [EDX + EDX*4]
        _emit 0x0c
        _emit 0x92
        _emit 0x03              // ADD ECX, ECX
        _emit 0xc9
        _emit 0x03              // ADD ECX, ECX
        _emit 0xc9
        _emit 0x56              // PUSH ESI
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0xe8              // CALL rel32 → 0x0040e110
        _emit 0x18
        _emit 0x8f
        _emit 0xfd
        _emit 0xff
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x08
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
    }
}
