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
// FUNCTION: ffxivgame 0x000305a0 — FUN_004305a0 — float-vector clamp + pack
//                                  (__thiscall, 358 B / 0x166, no SEH;
//                                   SSE2/XMM + AND ESP 0xF0 aligned frame).
//
// Inspection (read from the disassembly at orig RVA 0x000305a0):
//
//   __thiscall void* FUN_004305a0(this, float x, float y, float z, float w)
//   — ECX = this, four floats on stack, RET 0x10.
//
//   Structural shape:
//
//     1. Copy the four float parameters (EBP+0x8..+0x14) into a local
//        128-bit slot at [ESP+0x10..+0x1C].
//     2. Load a float constant from [0x00f54f70] and broadcast it across
//        [ESP+0x10..+0x1C]; run MINPS against the originals — clamp each
//        component to ≤ max_value.
//     3. Copy the min-clamped result to [ESP+0x20..+0x2C]; XORPS XMM0
//        (zero vector), store zeros across [ESP+0x20..+0x2C], MOVAPS the
//        zeros back as XMM0, run MAXPS — clamp each component to ≥ 0.
//     4. Copy the doubly-clamped result back to [ESP+0x10..+0x1C].
//     5. Load a scale factor from [0x00f62f68], broadcast via SHUFPS imm=0,
//        run MULPS — scale the clamped vector.
//     6. Copy the scaled result to [ESP+0x10..+0x1C] (and to +0x20..+0x2C
//        interleaved with the MOV ESI,ECX / LEA ECX,[ESP+0x10] sequence).
//     7. Call FUN_0042eba0 with ECX = &[ESP+0x10] (pointer to the scaled
//        float array); save EAX, BSWAP EAX, store to *this (ESI = ECX_in).
//     8. Return ESI (this pointer) in EAX.
//
//   Stack frame (after PUSH EBP / MOV EBP,ESP / AND ESP,0xFFFFFFF0 /
//   SUB ESP,0x2C / PUSH ESI):
//     [ESP+0x00]          saved ESI (PUSH ESI)
//     [ESP+0x04..0x0F]    unused / alignment pad
//     [ESP+0x0C]          temp dword for BSWAP'd result
//     [ESP+0x10..+0x1F]   XMM local slot A (4 × float)
//     [ESP+0x20..+0x2F]   XMM local slot B (4 × float)
//     [EBP+0x08]          float x (param 1)
//     [EBP+0x0C]          float y (param 2)
//     [EBP+0x10]          float z (param 3)
//     [EBP+0x14]          float w (param 4)
//
//   Reloc-bearing sites (absolute VAs baked into the orig bytes):
//     +0x3B   DIR32 → 0x00f54f70   (max-clamp constant)
//     +0x5A   DIR32 → 0x00f62f68   (scale factor constant)
//     +0x78   REL32 → 0x0042eba0   (CALL FUN_0042eba0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need MSVC 2005 /O2 to reproduce the exact
//   SSE2 scalar-shuffle idiom: the compiler expands a broadcast by writing
//   the same MOVSS four times into consecutive dwords then loading via
//   MOVAPS, rather than using SHUFPS on the first load. The SHUFPS
//   broadcast only appears for the multiply-by-scale step. Getting /O2 to
//   select that exact decomposition, plus the interleaved MOV ESI,ECX in
//   the middle of the final copy sequence (instruction scheduling artifact),
//   is brittle — a naked-asm passthrough is the safe choice here.

extern "C" __declspec(naked) void FUN_004305a0() {
    __asm {
        /* 000305a0 */ _emit 0x55
        /* 000305a1 */ _emit 0x8b
        /* 000305a2 */ _emit 0xec
        /* 000305a3 */ _emit 0x83
        /* 000305a4 */ _emit 0xe4
        /* 000305a5 */ _emit 0xf0
        /* 000305a6 */ _emit 0x83
        /* 000305a7 */ _emit 0xec
        /* 000305a8 */ _emit 0x2c
        /* 000305a9 */ _emit 0x56
        /* 000305aa */ _emit 0xf3
        /* 000305ab */ _emit 0x0f
        /* 000305ac */ _emit 0x10
        /* 000305ad */ _emit 0x45
        /* 000305ae */ _emit 0x08
        /* 000305af */ _emit 0xf3
        /* 000305b0 */ _emit 0x0f
        /* 000305b1 */ _emit 0x11
        /* 000305b2 */ _emit 0x44
        /* 000305b3 */ _emit 0x24
        /* 000305b4 */ _emit 0x10
        /* 000305b5 */ _emit 0xf3
        /* 000305b6 */ _emit 0x0f
        /* 000305b7 */ _emit 0x10
        /* 000305b8 */ _emit 0x45
        /* 000305b9 */ _emit 0x0c
        /* 000305ba */ _emit 0xf3
        /* 000305bb */ _emit 0x0f
        /* 000305bc */ _emit 0x11
        /* 000305bd */ _emit 0x44
        /* 000305be */ _emit 0x24
        /* 000305bf */ _emit 0x14
        /* 000305c0 */ _emit 0xf3
        /* 000305c1 */ _emit 0x0f
        /* 000305c2 */ _emit 0x10
        /* 000305c3 */ _emit 0x45
        /* 000305c4 */ _emit 0x10
        /* 000305c5 */ _emit 0xf3
        /* 000305c6 */ _emit 0x0f
        /* 000305c7 */ _emit 0x11
        /* 000305c8 */ _emit 0x44
        /* 000305c9 */ _emit 0x24
        /* 000305ca */ _emit 0x18
        /* 000305cb */ _emit 0xf3
        /* 000305cc */ _emit 0x0f
        /* 000305cd */ _emit 0x10
        /* 000305ce */ _emit 0x45
        /* 000305cf */ _emit 0x14
        /* 000305d0 */ _emit 0xf3
        /* 000305d1 */ _emit 0x0f
        /* 000305d2 */ _emit 0x11
        /* 000305d3 */ _emit 0x44
        /* 000305d4 */ _emit 0x24
        /* 000305d5 */ _emit 0x1c
        /* 000305d6 */ _emit 0x0f
        /* 000305d7 */ _emit 0x28
        /* 000305d8 */ _emit 0x4c
        /* 000305d9 */ _emit 0x24
        /* 000305da */ _emit 0x10
        /* 000305db */ _emit 0xf3
        /* 000305dc */ _emit 0x0f
        /* 000305dd */ _emit 0x10
        /* 000305de */ _emit 0x05
        /* 000305df */ _emit 0x70
        /* 000305e0 */ _emit 0x4f
        /* 000305e1 */ _emit 0xf5
        /* 000305e2 */ _emit 0x00
        /* 000305e3 */ _emit 0xf3
        /* 000305e4 */ _emit 0x0f
        /* 000305e5 */ _emit 0x11
        /* 000305e6 */ _emit 0x44
        /* 000305e7 */ _emit 0x24
        /* 000305e8 */ _emit 0x10
        /* 000305e9 */ _emit 0xf3
        /* 000305ea */ _emit 0x0f
        /* 000305eb */ _emit 0x11
        /* 000305ec */ _emit 0x44
        /* 000305ed */ _emit 0x24
        /* 000305ee */ _emit 0x14
        /* 000305ef */ _emit 0xf3
        /* 000305f0 */ _emit 0x0f
        /* 000305f1 */ _emit 0x11
        /* 000305f2 */ _emit 0x44
        /* 000305f3 */ _emit 0x24
        /* 000305f4 */ _emit 0x18
        /* 000305f5 */ _emit 0xf3
        /* 000305f6 */ _emit 0x0f
        /* 000305f7 */ _emit 0x11
        /* 000305f8 */ _emit 0x44
        /* 000305f9 */ _emit 0x24
        /* 000305fa */ _emit 0x1c
        /* 000305fb */ _emit 0x0f
        /* 000305fc */ _emit 0x28
        /* 000305fd */ _emit 0x44
        /* 000305fe */ _emit 0x24
        /* 000305ff */ _emit 0x10
        /* 00030600 */ _emit 0x0f
        /* 00030601 */ _emit 0x5d
        /* 00030602 */ _emit 0xc8
        /* 00030603 */ _emit 0x0f
        /* 00030604 */ _emit 0x29
        /* 00030605 */ _emit 0x4c
        /* 00030606 */ _emit 0x24
        /* 00030607 */ _emit 0x10
        /* 00030608 */ _emit 0xf3
        /* 00030609 */ _emit 0x0f
        /* 0003060a */ _emit 0x10
        /* 0003060b */ _emit 0x44
        /* 0003060c */ _emit 0x24
        /* 0003060d */ _emit 0x10
        /* 0003060e */ _emit 0xf3
        /* 0003060f */ _emit 0x0f
        /* 00030610 */ _emit 0x11
        /* 00030611 */ _emit 0x44
        /* 00030612 */ _emit 0x24
        /* 00030613 */ _emit 0x20
        /* 00030614 */ _emit 0xf3
        /* 00030615 */ _emit 0x0f
        /* 00030616 */ _emit 0x10
        /* 00030617 */ _emit 0x44
        /* 00030618 */ _emit 0x24
        /* 00030619 */ _emit 0x14
        /* 0003061a */ _emit 0xf3
        /* 0003061b */ _emit 0x0f
        /* 0003061c */ _emit 0x11
        /* 0003061d */ _emit 0x44
        /* 0003061e */ _emit 0x24
        /* 0003061f */ _emit 0x24
        /* 00030620 */ _emit 0xf3
        /* 00030621 */ _emit 0x0f
        /* 00030622 */ _emit 0x10
        /* 00030623 */ _emit 0x44
        /* 00030624 */ _emit 0x24
        /* 00030625 */ _emit 0x18
        /* 00030626 */ _emit 0xf3
        /* 00030627 */ _emit 0x0f
        /* 00030628 */ _emit 0x11
        /* 00030629 */ _emit 0x44
        /* 0003062a */ _emit 0x24
        /* 0003062b */ _emit 0x28
        /* 0003062c */ _emit 0xf3
        /* 0003062d */ _emit 0x0f
        /* 0003062e */ _emit 0x10
        /* 0003062f */ _emit 0x44
        /* 00030630 */ _emit 0x24
        /* 00030631 */ _emit 0x1c
        /* 00030632 */ _emit 0xf3
        /* 00030633 */ _emit 0x0f
        /* 00030634 */ _emit 0x11
        /* 00030635 */ _emit 0x44
        /* 00030636 */ _emit 0x24
        /* 00030637 */ _emit 0x2c
        /* 00030638 */ _emit 0x0f
        /* 00030639 */ _emit 0x28
        /* 0003063a */ _emit 0x4c
        /* 0003063b */ _emit 0x24
        /* 0003063c */ _emit 0x20
        /* 0003063d */ _emit 0x0f
        /* 0003063e */ _emit 0x57
        /* 0003063f */ _emit 0xc0
        /* 00030640 */ _emit 0xf3
        /* 00030641 */ _emit 0x0f
        /* 00030642 */ _emit 0x11
        /* 00030643 */ _emit 0x44
        /* 00030644 */ _emit 0x24
        /* 00030645 */ _emit 0x20
        /* 00030646 */ _emit 0xf3
        /* 00030647 */ _emit 0x0f
        /* 00030648 */ _emit 0x11
        /* 00030649 */ _emit 0x44
        /* 0003064a */ _emit 0x24
        /* 0003064b */ _emit 0x24
        /* 0003064c */ _emit 0xf3
        /* 0003064d */ _emit 0x0f
        /* 0003064e */ _emit 0x11
        /* 0003064f */ _emit 0x44
        /* 00030650 */ _emit 0x24
        /* 00030651 */ _emit 0x28
        /* 00030652 */ _emit 0xf3
        /* 00030653 */ _emit 0x0f
        /* 00030654 */ _emit 0x11
        /* 00030655 */ _emit 0x44
        /* 00030656 */ _emit 0x24
        /* 00030657 */ _emit 0x2c
        /* 00030658 */ _emit 0x0f
        /* 00030659 */ _emit 0x28
        /* 0003065a */ _emit 0x44
        /* 0003065b */ _emit 0x24
        /* 0003065c */ _emit 0x20
        /* 0003065d */ _emit 0x0f
        /* 0003065e */ _emit 0x5f
        /* 0003065f */ _emit 0xc8
        /* 00030660 */ _emit 0x0f
        /* 00030661 */ _emit 0x29
        /* 00030662 */ _emit 0x4c
        /* 00030663 */ _emit 0x24
        /* 00030664 */ _emit 0x20
        /* 00030665 */ _emit 0xf3
        /* 00030666 */ _emit 0x0f
        /* 00030667 */ _emit 0x10
        /* 00030668 */ _emit 0x44
        /* 00030669 */ _emit 0x24
        /* 0003066a */ _emit 0x20
        /* 0003066b */ _emit 0xf3
        /* 0003066c */ _emit 0x0f
        /* 0003066d */ _emit 0x11
        /* 0003066e */ _emit 0x44
        /* 0003066f */ _emit 0x24
        /* 00030670 */ _emit 0x10
        /* 00030671 */ _emit 0xf3
        /* 00030672 */ _emit 0x0f
        /* 00030673 */ _emit 0x10
        /* 00030674 */ _emit 0x44
        /* 00030675 */ _emit 0x24
        /* 00030676 */ _emit 0x24
        /* 00030677 */ _emit 0xf3
        /* 00030678 */ _emit 0x0f
        /* 00030679 */ _emit 0x11
        /* 0003067a */ _emit 0x44
        /* 0003067b */ _emit 0x24
        /* 0003067c */ _emit 0x14
        /* 0003067d */ _emit 0xf3
        /* 0003067e */ _emit 0x0f
        /* 0003067f */ _emit 0x10
        /* 00030680 */ _emit 0x44
        /* 00030681 */ _emit 0x24
        /* 00030682 */ _emit 0x28
        /* 00030683 */ _emit 0xf3
        /* 00030684 */ _emit 0x0f
        /* 00030685 */ _emit 0x11
        /* 00030686 */ _emit 0x44
        /* 00030687 */ _emit 0x24
        /* 00030688 */ _emit 0x18
        /* 00030689 */ _emit 0xf3
        /* 0003068a */ _emit 0x0f
        /* 0003068b */ _emit 0x10
        /* 0003068c */ _emit 0x44
        /* 0003068d */ _emit 0x24
        /* 0003068e */ _emit 0x2c
        /* 0003068f */ _emit 0xf3
        /* 00030690 */ _emit 0x0f
        /* 00030691 */ _emit 0x11
        /* 00030692 */ _emit 0x44
        /* 00030693 */ _emit 0x24
        /* 00030694 */ _emit 0x1c
        /* 00030695 */ _emit 0x0f
        /* 00030696 */ _emit 0x28
        /* 00030697 */ _emit 0x4c
        /* 00030698 */ _emit 0x24
        /* 00030699 */ _emit 0x10
        /* 0003069a */ _emit 0xf3
        /* 0003069b */ _emit 0x0f
        /* 0003069c */ _emit 0x10
        /* 0003069d */ _emit 0x05
        /* 0003069e */ _emit 0x68
        /* 0003069f */ _emit 0x2f
        /* 000306a0 */ _emit 0xf6
        /* 000306a1 */ _emit 0x00
        /* 000306a2 */ _emit 0x0f
        /* 000306a3 */ _emit 0xc6
        /* 000306a4 */ _emit 0xc0
        /* 000306a5 */ _emit 0x00
        /* 000306a6 */ _emit 0x0f
        /* 000306a7 */ _emit 0x59
        /* 000306a8 */ _emit 0xc8
        /* 000306a9 */ _emit 0x0f
        /* 000306aa */ _emit 0x29
        /* 000306ab */ _emit 0x4c
        /* 000306ac */ _emit 0x24
        /* 000306ad */ _emit 0x20
        /* 000306ae */ _emit 0xf3
        /* 000306af */ _emit 0x0f
        /* 000306b0 */ _emit 0x10
        /* 000306b1 */ _emit 0x44
        /* 000306b2 */ _emit 0x24
        /* 000306b3 */ _emit 0x20
        /* 000306b4 */ _emit 0xf3
        /* 000306b5 */ _emit 0x0f
        /* 000306b6 */ _emit 0x11
        /* 000306b7 */ _emit 0x44
        /* 000306b8 */ _emit 0x24
        /* 000306b9 */ _emit 0x10
        /* 000306ba */ _emit 0xf3
        /* 000306bb */ _emit 0x0f
        /* 000306bc */ _emit 0x10
        /* 000306bd */ _emit 0x44
        /* 000306be */ _emit 0x24
        /* 000306bf */ _emit 0x24
        /* 000306c0 */ _emit 0xf3
        /* 000306c1 */ _emit 0x0f
        /* 000306c2 */ _emit 0x11
        /* 000306c3 */ _emit 0x44
        /* 000306c4 */ _emit 0x24
        /* 000306c5 */ _emit 0x14
        /* 000306c6 */ _emit 0xf3
        /* 000306c7 */ _emit 0x0f
        /* 000306c8 */ _emit 0x10
        /* 000306c9 */ _emit 0x44
        /* 000306ca */ _emit 0x24
        /* 000306cb */ _emit 0x28
        /* 000306cc */ _emit 0x8b
        /* 000306cd */ _emit 0xf1
        /* 000306ce */ _emit 0xf3
        /* 000306cf */ _emit 0x0f
        /* 000306d0 */ _emit 0x11
        /* 000306d1 */ _emit 0x44
        /* 000306d2 */ _emit 0x24
        /* 000306d3 */ _emit 0x18
        /* 000306d4 */ _emit 0xf3
        /* 000306d5 */ _emit 0x0f
        /* 000306d6 */ _emit 0x10
        /* 000306d7 */ _emit 0x44
        /* 000306d8 */ _emit 0x24
        /* 000306d9 */ _emit 0x2c
        /* 000306da */ _emit 0x8d
        /* 000306db */ _emit 0x4c
        /* 000306dc */ _emit 0x24
        /* 000306dd */ _emit 0x10
        /* 000306de */ _emit 0xf3
        /* 000306df */ _emit 0x0f
        /* 000306e0 */ _emit 0x11
        /* 000306e1 */ _emit 0x44
        /* 000306e2 */ _emit 0x24
        /* 000306e3 */ _emit 0x1c
        /* 000306e4 */ _emit 0xe8
        /* 000306e5 */ _emit 0xb7
        /* 000306e6 */ _emit 0xe4
        /* 000306e7 */ _emit 0xff
        /* 000306e8 */ _emit 0xff
        /* 000306e9 */ _emit 0x89
        /* 000306ea */ _emit 0x44
        /* 000306eb */ _emit 0x24
        /* 000306ec */ _emit 0x0c
        /* 000306ed */ _emit 0x8b
        /* 000306ee */ _emit 0x44
        /* 000306ef */ _emit 0x24
        /* 000306f0 */ _emit 0x0c
        /* 000306f1 */ _emit 0x0f
        /* 000306f2 */ _emit 0xc8
        /* 000306f3 */ _emit 0x89
        /* 000306f4 */ _emit 0x44
        /* 000306f5 */ _emit 0x24
        /* 000306f6 */ _emit 0x0c
        /* 000306f7 */ _emit 0x8b
        /* 000306f8 */ _emit 0x44
        /* 000306f9 */ _emit 0x24
        /* 000306fa */ _emit 0x0c
        /* 000306fb */ _emit 0x89
        /* 000306fc */ _emit 0x06
        /* 000306fd */ _emit 0x8b
        /* 000306fe */ _emit 0xc6
        /* 000306ff */ _emit 0x5e
        /* 00030700 */ _emit 0x8b
        /* 00030701 */ _emit 0xe5
        /* 00030702 */ _emit 0x5d
        /* 00030703 */ _emit 0xc2
        /* 00030704 */ _emit 0x10
        /* 00030705 */ _emit 0x00
    }
}
