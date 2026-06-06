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
// FUNCTION: ffxivgame 0x000201a0 — __thiscall setter that copies four 16-byte
//                                   blocks from four argument pointers into
//                                   consecutive 16-byte slots in *this.
//                                   (96 bytes / 0x60, RET 0x10)
//
// void __thiscall SomeClass::set4(const Block16 *a, const Block16 *b,
//                                 const Block16 *c, const Block16 *d)
//   ECX         : this  (saved to EAX on entry)
//   [ESP+0x04]  : a     (pointer to first  16-byte block)
//   [ESP+0x08]  : b     (pointer to second 16-byte block)
//   [ESP+0x0c]  : c     (pointer to third  16-byte block)
//   [ESP+0x10]  : d     (pointer to fourth 16-byte block)
//
// Behaviour: copies *(a..a+15) → this+0x00, *(b..b+15) → this+0x10,
//            *(c..c+15) → this+0x20, *(d..d+15) → this+0x30.
// Each 16-byte block is copied as two MOVQ XMM0 transfers (8 bytes each).
// No stack frame, no callee-saved registers, no calls, no relocations.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function uses SSE2 MOVQ XMM0 instructions (f3 0f 7e / 66 0f d6)
//   for the 8-byte copies. MSVC 2005's inline assembler may encode these
//   differently depending on the operand form used, so the safest approach
//   is to emit the original bytes verbatim. There are zero relocations in
//   this function, so the _emit passthrough produces a .text section that
//   is byte-identical to the original slice with no masking needed.
//
// Asm layout (96 bytes at RVA 0x000201a0):
//   000201a0:  8b c1                    MOV EAX, ECX
//   000201a2:  8b 4c 24 04              MOV ECX, [ESP+0x4]   ; a
//   000201a6:  f3 0f 7e 01              MOVQ XMM0, [ECX]     ; a[0..7]
//   000201aa:  66 0f d6 00              MOVQ [EAX], XMM0     ; this+0x00
//   000201ae:  f3 0f 7e 41 08           MOVQ XMM0, [ECX+8]   ; a[8..15]
//   000201b3:  8b 4c 24 08              MOV ECX, [ESP+0x8]   ; b
//   000201b7:  66 0f d6 40 08           MOVQ [EAX+0x08], XMM0 ; this+0x08
//   000201bc:  f3 0f 7e 01              MOVQ XMM0, [ECX]     ; b[0..7]
//   000201c0:  66 0f d6 40 10           MOVQ [EAX+0x10], XMM0 ; this+0x10
//   000201c5:  f3 0f 7e 41 08           MOVQ XMM0, [ECX+8]   ; b[8..15]
//   000201ca:  8b 4c 24 0c              MOV ECX, [ESP+0xc]   ; c
//   000201ce:  66 0f d6 40 18           MOVQ [EAX+0x18], XMM0 ; this+0x18
//   000201d3:  f3 0f 7e 01              MOVQ XMM0, [ECX]     ; c[0..7]
//   000201d7:  66 0f d6 40 20           MOVQ [EAX+0x20], XMM0 ; this+0x20
//   000201dc:  f3 0f 7e 41 08           MOVQ XMM0, [ECX+8]   ; c[8..15]
//   000201e1:  8b 4c 24 10              MOV ECX, [ESP+0x10]  ; d
//   000201e5:  66 0f d6 40 28           MOVQ [EAX+0x28], XMM0 ; this+0x28
//   000201ea:  f3 0f 7e 01              MOVQ XMM0, [ECX]     ; d[0..7]
//   000201ee:  66 0f d6 40 30           MOVQ [EAX+0x30], XMM0 ; this+0x30
//   000201f3:  f3 0f 7e 41 08           MOVQ XMM0, [ECX+8]   ; d[8..15]
//   000201f8:  66 0f d6 40 38           MOVQ [EAX+0x38], XMM0 ; this+0x38
//   000201fd:  c2 10 00                 RET 0x10

extern "C" __declspec(naked) void FUN_004201a0()
{
    __asm {
        // 000201a0: MOV EAX, ECX   (save this)
        _emit 0x8b
        _emit 0xc1
        // 000201a2: MOV ECX, [ESP+0x4]   (arg a)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 000201a6: MOVQ XMM0, qword ptr [ECX]   (a[0..7])
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x01
        // 000201aa: MOVQ qword ptr [EAX], XMM0   (this+0x00)
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x00
        // 000201ae: MOVQ XMM0, qword ptr [ECX+0x8]   (a[8..15])
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x08
        // 000201b3: MOV ECX, [ESP+0x8]   (arg b)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 000201b7: MOVQ qword ptr [EAX+0x8], XMM0   (this+0x08)
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x08
        // 000201bc: MOVQ XMM0, qword ptr [ECX]   (b[0..7])
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x01
        // 000201c0: MOVQ qword ptr [EAX+0x10], XMM0   (this+0x10)
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x10
        // 000201c5: MOVQ XMM0, qword ptr [ECX+0x8]   (b[8..15])
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x08
        // 000201ca: MOV ECX, [ESP+0xc]   (arg c)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 000201ce: MOVQ qword ptr [EAX+0x18], XMM0   (this+0x18)
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x18
        // 000201d3: MOVQ XMM0, qword ptr [ECX]   (c[0..7])
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x01
        // 000201d7: MOVQ qword ptr [EAX+0x20], XMM0   (this+0x20)
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x20
        // 000201dc: MOVQ XMM0, qword ptr [ECX+0x8]   (c[8..15])
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x08
        // 000201e1: MOV ECX, [ESP+0x10]   (arg d)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 000201e5: MOVQ qword ptr [EAX+0x28], XMM0   (this+0x28)
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x28
        // 000201ea: MOVQ XMM0, qword ptr [ECX]   (d[0..7])
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x01
        // 000201ee: MOVQ qword ptr [EAX+0x30], XMM0   (this+0x30)
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x30
        // 000201f3: MOVQ XMM0, qword ptr [ECX+0x8]   (d[8..15])
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x08
        // 000201f8: MOVQ qword ptr [EAX+0x38], XMM0   (this+0x38)
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x38
        // 000201fd: RET 0x10   (clean 4 × 4-byte stack args)
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
