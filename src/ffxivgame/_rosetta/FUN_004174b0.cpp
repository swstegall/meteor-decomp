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
// FUNCTION: ffxivgame 0x000174b0 — sorted-array push wrapper, __thiscall,
//                                   2 stack args, RET 0x8 (36 B / 0x24)
//
// Thin wrapper around FUN_00416a00 (the sorted-array insert):
//   1. If this->count (field_0xc) == this->limit (field_0x8) → return 1 (full).
//   2. Otherwise push nullptr, &param2, &param1 onto the stack and tail-call
//      FUN_00416a00(__thiscall, RET 0xc) with the same `this` in ECX.
//
// Because FUN_00416a00 is __thiscall with RET 0xc (callee-cleans 3 args),
// it returns directly to FUN_004174b0's caller; the trailing RET 0x8 at
// 0x174d1 cleans FUN_004174b0's own 2 stack args after FUN_00416a00 returns.
//
// The LEA EDX,[ESP+0xc] / PUSH EDX / LEA EAX,[ESP+0xc] / PUSH EAX sequence
// computes &param2 and &param1 on the fly: after PUSH 0 the original [ESP+8]
// (param2) lands at [new ESP+0xc]; after PUSH EDX the original [ESP+4] (param1)
// lands at [new ESP+0xc]. MSVC reuses the same displacement both times because
// each PUSH shifts ESP by exactly 4.
//
// Calling convention: __thiscall — ECX = this, two DWORD stack args, RET 0x8.
// Frame: none (/Oy).
//
// Asm (36 bytes @ orig RVA 0x000174b0):
//   8b 41 0c             MOV EAX,[ECX+0xc]         ; this->count
//   3b 41 08             CMP EAX,[ECX+0x8]          ; compare with this->limit
//   75 08                JNZ +8 (→ 0x174c0)         ; if count != limit, go insert
//   b8 01 00 00 00       MOV EAX,0x1               ; return 1 (full)
//   c2 08 00             RET 0x8
//   6a 00                PUSH 0x0                   ; out_ptr = nullptr
//   8d 54 24 0c          LEA EDX,[ESP+0xc]          ; &param2
//   52                   PUSH EDX
//   8d 44 24 0c          LEA EAX,[ESP+0xc]          ; &param1
//   50                   PUSH EAX
//   e8 RR RR RR RR       CALL FUN_00416a00           ; (reloc — REL32)
//   c2 08 00             RET 0x8

extern "C" void FUN_00416a00();   // forward declaration for the CALL relocation

extern "C" __declspec(naked) void FUN_004174b0() {
    __asm {
        // 000174b0: 8b 41 0c   MOV EAX,[ECX+0xc]
        _emit 0x8b
        _emit 0x41
        _emit 0x0c
        // 000174b3: 3b 41 08   CMP EAX,[ECX+0x8]
        _emit 0x3b
        _emit 0x41
        _emit 0x08
        // 000174b6: 75 08      JNZ +8 (→ 0x174c0)
        _emit 0x75
        _emit 0x08
        // 000174b8: b8 01 00 00 00   MOV EAX,1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000174bd: c2 08 00   RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // 000174c0: 6a 00      PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 000174c2: 8d 54 24 0c   LEA EDX,[ESP+0xc]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 000174c6: 52         PUSH EDX
        _emit 0x52
        // 000174c7: 8d 44 24 0c   LEA EAX,[ESP+0xc]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 000174cb: 50         PUSH EAX
        _emit 0x50
        // 000174cc: e8 RR RR RR RR   CALL FUN_00416a00  (reloc)
        call FUN_00416a00
        // 000174d1: c2 08 00   RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
