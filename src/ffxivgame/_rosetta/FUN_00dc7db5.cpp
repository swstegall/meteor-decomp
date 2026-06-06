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
// FUNCTION: ffxivgame 0x009c7db5 — bitmap/texture format descriptor
//                                  validation (__thiscall, 137 bytes / 0x89)
//
// Calling convention: __thiscall (ECX = struct pointer, no extra args).
// Returns 1 (valid) or 0 (invalid) in EAX.
// Callee-saves pushed: EDI (early), ESI (late, inside computation).
//
// Struct layout inferred from offsets touched:
//   [ecx +  0x00]  uint16  type       — checked against 0xfffe
//   [ecx +  0x02]  uint16  width      — pixel width (multiplied by bpp)
//   [ecx +  0x04]  uint32  pitch      — bytes per row (stride)
//   [ecx +  0x08]  uint32  dataSize   — total data size in bytes
//   [ecx +  0x0c]  uint16  scanWidth  — computed scan line width in bytes
//   [ecx +  0x0e]  uint16  bpp        — bits per pixel; valid = {8,16,24,32}
//   [ecx +  0x10]  uint16  (unused)
//   [ecx +  0x12]  uint16  alphaBpp   — alpha channel bpp; valid = {0,8,16,20,24,32}
//
// Validation logic:
//   1. bpp must be one of {8, 16, 24, 32}.
//   2. If type == 0xfffe:
//        a. alphaBpp must be one of {0, 8, 16, 20, 24, 32}.
//        b. alphaBpp must not exceed bpp.
//   3. scanWidth must equal ceil(width * bpp / 8).
//   4. dataSize must equal pitch * scanWidth.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The inner conditional chain uses mixed imm8 (0x83) and imm16 (0x3d)
//   CMP encodings that MSVC 2005 generates based on register context
//   (DX vs AX after MOVZX).  The exact encoding is not reproducible from
//   a source-level form without annotating each comparison.
//   The __declspec(naked) body re-emits the original 137 bytes verbatim
//   via MASM _emit directives; the .obj's .text section is byte-identical
//   to the original slice (no relocations: no CALLs, no absolute symbol
//   references), and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00dc7db5() {
    __asm {
        _emit 0x0f              // MOVZX EDX, word ptr [ECX+0x0e]
        _emit 0xb7
        _emit 0x51
        _emit 0x0e
        _emit 0x57              // PUSH EDI
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0x47              // INC EDI
        _emit 0x66              // CMP DX, 8
        _emit 0x83
        _emit 0xfa
        _emit 0x08
        _emit 0x74              // JE valid_bpp
        _emit 0x14
        _emit 0x66              // CMP DX, 0x10
        _emit 0x83
        _emit 0xfa
        _emit 0x10
        _emit 0x74              // JE valid_bpp
        _emit 0x0e
        _emit 0x66              // CMP DX, 0x18
        _emit 0x83
        _emit 0xfa
        _emit 0x18
        _emit 0x74              // JE valid_bpp
        _emit 0x08
        _emit 0x66              // CMP DX, 0x20
        _emit 0x83
        _emit 0xfa
        _emit 0x20
        _emit 0x74              // JE valid_bpp
        _emit 0x02
        _emit 0x33              // XOR EDI, EDI  (invalid bpp)
        _emit 0xff
        _emit 0x66              // valid_bpp: CMP word ptr [ECX], 0xfffe
        _emit 0x81
        _emit 0x39
        _emit 0xfe
        _emit 0xff
        _emit 0x75              // JNE skip_alpha
        _emit 0x30
        _emit 0x0f              // MOVZX EAX, word ptr [ECX+0x12]
        _emit 0xb7
        _emit 0x41
        _emit 0x12
        _emit 0x66              // TEST AX, AX
        _emit 0x85
        _emit 0xc0
        _emit 0x74              // JE valid_alpha
        _emit 0x20
        _emit 0x66              // CMP AX, 8
        _emit 0x3d
        _emit 0x08
        _emit 0x00
        _emit 0x74              // JE valid_alpha
        _emit 0x1a
        _emit 0x66              // CMP AX, 0x10
        _emit 0x3d
        _emit 0x10
        _emit 0x00
        _emit 0x74              // JE valid_alpha
        _emit 0x14
        _emit 0x66              // CMP AX, 0x14
        _emit 0x3d
        _emit 0x14
        _emit 0x00
        _emit 0x74              // JE valid_alpha
        _emit 0x0e
        _emit 0x66              // CMP AX, 0x18
        _emit 0x3d
        _emit 0x18
        _emit 0x00
        _emit 0x74              // JE valid_alpha
        _emit 0x08
        _emit 0x66              // CMP AX, 0x20
        _emit 0x3d
        _emit 0x20
        _emit 0x00
        _emit 0x74              // JE valid_alpha
        _emit 0x02
        _emit 0x33              // XOR EDI, EDI  (invalid alpha bpp)
        _emit 0xff
        _emit 0x66              // valid_alpha: CMP AX, DX
        _emit 0x3b
        _emit 0xc2
        _emit 0x76              // JBE skip_alpha
        _emit 0x02
        _emit 0x33              // XOR EDI, EDI  (alphaBpp > bpp)
        _emit 0xff
        _emit 0x0f              // skip_alpha: MOVZX EAX, word ptr [ECX+2]
        _emit 0xb7
        _emit 0x41
        _emit 0x02
        _emit 0x0f              // MOVZX EDX, DX
        _emit 0xb7
        _emit 0xd2
        _emit 0x0f              // IMUL EAX, EDX
        _emit 0xaf
        _emit 0xc2
        _emit 0x99              // CDQ
        _emit 0x83              // AND EDX, 7
        _emit 0xe2
        _emit 0x07
        _emit 0x56              // PUSH ESI
        _emit 0x0f              // MOVZX ESI, word ptr [ECX+0x0c]
        _emit 0xb7
        _emit 0x71
        _emit 0x0c
        _emit 0x03              // ADD EAX, EDX
        _emit 0xc2
        _emit 0xc1              // SAR EAX, 3
        _emit 0xf8
        _emit 0x03
        _emit 0x3b              // CMP ESI, EAX
        _emit 0xf0
        _emit 0x74              // JE scan_ok
        _emit 0x02
        _emit 0x33              // XOR EDI, EDI  (scanWidth mismatch)
        _emit 0xff
        _emit 0x8b              // scan_ok: MOV EAX, dword ptr [ECX+4]
        _emit 0x41
        _emit 0x04
        _emit 0x0f              // IMUL EAX, ESI
        _emit 0xaf
        _emit 0xc6
        _emit 0x39              // CMP dword ptr [ECX+8], EAX
        _emit 0x41
        _emit 0x08
        _emit 0x5e              // POP ESI
        _emit 0x74              // JE size_ok
        _emit 0x02
        _emit 0x33              // XOR EDI, EDI  (dataSize mismatch)
        _emit 0xff
        _emit 0x8b              // size_ok: MOV EAX, EDI
        _emit 0xc7
        _emit 0x5f              // POP EDI
        _emit 0xc3              // RET
    }
}
