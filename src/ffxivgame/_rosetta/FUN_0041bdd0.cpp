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
// FUNCTION: ffxivgame 0x0001bdd0 — __cdecl byte-arg forwarder to FUN_004236e0 (23 B)
//
// Takes one unsigned-byte argument, zero-extends it to a DWORD, loads the
// global __thiscall receiver from 0x0132987c into ECX, then calls
// FUN_004236e0 with arguments (0xae, byte_arg).
//
// Calling convention: __cdecl (bare RET — caller owns argument cleanup).
// Stack frame: none (no prologue/epilogue — ESP-relative leaf only).
//
// Asm (23 bytes @ orig RVA 0x0001bdd0):
//   0f b6 44 24 04     MOVZX EAX, byte ptr [ESP+0x4]    ; zero-extend byte arg → EAX
//   8b 0d 7c 98 32 01  MOV ECX, dword ptr [0x0132987c]  ; this = *g_obj_132987c
//   50                 PUSH EAX                          ; push byte_arg (2nd callee arg)
//   68 ae 00 00 00     PUSH 0xae                         ; push 174 (1st callee arg)
//   e8 fa 78 00 00     CALL FUN_004236e0                 ; __thiscall method
//   c3                 RET
//
// Reloc-bearing sites (masked by compare.py):
//   +0x06  DIR32 immediate → 0x0132987c  (global object pointer)
//   +0x11  CALL  rel32    → VA 0x004236e0 (FUN_004236e0)
//
// Structural twin of FUN_0041d0c0 (same global, same callee, different
// arg encoding — that sibling takes a DWORD index; this one takes a BYTE).

extern "C" {

// Global object pointer at VA 0x0132987c; loaded into ECX as `this` before
// the call to FUN_004236e0.  Also referenced by FUN_0041d0c0, FUN_0041cfd0,
// FUN_00418290, and other wrappers in this cluster.
extern int g_obj_132987c;

// __thiscall member at VA 0x004236e0.  Called here with args (0xae, byte_val).
void FUN_004236e0();

__declspec(naked) void FUN_0041bdd0() {
    __asm {
        movzx eax, byte ptr [esp+4]
        mov   ecx, dword ptr [g_obj_132987c]
        push  eax
        push  0xae
        call  FUN_004236e0
        ret
    }
}

}  // extern "C"
