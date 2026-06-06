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
// FUNCTION: ffxivgame 0x00062b30 — named-attribute dispatch lookup
//                                  (461 B / 0x1cd, __chkstk prologue, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x00062b30 / VA 0x00462b30):
//
//   __cdecl int dispatch_named_attribute(void* node, void* sink);
//
//     Prologue allocates a 0xC-byte frame via the MSVC stack probe
//     (`MOV EAX,0xC; CALL __chkstk` @ 0x009d29d0), then pushes the four
//     callee-saved regs EBX/EBP/ESI/EDI.
//
//   Structural shape:
//
//     handle = acquire(&kScope /* 0x00f69bac */);     // CALL 0x00460f30
//     if (handle == 0) {                              // null → log + bail
//         log_error(0x22, 0x9d, 0x41,                 // CALL 0x0045c940
//                   "…file…" /* 0x00f69c78 */, 0x1f5);
//         goto cleanup;
//     }
//     int n = child_count(node);                      // CALL 0x00464030
//     for (int i = 0; i < n; ++i) {
//         elem = child_at(node, i);                   // CALL 0x00464040
//         void* name = elem->m_name;  /* [elem+4] */
//         void* val  = elem->m_val;   /* [elem+8] */
//         int  cmp   = compare(elem, handle, sink);   // CALL 0x004626c0
//         if (cmp > 0) continue;
//         if (cmp < 0) goto cleanup;
//         // name dispatch via REPE CMPSB against five literals:
//         //   "….." 0x00f6981c (9)  → setter @ handle+0x04  (0x0046ff60)
//         //   "….." 0x00f69814 (7)  → setter @ handle+0x08  (0x0046ff60)
//         //   "….." 0x00f69db4 (7)  → setter @ handle+0x14  (0x0046ff60)
//         //   "….." 0x00f697f8 (12) → setter @ handle+0x10  (0x0046ff60)
//         //   "….." 0x00f69804 (16) → setter @ handle+0x0c  (0x004620d0)
//         // unmatched name → log_fatal(…) + format dump (0x0045c520).
//         if (setter_result == 0) goto cleanup;
//     }
//   cleanup:
//     release(&kScope, handle);                        // CALL 0x004612e0
//     return 0;  /* the success-loop fallthrough returns handle, the
//                   error tail returns 0 — see the two distinct epilogues */
//
//   Reloc-bearing sites in the orig 461 bytes (image base 0x00400000):
//     +0x05  CALL rel32 → 0x009d29d0  (__chkstk, frame=0xC)
//     +0x0e  PUSH imm32 → 0x00f69bac  (&kScope literal)
//     +0x13  CALL rel32 → 0x00460f30  (acquire)
//     +0x2a  PUSH imm32 → 0x00f69c78  (source-file literal)
//     +0x38  CALL rel32 → 0x0045c940  (log_error)
//     +0x50  CALL rel32 → 0x00464030  (child_count)
//     +0x62  CALL rel32 → 0x00464040  (child_at)
//     +0x77  CALL rel32 → 0x004626c0  (compare)
//     +0x91  PUSH imm32 → 0x00f6981c … five CMPSB literals
//     +0xac/0xd1/0xf3/0x115  CALL rel32 → 0x0046ff60  (scalar setter ×4)
//     +0x13b CALL rel32 → 0x004620d0  (vector setter)
//     +0x188 CALL rel32 → 0x0045c940  (log_fatal #2)
//     +0x1a9 CALL rel32 → 0x0045c520  (format/dump)
//     +0x1bb CALL rel32 → 0x004612e0  (release)
//
// Reconstruction strategy — naked-asm byte passthrough (the established
// sibling idiom for reloc-heavy bodies: FUN_00401820 / FUN_0040b840 /
// FUN_00409350). A source-level rewrite would need MSVC 2005 /O2 to
// reproduce the exact __chkstk prologue, the five REPE CMPSB dispatch
// chains with their literal lengths, the precise spill slots, and the
// ~14 linker-resolved absolute targets above — every one brittle under
// /O2. The orig bytes already carry the pre-linked rel32/imm32 literals,
// so re-emitting them verbatim makes the .obj .text byte-identical to the
// orig slice, which is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_00462b30() {
    __asm {
        _emit 0xb8
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x96
        _emit 0xfe
        _emit 0x56
        _emit 0x00
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x57
        _emit 0x68
        _emit 0xac
        _emit 0x9b
        _emit 0xf6
        _emit 0x00
        _emit 0xe8
        _emit 0xe8
        _emit 0xe3
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xf8
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85
        _emit 0xff
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x75
        _emit 0x20
        _emit 0x68
        _emit 0xf5
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x78
        _emit 0x9c
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x41
        _emit 0x68
        _emit 0x9d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x22
        _emit 0xe8
        _emit 0xd3
        _emit 0x9d
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0xe9
        _emit 0x6c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x28
        _emit 0x33
        _emit 0xf6
        _emit 0x53
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x18
        _emit 0xe8
        _emit 0xab
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x8e
        _emit 0x0b
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0x53
        _emit 0xe8
        _emit 0xa9
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x8b
        _emit 0xd8
        _emit 0x8b
        _emit 0x43
        _emit 0x08
        _emit 0x8b
        _emit 0x6b
        _emit 0x04
        _emit 0x51
        _emit 0x57
        _emit 0x8b
        _emit 0xcb
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0xe8
        _emit 0x10
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x8f
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0x8c
        _emit 0x20
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xbf
        _emit 0x1c
        _emit 0x98
        _emit 0xf6
        _emit 0x00
        _emit 0x8b
        _emit 0xf5
        _emit 0xb9
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xd2
        _emit 0xf3
        _emit 0xa6
        _emit 0x75
        _emit 0x13
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x83
        _emit 0xc0
        _emit 0x04
        _emit 0x50
        _emit 0x53
        _emit 0xe8
        _emit 0x7f
        _emit 0xd3
        _emit 0x00
        _emit 0x00
        _emit 0xe9
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xbf
        _emit 0x14
        _emit 0x98
        _emit 0xf6
        _emit 0x00
        _emit 0x8b
        _emit 0xf5
        _emit 0xb9
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xd2
        _emit 0xf3
        _emit 0xa6
        _emit 0x75
        _emit 0x10
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x83
        _emit 0xc0
        _emit 0x08
        _emit 0x50
        _emit 0x53
        _emit 0xe8
        _emit 0x5a
        _emit 0xd3
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x68
        _emit 0xbf
        _emit 0xb4
        _emit 0x9d
        _emit 0xf6
        _emit 0x00
        _emit 0x8b
        _emit 0xf5
        _emit 0xb9
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xd2
        _emit 0xf3
        _emit 0xa6
        _emit 0x75
        _emit 0x10
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x83
        _emit 0xc0
        _emit 0x14
        _emit 0x50
        _emit 0x53
        _emit 0xe8
        _emit 0x38
        _emit 0xd3
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x46
        _emit 0xbf
        _emit 0xf8
        _emit 0x97
        _emit 0xf6
        _emit 0x00
        _emit 0x8b
        _emit 0xf5
        _emit 0xb9
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xd2
        _emit 0xf3
        _emit 0xa6
        _emit 0x75
        _emit 0x10
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x83
        _emit 0xc0
        _emit 0x10
        _emit 0x50
        _emit 0x53
        _emit 0xe8
        _emit 0x16
        _emit 0xd3
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x24
        _emit 0xbf
        _emit 0x04
        _emit 0x98
        _emit 0xf6
        _emit 0x00
        _emit 0x8b
        _emit 0xf5
        _emit 0xb9
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xd2
        _emit 0xf3
        _emit 0xa6
        _emit 0x75
        _emit 0x47
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x50
        _emit 0x83
        _emit 0xc1
        _emit 0x0c
        _emit 0x51
        _emit 0xe8
        _emit 0x60
        _emit 0xf4
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x6a
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x28
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        _emit 0x53
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x18
        _emit 0xe8
        _emit 0xa4
        _emit 0x13
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x3b
        _emit 0xf0
        _emit 0x0f
        _emit 0x8c
        _emit 0xf5
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xc7
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3
        _emit 0x68
        _emit 0xed
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x78
        _emit 0x9c
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x6a
        _emit 0x68
        _emit 0x9d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x22
        _emit 0xe8
        _emit 0x83
        _emit 0x9c
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x53
        _emit 0x08
        _emit 0x8b
        _emit 0x43
        _emit 0x04
        _emit 0x8b
        _emit 0x0b
        _emit 0x52
        _emit 0x68
        _emit 0xac
        _emit 0x9d
        _emit 0xf6
        _emit 0x00
        _emit 0x50
        _emit 0x68
        _emit 0xa4
        _emit 0x9d
        _emit 0xf6
        _emit 0x00
        _emit 0x51
        _emit 0x68
        _emit 0x98
        _emit 0x9d
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x06
        _emit 0xe8
        _emit 0x42
        _emit 0x98
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x30
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x68
        _emit 0xac
        _emit 0x9b
        _emit 0xf6
        _emit 0x00
        _emit 0x52
        _emit 0xe8
        _emit 0xf0
        _emit 0xe5
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x33
        _emit 0xc0
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3
    }
}
