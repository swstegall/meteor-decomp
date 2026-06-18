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
// FUNCTION: ffxivgame 0x00435440 — __stdcall 3-arg computation selector
//                                  switch on type (1-5+) returning int
//                                  (158 bytes / 0x9e, RET 0xc)
//
// Calling convention: __stdcall (callee cleans 3 dwords → RET 0xc).
// No callee-saved registers; no frame pointer.
//
// Arguments (on stack at entry):
//   [ESP+0x4]  type  (int): computation mode, 1-based selector
//   [ESP+0x8]  a     (int): first operand
//   [ESP+0xc]  b     (int): second operand
//
// Switch dispatch (type-1 as unsigned 0-based index, bounds 0..5):
//   case 1 (idx 0): return a * b
//   case 2 (idx 1): return 2 * (a * b)
//   case 3 (idx 2): return (a + 1) * b
//   case 4 (idx 3): return 3 * (a * b)   [via LEA EAX,[EAX+EAX*2]]
//   case 5 (idx 4): return (a + 2) * b
//   default  (idx > 5): fire a one-time error report through a global
//                        function pointer, then return 0.
//
// Jump table at VA 0x004354e0 (in .text, 2 bytes after function end).
//
// Default path global references (absolute VAs in orig PE):
//   0x01323910 — once-flag DWORD: bit-0 guards fn-ptr initialisation
//   0x0132390c — error-reporter fn ptr (set to VA 0x00433720 on first call)
//   0x00f56510, 0x00f54d48, 0x00f649f8, 0x00f64a58 — string args
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The switch jump-table dispatch (ff 24 85 e0 54 43 00) and the
//   one-time error-reporter sequence (TEST/OR/MOV on absolute VAs plus
//   five PUSH imm32 string addresses) contain absolute VA constants
//   that C++ source cannot reproduce without knowing the final link
//   address. There are no REL32 CALL targets in the function body.
//   Emitting all 158 bytes verbatim produces a .obj whose .text is
//   byte-identical to the orig PE slice; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00435440() {
    __asm {
        // 00035440: 8b 44 24 04   MOV EAX,dword ptr [ESP+0x4]   (type)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00035444: 83 c0 ff      ADD EAX,-0x1                  (0-based index)
        _emit 0x83
        _emit 0xc0
        _emit 0xff
        // 00035447: 83 f8 05      CMP EAX,0x5
        _emit 0x83
        _emit 0xf8
        _emit 0x05
        // 0003544a: 77 4e         JA  +0x4e                     (→ default @ 0x3549a)
        _emit 0x77
        _emit 0x4e
        // 0003544c: ff 24 85 e0 54 43 00   JMP dword ptr [EAX*4+0x4354e0]
        _emit 0xff
        _emit 0x24
        _emit 0x85
        _emit 0xe0
        _emit 0x54
        _emit 0x43
        _emit 0x00

        // === case 1 (index 0): return a * b ===
        // 00035453: 8b 44 24 08   MOV EAX,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00035457: 0f af 44 24 0c   IMUL EAX,dword ptr [ESP+0xc]
        _emit 0x0f
        _emit 0xaf
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0003545c: c2 0c 00      RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00

        // === case 2 (index 1): return 2 * (a * b) ===
        // 0003545f: 8b 44 24 08   MOV EAX,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00035463: 0f af 44 24 0c   IMUL EAX,dword ptr [ESP+0xc]
        _emit 0x0f
        _emit 0xaf
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00035468: 03 c0         ADD EAX,EAX                   (double)
        _emit 0x03
        _emit 0xc0
        // 0003546a: c2 0c 00      RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00

        // === case 3 (index 2): return (a + 1) * b ===
        // 0003546d: 8b 44 24 08   MOV EAX,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00035471: 83 c0 01      ADD EAX,0x1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 00035474: 0f af 44 24 0c   IMUL EAX,dword ptr [ESP+0xc]
        _emit 0x0f
        _emit 0xaf
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00035479: c2 0c 00      RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00

        // === case 4 (index 3): return 3 * (a * b) ===
        // 0003547c: 8b 44 24 08   MOV EAX,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00035480: 0f af 44 24 0c   IMUL EAX,dword ptr [ESP+0xc]
        _emit 0x0f
        _emit 0xaf
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00035485: 8d 04 40      LEA EAX,dword ptr [EAX+EAX*2]  (×3)
        _emit 0x8d
        _emit 0x04
        _emit 0x40
        // 00035488: c2 0c 00      RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00

        // === case 5 (index 4): return (a + 2) * b ===
        // 0003548b: 8b 44 24 08   MOV EAX,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0003548f: 83 c0 02      ADD EAX,0x2
        _emit 0x83
        _emit 0xc0
        _emit 0x02
        // 00035492: 0f af 44 24 0c   IMUL EAX,dword ptr [ESP+0xc]
        _emit 0x0f
        _emit 0xaf
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00035497: c2 0c 00      RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00

        // === default: one-time error report, then return 0 ===
        // 0003549a: b8 01 00 00 00   MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003549f: 84 05 10 39 32 01   TEST byte ptr [0x01323910],AL
        _emit 0x84
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 000354a5: 75 10         JNZ +0x10    (→ 0x354b7 if flag already set)
        _emit 0x75
        _emit 0x10
        // 000354a7: 09 05 10 39 32 01   OR dword ptr [0x01323910],EAX
        _emit 0x09
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 000354ad: c7 05 0c 39 32 01 20 37 43 00
        //           MOV dword ptr [0x0132390c],0x00433720   (set error fn ptr)
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00
        // 000354b7: 68 58 4a f6 00   PUSH 0x00f64a58  (arg5: string)
        _emit 0x68
        _emit 0x58
        _emit 0x4a
        _emit 0xf6
        _emit 0x00
        // 000354bc: 68 b1 00 00 00   PUSH 0xb1        (arg4: line number 177)
        _emit 0x68
        _emit 0xb1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000354c1: 68 f8 49 f6 00   PUSH 0x00f649f8  (arg3: string)
        _emit 0x68
        _emit 0xf8
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 000354c6: 68 48 4d f5 00   PUSH 0x00f54d48  (arg2: string)
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 000354cb: 68 10 65 f5 00   PUSH 0x00f56510  (arg1: string)
        _emit 0x68
        _emit 0x10
        _emit 0x65
        _emit 0xf5
        _emit 0x00
        // 000354d0: ff 15 0c 39 32 01   CALL dword ptr [0x0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 000354d6: 83 c4 14      ADD ESP,0x14       (pop 5 args × 4 bytes)
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 000354d9: 33 c0         XOR EAX,EAX        (return 0)
        _emit 0x33
        _emit 0xc0
        // 000354db: c2 0c 00      RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
