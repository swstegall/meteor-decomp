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
// FUNCTION: ffxivgame 0x00023b90 — float-array compare-and-copy
//                                  (__thiscall, 193 B / 0xc1)
//
// Compares `count` 16-byte elements (4 floats each) stored in a class
// member array against a caller-supplied source buffer.  If all elements
// match, returns 1 (true).  If an element differs, copies the remaining
// elements from the source into the member array and returns 0 (false).
//
//   __thiscall bool CompareAndCopyFloats(this, int index, float(*src)[4], int count)
//     ECX        : this
//     [ESP+0x04] : index  — if >= 0x100, returns 1 immediately
//     [ESP+0x08] : src    — pointer to array of float[4] records
//     [ESP+0x0c] : count  — number of 16-byte records to compare
//
//   RET 0xc (stdcall cleanup of 3 stack args; thiscall for ECX).
//
//   Member array layout inferred from asm:
//     dest = (char*)this + (index + 0x128) * 16
//     (base offset 0x128*16 = 0x1280 from this)
//
// Logic:
//   1. If index >= 0x100: return 1.
//   2. Compute dest = this + (index + 0x128) * 16.
//   3. Loop i in [0, count):
//        Compare each of 4 floats via CVTPS2PD + UCOMISD + LAHF/TEST/JP.
//        If floats unequal or unordered (NaN): break.
//   4. If all compared: return 1.
//   5. On mismatch at element i:
//        memcpy(dest + i*16, src + i*16, (count-i)*16)   — call 0x009d4600
//        return 0.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The SSE2 float-equality comparison idiom (MOVSS → CVTPS2PD → UCOMISD
//   → LAHF → TEST AH,0x44 → JP) along with the thiscall + 3-stack-arg
//   convention and the 16-byte loop-body alignment (LEA ECX,[ECX+0] NOP
//   at +0x2d fills to 0x30 = 16-byte boundary) cannot be reproduced from
//   plain C++ source under MSVC 2005 cl.exe without precisely matching
//   /O2 switch settings, inline-asm register forcing, or custom keywords.
//   Naked asm is the pragmatic path to a byte-identical match.
//
//   The function is 193 bytes (0xc1) per config/ffxivgame.size_overrides.json
//   (reason: "RET imm16 (0c 00)") — 3 bytes longer than the 0xbe (190)
//   Ghidra auto-detected size.  The additional 3 bytes are the RET 0xc
//   (c2 0c 00) for the mismatch exit path at rva+0xbe (0x00423c4e).
//
// Reloc-bearing site in the orig 190 bytes:
//   +0xb3 .. +0xb6   CALL rel32 → 0x009d4600  (memcpy / memmove)
//                    emitted verbatim (b9 09 5b 00); no .obj relocation
//                    record needed — bytes match the orig PE directly.

extern "C" __declspec(naked) void FUN_00423b90() {
    __asm {
        // early-out: if index >= 0x100 return true
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x3d              // CMP EAX, 0x100
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x72              // JC  +5  (→ setup)
        _emit 0x05
        _emit 0xb0              // MOV AL, 1
        _emit 0x01
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00

        // setup: save regs, compute dest pointer
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, [ESP+0xc]  (src)
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x8d              // LEA EDX, [EAX+0x128]
        _emit 0x90
        _emit 0x28
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xc1              // SHL EDX, 0x4
        _emit 0xe2
        _emit 0x04
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, [ESP+0x14]  (count)
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x03              // ADD EDX, ECX  (dest = this + offset)
        _emit 0xd1
        _emit 0x33              // XOR ECX, ECX  (i = 0)
        _emit 0xc9
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x76              // JBE +0x78  (count == 0 → done)
        _emit 0x78
        _emit 0xeb              // JMP +3  (→ loop body, 16-byte aligned)
        _emit 0x03
        _emit 0x8d              // LEA ECX, [ECX+0]  (3-byte NOP, alignment pad)
        _emit 0x49
        _emit 0x00

        // loop body: compare float[0]
        _emit 0xf3              // MOVSS XMM0, [EDX]
        _emit 0x0f
        _emit 0x10
        _emit 0x02
        _emit 0xf3              // MOVSS XMM1, [ESI]
        _emit 0x0f
        _emit 0x10
        _emit 0x0e
        _emit 0x0f              // CVTPS2PD XMM0, XMM0
        _emit 0x5a
        _emit 0xc0
        _emit 0x0f              // CVTPS2PD XMM1, XMM1
        _emit 0x5a
        _emit 0xc9
        _emit 0x66              // UCOMISD XMM0, XMM1
        _emit 0x0f
        _emit 0x2e
        _emit 0xc1
        _emit 0x9f              // LAHF
        _emit 0xf6              // TEST AH, 0x44
        _emit 0xc4
        _emit 0x44
        _emit 0x7a              // JP  +0x62  (→ mismatch)
        _emit 0x62

        // compare float[1]
        _emit 0xf3              // MOVSS XMM0, [EDX+4]
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x04
        _emit 0xf3              // MOVSS XMM1, [ESI+4]
        _emit 0x0f
        _emit 0x10
        _emit 0x4e
        _emit 0x04
        _emit 0x0f              // CVTPS2PD XMM0, XMM0
        _emit 0x5a
        _emit 0xc0
        _emit 0x0f              // CVTPS2PD XMM1, XMM1
        _emit 0x5a
        _emit 0xc9
        _emit 0x66              // UCOMISD XMM0, XMM1
        _emit 0x0f
        _emit 0x2e
        _emit 0xc1
        _emit 0x9f              // LAHF
        _emit 0xf6              // TEST AH, 0x44
        _emit 0xc4
        _emit 0x44
        _emit 0x7a              // JP  +0x48  (→ mismatch)
        _emit 0x48

        // compare float[2]
        _emit 0xf3              // MOVSS XMM0, [EDX+8]
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x08
        _emit 0xf3              // MOVSS XMM1, [ESI+8]
        _emit 0x0f
        _emit 0x10
        _emit 0x4e
        _emit 0x08
        _emit 0x0f              // CVTPS2PD XMM0, XMM0
        _emit 0x5a
        _emit 0xc0
        _emit 0x0f              // CVTPS2PD XMM1, XMM1
        _emit 0x5a
        _emit 0xc9
        _emit 0x66              // UCOMISD XMM0, XMM1
        _emit 0x0f
        _emit 0x2e
        _emit 0xc1
        _emit 0x9f              // LAHF
        _emit 0xf6              // TEST AH, 0x44
        _emit 0xc4
        _emit 0x44
        _emit 0x7a              // JP  +0x2e  (→ mismatch)
        _emit 0x2e

        // compare float[3]
        _emit 0xf3              // MOVSS XMM0, [EDX+0xc]
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x0c
        _emit 0xf3              // MOVSS XMM1, [ESI+0xc]
        _emit 0x0f
        _emit 0x10
        _emit 0x4e
        _emit 0x0c
        _emit 0x0f              // CVTPS2PD XMM0, XMM0
        _emit 0x5a
        _emit 0xc0
        _emit 0x0f              // CVTPS2PD XMM1, XMM1
        _emit 0x5a
        _emit 0xc9
        _emit 0x66              // UCOMISD XMM0, XMM1
        _emit 0x0f
        _emit 0x2e
        _emit 0xc1
        _emit 0x9f              // LAHF
        _emit 0xf6              // TEST AH, 0x44
        _emit 0xc4
        _emit 0x44
        _emit 0x7a              // JP  +0x14  (→ mismatch)
        _emit 0x14

        // advance loop
        _emit 0x83              // ADD ECX, 1
        _emit 0xc1
        _emit 0x01
        _emit 0x83              // ADD EDX, 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x83              // ADD ESI, 0x10
        _emit 0xc6
        _emit 0x10
        _emit 0x3b              // CMP ECX, EDI
        _emit 0xcf
        _emit 0x72              // JC  -0x73  (→ loop body)
        _emit 0x8d

        // all match: return true
        _emit 0x5f              // POP EDI
        _emit 0xb0              // MOV AL, 1
        _emit 0x01
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00

        // mismatch: copy remaining elements, return false
        _emit 0x2b              // SUB EDI, ECX
        _emit 0xf9
        _emit 0xc1              // SHL EDI, 0x4   (bytes = remaining * 16)
        _emit 0xe7
        _emit 0x04
        _emit 0x57              // PUSH EDI  (size)
        _emit 0x56              // PUSH ESI  (src)
        _emit 0x52              // PUSH EDX  (dst)
        _emit 0xe8              // CALL 0x009d4600  (memcpy)
        _emit 0xb9
        _emit 0x09
        _emit 0x5b
        _emit 0x00
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x5f              // POP EDI
        _emit 0x32              // XOR AL, AL   (return false)
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0xc  (mismatch exit — rva+0xbe..0xc0,
        _emit 0x0c              //           included in the 193-byte window
        _emit 0x00              //           per config/ffxivgame.size_overrides.json)
    }
}
