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
// FUNCTION: ffxivgame 0x009e69b5 — __cdecl FPU control-word setter (23 B)
//
// Asm (23 bytes @ orig RVA 0x005e69b5 / VA 0x009e69b5):
//   8b 54 24 04         MOV  EDX, dword ptr [ESP+4]    ; load first arg
//   81 e2 00 03 00 00   AND  EDX, 0x300                ; keep bits 8-9 (precision control)
//   83 ca 7f            OR   EDX, 0x7f                 ; mask all FP exceptions, preserve PC
//   66 89 54 24 06      MOV  word ptr [ESP+6], DX      ; write 16-bit CW to high word of arg slot
//   d9 6c 24 06         FLDCW word ptr [ESP+6]         ; load FPU control word
//   c3                  RET                             ; __cdecl (caller cleans up)
//
// Calling convention: __cdecl (stack arg at [ESP+4], bare RET, no ECX usage).
// No prologue — /Oy frame-pointer omission; function is a leaf.
//
// This helper accepts a value whose bits 8-9 encode the x87 precision-control
// field (PC: 00=24-bit, 10=53-bit, 11=64-bit extended), masks to isolate only
// those two bits, ORs in 0x7f (sets exception masks IM/DM/ZM/OM/UM/PM and
// bit 6), then issues FLDCW to install the resulting word as the FPU control
// word. The high word of the caller's first-argument stack slot ([ESP+6]) is
// reused as the mandatory 16-bit memory operand for FLDCW — no separate stack
// frame is needed.

extern "C" __declspec(naked) void FUN_009e69b5() {
    __asm {
        _emit 0x8b              // MOV EDX, dword ptr [ESP+4]
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x81              // AND EDX, 0x00000300
        _emit 0xe2
        _emit 0x00
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x83              // OR EDX, 0x7f
        _emit 0xca
        _emit 0x7f
        _emit 0x66              // MOV word ptr [ESP+6], DX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x06
        _emit 0xd9              // FLDCW word ptr [ESP+6]
        _emit 0x6c
        _emit 0x24
        _emit 0x06
        _emit 0xc3              // RET
    }
}
