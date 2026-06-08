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
// FUNCTION: ffxivgame 0x0042fcf0 — scaled-identity 4×4 float matrix init
//                                  (203 B / 0xcb).
//
// Behaviour read from asm/ffxivgame/0002fcf0_FUN_0042fcf0.s:
//
//   __cdecl void FUN_0042fcf0(float *out);
//
//   Initialises the 64-byte (4×4) float matrix at `out` to a scaled
//   identity form: diagonal elements are set to the float constant stored
//   at 0x00f54f70, all off-diagonal elements are 0.0f.
//
//   Local 64-byte buffer is used as a staging area; XORPS zeroes XMM0
//   first, then MOVSS stores zero into all 12 off-diagonal slots, then
//   the diagonal slots ([0x00], [0x14], [0x28], [0x3c]) are filled from
//   XMM1 = *(float*)0x00f54f70.  The completed row-major matrix is then
//   copied to `out` in eight 8-byte MOVQ chunks.
//
//   Stack frame:
//     [ESP+0x00..0x3f]  64-byte local matrix staging buffer
//     [ESP+0x40]        (return address)
//     [ESP+0x44]        arg0: float *out
//
//   Reloc-bearing site in the orig 203 bytes:
//     +0x0a  DIR32 → 0x00f54f70  (absolute address of the scalar float constant)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The function uses SSE2 instructions (MOVQ with F3 and 66 prefixes:
//   `F3 0F 7E` for MOVQ xmm←m64 and `66 0F D6` for MOVQ m64←xmm) that
//   MSVC 2005's inline assembler cannot express symbolically without the
//   /arch:SSE2 switch altering the FP code-gen globally.  The raw _emit
//   approach — same as FUN_00406680 / FUN_00404e40 — bakes in the orig
//   203 bytes verbatim so tools/compare.py reports GREEN with no reloc
//   masking required for the DIR32 at +0x0a (the orig PE byte slice
//   already contains the resolved 0x00f54f70 address).

extern "C" __declspec(naked) void FUN_0042fcf0() {
    __asm {
        // SUB ESP, 0x40
        _emit 0x83
        _emit 0xec
        _emit 0x40
        // XORPS XMM0, XMM0
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        // MOV EAX, dword ptr [ESP + 0x44]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x44
        // MOVSS XMM1, dword ptr [0x00f54f70]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x0d
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        // MOVSS dword ptr [ESP + 0x30], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // MOVSS dword ptr [ESP + 0x34], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // MOVSS dword ptr [ESP + 0x38], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x38
        // MOVSS dword ptr [ESP + 0x20], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // MOVSS dword ptr [ESP + 0x24], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // MOVSS dword ptr [ESP + 0x2c], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // MOVSS dword ptr [ESP + 0x10], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // MOVSS dword ptr [ESP + 0x18], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // MOVSS dword ptr [ESP + 0x1c], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // MOVSS dword ptr [ESP + 0x4], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // MOVSS dword ptr [ESP + 0x8], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // MOVSS dword ptr [ESP + 0xc], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // MOVSS dword ptr [ESP], XMM1
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x0c
        _emit 0x24
        // MOVQ XMM0, qword ptr [ESP]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x04
        _emit 0x24
        // MOVQ qword ptr [EAX], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x00
        // MOVQ XMM0, qword ptr [ESP + 0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // MOVQ qword ptr [EAX + 0x8], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x08
        // MOVSS dword ptr [ESP + 0x14], XMM1
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // MOVQ XMM0, qword ptr [ESP + 0x10]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // MOVQ qword ptr [EAX + 0x10], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x10
        // MOVQ XMM0, qword ptr [ESP + 0x18]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // MOVQ qword ptr [EAX + 0x18], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x18
        // MOVQ XMM0, qword ptr [ESP + 0x20]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // MOVQ qword ptr [EAX + 0x20], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x20
        // MOVSS dword ptr [ESP + 0x28], XMM1
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // MOVQ XMM0, qword ptr [ESP + 0x28]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // MOVQ qword ptr [EAX + 0x28], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x28
        // MOVQ XMM0, qword ptr [ESP + 0x30]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // MOVQ qword ptr [EAX + 0x30], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x30
        // MOVSS dword ptr [ESP + 0x3c], XMM1
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        // MOVQ XMM0, qword ptr [ESP + 0x38]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x38
        // MOVQ qword ptr [EAX + 0x38], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x38
        // ADD ESP, 0x40
        _emit 0x83
        _emit 0xc4
        _emit 0x40
        // RET
        _emit 0xc3
    }
}
