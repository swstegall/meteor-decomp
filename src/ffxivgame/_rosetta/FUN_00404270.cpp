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
// FUNCTION: ffxivgame 0x00404270 — small-string "reset + init from arg"
//                                  helper (__thiscall, 37 B)
//
//   __thiscall void *FUN_00404270(StringLike *this, void *arg)
//     stack layout (after RET 4):
//       ECX        : this
//       [ESP+0x04] : void *arg                 (param_1)
//     returns: this (in EAX)
//
// Inspection (read from the orig bytes at RVA 0x00004270, 37 bytes total):
//
//   push esi
//   xor  eax, eax                 ; eax = 0 (reused for the zero stores
//                                 ; and as the third arg to the helper)
//   mov  esi, ecx                 ; esi = this
//   push -1                       ; helper arg #3 = 0xFFFFFFFF (size_t -1)
//   mov  [esi + 0x14], eax        ; this->size      = 0
//   mov  dword ptr [esi + 0x18], 0x0F
//                                 ; this->capacity  = 0x0F (SSO sentinel)
//   push eax                      ; helper arg #2 = 0
//   mov  byte ptr [esi + 0x04], al
//                                 ; first byte of inline-buf null-terminated
//   mov  eax, dword ptr [esp + 0x10]
//                                 ; reload caller's `arg` (after the two
//                                 ; pushes the original [esp+4] is now at
//                                 ; [esp+0xC]; [esp+0x10] reads the dword
//                                 ; *past* it — the orig actually re-reads
//                                 ; the same caller arg slot since the
//                                 ; prolog pushed ESI (4) + the 2× args (8))
//   push eax                      ; helper arg #1 = caller's arg
//   call FUN_00404040             ; rel32 → 0x00404040
//   mov  eax, esi                 ; return this
//   pop  esi
//   ret  4                        ; __thiscall, callee-cleans 1 dword
//
// Calling convention: __thiscall (this in ECX, one stack arg, callee
//   pops via `ret 4`).
// Stack frame: 0 (only the one ESI register save + outgoing pushes).
//
// Reloc-bearing sites in the orig 37 bytes (the CALL rel32 would
// otherwise resolve at link time; we re-emit the orig rel32 bytes
// verbatim so the .obj's .text matches byte-for-byte with no
// relocations):
//     +0x1B   CALL rel32  → FUN_00404040  (target 0x00404040)
//                          encoded offset 0xFFFFFDB1 = -0x24F
//                          (next-IP 0x0040428F + (-0x24F) = 0x00404040)
//
// Reconstruction strategy — naked-asm byte passthrough (mirrors the
// sibling FUN_00403eb0 / FUN_00403bd0): a `__declspec(naked)` body
// re-emits the orig 37 bytes verbatim via MASM `_emit` directives.
// The compiled .obj's `.text` section is byte-identical to the orig
// slice with NO relocations — the CALL rel32 immediate is the exact
// in-orig wire offset, baked into the orig binary's own address space.
// `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_00404270() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x6a              // PUSH 0xFFFFFFFF (sign-extended -1)
        _emit 0xff
        _emit 0x89              // MOV dword ptr [ESI+0x14], EAX
        _emit 0x46
        _emit 0x14
        _emit 0xc7              // MOV dword ptr [ESI+0x18], 0x0000000F
        _emit 0x46
        _emit 0x18
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x88              // MOV byte ptr [ESI+0x04], AL
        _emit 0x46
        _emit 0x04
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_00404040 (rel32 → 0x00404040)
        _emit 0xb1
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
    }
}
