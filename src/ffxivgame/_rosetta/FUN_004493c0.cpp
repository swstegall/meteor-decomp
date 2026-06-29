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
// FUNCTION: ffxivgame 0x004493c0 — wide-string SSO assign/fill member (109 B)
//
// __thiscall void StringClass::Assign(size_t index, size_t count, wchar_t c)
//   ECX = this  (thiscall)
//   [ESP+0x4]  = index  (size_t)
//   [ESP+0x8]  = count  (size_t)
//   [ESP+0xC]  = c      (wchar_t, DWORD-padded on stack)
//
// The object has an SSO (small-string-optimisation) field at offset 0x18
// (capacity threshold 8). When capacity >= 8 the wide-char buffer pointer
// lives at offset 0x4; when capacity < 8 the buffer is inline at offset 0x4.
//
// Behaviour:
//   if count == 1:
//       buf = (capacity >= 8) ? *reinterpret_cast<wchar_t**>(this+4)
//                             : reinterpret_cast<wchar_t*>(this+4)
//       buf[index] = c
//       return
//   else:
//       buf = (capacity >= 8) ? *reinterpret_cast<wchar_t**>(this+4)
//                             : reinterpret_cast<wchar_t*>(this+4)
//       if count > 0:
//           fill buf+index with (count) copies of c using REP STOSD + REP STOSW
//       return
//
// No CALL instructions and no absolute addresses — emitted as a naked
// byte-passthrough so the .obj's .text is identical to the orig slice.
//
// Original 109 bytes (RVA 0x000493c0 – 0x0004942c):
//
//   8b c1 8b 4c 24 08 83 f9 01 75 2c 83 78 18 08 72
//   13 8b 40 04 8b 4c 24 04 66 8b 54 24 0c 66 89 14
//   48 c2 0c 00 8b 4c 24 04 66 8b 54 24 0c 83 c0 04
//   66 89 14 48 c2 0c 00 83 78 18 08 72 05 8b 40 04
//   eb 03 83 c0 04 85 c9 8b 54 24 04 57 8d 3c 50 76
//   18 8b 44 24 10 66 8b d0 c1 e2 10 66 8b d0 d1 e9
//   8b c2 f3 ab 13 c9 66 f3 ab 5f c2 0c 00

extern "C" __declspec(naked) void FUN_004493c0() {
    __asm {
        _emit 0x8b              // MOV EAX, ECX
        _emit 0xc1
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x83              // CMP ECX, 0x1
        _emit 0xf9
        _emit 0x01
        _emit 0x75              // JNZ count_ne_1 (+0x2c)
        _emit 0x2c
        // --- count == 1: single char assign ---
        _emit 0x83              // CMP dword ptr [EAX+0x18], 0x8
        _emit 0x78
        _emit 0x18
        _emit 0x08
        _emit 0x72              // JC inline_single (+0x13)
        _emit 0x13
        // capacity >= 8: use heap pointer
        _emit 0x8b              // MOV EAX, dword ptr [EAX+0x4]
        _emit 0x40
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x66              // MOV DX, word ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x66              // MOV word ptr [EAX+ECX*2], DX
        _emit 0x89
        _emit 0x14
        _emit 0x48
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
        // capacity < 8: inline buffer at this+4
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x4]  (inline_single:)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x66              // MOV DX, word ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x83              // ADD EAX, 0x4
        _emit 0xc0
        _emit 0x04
        _emit 0x66              // MOV word ptr [EAX+ECX*2], DX
        _emit 0x89
        _emit 0x14
        _emit 0x48
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
        // --- count != 1: fill path (count_ne_1:) ---
        _emit 0x83              // CMP dword ptr [EAX+0x18], 0x8
        _emit 0x78
        _emit 0x18
        _emit 0x08
        _emit 0x72              // JC inline_fill (+0x5)
        _emit 0x05
        _emit 0x8b              // MOV EAX, dword ptr [EAX+0x4]
        _emit 0x40
        _emit 0x04
        _emit 0xeb              // JMP do_fill (+0x3)
        _emit 0x03
        _emit 0x83              // ADD EAX, 0x4  (inline_fill:)
        _emit 0xc0
        _emit 0x04
        // --- (do_fill:) ---
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x4]
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA EDI, [EAX+EDX*2]
        _emit 0x3c
        _emit 0x50
        _emit 0x76              // JBE done (+0x18)
        _emit 0x18
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x66              // MOV DX, AX
        _emit 0x8b
        _emit 0xd0
        _emit 0xc1              // SHL EDX, 0x10
        _emit 0xe2
        _emit 0x10
        _emit 0x66              // MOV DX, AX
        _emit 0x8b
        _emit 0xd0
        _emit 0xd1              // SHR ECX, 0x1
        _emit 0xe9
        _emit 0x8b              // MOV EAX, EDX
        _emit 0xc2
        _emit 0xf3              // REP STOSD
        _emit 0xab
        _emit 0x13              // ADC ECX, ECX
        _emit 0xc9
        _emit 0x66              // REP STOSW
        _emit 0xf3
        _emit 0xab
        // --- (done:) ---
        _emit 0x5f              // POP EDI
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
