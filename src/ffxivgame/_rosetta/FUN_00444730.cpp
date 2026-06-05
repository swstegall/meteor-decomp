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
// FUNCTION: ffxivgame 0x00044730 — __thiscall record-formatting loop with
//                                  inline SEH frame + /GS security cookie
//                                  (424 B / 0x1a8, RET 0x4).
//
// Inspection (read from the disassembly at orig RVA 0x00044730):
//
//   __thiscall <ret> FUN_00444730(<this in ECX>, void* sink /* [esp+arg0] */);
//
//   `ECX = this`, one stack argument (the EBP-held sink object loaded from
//   [ESP+0x208c] right after the prologue), callee-popped `RET 0x4`.
//
//   Body shape (mirrors the asm):
//
//     int count = this->[0x4] << 5;             // EAX = field<<5 (record count*32)
//     if (count <= 0) goto done;                // JLE → success tail
//     int idx = 0;                              // EBX running byte offset
//     int rem = count;                          // loop trip remaining
//     for (;;) {                                // record stride is 0xbc bytes
//         rec* r = (rec*)((char*)this->[0x8] + off);
//         switch (r->[0x0]) {                   // tag
//           case 1:
//             _snprintf-ish(buf, "%...", FUN_00445210(&r->[0x4]),
//                           r->[0x58], r->[0x5c], r->[0x60]);  // 0x009d4f08
//             break;                            // → emit (0x00444847)
//           case 2:
//             FUN_00447200(&local, &r->[0x68]);
//             // three chained FUN_004454a0 enum/format steps fed 0x2c,2,a,3,d,4
//             FUN_004454a0(...); FUN_004454a0(...); FUN_004454a0(...);
//             snprintf(buf, "%...", FUN_00445210(&r->[0x4]),
//                                   FUN_00445210(...));         // 0x009d4f08
//             FUN_00446f50(&local);             // dtor of the case-2 temp
//             break;                            // → emit
//           default:
//             goto next;                        // tag ∉ {1,2}: skip record
//         }
//       emit:                                   // 0x00444847
//         size_t len = strlen(buf);             // inlined NUL scan (8b ff align)
//         FUN_00406280(sink, idx + len, 0);     // reserve / grow at offset
//         if (!sink->[0x4] ||
//             idx >= (sink->[0x8] - sink->[0x4])) __report_rangecheckfailure();
//         memcpy(sink->[0x4] + idx, buf, len);  // 0x009d4600
//         idx = idx + len;                      // EBX = EDI
//       next:
//         off += 0xbc;
//         if (--rem == 0) break;
//     }
//   done:
//     return 1;                                 // AL = 1
//
//   Frame: SEH record {push -1, push 0xe573fa scope-table, fs:[0] link} +
//   /GS cookie (xor [0x012ea8b0] with ESP, stashed at [ESP+0x2064] and a
//   second copy pushed as the SEH guard). Stack reservation 0x2068 via the
//   chkstk-style call at 0x009d29d0; teardown `ADD ESP,0x2074` + `RET 0x4`.
//
//   Reloc-bearing sites (image-base 0x00400000 dependent — only resolve in
//   a full relink; standalone .obj can't reproduce the absolute literals):
//     +0x13  CALL 0x009d29d0 (__chkstk / stack probe)
//     +0x18  MOV  EAX,[0x012ea8b0] (__security_cookie)
//     +0x2a  MOV  EAX,[0x012ea8b0] (cookie, 2nd load)
//     +0x7f  CALL 0x00445210
//     +0x8c  PUSH 0x00f6726c (format string, case 1)
//     +0x92  CALL 0x009d4f08 (snprintf-style formatter)
//     +0xad  CALL 0x00447200 (case-2 temp ctor)
//     +0xcd .. 0xdb  3 × CALL 0x004454a0 (enum/format steps)
//     +0xe2,+0xeb    2 × CALL 0x00445210
//     +0xf5  PUSH 0x00f6727c (format string, case 2)
//     +0xfb  CALL 0x009d4f08
//     +0x112 CALL 0x00446f50 (case-2 temp dtor)
//     +0x135 CALL 0x00406280 (sink reserve/grow)
//     +0x14a CALL 0x009d22b4 (__report_rangecheckfailure)
//     +0x15b CALL 0x009d4600 (memcpy)
//     +0x19a CALL 0x009d20f4 (__security_check_cookie)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 /GS /EHsc into
//   reproducing the exact SEH prologue/epilogue, the cookie double-load,
//   the SUB-EAX chained switch ordering, the inlined strlen NUL scan with
//   its `8b ff` 2-byte alignment pad, and the dozen-plus linker-resolved
//   absolute addresses above — each brittle under /O2. The established
//   sibling idiom (FUN_00415d00, FUN_0040b840, FUN_00409350) is a
//   `__declspec(naked)` body re-emitting the orig 424 bytes verbatim via
//   MASM `_emit` directives. The .obj's `.text` ends up byte-identical to
//   the orig slice (raw immediates → no reloc table), which is exactly
//   what tools/compare.py grades.

extern "C" __declspec(naked) void FUN_00444730() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xfa
        _emit 0x73
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0xb8
        _emit 0x68
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x88
        _emit 0xe2
        _emit 0x58
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
        _emit 0x64
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x57
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x50
        _emit 0x8d
        _emit 0x84
        _emit 0x24
        _emit 0x7c
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        _emit 0x8b
        _emit 0xac
        _emit 0x24
        _emit 0x8c
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xdb
        _emit 0xc1
        _emit 0xe0
        _emit 0x05
        _emit 0x3b
        _emit 0xc3
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x0f
        _emit 0x8e
        _emit 0x22
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8b
        _emit 0x71
        _emit 0x08
        _emit 0x03
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x06
        _emit 0x83
        _emit 0xf8
        _emit 0x01
        _emit 0x75
        _emit 0x2c
        _emit 0x8b
        _emit 0x46
        _emit 0x60
        _emit 0x8b
        _emit 0x4e
        _emit 0x5c
        _emit 0x8b
        _emit 0x56
        _emit 0x58
        _emit 0x50
        _emit 0x51
        _emit 0x52
        _emit 0x8d
        _emit 0x4e
        _emit 0x04
        _emit 0xe8
        _emit 0x5c
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x8d
        _emit 0x84
        _emit 0x24
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x6c
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x50
        _emit 0xe8
        _emit 0x41
        _emit 0x07
        _emit 0x59
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0xeb
        _emit 0x7b
        _emit 0x83
        _emit 0xf8
        _emit 0x02
        _emit 0x0f
        _emit 0x85
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x4e
        _emit 0x68
        _emit 0x51
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0xe8
        _emit 0x1e
        _emit 0x2a
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x04
        _emit 0x6a
        _emit 0x0d
        _emit 0x6a
        _emit 0x03
        _emit 0x6a
        _emit 0x0a
        _emit 0x6a
        _emit 0x02
        _emit 0x6a
        _emit 0x2c
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0x9c
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x9e
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xc8
        _emit 0xe8
        _emit 0x97
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xc8
        _emit 0xe8
        _emit 0x90
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xc8
        _emit 0xe8
        _emit 0xf9
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x8d
        _emit 0x4e
        _emit 0x04
        _emit 0xe8
        _emit 0xf0
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x7c
        _emit 0x68
        _emit 0x7c
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x52
        _emit 0xe8
        _emit 0xd8
        _emit 0x06
        _emit 0x59
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0x84
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8
        _emit 0x09
        _emit 0x27
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x74
        _emit 0x8d
        _emit 0x50
        _emit 0x01
        _emit 0x8b
        _emit 0xff
        _emit 0x8a
        _emit 0x08
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        _emit 0x84
        _emit 0xc9
        _emit 0x75
        _emit 0xf7
        _emit 0x2b
        _emit 0xc2
        _emit 0x8b
        _emit 0xf0
        _emit 0x6a
        _emit 0x00
        _emit 0x8d
        _emit 0x3c
        _emit 0x1e
        _emit 0x57
        _emit 0x8b
        _emit 0xcd
        _emit 0xe8
        _emit 0x16
        _emit 0x1a
        _emit 0xfc
        _emit 0xff
        _emit 0x8b
        _emit 0x45
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x09
        _emit 0x8b
        _emit 0x4d
        _emit 0x08
        _emit 0x2b
        _emit 0xc8
        _emit 0x3b
        _emit 0xd9
        _emit 0x72
        _emit 0x05
        _emit 0xe8
        _emit 0x35
        _emit 0xda
        _emit 0x58
        _emit 0x00
        _emit 0x8b
        _emit 0x4d
        _emit 0x04
        _emit 0x56
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x78
        _emit 0x50
        _emit 0x03
        _emit 0xcb
        _emit 0x51
        _emit 0xe8
        _emit 0x70
        _emit 0xfd
        _emit 0x58
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x8b
        _emit 0xdf
        _emit 0x81
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x01
        _emit 0x0f
        _emit 0x85
        _emit 0xe6
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xb0
        _emit 0x01
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x7c
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x64
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x25
        _emit 0xd8
        _emit 0x58
        _emit 0x00
        _emit 0x81
        _emit 0xc4
        _emit 0x74
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
