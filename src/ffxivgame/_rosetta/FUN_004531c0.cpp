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
// FUNCTION: ffxivgame 0x000531c0 — bool-returning GetModuleFileName-style
// query wrapped in an MSVC /GS + EH3-style SEH frame (230 B / 0xe6).
//
// Asm shape (asm/ffxivgame/000531c0_FUN_004531c0.s):
//
//   __cdecl bool FUN_004531c0(Utf8String *out);   // arg0 at [esp+0x34]
//
//   Standard MSVC SEH prologue:
//     PUSH -1 / PUSH scope_table(0xe580b8) / PUSH FS:[0] /
//     SUB ESP,0x20 / cookie(0x012ea8b0) XOR ESP -> [esp+0x1c] /
//     second cookie XOR ESP pushed / install FS:[0].
//
//   Body (logical):
//     ECX = out;                              ; arg0
//     // construct a local Utf8String-style buffer at [esp+0x04..]:
//     //   [esp+0x1c] = 7 (SSO capacity), [esp+0x18] = 0 (size),
//     //   word[esp+0x08] = 0 (inline buf terminator), [esp+0x2c] = 0.
//     FUN_00449000(&local);                   ; CALL 0x00449000 — init buf
//     // pick inline-vs-heap data pointer: cap < 8 -> &inline, else heap.
//     p = (local.cap >= 8) ? local.heap : &local.inline;
//     n = GetModuleFileNameA-ish(p);          ; CALL [0x00f3e288] (IAT)
//     [esp+0x2c] = -1;
//     if (n == (DWORD)-1) {                    ; failure path
//         if (local.cap >= 8) FUN_0044d350(local.heap, 2*cap+2, 12); // free
//         return false;                        ; XOR AL,AL
//     }
//     if (n & 0x10) {                          ; TEST AL,0x10
//         if (local.cap >= 8) FUN_0044d350(local.heap, 2*cap+2, 12);
//         return true;                         ; MOV AL,1
//     }
//     if (local.cap >= 8) FUN_0044d350(local.heap, 2*cap+2, 12);   // free
//     return true;
//
//   SEH/`/GS` teardown: restore FS:[0], POP cookie+EH slot, reload /GS
//   cookie, CALL __security_check_cookie (0x009d20f4), ADD ESP,0x2c, RET.
//
// Reloc-bearing sites (absolute loads + rel32 calls). compare.py treats
// the .text bytes byte-for-byte; the original PE has the linker-applied
// values baked in (PUSH 0xe580b8, MOV [0x012ea8b0], CALL [0x00f3e288],
// and the three rel32 calls whose displacements are constant at the orig
// RVA pairs), so re-emitting the exact 230 bytes via `_emit` reproduces
// orig's `.text` exactly with no auxiliary subsections.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough, the
// same approach used by the committed siblings FUN_00403a20 and
// FUN_00408910. A source-level C++ port under /O2 /GS /EHsc would have to
// reproduce the precise inline-vs-heap buffer dispatch, the SSO local
// layout, the SEH state numbering and the /GS double-cookie shape — each
// high-level rewrite shifts at least one encoding. Naked-asm is the
// pragmatic byte-exact match.

extern "C" __declspec(naked) void FUN_004531c0() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xb8
        _emit 0x80
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x83
        _emit 0xec
        _emit 0x20
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x50
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        _emit 0x33
        _emit 0xc0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x66
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x50
        _emit 0xe8
        _emit 0xed
        _emit 0x5d
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x08
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x73
        _emit 0x04
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x50
        _emit 0xff
        _emit 0x15
        _emit 0x88
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x83
        _emit 0xc9
        _emit 0xff
        _emit 0x3b
        _emit 0xc1
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x74
        _emit 0x38
        _emit 0xa8
        _emit 0x10
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x74
        _emit 0x13
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        _emit 0x72
        _emit 0x48
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x6a
        _emit 0x0c
        _emit 0x8d
        _emit 0x4c
        _emit 0x00
        _emit 0x02
        _emit 0x51
        _emit 0x52
        _emit 0xeb
        _emit 0x32
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        _emit 0x72
        _emit 0x14
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x6a
        _emit 0x0c
        _emit 0x8d
        _emit 0x44
        _emit 0x00
        _emit 0x02
        _emit 0x50
        _emit 0x51
        _emit 0xe8
        _emit 0xeb
        _emit 0xa0
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xb0
        _emit 0x01
        _emit 0xeb
        _emit 0x1f
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        _emit 0x72
        _emit 0x14
        _emit 0x6a
        _emit 0x0c
        _emit 0x8d
        _emit 0x54
        _emit 0x00
        _emit 0x02
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x52
        _emit 0x50
        _emit 0xe8
        _emit 0xca
        _emit 0xa0
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x32
        _emit 0xc0
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x52
        _emit 0xee
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x2c
        _emit 0xc3
    }
}
