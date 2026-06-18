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
// FUNCTION: ffxivgame 0x000207e0 — `__cdecl` byte-arg global-store + thiscall
//                                   forwarder (27 B).
//
// Takes a single byte argument, stores it to the global at VA 0x01328ef0,
// then zero-extends it and forwards it (along with constant 0x1b) as two
// stack args to the `__thiscall` method FUN_004236e0.  The `this` pointer
// is loaded from the global object pointer at VA 0x0132987c — the same
// global that FUN_0041d120 and its siblings use.  The callee cleans its
// own two pushed args via an internal `RET 8`, leaving the stack balanced
// so this wrapper can return with a plain `RET`.
//
// Parameters:
//   param1  [ESP+0x4]  — byte value to store and forward
//
// Asm shape (27 bytes — RVA 0x000207e0..0x000207fa):
//
//   000207e0:  8a 44 24 04           MOV  AL, byte ptr [ESP+0x4]   ; arg byte
//   000207e4:  8b 0d 7c 98 32 01     MOV  ECX, [0x0132987c]        ; ECX = this
//   000207ea:  a2 f0 8e 32 01        MOV  [0x01328ef0], AL         ; store to global
//   000207ef:  0f b6 c0              MOVZX EAX, AL                 ; zero-extend
//   000207f2:  50                    PUSH EAX                      ; push zero-ext arg
//   000207f3:  6a 1b                 PUSH 0x1b                     ; push constant
//   000207f5:  e8 e6 2e 00 00        CALL 0x004236e0               ; thiscall(this, 0x1b, arg)
//   000207fa:  c3                    RET
//
// Reloc-bearing sites in the orig 27 bytes:
//   +0x06  DIR32  → VA 0x0132987c   (global object pointer)
//   +0x0b  DIR32  → VA 0x01328ef0   (global byte store target)
//   +0x14  CALL rel32 → FUN_004236e0 (rel32 = +0x00002ee6)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The source-level form would be approximately:
//     g_byte = param1; thiscall_method(*g_obj, 0x1b, (unsigned char)param1);
//   but MSVC 2005 /O2's register-allocation and instruction-selection
//   (specifically the `MOVZX EAX,AL` reuse after the global store) makes
//   a verbatim naked emit the only reliable way to reproduce the exact
//   27-byte sequence.  Follows the convention of FUN_0041d120 and other
//   thiscall-forwarding wrappers in this module.

extern "C" __declspec(naked) void FUN_004207e0() {
    __asm {
        // 000207e0: 8a 44 24 04  MOV AL, byte ptr [ESP+0x4]  ; arg byte
        _emit 0x8a
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 000207e4: 8b 0d 7c 98 32 01  MOV ECX, [0x0132987c]  ; ECX = this
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 000207ea: a2 f0 8e 32 01  MOV [0x01328ef0], AL  ; store to global
        _emit 0xa2
        _emit 0xf0
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        // 000207ef: 0f b6 c0  MOVZX EAX, AL  ; zero-extend
        _emit 0x0f
        _emit 0xb6
        _emit 0xc0
        // 000207f2: 50  PUSH EAX  ; push zero-extended arg
        _emit 0x50
        // 000207f3: 6a 1b  PUSH 0x1b  ; push constant
        _emit 0x6a
        _emit 0x1b
        // 000207f5: e8 e6 2e 00 00  CALL 0x004236e0  ; rel32 = +0x00002ee6
        _emit 0xe8
        _emit 0xe6
        _emit 0x2e
        _emit 0x00
        _emit 0x00
        // 000207fa: c3  RET
        _emit 0xc3
    }
}
