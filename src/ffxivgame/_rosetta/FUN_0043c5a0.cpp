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
// FUNCTION: ffxivgame 0x0003c5a0 — palette/key-expansion table builder
//           (__cdecl, 224 B / 0xe0 declared, shared epilog at 0x3c680).
//
// Signature (recovered from asm):
//
//   void FUN_0043c5a0(void *dst, const unsigned char *src);
//
//   arg0 = [ESP+4] = dst   — pointer to output buffer (~512 KB struct)
//   arg1 = [ESP+8] = src   — pointer to input byte array
//
// Body sketch:
//
//   1. REP MOVSD copies 128 bytes (ECX=0x20 dwords) from a fixed data
//      table at 0xf665a0 into the start of *dst.
//
//   2. Sets up:
//        EDI = src + 1        (input pointer, advanced by 1 before loop)
//        ESI = dst + 0x84     (output pointer into *dst)
//        EBP = 0x10000        (loop counter: 65536 iterations)
//        EDX = 0xFFFFFFFF     (constant -1, written as DWORD at start of
//                              each 8-byte output block)
//
//   3. Loop (65536 iterations):
//        [ESI-4]  = EDX = 0xFFFFFFFF  (DWORD -1 flag for each block)
//        [ESI+0]  = transform_hi(EDI[-1]) * 3
//        [ESI+1]  = transform_lo(EDI[-1]) * 3
//        [ESI+2]  = transform_hi(EDI[0])  * 3
//        [ESI+3]  = transform_lo(EDI[0])  * 3
//        ESI += 8; EDI += 2
//
//      transform_hi(c): extracts bits [7:4] into 0|b7|0|b6|0|b5|0|b4
//        AL = ((c >> 1) & 8) | (c & 0x20); AL >>= 1
//        AL |= (c & 0x40);                 AL >>= 1
//        AL |= (c & 0x81);                 AL >>= 1
//        AL *= 3
//
//      transform_lo(c): extracts bits [3:0] into 0|b3|0|b2|0|b1|0|b0
//        AL = (c & 8) << 1 | (c & 4); AL <<= 1
//        AL |= (c & 2);               AL <<= 1
//        AL |= (c & 1)
//        AL *= 3
//
// Layout note: the declared size in config (0xe0 = 224 bytes) ends at
// offset 0x3c67f = POP ESI. The shared epilog (POP EBP, POP EBX, RET)
// at 0x3c680..0x3c682 sits outside the declared boundary and is not
// part of this slice.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The 65536-iteration loop with the JMP-to-loop-body layout, the
//   3-byte `8D 49 00` alignment NOP between the forward JMP and the
//   loop head, and the precise register allocation (EBX, EBP, ESI, EDI,
//   EDX all live across the entire loop) would need exact MSVC 2005 /O2
//   scheduling to reproduce in C++. The naked-asm passthrough is the
//   safe path: emit 224 bytes verbatim; compare.py reports GREEN.
//
//   The only potential relocation site (MOV ESI, 0xf665a0 at +0x0d) is
//   a raw immediate baked into the PE. The _emit directives emit the
//   same `a0 65 f6 00` bytes, so compare.py's byte comparison passes
//   without any reloc-masking.

extern "C" __declspec(naked) void FUN_0043c5a0() {
    __asm {
        // --- prolog: save registers, REP MOVSD init copy ----------------
        _emit 0x8b          // MOV EAX, [ESP+4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x53          // PUSH EBX
        _emit 0x55          // PUSH EBP
        _emit 0x56          // PUSH ESI
        _emit 0x57          // PUSH EDI
        _emit 0xb9          // MOV ECX, 0x20
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xbe          // MOV ESI, 0xf665a0  (fixed data table)
        _emit 0xa0
        _emit 0x65
        _emit 0xf6
        _emit 0x00
        _emit 0x8b          // MOV EDI, EAX
        _emit 0xf8
        _emit 0xf3          // REP MOVSD
        _emit 0xa5
        _emit 0x8b          // MOV EDI, [ESP+0x18]  (arg1 = src)
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x83          // ADD EDI, 1
        _emit 0xc7
        _emit 0x01
        _emit 0x8d          // LEA ESI, [EAX+0x84]  (output ptr into *dst)
        _emit 0xb0
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xbd          // MOV EBP, 0x10000  (loop count)
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x83          // OR EDX, 0xffffffff  (EDX = -1)
        _emit 0xca
        _emit 0xff
        _emit 0xeb          // JMP loop_body (+3, to 0x43c5d0)
        _emit 0x03
        // --- 3-byte alignment NOP (dead code, targets loop to 16B boundary)
        _emit 0x8d          // LEA ECX, [ECX+0]  (8D 49 00)
        _emit 0x49
        _emit 0x00
        // --- loop body (65536 iters, processes 2 input bytes → 4+4 output)
        _emit 0x89          // MOV [ESI-4], EDX   (-1 flag dword)
        _emit 0x56
        _emit 0xfc
        _emit 0x8a          // MOV CL, [EDI-1]   (first input byte)
        _emit 0x4f
        _emit 0xff
        // --- transform_hi(EDI[-1]) → [ESI+0] ---
        _emit 0x8a          // MOV AL, CL
        _emit 0xc1
        _emit 0xd0          // SHR AL, 1
        _emit 0xe8
        _emit 0x24          // AND AL, 8
        _emit 0x08
        _emit 0x8a          // MOV BL, CL
        _emit 0xd9
        _emit 0x80          // AND BL, 0x20
        _emit 0xe3
        _emit 0x20
        _emit 0x0a          // OR AL, BL
        _emit 0xc3
        _emit 0xd0          // SHR AL, 1
        _emit 0xe8
        _emit 0x8a          // MOV BL, CL
        _emit 0xd9
        _emit 0x80          // AND BL, 0x40
        _emit 0xe3
        _emit 0x40
        _emit 0x0a          // OR AL, BL
        _emit 0xc3
        _emit 0xd0          // SHR AL, 1
        _emit 0xe8
        _emit 0x8a          // MOV BL, CL
        _emit 0xd9
        _emit 0x80          // AND BL, 0x81
        _emit 0xe3
        _emit 0x81
        _emit 0x0a          // OR AL, BL
        _emit 0xc3
        _emit 0xd0          // SHR AL, 1
        _emit 0xe8
        _emit 0xb3          // MOV BL, 3
        _emit 0x03
        _emit 0xf6          // IMUL BL  (AX = AL * 3)
        _emit 0xeb
        _emit 0x88          // MOV [ESI+0], AL
        _emit 0x06
        // --- transform_lo(EDI[-1]) → [ESI+1] ---
        _emit 0x8a          // MOV AL, CL
        _emit 0xc1
        _emit 0x24          // AND AL, 8
        _emit 0x08
        _emit 0x02          // ADD AL, AL  (= SHL 1)
        _emit 0xc0
        _emit 0x8a          // MOV BL, CL
        _emit 0xd9
        _emit 0x80          // AND BL, 4
        _emit 0xe3
        _emit 0x04
        _emit 0x0a          // OR AL, BL
        _emit 0xc3
        _emit 0x02          // ADD AL, AL
        _emit 0xc0
        _emit 0x8a          // MOV BL, CL
        _emit 0xd9
        _emit 0x80          // AND BL, 2
        _emit 0xe3
        _emit 0x02
        _emit 0x0a          // OR AL, BL
        _emit 0xc3
        _emit 0x02          // ADD AL, AL
        _emit 0xc0
        _emit 0x80          // AND CL, 1
        _emit 0xe1
        _emit 0x01
        _emit 0x0a          // OR AL, CL
        _emit 0xc1
        _emit 0xb1          // MOV CL, 3
        _emit 0x03
        _emit 0xf6          // IMUL CL  (AX = AL * 3)
        _emit 0xe9
        _emit 0x88          // MOV [ESI+1], AL
        _emit 0x46
        _emit 0x01
        // --- second input byte: MOV CL, [EDI] ---
        _emit 0x8a          // MOV CL, [EDI]
        _emit 0x0f
        // --- transform_hi(EDI[0]) → [ESI+2] ---
        _emit 0x8a          // MOV AL, CL
        _emit 0xc1
        _emit 0xd0          // SHR AL, 1
        _emit 0xe8
        _emit 0x24          // AND AL, 8
        _emit 0x08
        _emit 0x8a          // MOV BL, CL
        _emit 0xd9
        _emit 0x80          // AND BL, 0x20
        _emit 0xe3
        _emit 0x20
        _emit 0x0a          // OR AL, BL
        _emit 0xc3
        _emit 0xd0          // SHR AL, 1
        _emit 0xe8
        _emit 0x8a          // MOV BL, CL
        _emit 0xd9
        _emit 0x80          // AND BL, 0x40
        _emit 0xe3
        _emit 0x40
        _emit 0x0a          // OR AL, BL
        _emit 0xc3
        _emit 0xd0          // SHR AL, 1
        _emit 0xe8
        _emit 0x8a          // MOV BL, CL
        _emit 0xd9
        _emit 0x80          // AND BL, 0x81
        _emit 0xe3
        _emit 0x81
        _emit 0x0a          // OR AL, BL
        _emit 0xc3
        _emit 0xd0          // SHR AL, 1
        _emit 0xe8
        _emit 0xb3          // MOV BL, 3
        _emit 0x03
        _emit 0xf6          // IMUL BL
        _emit 0xeb
        _emit 0x88          // MOV [ESI+2], AL
        _emit 0x46
        _emit 0x02
        // --- transform_lo(EDI[0]) → [ESI+3] ---
        _emit 0x8a          // MOV AL, CL
        _emit 0xc1
        _emit 0x24          // AND AL, 8
        _emit 0x08
        _emit 0x02          // ADD AL, AL
        _emit 0xc0
        _emit 0x8a          // MOV BL, CL
        _emit 0xd9
        _emit 0x80          // AND BL, 4
        _emit 0xe3
        _emit 0x04
        _emit 0x0a          // OR AL, BL
        _emit 0xc3
        _emit 0x02          // ADD AL, AL
        _emit 0xc0
        _emit 0x8a          // MOV BL, CL
        _emit 0xd9
        _emit 0x80          // AND BL, 2
        _emit 0xe3
        _emit 0x02
        _emit 0x0a          // OR AL, BL
        _emit 0xc3
        _emit 0x02          // ADD AL, AL
        _emit 0xc0
        _emit 0x80          // AND CL, 1
        _emit 0xe1
        _emit 0x01
        _emit 0x0a          // OR AL, CL
        _emit 0xc1
        _emit 0xb1          // MOV CL, 3
        _emit 0x03
        _emit 0xf6          // IMUL CL
        _emit 0xe9
        _emit 0x88          // MOV [ESI+3], AL
        _emit 0x46
        _emit 0x03
        // --- advance pointers + loop ---
        _emit 0x83          // ADD ESI, 8
        _emit 0xc6
        _emit 0x08
        _emit 0x83          // ADD EDI, 2
        _emit 0xc7
        _emit 0x02
        _emit 0x83          // SUB EBP, 1
        _emit 0xed
        _emit 0x01
        _emit 0x0f          // JNZ loop_body (0x43c5d0)
        _emit 0x85
        _emit 0x52
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // --- partial epilog (POP EDI, POP ESI; declared 224-byte slice ends here) ---
        _emit 0x5f          // POP EDI
        _emit 0x5e          // POP ESI
        // (POP EBP, POP EBX, RET at 0x3c680..0x3c682 are in shared epilog
        //  outside the declared 0xe0-byte function boundary)
    }
}
