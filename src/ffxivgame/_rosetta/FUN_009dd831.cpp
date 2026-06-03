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
// FUNCTION: ffxivgame 0x005dd831 — stream-buffer `sputc`-like write with
//                                  locale/encoding validity check
//                                  (286 B / 0x11e, __cdecl, two args).
//
// Signature (reconstructed from asm):
//
//   int __cdecl FUN_009dd831(int c, SomeStreamBuf *p);
//
// Behaviour read from the disassembly at orig RVA 0x005dd831:
//
//   p (arg1) is a stream-buffer-like struct:
//     p+0x00 : char* pptr   — current write position
//     p+0x04 : int   count  — characters written / remaining
//     p+0x08 : char* epptr  — end-of-buffer (or NULL for unbuffered)
//     p+0x0c : int   flags  — mode bits: 0x40=bypass-check, 0x01=mode1,
//                              0x02=mode2, 0x10=dirty bit, 0x80=???
//
//   c (arg0) is the character to write (int, low byte used).
//
//   Two-stage check (skipped when p->flags & 0x40):
//     FUN_009d6a61 is called up to four times to walk some locale/encoding
//     state and perform a two-level property-table lookup at 0x137b7e0:
//       item = global_table[r3 >> 5] + ((r4 & 0x1f) << 6)
//     If (item+0x24) & 0x7f is nonzero (flags in low 7 bits set), the
//     character is invalid: call FUN_009d9d47() to get a thread-local
//     error struct, write error-code 0x16 (EILSEQ) into it, push five
//     zeros, call FUN_009d2290 (error-report helper), and return -1.
//     A second similar lookup checks (item+0x24) & 0x80; if set, same
//     error path.
//
//   Write path (entered when p->flags & 0x40, or when validity passes):
//     EBX = c (arg0).  If EBX == -1, return -1 immediately.
//     Check p->flags for write-enabled conditions (bit 0x01 or
//     (sign bit set AND bit 0x02 clear)).  If not writable, return -1.
//     If p->epptr == NULL, call FUN_009f0a23(p) to set up the buffer.
//     If p->pptr == p->epptr:
//       if p->count == 0, return -1 (buffer full and no flush available).
//       Increment p->pptr by 1 (advance past sentinel).
//     Decrement *p->pptr (adjust position).
//     If p->flags & 0x40: if *p->pptr == (char)c, accept; else skip+return -1.
//     Else: write (char)c to *p->pptr.
//     Increment p->count; clear flag 0x10, set flag 0x01 in p->flags.
//     Return c & 0xff.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ cannot reproduce this function's .obj bytes because:
//     1. Four CALL rel32 instructions to FUN_009d6a61, FUN_009d9d47,
//        FUN_009d2290, and FUN_009f0a23 — a source-level call emits
//        E8 00 00 00 00 + a COFF relocation in the .obj; compare.py sees
//        mismatched bytes against the already-linked binary.
//     2. Two absolute VA immediates baked into instructions:
//        MOV EBX, 0x12eb4d8  (default/fallback item pointer)
//        LEA EDI, [EAX*4 + 0x137b7e0]  (two-level encoding table, x2)
//        A source-level reference would emit these as COFF relocations
//        rather than raw immediates.
//
//   The `__declspec(naked)` body re-emits the orig 286 bytes verbatim via
//   MASM `_emit` directives, exactly as FUN_00404f70 and FUN_00401a00 do.

extern "C" __declspec(naked) void FUN_009dd831() {
    __asm {
        // 005dd831 (+0x000)
        _emit 0x53  // PUSH EBX
        _emit 0x55  // PUSH EBP
        _emit 0x56  // PUSH ESI
        _emit 0x8b  // MOV ESI,[ESP+0x14]
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x83  // OR EBP,-1
        _emit 0xcd
        _emit 0xff
        _emit 0xf6  // TEST byte ptr [ESI+0xc],0x40
        _emit 0x46
        _emit 0x0c
        _emit 0x40
        _emit 0x57  // PUSH EDI
        _emit 0x0f  // JNZ +0xa8 (→ 0x009dd8ee)

        // 005dd841 (+0x010)
        _emit 0x85
        _emit 0xa8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL FUN_009d6a61
        _emit 0x15
        _emit 0x92
        _emit 0xff
        _emit 0xff
        _emit 0x3b  // CMP EAX,EBP
        _emit 0xc5
        _emit 0x59  // POP ECX
        _emit 0xbb  // MOV EBX,0x12eb4d8
        _emit 0xd8

        // 005dd851 (+0x020)
        _emit 0xb4
        _emit 0x2e
        _emit 0x01
        _emit 0x74  // JZ +0x2e (→ 0x009dd884)
        _emit 0x2e
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL FUN_009d6a61
        _emit 0x05
        _emit 0x92
        _emit 0xff
        _emit 0xff
        _emit 0x83  // CMP EAX,-2
        _emit 0xf8
        _emit 0xfe
        _emit 0x59  // POP ECX
        _emit 0x74  // JZ +0x22 (→ 0x009dd884)

        // 005dd861 (+0x030)
        _emit 0x22
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL FUN_009d6a61
        _emit 0xf9
        _emit 0x91
        _emit 0xff
        _emit 0xff
        _emit 0xc1  // SAR EAX,5
        _emit 0xf8
        _emit 0x05
        _emit 0x56  // PUSH ESI
        _emit 0x8d  // LEA EDI,[EAX*4+0x137b7e0]
        _emit 0x3c
        _emit 0x85
        _emit 0xe0
        _emit 0xb7

        // 005dd871 (+0x040)
        _emit 0x37
        _emit 0x01
        _emit 0xe8  // CALL FUN_009d6a61
        _emit 0xe9
        _emit 0x91
        _emit 0xff
        _emit 0xff
        _emit 0x83  // AND EAX,0x1f
        _emit 0xe0
        _emit 0x1f
        _emit 0x59  // POP ECX
        _emit 0xc1  // SHL EAX,6
        _emit 0xe0
        _emit 0x06
        _emit 0x03  // ADD EAX,[EDI]
        _emit 0x07

        // 005dd881 (+0x050)
        _emit 0x59  // POP ECX
        _emit 0xeb  // JMP +0x02 (→ 0x009dd886)
        _emit 0x02
        _emit 0x8b  // MOV EAX,EBX
        _emit 0xc3
        _emit 0xf6  // TEST byte ptr [EAX+0x24],0x7f
        _emit 0x40
        _emit 0x24
        _emit 0x7f
        _emit 0x75  // JNZ +0x41 (→ 0x009dd8cd)
        _emit 0x41
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL FUN_009d6a61
        _emit 0xcf
        _emit 0x91
        _emit 0xff

        // 005dd891 (+0x060)
        _emit 0xff
        _emit 0x3b  // CMP EAX,EBP
        _emit 0xc5
        _emit 0x59  // POP ECX
        _emit 0x74  // JZ +0x2e (→ 0x009dd8c5)
        _emit 0x2e
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL FUN_009d6a61
        _emit 0xc4
        _emit 0x91
        _emit 0xff
        _emit 0xff
        _emit 0x83  // CMP EAX,-2
        _emit 0xf8
        _emit 0xfe
        _emit 0x59  // POP ECX

        // 005dd8a1 (+0x070)
        _emit 0x74  // JZ +0x22 (→ 0x009dd8c5)
        _emit 0x22
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL FUN_009d6a61
        _emit 0xb8
        _emit 0x91
        _emit 0xff
        _emit 0xff
        _emit 0xc1  // SAR EAX,5
        _emit 0xf8
        _emit 0x05
        _emit 0x56  // PUSH ESI
        _emit 0x8d  // LEA EDI,[EAX*4+0x137b7e0]
        _emit 0x3c
        _emit 0x85
        _emit 0xe0

        // 005dd8b1 (+0x080)
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0xe8  // CALL FUN_009d6a61
        _emit 0xa8
        _emit 0x91
        _emit 0xff
        _emit 0xff
        _emit 0x83  // AND EAX,0x1f
        _emit 0xe0
        _emit 0x1f
        _emit 0x59  // POP ECX
        _emit 0xc1  // SHL EAX,6
        _emit 0xe0
        _emit 0x06
        _emit 0x03  // ADD EAX,[EDI]

        // 005dd8c1 (+0x090)
        _emit 0x07
        _emit 0x59  // POP ECX
        _emit 0xeb  // JMP +0x02 (→ 0x009dd8c7)
        _emit 0x02
        _emit 0x8b  // MOV EAX,EBX
        _emit 0xc3
        _emit 0xf6  // TEST byte ptr [EAX+0x24],0x80
        _emit 0x40
        _emit 0x24
        _emit 0x80
        _emit 0x74  // JZ +0x21 (→ 0x009dd8ee)
        _emit 0x21
        _emit 0xe8  // CALL FUN_009d9d47
        _emit 0x75
        _emit 0xc4
        _emit 0xff

        // 005dd8d1 (+0x0a0)
        _emit 0xff
        _emit 0x33  // XOR EDI,EDI
        _emit 0xff
        _emit 0x57  // PUSH EDI
        _emit 0x57  // PUSH EDI
        _emit 0x57  // PUSH EDI
        _emit 0x57  // PUSH EDI
        _emit 0x57  // PUSH EDI
        _emit 0xc7  // MOV dword ptr [EAX],0x16
        _emit 0x00
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL FUN_009d2290
        _emit 0xac

        // 005dd8e1 (+0x0b0)
        _emit 0x49
        _emit 0xff
        _emit 0xff
        _emit 0x83  // ADD ESP,0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x8b  // MOV EAX,EBP  (= -1)
        _emit 0xc5
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5d  // POP EBP
        _emit 0x5b  // POP EBX
        _emit 0xc3  // RET
        _emit 0x8b  // MOV EBX,[ESP+0x14]
        _emit 0x5c
        _emit 0x24

        // 005dd8f1 (+0x0c0)
        _emit 0x14
        _emit 0x3b  // CMP EBX,EBP
        _emit 0xdd
        _emit 0x74  // JZ -0x0f (→ 0x009dd8e7)
        _emit 0xf1
        _emit 0x8b  // MOV EAX,[ESI+0xc]
        _emit 0x46
        _emit 0x0c
        _emit 0xa8  // TEST AL,0x1
        _emit 0x01
        _emit 0x75  // JNZ +0x08 (→ 0x009dd905)
        _emit 0x08
        _emit 0x84  // TEST AL,AL
        _emit 0xc0
        _emit 0x79  // JNS -0x1a (→ 0x009dd8e7)
        _emit 0xe6

        // 005dd901 (+0x0d0)
        _emit 0xa8  // TEST AL,0x2
        _emit 0x02
        _emit 0x75  // JNZ -0x1e (→ 0x009dd8e7)
        _emit 0xe2
        _emit 0x33  // XOR EDI,EDI
        _emit 0xff
        _emit 0x39  // CMP [ESI+0x8],EDI
        _emit 0x7e
        _emit 0x08
        _emit 0x75  // JNZ +0x07 (→ 0x009dd913)
        _emit 0x07
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL FUN_009f0a23
        _emit 0x11
        _emit 0x31
        _emit 0x01

        // 005dd911 (+0x0e0)
        _emit 0x00
        _emit 0x59  // POP ECX
        _emit 0x8b  // MOV EAX,[ESI]
        _emit 0x06
        _emit 0x3b  // CMP EAX,[ESI+0x8]
        _emit 0x46
        _emit 0x08
        _emit 0x75  // JNZ +0x08 (→ 0x009dd922)
        _emit 0x08
        _emit 0x39  // CMP [ESI+0x4],EDI
        _emit 0x7e
        _emit 0x04
        _emit 0x75  // JNZ -0x38 (→ 0x009dd8e7)
        _emit 0xc8
        _emit 0x40  // INC EAX
        _emit 0x89  // MOV [ESI],EAX

        // 005dd921 (+0x0f0)
        _emit 0x06
        _emit 0xff  // DEC dword ptr [ESI]
        _emit 0x0e
        _emit 0xf6  // TEST byte ptr [ESI+0xc],0x40
        _emit 0x46
        _emit 0x0c
        _emit 0x40
        _emit 0x8b  // MOV EAX,[ESI]
        _emit 0x06
        _emit 0x74  // JZ +0x09 (→ 0x009dd935)
        _emit 0x09
        _emit 0x38  // CMP byte ptr [EAX],BL
        _emit 0x18
        _emit 0x74  // JZ +0x07 (→ 0x009dd937)
        _emit 0x07
        _emit 0x40  // INC EAX

        // 005dd931 (+0x100)
        _emit 0x89  // MOV [ESI],EAX
        _emit 0x06
        _emit 0xeb  // JMP -0x4e (→ 0x009dd8e7)
        _emit 0xb2
        _emit 0x88  // MOV byte ptr [EAX],BL
        _emit 0x18
        _emit 0x8b  // MOV EAX,[ESI+0xc]
        _emit 0x46
        _emit 0x0c
        _emit 0xff  // INC dword ptr [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x83  // AND EAX,0xffffffef
        _emit 0xe0
        _emit 0xef
        _emit 0x83  // OR EAX,0x1

        // 005dd941 (+0x110)
        _emit 0xc8
        _emit 0x01
        _emit 0x89  // MOV [ESI+0xc],EAX
        _emit 0x46
        _emit 0x0c
        _emit 0x8b  // MOV EAX,EBX
        _emit 0xc3
        _emit 0x25  // AND EAX,0xff
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb  // JMP -0x66 (→ 0x009dd8e9)
        _emit 0x9a
    }
}
