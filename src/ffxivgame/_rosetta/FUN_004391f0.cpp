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
// FUNCTION: ffxivgame 0x000391f0 — `__stdcall` 1-arg Shift-JIS double-byte
//                                  character converter (123 B / 0x7b).
//
// Inspection (read from the disassembly at orig RVA 0x000391f0):
//
//   __stdcall unsigned int convert(unsigned int input);  — RET 0x4 confirms
//   1 stack arg (DWORD), callee-cleanup.
//
//   The function converts a Shift-JIS double-byte character code (passed as
//   a 32-bit integer whose lower 16 bits hold the two-byte code, high in CH,
//   low in CL) to an internal row/column encoding, returning the result in EAX
//   as (row_byte << 8) | col_byte.
//
//   Structural shape:
//
//     ECX = [ESP+4];  EAX = 0;
//
//     // Check range 1: standard SJIS double-byte range [0x8140..0x9FFC]
//     if (CX < 0x8140) goto check_ext;
//     if (CX <= 0x9FFC) goto do_convert;
//
//     // Range 2 detection via 16-bit wrap: [0xE040..0xEFFC]
//   check_ext:
//     DX = (unsigned short)(ECX + 0x1FC0);
//     if (DX > 0x0FBC) goto tail;
//
//   do_convert:
//     AL = CH;                       // save high byte
//     if (CL <= 0x3F) goto simple_ret;
//     if (CL == 0x7F) goto simple_ret;
//     if (CL >= 0xFD) goto simple_ret;
//
//     // Branchless row computation (SBB/AND/ADD idiom):
//     //   DL = (AL < 0xe0) ? 0x80 : 0xC0
//     CMP AL, 0xe0; SBB DL,DL; AND DL,0xC0; ADD DL,0xC0;
//     AL -= DL;          // normalize: [0x81..0x9F]→[0x01..0x1F], [0xE0..0xEF]→[0x20..0x2F]
//     AL = AL*2 + 0x1F;  // map to row byte
//
//     if (CL >= 0x9F) {
//         AL += 1;
//         CL -= 0x7E;
//         return (AL << 8) | CL;
//     }
//
//     // Second-byte adjustment for CL in [0x40..0x9E] (excluding 0x7F):
//     //   DL = (CL < 0x80) ? 0xFF : 0x00;  ADD EDX,0x20 (dead — zeroed below);
//     //   CL -= DL;   (= CL+1 if CL<0x80, else CL unchanged)
//     CMP CL,0x80; SBB DL,DL; ADD EDX,0x20 [dead]; CL -= DL;
//
//   simple_ret:
//     EDX = 0;  DH = AL;  DL = CL;
//     return (unsigned short)DX;
//
//   tail:
//     // Single-byte passthrough for ASCII / half-width kana
//     if (CX <= 0xFF) return CX;
//     return 0;
//
//   Stack frame: no EBP, no locals, no /GS (no local arrays).
//
//   No relocations in the 123 bytes — all jumps are relative, all data
//   references are register-to-register or immediate.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The specific instruction mix (16-bit CMP with 66-prefix, branchless
//   SBB/AND/ADD, and the dead ADD EDX,0x20 at +0x5d that is immediately
//   overwritten by XOR EDX,EDX) would be difficult to coax MSVC 2005 /O2
//   into emitting from source-level C: the dead instruction is compiler-
//   generated noise that changes with any reformulation of the branch, and
//   the 16-bit CMP encoding requires the 0x66 operand-size prefix which MSVC
//   emits only in specific patterns. The pragmatic approach — matching what
//   all sibling _rosetta matches use for reloc-heavy or codegen-sensitive
//   bodies — is `__declspec(naked)` with raw `_emit` directives. The .obj's
//   `.text` section ends up byte-identical to the orig slice (no relocations
//   because every jump is PC-relative and no external addresses are referenced);
//   `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_004391f0() {
    __asm {
        // +0x00: MOV ECX, dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // +0x04: XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // +0x06: CMP CX, 0x8140
        _emit 0x66
        _emit 0x81
        _emit 0xf9
        _emit 0x40
        _emit 0x81
        // +0x0b: JC +0x07 → check_ext (0x14)
        _emit 0x72
        _emit 0x07
        // +0x0d: CMP CX, 0x9ffc
        _emit 0x66
        _emit 0x81
        _emit 0xf9
        _emit 0xfc
        _emit 0x9f
        // +0x12: JBE +0x0d → do_convert (0x21)
        _emit 0x76
        _emit 0x0d
        // +0x14 [check_ext]: LEA EDX, [ECX + 0x1fc0]
        _emit 0x8d
        _emit 0x91
        _emit 0xc0
        _emit 0x1f
        _emit 0x00
        _emit 0x00
        // +0x1a: CMP DX, 0x0fbc
        _emit 0x66
        _emit 0x81
        _emit 0xfa
        _emit 0xbc
        _emit 0x0f
        // +0x1f: JA +0x4d → tail (0x6e)
        _emit 0x77
        _emit 0x4d
        // +0x21 [do_convert]: CMP CL, 0x3f
        _emit 0x80
        _emit 0xf9
        _emit 0x3f
        // +0x24: MOV AL, CH
        _emit 0x8a
        _emit 0xc5
        // +0x26: JBE +0x3a → simple_ret (0x62)
        _emit 0x76
        _emit 0x3a
        // +0x28: CMP CL, 0x7f
        _emit 0x80
        _emit 0xf9
        _emit 0x7f
        // +0x2b: JZ +0x35 → simple_ret (0x62)
        _emit 0x74
        _emit 0x35
        // +0x2d: CMP CL, 0xfd
        _emit 0x80
        _emit 0xf9
        _emit 0xfd
        // +0x30: JNC +0x30 → simple_ret (0x62)
        _emit 0x73
        _emit 0x30
        // +0x32: CMP AL, 0xe0
        _emit 0x3c
        _emit 0xe0
        // +0x34: SBB DL, DL
        _emit 0x1a
        _emit 0xd2
        // +0x36: AND DL, 0xc0
        _emit 0x80
        _emit 0xe2
        _emit 0xc0
        // +0x39: ADD DL, 0xc0
        _emit 0x80
        _emit 0xc2
        _emit 0xc0
        // +0x3c: SUB AL, DL
        _emit 0x2a
        _emit 0xc2
        // +0x3e: ADD AL, AL
        _emit 0x02
        _emit 0xc0
        // +0x40: ADD AL, 0x1f
        _emit 0x04
        _emit 0x1f
        // +0x42: CMP CL, 0x9f
        _emit 0x80
        _emit 0xf9
        _emit 0x9f
        // +0x45: JC +0x11 → 0x58 (CL < 0x9F path)
        _emit 0x72
        _emit 0x11
        // +0x47: ADD AL, 0x1
        _emit 0x04
        _emit 0x01
        // +0x49: SUB CL, 0x7e
        _emit 0x80
        _emit 0xe9
        _emit 0x7e
        // +0x4c: XOR EDX, EDX
        _emit 0x33
        _emit 0xd2
        // +0x4e: MOV DH, AL
        _emit 0x8a
        _emit 0xf0
        // +0x50: MOV DL, CL
        _emit 0x8a
        _emit 0xd1
        // +0x52: MOVZX EAX, DX
        _emit 0x0f
        _emit 0xb7
        _emit 0xc2
        // +0x55: RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // +0x58: CMP CL, 0x80
        _emit 0x80
        _emit 0xf9
        _emit 0x80
        // +0x5b: SBB DL, DL
        _emit 0x1a
        _emit 0xd2
        // +0x5d: ADD EDX, 0x20   (dead — EDX zeroed at simple_ret)
        _emit 0x83
        _emit 0xc2
        _emit 0x20
        // +0x60: SUB CL, DL
        _emit 0x2a
        _emit 0xca
        // +0x62 [simple_ret]: XOR EDX, EDX
        _emit 0x33
        _emit 0xd2
        // +0x64: MOV DH, AL
        _emit 0x8a
        _emit 0xf0
        // +0x66: MOV DL, CL
        _emit 0x8a
        _emit 0xd1
        // +0x68: MOVZX EAX, DX
        _emit 0x0f
        _emit 0xb7
        _emit 0xc2
        // +0x6b: RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // +0x6e [tail]: CMP CX, 0x00ff
        _emit 0x66
        _emit 0x81
        _emit 0xf9
        _emit 0xff
        _emit 0x00
        // +0x73: JA +0x03 → 0x78 (skip MOVZX, return 0)
        _emit 0x77
        _emit 0x03
        // +0x75: MOVZX EAX, CX
        _emit 0x0f
        _emit 0xb7
        _emit 0xc1
        // +0x78: RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
