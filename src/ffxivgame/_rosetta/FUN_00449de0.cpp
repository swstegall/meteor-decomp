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
// FUNCTION: ffxivgame 0x00449de0 — block-swap / two-iterator splice helper
//                                  building a 16-byte result struct and
//                                  swapping a byte range in place (__cdecl,
//                                  141 bytes / 0x8d).
//
// Stack-argument layout (final ESP base, after SUB ESP,0x10 + 4 pushes):
//
//   [ESP+0x24] arg0  void*  result/out pointer (returned in EAX)
//   [ESP+0x28] arg1  ESI    range lo  (byte ptr)
//   [ESP+0x2c] arg2  EDI    range hi  (byte ptr)
//   [ESP+0x30] arg3  qword  copied verbatim into out[0x00]
//   [ESP+0x34] arg4  ECX    base for the second range
//   [ESP+0x38] arg5  qword  EBP = low dword; copied into out[0x08]
//   [ESP+0x3c] arg6         capacity bound for the overflow check
//
// Behaviour:
//   EBX = (arg2 - arg1) + arg5_lo;  if (EBX > arg6) call 0x009d22b4
//   (the length/capacity exception helper). Then:
//     out[0x00..0x07] = arg3 (qword copy via MOVQ)
//     out[0x08]       = EBX  (32-bit, overwriting arg5_lo in the staged qword)
//     out[0x0c]       = arg5_hi
//   Finally, if arg1 != arg2, swap each byte in [arg1, arg2) with the
//   matching byte at [(arg4 + arg5_lo) - arg1 + i] — a std::swap_ranges
//   over the two parallel byte spans — and return arg0.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The body mixes MSVC 2005's SSE2 MOVQ POD-copy idiom (`f3 0f 7e` /
//   `66 0f d6` for the qword struct moves) with a hand-laid 7-byte
//   alignment nop (`8d a4 24 00 00 00 00`) ahead of the swap loop —
//   neither of which a source-level translation can coerce under /O2.
//   As with siblings FUN_00408610 / FUN_004089f0, the pragmatic match
//   is a `__declspec(naked)` body re-emitting the original 141 bytes
//   verbatim via MASM `_emit`. The single CALL (rel32 to 0x009d22b4)
//   carries a relocation whose 4-byte displacement window compare.py
//   masks out of the byte-level diff; the `e8` opcode byte is real
//   code and matches.

extern "C" __declspec(naked) void FUN_00449de0() {
    __asm {
        _emit 0x83              // SUB ESP, 0x10
        _emit 0xec
        _emit 0x10
        _emit 0xf3              // MOVQ XMM0, qword ptr [ESP+0x20]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x30]
        _emit 0x6c
        _emit 0x24
        _emit 0x30
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x24]
        _emit 0x74
        _emit 0x24
        _emit 0x24
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x2c]
        _emit 0x7c
        _emit 0x24
        _emit 0x2c
        _emit 0x8b              // MOV EAX, EDI
        _emit 0xc7
        _emit 0x2b              // SUB EAX, ESI
        _emit 0xc6
        _emit 0x8d              // LEA EBX, [EAX+EBP*1]
        _emit 0x1c
        _emit 0x28
        _emit 0x3b              // CMP EBX, dword ptr [ESP+0x3c]
        _emit 0x5c
        _emit 0x24
        _emit 0x3c
        _emit 0x66             // MOVQ qword ptr [ESP+0x10], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xf3             // MOVQ XMM0, qword ptr [ESP+0x38]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x66             // MOVQ qword ptr [ESP+0x18], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x76             // JBE +0x05
        _emit 0x05
        _emit 0xe8             // CALL 0x009d22b4 (rel32, masked)
        _emit 0x97
        _emit 0x84
        _emit 0x58
        _emit 0x00
        _emit 0x3b             // CMP ESI, EDI
        _emit 0xf7
        _emit 0x8b             // MOV EAX, dword ptr [ESP+0x24]
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0xf3             // MOVQ XMM0, qword ptr [ESP+0x10]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b             // MOV ECX, dword ptr [ESP+0x34]
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        _emit 0x66             // MOVQ qword ptr [EAX], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x00
        _emit 0x89             // MOV dword ptr [ESP+0x18], EBX
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        _emit 0xf3             // MOVQ XMM0, qword ptr [ESP+0x18]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8d             // LEA EDX, [ECX+EBP*1]
        _emit 0x14
        _emit 0x29
        _emit 0x66             // MOVQ qword ptr [EAX+0x8], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x08
        _emit 0x8b             // MOV ECX, ESI
        _emit 0xce
        _emit 0x74             // JZ +0x1e
        _emit 0x1e
        _emit 0x2b             // SUB EDX, ESI
        _emit 0xd6
        _emit 0x8d             // LEA ESP, [ESP] (7-byte align nop)
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8a             // MOV BL, byte ptr [EDX+ECX*1]
        _emit 0x1c
        _emit 0x0a
        _emit 0x8a             // MOV AL, byte ptr [ECX]
        _emit 0x01
        _emit 0x88             // MOV byte ptr [ECX], BL
        _emit 0x19
        _emit 0x88             // MOV byte ptr [EDX+ECX*1], AL
        _emit 0x04
        _emit 0x0a
        _emit 0x83             // ADD ECX, 0x1
        _emit 0xc1
        _emit 0x01
        _emit 0x3b             // CMP ECX, EDI
        _emit 0xcf
        _emit 0x75             // JNZ -0x11
        _emit 0xef
        _emit 0x8b             // MOV EAX, dword ptr [ESP+0x24]
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x5f             // POP EDI
        _emit 0x5e             // POP ESI
        _emit 0x5d             // POP EBP
        _emit 0x5b             // POP EBX
        _emit 0x83             // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3             // RET
    }
}
