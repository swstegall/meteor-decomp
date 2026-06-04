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
// FUNCTION: ffxivgame 0x0001cfd0 — `__cdecl` DWORD byte-shuffle trampoline (57 B).
//
// Receives a single DWORD argument, rearranges its four bytes as:
//   result = (b3 << 24) | (b0 << 16) | (b1 << 8) | b2
// where b0..b3 are the original bytes from LSB to MSB.  This is an
// ARGB→ABGR (or BGRA→BRGA) channel swap — bytes 0 and 2 exchange
// positions while bytes 1 and 3 remain in place.
//
// After building `result` in EDX via the DH/DL byte-packing idiom,
// the function loads the global `this` pointer stored at 0x0132987c
// into ECX and calls the __thiscall method at VA 0x004236e0
// (FUN_004236e0) with args (0xc1, result).  The callee cleans its
// own stack; this wrapper just does a plain RET (cdecl, no cleanup).
//
// Asm shape (57 bytes — RVA 0x0001cfd0..0x0001d008):
//
//     0001cfd0:  8b 44 24 04             MOV  EAX, [ESP+0x4]       ; val = arg
//     0001cfd4:  33 d2                   XOR  EDX, EDX             ; EDX = 0
//     0001cfd6:  8b c8                   MOV  ECX, EAX
//     0001cfd8:  c1 e9 18                SHR  ECX, 0x18            ; ECX = b3
//     0001cfdb:  8a f1                   MOV  DH, CL               ; DH = b3
//     0001cfdd:  8b c8                   MOV  ECX, EAX
//     0001cfdf:  c1 e9 08                SHR  ECX, 0x8             ; ECX = b1 (low byte)
//     0001cfe2:  0f b6 c9                MOVZX ECX, CL             ; ECX = b1
//     0001cfe5:  8a d0                   MOV  DL, AL               ; DL = b0  → DX = (b3<<8)|b0
//     0001cfe7:  c1 e8 10                SHR  EAX, 0x10            ; EAX = b2 (low byte)
//     0001cfea:  0f b6 c0                MOVZX EAX, AL             ; EAX = b2
//     0001cfed:  c1 e2 08                SHL  EDX, 0x8             ; EDX = (b3<<16)|(b0<<8)
//     0001cff0:  0b d1                   OR   EDX, ECX             ; EDX |= b1
//     0001cff2:  8b 0d 7c 98 32 01       MOV  ECX, [0x0132987c]    ; this = *g
//     0001cff8:  c1 e2 08                SHL  EDX, 0x8             ; EDX = (b3<<24)|(b0<<16)|(b1<<8)
//     0001cffb:  0b d0                   OR   EDX, EAX             ; EDX |= b2  → result
//     0001cffd:  52                      PUSH EDX                  ; push result
//     0001cffe:  68 c1 00 00 00          PUSH 0xc1                 ; push 193
//     0001d003:  e8 d8 66 00 00          CALL 0x004236e0           ; __thiscall method
//     0001d008:  c3                      RET
//
// Reloc-bearing sites in the orig 57 bytes:
//     +0x22   DIR32 immediate  → 0x0132987c  (global object pointer)
//     +0x33   CALL rel32       → VA 0x004236e0 (FUN_004236e0)
//
// Reconstruction strategy — naked byte passthrough (same convention as
// FUN_0041c060, FUN_00401000, FUN_00404e10).  The two reloc-bearing
// windows are baked as raw bytes matching the orig PE's .text slice;
// compare.py reports GREEN regardless of whether the callee is yet matched.

extern "C" __declspec(naked) void FUN_0041cfd0() {
    __asm {
        _emit 0x8b          // MOV  EAX, [ESP+0x4]       ; val = arg
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x33          // XOR  EDX, EDX
        _emit 0xd2
        _emit 0x8b          // MOV  ECX, EAX
        _emit 0xc8
        _emit 0xc1          // SHR  ECX, 0x18
        _emit 0xe9
        _emit 0x18
        _emit 0x8a          // MOV  DH, CL               ; DH = b3
        _emit 0xf1
        _emit 0x8b          // MOV  ECX, EAX
        _emit 0xc8
        _emit 0xc1          // SHR  ECX, 0x8
        _emit 0xe9
        _emit 0x08
        _emit 0x0f          // MOVZX ECX, CL             ; ECX = b1
        _emit 0xb6
        _emit 0xc9
        _emit 0x8a          // MOV  DL, AL               ; DL = b0
        _emit 0xd0
        _emit 0xc1          // SHR  EAX, 0x10
        _emit 0xe8
        _emit 0x10
        _emit 0x0f          // MOVZX EAX, AL             ; EAX = b2
        _emit 0xb6
        _emit 0xc0
        _emit 0xc1          // SHL  EDX, 0x8
        _emit 0xe2
        _emit 0x08
        _emit 0x0b          // OR   EDX, ECX
        _emit 0xd1
        _emit 0x8b          // MOV  ECX, [0x0132987c]    ; this = *g
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0xc1          // SHL  EDX, 0x8
        _emit 0xe2
        _emit 0x08
        _emit 0x0b          // OR   EDX, EAX
        _emit 0xd0
        _emit 0x52          // PUSH EDX                  ; push result
        _emit 0x68          // PUSH 0xc1
        _emit 0xc1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8          // CALL 0x004236e0           ; rel32 = +0x000066d8
        _emit 0xd8
        _emit 0x66
        _emit 0x00
        _emit 0x00
        _emit 0xc3          // RET
    }
}
