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
// FUNCTION: ffxivgame 0x000754a0 — __cdecl size-class lookup helper (66 B / 0x42)
//
// Reads one integer argument from [ESP+4], maps three known OpenSSL buffer
// sizes to their corresponding maximum-record-length constants, and falls
// back to ERR_put_error for unrecognised sizes.
//
//   int __cdecl FUN_004754a0(int size_class)
//
//   Mappings:
//     0x3a (58)  → 0x80  (128)
//     0x78 (120) → 0x40  (64)
//     0xa0 (160) → 0x28  (40)
//     else       → ERR_put_error(6, 0x6d, 0x6c, <file>, 0xa3); return 0
//
// Asm shape (66 bytes, no prologue/epilogue — /O2 /Oy frame-pointer omission):
//
//   000754a0  8b 44 24 04        MOV EAX, [ESP+4]      ; n = arg
//   000754a4  83 f8 3a           CMP EAX, 0x3a
//   000754a7  75 06              JNZ  +6
//   000754a9  b8 80 00 00 00     MOV EAX, 0x80
//   000754ae  c3                 RET
//   000754af  83 f8 78           CMP EAX, 0x78
//   000754b2  75 06              JNZ  +6
//   000754b4  b8 40 00 00 00     MOV EAX, 0x40
//   000754b9  c3                 RET
//   000754ba  3d a0 00 00 00     CMP EAX, 0xa0         ; imm32 (0xa0 > 127)
//   000754bf  75 06              JNZ  +6
//   000754c1  b8 28 00 00 00     MOV EAX, 0x28
//   000754c6  c3                 RET
//   000754c7  68 a3 00 00 00     PUSH 0xa3             ; line (imm32, 163>127)
//   000754cc  68 68 ae f7 00     PUSH 0xf7ae68         ; file ptr literal
//   000754d1  6a 6c              PUSH 0x6c             ; reason
//   000754d3  6a 6d              PUSH 0x6d             ; func
//   000754d5  6a 06              PUSH 0x6              ; lib
//   000754d7  e8 64 74 fe ff     CALL 0x0045c940       ; ERR_put_error (rel32, masked)
//   000754dc  83 c4 14           ADD ESP, 0x14         ; caller cleans 5 dwords (__cdecl)
//   000754df  33 c0              XOR EAX, EAX          ; return 0
//   000754e1  c3                 RET

extern "C" {

// ERR_put_error (OpenSSL) — __cdecl, 5 args, caller cleans.
// The fourth argument (file) is passed as a raw integer literal (0xf7ae68)
// matching the in-binary string pointer; the compiler emits PUSH imm32
// with no relocation, which byte-matches the orig binary directly.
void __cdecl FUN_0045c940(int lib, int func, int reason, int file, int line);

int __cdecl FUN_004754a0(int n)
{
    if (n == 0x3a) return 0x80;
    if (n == 0x78) return 0x40;
    if (n == 0xa0) return 0x28;
    FUN_0045c940(6, 0x6d, 0x6c, 0xf7ae68, 0xa3);
    return 0;
}

}  // extern "C"
