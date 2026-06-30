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
// FUNCTION: ffxivgame 0x0046cdb0 — OpenSSL `i2c_ASN1_INTEGER` (296 B / 0x128)
//                                   Converts an ASN1_INTEGER to its BER/DER
//                                   byte encoding, computing the output length
//                                   and optionally writing the encoded bytes.
//
// Inspection (read from the disassembly at orig RVA 0x0006cdb0):
//
//   __cdecl int i2c_ASN1_INTEGER(ASN1_INTEGER *a, unsigned char **pp);
//
//   a   = EDI (arg1 at [ESP+0x10] after PUSH EBX/ESI/EDI)
//   pp  = [ESP+0x18] after PUSH EBP (arg2)
//   ret = EAX
//
//   ASN1_STRING (== ASN1_INTEGER) layout (OpenSSL 0.9.8):
//     +0x00  int       length
//     +0x04  int       type    (bit 0x100 == V_ASN1_NEG)
//     +0x08  uchar *   data
//
//   Dead bytes at 0x6ce8d..0x6ce8f: 8d 49 00 (LEA ECX,[ECX+0] — 3-byte NOP,
//   unreachable between JNZ→0x6cea1 and JMP→0x6ce90).
//   The last byte of the function (offset 0x127 = addr 0x6ced7) is 0x33,
//   the first byte of `XOR EAX,EAX`; the remaining bytes of that epilogue
//   are shared with the following function at 0x6ced8.
//
//   The function contains one internal rel32 CALL to memcpy (0x009d4600)
//   already resolved in the linked binary; raw _emit passthrough preserves
//   the displacement byte-identically for compare.py.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Same strategy as FUN_004014b0, FUN_00401a00, FUN_00408f10, etc.

extern "C" __declspec(naked) void FUN_0046cdb0() {
    __asm {
        _emit 0x53
        _emit 0x56
        _emit 0x57
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x33
        _emit 0xf6
        _emit 0x32
        _emit 0xdb
        _emit 0x85
        _emit 0xff
        _emit 0x0f
        _emit 0x84
        _emit 0x12
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x57
        _emit 0x08
        _emit 0x85
        _emit 0xd2
        _emit 0x0f
        _emit 0x84
        _emit 0x07
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x0f
        _emit 0x55
        _emit 0x8b
        _emit 0x6f
        _emit 0x04
        _emit 0x81
        _emit 0xe5
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x85
        _emit 0xc9
        _emit 0x75
        _emit 0x0a
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x43
        _emit 0x85
        _emit 0xed
        _emit 0x0f
        _emit 0xb6
        _emit 0x02
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x75
        _emit 0x09
        _emit 0x83
        _emit 0xf8
        _emit 0x7f
        _emit 0x7e
        _emit 0x2f
        _emit 0x32
        _emit 0xdb
        _emit 0xeb
        _emit 0x26
        _emit 0x3d
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x7f
        _emit 0x1c
        _emit 0x75
        _emit 0x22
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x3b
        _emit 0xc8
        _emit 0x7e
        _emit 0x19
        _emit 0x8b
        _emit 0xff
        _emit 0x80
        _emit 0x3c
        _emit 0x02
        _emit 0x00
        _emit 0x75
        _emit 0x09
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        _emit 0x3b
        _emit 0xc1
        _emit 0x7c
        _emit 0xf3
        _emit 0xeb
        _emit 0x08
        _emit 0x80
        _emit 0xcb
        _emit 0xff
        _emit 0xbe
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x85
        _emit 0xd2
        _emit 0x75
        _emit 0x09
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x5d
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0xc3
        _emit 0x85
        _emit 0xf6
        _emit 0x8b
        _emit 0x0a
        _emit 0x74
        _emit 0x05
        _emit 0x88
        _emit 0x19
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        _emit 0x8b
        _emit 0x37
        _emit 0x85
        _emit 0xf6
        _emit 0x75
        _emit 0x0e
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x5d
        _emit 0x5f
        _emit 0x5e
        _emit 0xc6
        _emit 0x01
        _emit 0x00
        _emit 0x01
        _emit 0x02
        _emit 0x5b
        _emit 0xc3
        _emit 0x85
        _emit 0xed
        _emit 0x8b
        _emit 0x47
        _emit 0x08
        _emit 0x75
        _emit 0x1a
        _emit 0x56
        _emit 0x50
        _emit 0x51
        _emit 0xe8
        _emit 0x96
        _emit 0x77
        _emit 0x56
        _emit 0x00
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x01
        _emit 0x02
        _emit 0x5d
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0xc3
        _emit 0x80
        _emit 0x7c
        _emit 0x30
        _emit 0xff
        _emit 0x00
        _emit 0x8d
        _emit 0x44
        _emit 0x30
        _emit 0xff
        _emit 0x8d
        _emit 0x4c
        _emit 0x31
        _emit 0xff
        _emit 0x75
        _emit 0x16
        _emit 0xeb
        _emit 0x03
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        _emit 0xc6
        _emit 0x01
        _emit 0x00
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x83
        _emit 0xe9
        _emit 0x01
        _emit 0x83
        _emit 0xee
        _emit 0x01
        _emit 0x80
        _emit 0x38
        _emit 0x00
        _emit 0x74
        _emit 0xef
        _emit 0x8a
        _emit 0x18
        _emit 0xf6
        _emit 0xd3
        _emit 0x80
        _emit 0xc3
        _emit 0x01
        _emit 0x88
        _emit 0x19
        _emit 0x83
        _emit 0xee
        _emit 0x01
        _emit 0x83
        _emit 0xe9
        _emit 0x01
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x85
        _emit 0xf6
        _emit 0x7e
        _emit 0x13
        _emit 0x8a
        _emit 0x18
        _emit 0xf6
        _emit 0xd3
        _emit 0x88
        _emit 0x19
        _emit 0x83
        _emit 0xee
        _emit 0x01
        _emit 0x83
        _emit 0xe9
        _emit 0x01
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x85
        _emit 0xf6
        _emit 0x7f
        _emit 0xed
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x01
        _emit 0x02
        _emit 0x5d
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0xc3
        _emit 0x5f
        _emit 0x5e
        _emit 0x33
    }
}
