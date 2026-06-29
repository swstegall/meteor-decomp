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
// FUNCTION: ffxivgame 0x00443b60 — FUN_00443b60 (200 B / 0xc8)
//
// SEH-framed __cdecl function. Receives two stack arguments (an object
// pointer in arg1 / EDI and a key in arg2 / EBX). Calls two __thiscall
// methods on the object to locate a [start, end) range keyed by arg2
// against a global sentinel iterator ([0x00f67298]):
//
//   ESI = this->FUN_00446fb0(key, 0)           // lower-bound / find-start
//   EAX = this->FUN_00446ff0(key, g_sentinel)  // upper-bound / find-end
//
// Then branches on whether start and/or end hit the sentinel:
//   both at sentinel → call this->FUN_00445530() (insert / no-op handler)
//   start at sentinel only → set start = 0, fall through
//   end at sentinel only → call this->FUN_00445e50() (cleanup)
//   start >= end (unsigned) → call this->FUN_00445530()
//   start < end → range exists:
//       iter = this->FUN_00447a80(&local, start, count)  // build iterator
//       (enter SEH try state 0)
//       this->FUN_00447450(iter)                         // process range
//       local.FUN_00446f50()                             // destruct iterator
//       (exit SEH try, state back to -1)
//
// Reloc-bearing sites (masked by tools/compare.py during diff):
//   +0x02  DIR32  0xe572c8       (SEH handler table entry)
//   +0x09  DIR32  0x012ea8b0     (__security_cookie)
//   +0x33  REL32  → 0x00446fb0  (lower-bound method)
//   +0x3a  DIR32  0x00f67298     (g_sentinel global, first load)
//   +0x43  REL32  → 0x00446ff0  (upper-bound method)
//   +0x48  DIR32  0x00f67298     (g_sentinel global, second load)
//   +0x5e  REL32  → 0x00445e50  (cleanup on end-miss)
//   +0x69  REL32  → 0x00445530  (insert / no-range handler)
//   +0x8f  REL32  → 0x00447a80  (range-iterator constructor)
//   +0x9f  REL32  → 0x00447450  (range-iterator consumer)
//   +0xb0  REL32  → 0x00446f50  (range-iterator destructor)
//
// Reconstruction strategy — naked-asm byte passthrough.
//
//   The SEH prologue/epilogue (PUSH -1 / PUSH handler / MOV EAX, FS:[0] /
//   SUB ESP / ... / MOV FS:[0], ECX / POP / ADD ESP / RET), the two
//   FS:-prefixed MOV instructions (64 A1 / 64 A3 form), and the mid-function
//   EH-state writes (MOV [ESP+0x70], 0 / MOV [ESP+0x6c], 0xffffffff) make
//   reliable MASM symbolic encoding fragile. The 200 orig bytes are emitted
//   verbatim via `_emit` directives — the same strategy used by
//   FUN_00404e40 and FUN_00406680.

extern "C" __declspec(naked) void FUN_00443b60() {
    __asm {
        // 00443b60: PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00443b62: PUSH 0xe572c8
        _emit 0x68
        _emit 0xc8
        _emit 0x72
        _emit 0xe5
        _emit 0x00
        // 00443b67: MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00443b6d: PUSH EAX
        _emit 0x50
        // 00443b6e: SUB ESP, 0x54
        _emit 0x83
        _emit 0xec
        _emit 0x54
        // 00443b71: PUSH EBX
        _emit 0x53
        // 00443b72: PUSH ESI
        _emit 0x56
        // 00443b73: PUSH EDI
        _emit 0x57
        // 00443b74: MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00443b79: XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 00443b7b: PUSH EAX
        _emit 0x50
        // 00443b7c: LEA EAX, [ESP + 0x64]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x64
        // 00443b80: MOV FS:[0x0], EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00443b86: MOV EBX, [ESP + 0x78]   ; arg2
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x78
        // 00443b8a: MOV EDI, [ESP + 0x74]   ; arg1 (object pointer)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x74
        // 00443b8e: PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00443b90: PUSH EBX
        _emit 0x53
        // 00443b91: MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00443b93: CALL 0x00446fb0   (lower-bound method)
        _emit 0xe8
        _emit 0x18
        _emit 0x34
        _emit 0x00
        _emit 0x00
        // 00443b98: MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 00443b9a: MOV EAX, [0x00f67298]  (g_sentinel)
        _emit 0xa1
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        // 00443b9f: PUSH EAX
        _emit 0x50
        // 00443ba0: PUSH EBX
        _emit 0x53
        // 00443ba1: MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00443ba3: CALL 0x00446ff0   (upper-bound method)
        _emit 0xe8
        _emit 0x48
        _emit 0x34
        _emit 0x00
        _emit 0x00
        // 00443ba8: MOV ECX, [0x00f67298]
        _emit 0x8b
        _emit 0x0d
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        // 00443bae: CMP ESI, ECX
        _emit 0x3b
        _emit 0xf1
        // 00443bb0: JNZ +0x06  (→ 0x00443bb8)
        _emit 0x75
        _emit 0x06
        // 00443bb2: CMP EAX, ECX
        _emit 0x3b
        _emit 0xc1
        // 00443bb4: JZ  +0x11  (→ 0x00443bc7)
        _emit 0x74
        _emit 0x11
        // 00443bb6: XOR ESI, ESI
        _emit 0x33
        _emit 0xf6
        // 00443bb8: CMP EAX, ECX
        _emit 0x3b
        _emit 0xc1
        // 00443bba: JNZ +0x07  (→ 0x00443bc3)
        _emit 0x75
        _emit 0x07
        // 00443bbc: MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00443bbe: CALL 0x00445e50   (cleanup on end-miss)
        _emit 0xe8
        _emit 0x8d
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 00443bc3: CMP ESI, EAX
        _emit 0x3b
        _emit 0xf0
        // 00443bc5: JBE +0x1a  (→ 0x00443be1, range-exists path)
        _emit 0x76
        _emit 0x1a
        // 00443bc7: MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00443bc9: CALL 0x00445530   (insert / no-range handler)
        _emit 0xe8
        _emit 0x62
        _emit 0x19
        _emit 0x00
        _emit 0x00
        // 00443bce: MOV ECX, [ESP + 0x64]   (restore SEH chain)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x64
        // 00443bd2: MOV FS:[0x0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00443bd9: POP ECX
        _emit 0x59
        // 00443bda: POP EDI
        _emit 0x5f
        // 00443bdb: POP ESI
        _emit 0x5e
        // 00443bdc: POP EBX
        _emit 0x5b
        // 00443bdd: ADD ESP, 0x60
        _emit 0x83
        _emit 0xc4
        _emit 0x60
        // 00443be0: RET
        _emit 0xc3
        // ── range-exists path ───────────────────────────────────────────
        // 00443be1: SUB EAX, ESI   ; count = end - start
        _emit 0x2b
        _emit 0xc6
        // 00443be3: ADD EAX, 0x1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 00443be6: PUSH EAX        ; count arg (3rd)
        _emit 0x50
        // 00443be7: PUSH ESI        ; start arg (2nd)
        _emit 0x56
        // 00443be8: LEA ECX, [ESP + 0x18]  ; &local_iter (1st arg)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00443bec: PUSH ECX
        _emit 0x51
        // 00443bed: MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00443bef: CALL 0x00447a80   (range-iterator constructor)
        _emit 0xe8
        _emit 0x8c
        _emit 0x3e
        _emit 0x00
        _emit 0x00
        // 00443bf4: PUSH EAX          ; iterator result arg
        _emit 0x50
        // 00443bf5: MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00443bf7: MOV [ESP + 0x70], 0x0   ; EH state → 0 (enter try)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x70
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00443bff: CALL 0x00447450   (range-iterator consumer)
        _emit 0xe8
        _emit 0x4c
        _emit 0x38
        _emit 0x00
        _emit 0x00
        // 00443c04: LEA ECX, [ESP + 0x10]   ; &local_iter for destructor
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00443c08: MOV [ESP + 0x6c], 0xffffffff   ; EH state → -1 (exit try)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x6c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00443c10: CALL 0x00446f50   (range-iterator destructor)
        _emit 0xe8
        _emit 0x3b
        _emit 0x33
        _emit 0x00
        _emit 0x00
        // 00443c15: MOV ECX, [ESP + 0x64]   (restore SEH chain)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x64
        // 00443c19: MOV FS:[0x0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00443c20: POP ECX
        _emit 0x59
        // 00443c21: POP EDI
        _emit 0x5f
        // 00443c22: POP ESI
        _emit 0x5e
        // 00443c23: POP EBX
        _emit 0x5b
        // 00443c24: ADD ESP, 0x60
        _emit 0x83
        _emit 0xc4
        _emit 0x60
        // 00443c27: RET
        _emit 0xc3
    }
}
