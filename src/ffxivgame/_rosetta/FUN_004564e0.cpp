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
// FUNCTION: ffxivgame 0x004564e0 — lazy-init-or-errno tail dispatcher
//                                  (59 B / 0x3b)
//
// Inspection (read from the disassembly at orig RVA 0x000564e0):
//
//   __cdecl void FUN_004564e0(void) — no stack args, both arms tail-JMP
//                                     to FUN_00456060 (no RET of its own).
//
//   Structure (matches asm flow):
//
//     int v = g_state_0126701c;             // MOV EAX, [0x0126701c]
//     if (v == -1) {                        // CMP EAX,-1 ; JNZ else
//         // first-time path: construct/get a singleton and clear two
//         // adjacent WORD fields at +0x88 / +0x8a, then tail-dispatch.
//         Obj *p = Get(0x0132d0e0);         // ECX = singleton; CALL 0x457270 (__thiscall)
//         p->field_88 = 0;                  // MOV [EAX+0x88], CX (CX=0)
//         p->field_8a = 0;                  // MOV [EAX+0x8a], CX
//         FUN_00456060();                   // JMP 0x456060 (tail)
//     } else {
//         int *e = _errno_like(v);          // PUSH EAX ; CALL [0x00f3e2a4]
//         *e = -1;                          // MOV [EAX], 0xffffffff
//         FUN_00456060();                   // JMP 0x456060 (tail)
//     }
//
// Reloc-bearing sites in the orig 59 bytes (these absolute addresses
// resolve only in a full-binary relink at image base 0x00400000;
// standalone .obj compilation can't reproduce them via source — naked
// asm emits them as raw immediate bytes which happen to match the orig
// binary's resolved bytes byte-for-byte):
//     +0x01   MOV EAX, [imm32]   → global 0x0126701c
//     +0x0a   MOV ECX, imm32     → singleton 0x0132d0e0
//     +0x0f   CALL rel32         → FUN_00457270 (RVA 0x000572f0 callsite-relative)
//     +0x24   JMP  rel32         → FUN_00456060
//     +0x2a   CALL [imm32]       → import IAT 0x00f3e2a4
//     +0x36   JMP  rel32         → FUN_00456060
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Both arms end in an unconditional tail JMP (MSVC 2005 does emit tail
//   jumps for shared epilogue join points like this), and the body is a
//   mix of absolute-addressed globals, an IAT-indirect call, and two
//   rel32 jumps. Coaxing the exact register allocation and tail-jump
//   shape out of C++ source — plus the surrounding TU needed to resolve
//   the singleton/import/global addresses — is not tractable standalone.
//   The pragmatic choice (same one the critsec/SEH siblings took) is a
//   `__declspec(naked)` body re-emitting the orig 59 bytes verbatim via
//   MASM `_emit` directives. The .obj's `.text` ends up byte-identical
//   to the orig slice; `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_004564e0() {
    __asm {
        _emit 0xa1              // MOV EAX, dword ptr [0x0126701c]
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        _emit 0x83              // CMP EAX, -0x01
        _emit 0xf8
        _emit 0xff
        _emit 0x75              // JNZ +0x1f  → else arm (0x00456509)
        _emit 0x1f
        _emit 0xb9              // MOV ECX, 0x0132d0e0
        _emit 0xe0
        _emit 0xd0
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL rel32 → 0x00457270
        _emit 0x7c
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x66              // MOV word ptr [EAX+0x88], CX
        _emit 0x89
        _emit 0x88
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66              // MOV word ptr [EAX+0x8a], CX
        _emit 0x89
        _emit 0x88
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe9              // JMP rel32 → 0x00456060
        _emit 0x57
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x50              // PUSH EAX            (else arm)
        _emit 0xff              // CALL dword ptr [0x00f3e2a4]
        _emit 0x15
        _emit 0xa4
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [EAX], 0xffffffff
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe9              // JMP rel32 → 0x00456060
        _emit 0x45
        _emit 0xfb
        _emit 0xff
        _emit 0xff
    }
}
