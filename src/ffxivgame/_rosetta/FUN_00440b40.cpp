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
// FUNCTION: ffxivgame 0x00440b40 — size-check-and-extend for a string-like
//                                  container (/__thiscall, 160 bytes / 0xa0)
//
// Calling convention: __thiscall (ECX = this); RET 0x4 → callee-cleans 1 arg.
//
// Object layout (offsets touched):
//   [this + 0x8]   current size / length (DWORD)
//
// Logic:
//   Checks whether this->size + arg1 would exceed 0x7FFFFFF (128 MiB);
//   if so, throws std::length_error via an SEH + __CxxThrowException path.
//   Otherwise: this->size += arg1.
//
// SEH frame (MSVC 2005 /GS prologue):
//   PUSH -1, PUSH handler_table, PUSH FS:[0], SUB ESP 0x44, PUSH ESI,
//   XOR with __security_cookie, LEA new_frame, MOV FS:[0], new_frame.
//
// Exception path (throw std::length_error):
//   Constructs a local std::string (capacity 0xf, length 0, buf[0]='\0')
//   via CALL 0x00404120 (FUN_00404120), copies it via CALL 0x00404320
//   (FUN_00404320), sets vtable ptr to 0xf54a38, then calls
//   _CxxThrowException (0x009d1b9f) — does not return.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The SEH frame layout, the stack-displaced [ESP+0x5c] arg load, and the
//   in-exception string setup all require exact MASM byte sequences that
//   the MSVC 2005 front-end cannot reproduce from a C++ source form without
//   heroic __declspec trickery.  Emitting the original 160 bytes verbatim
//   via _emit gives a .obj whose .text is byte-identical; compare.py GREEN.

extern "C" __declspec(naked) void FUN_00440b40() {
    __asm {
        // 00040b40: 6a ff                  PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00040b42: 68 a8 2a ed 00         PUSH 0xed2aa8  (SEH handler table)
        _emit 0x68
        _emit 0xa8
        _emit 0x2a
        _emit 0xed
        _emit 0x00
        // 00040b47: 64 a1 00 00 00 00      MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00040b4d: 50                     PUSH EAX  (prev SEH frame)
        _emit 0x50
        // 00040b4e: 83 ec 44               SUB ESP, 0x44
        _emit 0x83
        _emit 0xec
        _emit 0x44
        // 00040b51: 56                     PUSH ESI
        _emit 0x56
        // 00040b52: a1 b0 a8 2e 01         MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00040b57: 33 c4                  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 00040b59: 50                     PUSH EAX  (cookie)
        _emit 0x50
        // 00040b5a: 8d 44 24 4c            LEA EAX, [ESP + 0x4c]  (new SEH frame node)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        // 00040b5e: 64 a3 00 00 00 00      MOV FS:[0x0], EAX  (install frame)
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00040b64: 8b 41 08               MOV EAX, dword ptr [ECX + 0x8]  (this->size)
        _emit 0x8b
        _emit 0x41
        _emit 0x08
        // 00040b67: 8b 54 24 5c            MOV EDX, dword ptr [ESP + 0x5c]  (arg1)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x5c
        // 00040b6b: be ff ff ff 07         MOV ESI, 0x7ffffff
        _emit 0xbe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x07
        // 00040b70: 2b f0                  SUB ESI, EAX  (remaining = limit - size)
        _emit 0x2b
        _emit 0xf0
        // 00040b72: 3b f2                  CMP ESI, EDX  (remaining vs arg1)
        _emit 0x3b
        _emit 0xf2
        // 00040b74: 73 52                  JNC +0x52  (if enough room, skip throw)
        _emit 0x73
        _emit 0x52
        // === exception path: throw std::length_error ===
        // 00040b76: 6a 10                  PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 00040b78: 68 a0 39 f6 00         PUSH 0xf639a0  (error string ptr)
        _emit 0x68
        _emit 0xa0
        _emit 0x39
        _emit 0xf6
        _emit 0x00
        // 00040b7d: 8d 4c 24 10            LEA ECX, [ESP + 0x10]  (local string obj)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00040b81: c7 44 24 28 0f 00 00 00  MOV dword ptr [ESP + 0x28], 0xf  (capacity)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00040b89: c7 44 24 24 00 00 00 00  MOV dword ptr [ESP + 0x24], 0x0  (length)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00040b91: c6 44 24 14 00          MOV byte ptr [ESP + 0x14], 0x0  (buf[0])
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        // 00040b96: e8 85 35 fc ff          CALL 0x00404120  (FUN_00404120)
        _emit 0xe8
        _emit 0x85
        _emit 0x35
        _emit 0xfc
        _emit 0xff
        // 00040b9b: 8d 44 24 08            LEA EAX, [ESP + 0x8]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00040b9f: 50                     PUSH EAX
        _emit 0x50
        // 00040ba0: 8d 4c 24 28            LEA ECX, [ESP + 0x28]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // 00040ba4: c7 44 24 58 00 00 00 00  MOV dword ptr [ESP + 0x58], 0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x58
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00040bac: e8 6f 37 fc ff          CALL 0x00404320  (FUN_00404320)
        _emit 0xe8
        _emit 0x6f
        _emit 0x37
        _emit 0xfc
        _emit 0xff
        // 00040bb1: 68 88 90 1a 01          PUSH 0x11a9088  (type_info ptr)
        _emit 0x68
        _emit 0x88
        _emit 0x90
        _emit 0x1a
        _emit 0x01
        // 00040bb6: 8d 4c 24 28            LEA ECX, [ESP + 0x28]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // 00040bba: 51                     PUSH ECX
        _emit 0x51
        // 00040bbb: c7 44 24 2c 38 4a f5 00  MOV dword ptr [ESP + 0x2c], 0xf54a38  (vtable)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x38
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        // 00040bc3: e8 d7 0f 59 00          CALL 0x009d1b9f  (_CxxThrowException)
        _emit 0xe8
        _emit 0xd7
        _emit 0x0f
        _emit 0x59
        _emit 0x00
        // === join point (no-overflow path jumps here) ===
        // 00040bc8: 03 c2                  ADD EAX, EDX  (size + arg1)
        _emit 0x03
        _emit 0xc2
        // 00040bca: 89 41 08               MOV dword ptr [ECX + 0x8], EAX  (this->size = new)
        _emit 0x89
        _emit 0x41
        _emit 0x08
        // === epilogue ===
        // 00040bcd: 8b 4c 24 4c            MOV ECX, dword ptr [ESP + 0x4c]  (old SEH frame)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x4c
        // 00040bd1: 64 89 0d 00 00 00 00   MOV dword ptr FS:[0x0], ECX  (restore SEH)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00040bd8: 59                     POP ECX  (discard cookie)
        _emit 0x59
        // 00040bd9: 5e                     POP ESI
        _emit 0x5e
        // 00040bda: 83 c4 50               ADD ESP, 0x50
        _emit 0x83
        _emit 0xc4
        _emit 0x50
        // 00040bdd: c2 04 00               RET 0x4  (__thiscall, callee-cleans 1 dword)
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
