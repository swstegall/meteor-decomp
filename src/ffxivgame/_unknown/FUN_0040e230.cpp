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
// FUNCTION: ffxivgame 0x0000e230 — __thiscall struct-field setter (20 B / 0x14)
//
// Asm (20 bytes @ orig RVA 0x0000e230):
//   8b c1              MOV EAX, ECX           ; EAX = this (from __thiscall ECX)
//   8b 4c 24 04        MOV ECX, [ESP + 0x4]   ; ECX = param_1 (pointer)
//   8b 11              MOV EDX, [ECX]          ; EDX = *param_1
//   8b 4c 24 08        MOV ECX, [ESP + 0x8]   ; ECX = param_2 (value)
//   89 10              MOV [EAX], EDX          ; this[0x0] = *param_1
//   89 48 04           MOV [EAX + 0x4], ECX   ; this[0x4] = param_2
//   c2 08 00           RET 0x8                ; __thiscall clean 2 stack args
//
// Calling convention: __thiscall (ECX = this); RET 0x8 — two 4-byte stack params.
// No prologue — /Oy frame-pointer omission; leaf function, no callee-saved regs used.
// ECX is reused twice: once as the first stack arg (to load a pointer and deref it),
// then as the second stack arg (a direct value), after saving this to EAX.
//
// Observed call site in FUN_0040dbd0 @ 0x0000dc2b:
//   LEA ECX,[ESP+0x1c]         ; ECX = this (ptr to local struct)
//   PUSH 0x00f55758            ; param_2 = string literal address
//   PUSH EDX                   ; param_1 = pointer (EDX = [0x012652f8])
//   CALL FUN_0040e230          ; -> this[0] = *EDX, this[4] = 0x00f55758
//
// The struct being initialized has at least two 4-byte fields:
//   struct SomeConfig {         // __thiscall receiver
//       int   field_0x00;       // = *param_1  (dereferenced from caller's pointer)
//       int   field_0x04;       // = param_2   (value stored directly)
//   };
//
// Naked-asm chosen to guarantee the exact byte sequence — a source-level
// __thiscall method would need /O2 to pick EAX for `this` before reloading
// ECX from the stack, but register-allocator luck is not guaranteed.

extern "C" __declspec(naked) void FUN_0040e230() {
    __asm {
        // 0000e230: 8b c1      MOV EAX, ECX
        _emit 0x8b
        _emit 0xc1
        // 0000e232: 8b 4c 24 04  MOV ECX, [ESP+0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0000e236: 8b 11      MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 0000e238: 8b 4c 24 08  MOV ECX, [ESP+0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 0000e23c: 89 10      MOV [EAX], EDX
        _emit 0x89
        _emit 0x10
        // 0000e23e: 89 48 04   MOV [EAX+0x4], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 0000e241: c2 08 00   RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}

// vim: ts=4 sts=4 sw=4 et
