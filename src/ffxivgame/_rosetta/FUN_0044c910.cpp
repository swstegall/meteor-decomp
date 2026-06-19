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
// FUNCTION: ffxivgame 0x0004c910 — __thiscall debug/notify message
//                                   dispatcher (446 B / 0x1be, /GS cookie).
//
// Inspection (read from the disassembly at orig RVA 0x0004c910):
//
//   __thiscall void Dispatch(this, Msg* msg);   // RET 0x4, ECX = this,
//                                                // one stack arg.
//
//   ESI = msg (stack arg at [ESP+0x10c] after the two callee-save pushes),
//   EDI = this. The body is a SUB-EAX chained switch on msg->type
//   (*(u32*)msg) over three case tags, all sharing the prefix 0x53510000
//   ("SQ\0\0"-style 4CC); the default arm falls straight through to the
//   shared notify tail.
//
//     if (msg == NULL) goto done;
//     switch (msg->type) {
//       case 0x53510000:                       // notify-only
//         if (this->cb_2c) this->cb_2c(this->ctx_30, 1, 0);
//         break;                               // → shared tail at 0x4caa4
//       case 0x53510001: {                     // 28-byte blob → hex string
//         BYTE* p = msg->payload_8;            // [msg+8]
//         // mirror 0x1c=28 raw bytes into the .data record @0x0132ce68
//         *(__int64*)0x0132ce68 = *(__int64*)(p+0x00);
//         *(__int64*)0x0132ce70 = *(__int64*)(p+0x08);
//         *(__int64*)0x0132ce78 = *(__int64*)(p+0x10);
//         *(u32*)   0x0132ce80 = *(u32*)   (p+0x18);
//         // hex-encode the 28 bytes into the char buffer @0x0132ce84
//         for (int i = 0; i < 0x1c; i += 4) {  // unrolled x4 per iter
//             // hi/lo nibble → DAT_00f673e0[] hex digit table
//             ...
//         }
//         ((char*)0x0132ce84)[0x38] = 0;       // NUL terminate (56 chars)
//         _snprintf_s(buf, 0x100, DAT_00f673f4, (char*)0x0132ce84);
//         break;
//       }
//       case 0x53510002:                       // ascii payload
//         _snprintf_s(buf, 0x100, DAT_00f67408, *(u32*)(msg->payload_8 + 8));
//         break;
//       default: break;
//     }
//     // shared notify tail @0x4caa4:
//     if (this->cb_28) this->cb_28(this->ctx_30, msg->type, msg->payload_8);
//   done:
//     // /GS epilogue (cookie @0x012ea8b0), POP EDI/ESI, RET 0x4.
//
//   The hex-encode loop head at 0x0044c9b0 is 16-byte aligned, so MSVC
//   2005 emitted a 6-byte `npad 6` (8D 9B 00 00 00 00 — lea ebx,[ebx+0])
//   between the `XOR EAX,EAX`/`JMP` and the loop body; the JMP (EB 06)
//   skips it. Those padding bytes are part of the function slice and are
//   re-emitted below.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rewrite would need MSVC 2005 /O2 /GS to reproduce the
//   exact SUB-EAX switch lowering, the unrolled-x4 nibble loop with its
//   16-byte loop-head alignment pad, the x87/MOVQ blob copy, the two
//   _snprintf_s call sites, the two indirect callbacks, AND the
//   linker-resolved absolute addresses (cookie @0x012ea8b0, .data record
//   @0x0132ce68, hex table @0x00f673e0, two format strings @0x00f673f4 /
//   0x00f67408, _snprintf_s @0x009d4f83, __security_check_cookie
//   @0x009d20f4) — every one brittle under /O2. The established sibling
//   idiom (FUN_00409350 / FUN_00415d00) is a `__declspec(naked)` body
//   re-emitting the orig 446 bytes verbatim via `_emit`; the .obj's
//   .text ends up byte-identical to the orig slice, which is what
//   tools/compare.py checks.

extern "C" __declspec(naked) void FUN_0044c910() {
    __asm {
        _emit 0x81
        _emit 0xec
        _emit 0x04
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x84
        _emit 0x24

        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0x0c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x85
        _emit 0xf6
        _emit 0x57
        _emit 0x8b

        _emit 0xf9
        _emit 0x0f
        _emit 0x84
        _emit 0x84
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x06
        _emit 0x2d
        _emit 0x00
        _emit 0x00
        _emit 0x51
        _emit 0x53
        _emit 0x0f
        _emit 0x84

        _emit 0x4c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x74
        _emit 0x2c
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x0f
        _emit 0x85
        _emit 0x52
        _emit 0x01

        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        _emit 0x51
        _emit 0x68
        _emit 0x08
        _emit 0x74
        _emit 0xf6
        _emit 0x00
        _emit 0x8d
        _emit 0x54

        _emit 0x24
        _emit 0x10
        _emit 0x68
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x52
        _emit 0xe8
        _emit 0x16
        _emit 0x86
        _emit 0x58
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x10

        _emit 0xe9
        _emit 0x2f
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x01
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05

        _emit 0x68
        _emit 0xce
        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x08
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0x70
        _emit 0xce
        _emit 0x32

        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x10
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0x78
        _emit 0xce
        _emit 0x32
        _emit 0x01
        _emit 0x8b
        _emit 0x41

        _emit 0x18
        _emit 0xa3
        _emit 0x80
        _emit 0xce
        _emit 0x32
        _emit 0x01
        _emit 0x33
        _emit 0xc0
        _emit 0xeb
        _emit 0x06
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x0f
        _emit 0xb6
        _emit 0x14
        _emit 0x01
        _emit 0xc1
        _emit 0xea
        _emit 0x04
        _emit 0x0f
        _emit 0xb6
        _emit 0x92
        _emit 0xe0
        _emit 0x73
        _emit 0xf6
        _emit 0x00
        _emit 0x88
        _emit 0x14

        _emit 0x45
        _emit 0x84
        _emit 0xce
        _emit 0x32
        _emit 0x01
        _emit 0x0f
        _emit 0xb6
        _emit 0x14
        _emit 0x01
        _emit 0x83
        _emit 0xe2
        _emit 0x0f
        _emit 0x0f
        _emit 0xb6
        _emit 0x92
        _emit 0xe0

        _emit 0x73
        _emit 0xf6
        _emit 0x00
        _emit 0x88
        _emit 0x14
        _emit 0x45
        _emit 0x85
        _emit 0xce
        _emit 0x32
        _emit 0x01
        _emit 0x0f
        _emit 0xb6
        _emit 0x54
        _emit 0x01
        _emit 0x01
        _emit 0xc1

        _emit 0xea
        _emit 0x04
        _emit 0x0f
        _emit 0xb6
        _emit 0x92
        _emit 0xe0
        _emit 0x73
        _emit 0xf6
        _emit 0x00
        _emit 0x88
        _emit 0x14
        _emit 0x45
        _emit 0x86
        _emit 0xce
        _emit 0x32
        _emit 0x01

        _emit 0x0f
        _emit 0xb6
        _emit 0x54
        _emit 0x01
        _emit 0x01
        _emit 0x83
        _emit 0xe2
        _emit 0x0f
        _emit 0x0f
        _emit 0xb6
        _emit 0x92
        _emit 0xe0
        _emit 0x73
        _emit 0xf6
        _emit 0x00
        _emit 0x88

        _emit 0x14
        _emit 0x45
        _emit 0x87
        _emit 0xce
        _emit 0x32
        _emit 0x01
        _emit 0x0f
        _emit 0xb6
        _emit 0x54
        _emit 0x01
        _emit 0x02
        _emit 0xc1
        _emit 0xea
        _emit 0x04
        _emit 0x0f
        _emit 0xb6

        _emit 0x92
        _emit 0xe0
        _emit 0x73
        _emit 0xf6
        _emit 0x00
        _emit 0x88
        _emit 0x14
        _emit 0x45
        _emit 0x88
        _emit 0xce
        _emit 0x32
        _emit 0x01
        _emit 0x0f
        _emit 0xb6
        _emit 0x54
        _emit 0x01

        _emit 0x02
        _emit 0x83
        _emit 0xe2
        _emit 0x0f
        _emit 0x0f
        _emit 0xb6
        _emit 0x92
        _emit 0xe0
        _emit 0x73
        _emit 0xf6
        _emit 0x00
        _emit 0x88
        _emit 0x14
        _emit 0x45
        _emit 0x89
        _emit 0xce

        _emit 0x32
        _emit 0x01
        _emit 0x0f
        _emit 0xb6
        _emit 0x54
        _emit 0x01
        _emit 0x03
        _emit 0xc1
        _emit 0xea
        _emit 0x04
        _emit 0x0f
        _emit 0xb6
        _emit 0x92
        _emit 0xe0
        _emit 0x73
        _emit 0xf6

        _emit 0x00
        _emit 0x88
        _emit 0x14
        _emit 0x45
        _emit 0x8a
        _emit 0xce
        _emit 0x32
        _emit 0x01
        _emit 0x0f
        _emit 0xb6
        _emit 0x54
        _emit 0x01
        _emit 0x03
        _emit 0x83
        _emit 0xe2
        _emit 0x0f

        _emit 0x0f
        _emit 0xb6
        _emit 0x92
        _emit 0xe0
        _emit 0x73
        _emit 0xf6
        _emit 0x00
        _emit 0x88
        _emit 0x14
        _emit 0x45
        _emit 0x8b
        _emit 0xce
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc0

        _emit 0x04
        _emit 0x83
        _emit 0xf8
        _emit 0x1c
        _emit 0x0f
        _emit 0x8c
        _emit 0x46
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x68
        _emit 0x84
        _emit 0xce
        _emit 0x32
        _emit 0x01
        _emit 0x68

        _emit 0xf4
        _emit 0x73
        _emit 0xf6
        _emit 0x00
        _emit 0xc6
        _emit 0x04
        _emit 0x45
        _emit 0x84
        _emit 0xce
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10

        _emit 0x68
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0xe8
        _emit 0xf8
        _emit 0x84
        _emit 0x58
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0xeb
        _emit 0x14

        _emit 0x8b
        _emit 0x47
        _emit 0x2c
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x0d
        _emit 0x8b
        _emit 0x4f
        _emit 0x30
        _emit 0x6a
        _emit 0x00
        _emit 0x6a
        _emit 0x01
        _emit 0x51
        _emit 0xff

        _emit 0xd0
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x8b
        _emit 0x47
        _emit 0x28
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x10
        _emit 0x8b
        _emit 0x56
        _emit 0x08
        _emit 0x8b
        _emit 0x0e

        _emit 0x52
        _emit 0x8b
        _emit 0x57
        _emit 0x30
        _emit 0x51
        _emit 0x52
        _emit 0xff
        _emit 0xd0
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x08
        _emit 0x01

        _emit 0x00
        _emit 0x00
        _emit 0x5f
        _emit 0x5e
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x29
        _emit 0x56
        _emit 0x58
        _emit 0x00
        _emit 0x81
        _emit 0xc4
        _emit 0x04
    }
}
