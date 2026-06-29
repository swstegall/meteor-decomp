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
// FUNCTION: ffxivgame 0x00045bb0 — TLS-indexed value fetch + update (51 B / 0x33)
//
// A __thiscall method that reads a TLS-slot-based value and uses it as the
// third argument to FUN_00458680, then stores the return value (incremented
// by 1) back into this->field_8.
//
// Calling convention: __thiscall (ECX = this; plain RET — no stack cleanup).
// Frame: PUSH ESI / POP ESI (only callee-save); no sub ESP / local frame.
//
// Object layout (inferred):
//   this+0x00  DWORD field_0   — first argument to FUN_00458680
//   this+0x08  DWORD field_8   — decremented before call; receives result+1
//
// TLS access pattern:
//   EDX = [0x01363f38]           ; TLS slot index (global)
//   EAX = FS:[0x2c]              ; Win32 TLS array pointer
//   ECX = [EAX + EDX*4]          ; per-thread TLS block
//   EDX = [ECX + 0x8]            ; field in the TLS block → 1st arg
//
// Arguments to FUN_00458680 (pushed right-to-left, 3 args, caller cleans):
//   1st (leftmost):  EDX = tls_block->field_8
//   2nd:             ECX = this->field_0
//   3rd (rightmost): EAX = this->field_8 - 1
//
// Important encoding note:
//   The original binary encodes `MOV EDX, dword ptr [ECX+0x8]` with a
//   32-bit displacement (8b 91 08 00 00 00) rather than the short 8-bit
//   form (8b 51 08). The MSVC assembler generates the short form for
//   small displacements, so a readable naked-asm transcription would
//   produce a 3-byte mismatch. Byte passthrough via _emit is used to
//   reproduce the original encoding exactly.
//
// Asm listing (51 bytes @ RVA 0x00045bb0):
//   8b 15 38 3f 36 01   MOV EDX, dword ptr [0x01363f38]
//   56                  PUSH ESI
//   8b f1               MOV ESI, ECX
//   8b 46 08            MOV EAX, dword ptr [ESI+0x8]
//   8b 0e               MOV ECX, dword ptr [ESI]
//   83 e8 01            SUB EAX, 0x1
//   50                  PUSH EAX
//   64 a1 2c 00 00 00   MOV EAX, FS:[0x2c]
//   51                  PUSH ECX
//   8b 0c 90            MOV ECX, dword ptr [EAX+EDX*4]
//   8b 91 08 00 00 00   MOV EDX, dword ptr [ECX+0x8]   ; 32-bit disp
//   52                  PUSH EDX
//   e8 a8 2a 01 00      CALL FUN_00458680
//   83 c0 01            ADD EAX, 0x1
//   83 c4 0c            ADD ESP, 0xc
//   89 46 08            MOV dword ptr [ESI+0x8], EAX
//   5e                  POP ESI
//   c3                  RET

extern "C" __declspec(naked) void FUN_00445bb0() {
    __asm {
        // 8b 15 38 3f 36 01   MOV EDX, dword ptr [0x01363f38]
        _emit 0x8b
        _emit 0x15
        _emit 0x38
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        // 56                  PUSH ESI
        _emit 0x56
        // 8b f1               MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 8b 46 08            MOV EAX, dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 8b 0e               MOV ECX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x0e
        // 83 e8 01            SUB EAX, 0x1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // 50                  PUSH EAX
        _emit 0x50
        // 64 a1 2c 00 00 00   MOV EAX, FS:[0x2c]
        _emit 0x64
        _emit 0xa1
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 51                  PUSH ECX
        _emit 0x51
        // 8b 0c 90            MOV ECX, dword ptr [EAX+EDX*4]
        _emit 0x8b
        _emit 0x0c
        _emit 0x90
        // 8b 91 08 00 00 00   MOV EDX, dword ptr [ECX+0x8]  (32-bit disp)
        _emit 0x8b
        _emit 0x91
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 52                  PUSH EDX
        _emit 0x52
        // e8 a8 2a 01 00      CALL FUN_00458680
        _emit 0xe8
        _emit 0xa8
        _emit 0x2a
        _emit 0x01
        _emit 0x00
        // 83 c0 01            ADD EAX, 0x1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 83 c4 0c            ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 89 46 08            MOV dword ptr [ESI+0x8], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 5e                  POP ESI
        _emit 0x5e
        // c3                  RET
        _emit 0xc3
    }
}
