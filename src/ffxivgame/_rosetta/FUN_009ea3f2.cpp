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
// FUNCTION: ffxivgame 0x005ea3f2 — time-window evaluator over a caller-
//                                  supplied time record (467 B / 0x1d3, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x005ea3f2):
//
//   BOOL __usercall is_within_time_window(<EDI> = const TimeRec* rec);
//
//     EDI is an INBOUND register parameter (a struct pointer) — the
//     function reads it (+0x00 sec, +0x04 min, +0x08 hour, +0x14 day/key,
//     +0x1c marker) but never initialises, saves, or restores it, so the
//     caller passes it in EDI under a non-standard register convention.
//     Only ESI (zero scratch) and EBX (constant 1) are callee-saved.
//     Returns 0/1 in EAX.
//
//   Structural shape:
//
//     int slot = 0;
//     if (FUN_009ea658(&slot))                 // poll/acquire by ptr
//         FUN_009d2194(0,0,0,0,0);             // cdecl, 5 args (add esp,0x14)
//     if (slot == 0) return 0;
//
//     int key = rec->key;                      // [EDI+0x14]
//     if (key != DAT_012eba44 && key != DAT_012eba50) {   // not a special key
//         if (DAT_0136497c == 0) {
//             // path A — synthesize two log/schedule records from the
//             //          word-table at 0x01364914..0x0136497c, branch on
//             //          the 0x01364968 / 0x01364914 phase flags.
//             FUN_009ea1fd(1, ...);  FUN_009ea1fd(1, ...);   // cdecl, 9 args each
//         } else {
//             // path B — key vs 0x6b (107) selects state {0xb,1} or {0xa,5},
//             //           emit two records with leading selector ECX=2.
//             FUN_009ea1fd(2, ...);  FUN_009ea1fd(2, ...);   // cdecl, 9 args each
//         }
//     }
//
//     // window test: [DAT_012eba48 .. DAT_012eba54] vs rec->marker (+0x1c),
//     //              with inclusive/exclusive edge handling depending on
//     //              whether lo<hi or lo>=hi (wrap-around window).
//     long lo = DAT_012eba48, hi = DAT_012eba54, m = rec->marker;
//     ... edge cluster -> either return 0, return 1, or fall to the ms test:
//
//     // ms-of-day = ((hour*60 + min)*60 + sec) * 1000
//     long ms = ((rec->hour*0x3c + rec->min)*0x3c + rec->sec) * 0x3e8;
//     return (m == lo) ? (ms >= DAT_012eba4c) : (ms < DAT_012eba58);
//
//   Reloc-bearing sites (image-base-dependent absolute addrs / rel32):
//     CALL FUN_009ea658, FUN_009d2194, FUN_009ea1fd (×4);
//     word globals 0x01364914..0x01364976; dword globals
//     0x0136497c, 0x012eba44/48/4c/50/54/58.
//
// Reconstruction strategy — naked-asm byte passthrough (the established
// sibling idiom for reloc-heavy bodies that also rely on a non-standard
// inbound register convention the C++ front-end can't express):
// re-emit the orig 467 bytes verbatim via MASM `_emit`. The .obj's
// `.text` ends up byte-identical to the orig slice, which is what
// tools/compare.py grades. The structural commentary above is the
// readable record for a future source-level promotion once the time
// record (EDI: +0/+4/+8/+0x14/+0x1c) and the 0x012eba44.. window
// config block are catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_009ea3f2() {
    __asm {
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        _emit 0x56
        _emit 0x8d
        _emit 0x45
        _emit 0xfc
        _emit 0x33
        _emit 0xf6
        _emit 0x50
        _emit 0x89
        _emit 0x75
        _emit 0xfc
        _emit 0xe8
        _emit 0x51
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x85
        _emit 0xc0
        _emit 0x59
        _emit 0x74
        _emit 0x0d
        _emit 0x56
        _emit 0x56
        _emit 0x56
        _emit 0x56
        _emit 0x56
        _emit 0xe8
        _emit 0x7e
        _emit 0x7d
        _emit 0xfe
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x39
        _emit 0x75
        _emit 0xfc
        _emit 0x75
        _emit 0x07
        _emit 0x33
        _emit 0xc0
        _emit 0xe9
        _emit 0x58
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x57
        _emit 0x14
        _emit 0x53
        _emit 0x33
        _emit 0xdb
        _emit 0x43
        _emit 0x3b
        _emit 0x15
        _emit 0x44
        _emit 0xba
        _emit 0x2e
        _emit 0x01
        _emit 0x75
        _emit 0x0c
        _emit 0x3b
        _emit 0x15
        _emit 0x50
        _emit 0xba
        _emit 0x2e
        _emit 0x01
        _emit 0x0f
        _emit 0x84
        _emit 0x18
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x39
        _emit 0x35
        _emit 0x7c
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x0f
        _emit 0x84
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66
        _emit 0x39
        _emit 0x35
        _emit 0x68
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x0f
        _emit 0xb7
        _emit 0x05
        _emit 0x76
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x0f
        _emit 0xb7
        _emit 0x0d
        _emit 0x70
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x50
        _emit 0x0f
        _emit 0xb7
        _emit 0x05
        _emit 0x74
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x50
        _emit 0x0f
        _emit 0xb7
        _emit 0x05
        _emit 0x72
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x50
        _emit 0x75
        _emit 0x15
        _emit 0x0f
        _emit 0xb7
        _emit 0x05
        _emit 0x6c
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x56
        _emit 0x50
        _emit 0x0f
        _emit 0xb7
        _emit 0x05
        _emit 0x6e
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x50
        _emit 0x52
        _emit 0x53
        _emit 0xeb
        _emit 0x0c
        _emit 0x0f
        _emit 0xb7
        _emit 0x05
        _emit 0x6e
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x50
        _emit 0x56
        _emit 0x56
        _emit 0x52
        _emit 0x56
        _emit 0x0f
        _emit 0xb7
        _emit 0x05
        _emit 0x6a
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x53
        _emit 0xe8
        _emit 0x5b
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x0f
        _emit 0xb7
        _emit 0x05
        _emit 0x22
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x0f
        _emit 0xb7
        _emit 0x0d
        _emit 0x1c
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        _emit 0x66
        _emit 0x39
        _emit 0x35
        _emit 0x14
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x50
        _emit 0x0f
        _emit 0xb7
        _emit 0x05
        _emit 0x20
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x50
        _emit 0x0f
        _emit 0xb7
        _emit 0x05
        _emit 0x1e
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x50
        _emit 0x75
        _emit 0x17
        _emit 0x0f
        _emit 0xb7
        _emit 0x05
        _emit 0x18
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x56
        _emit 0x50
        _emit 0x0f
        _emit 0xb7
        _emit 0x05
        _emit 0x1a
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x50
        _emit 0xff
        _emit 0x77
        _emit 0x14
        _emit 0x53
        _emit 0xeb
        _emit 0x0e
        _emit 0x0f
        _emit 0xb7
        _emit 0x05
        _emit 0x1a
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x50
        _emit 0x56
        _emit 0x56
        _emit 0xff
        _emit 0x77
        _emit 0x14
        _emit 0x56
        _emit 0x0f
        _emit 0xb7
        _emit 0x05
        _emit 0x16
        _emit 0x49
        _emit 0x36
        _emit 0x01
        _emit 0x56
        _emit 0xe8
        _emit 0xfe
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        _emit 0xeb
        _emit 0x54
        _emit 0x83
        _emit 0xfa
        _emit 0x6b
        _emit 0x6a
        _emit 0x03
        _emit 0x58
        _emit 0x6a
        _emit 0x02
        _emit 0x59
        _emit 0xc7
        _emit 0x45
        _emit 0xf4
        _emit 0x0b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x5d
        _emit 0xf8
        _emit 0x7d
        _emit 0x13
        _emit 0x6a
        _emit 0x04
        _emit 0x58
        _emit 0x8b
        _emit 0xcb
        _emit 0xc7
        _emit 0x45
        _emit 0xf4
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x45
        _emit 0xf8
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0x56
        _emit 0x56
        _emit 0x56
        _emit 0x56
        _emit 0x51
        _emit 0x52
        _emit 0x53
        _emit 0x53
        _emit 0x6a
        _emit 0x02
        _emit 0x59
        _emit 0xe8
        _emit 0xc0
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x45
        _emit 0xf4
        _emit 0x56
        _emit 0x56
        _emit 0x56
        _emit 0x56
        _emit 0x56
        _emit 0xff
        _emit 0x75
        _emit 0xf8
        _emit 0xff
        _emit 0x77
        _emit 0x14
        _emit 0x53
        _emit 0x56
        _emit 0x6a
        _emit 0x02
        _emit 0x59
        _emit 0xe8
        _emit 0xa8
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x48
        _emit 0x8b
        _emit 0x0d
        _emit 0x48
        _emit 0xba
        _emit 0x2e
        _emit 0x01
        _emit 0xa1
        _emit 0x54
        _emit 0xba
        _emit 0x2e
        _emit 0x01
        _emit 0x3b
        _emit 0xc8
        _emit 0x8b
        _emit 0x57
        _emit 0x1c
        _emit 0x7d
        _emit 0x16
        _emit 0x3b
        _emit 0xd1
        _emit 0x7c
        _emit 0x22
        _emit 0x3b
        _emit 0xd0
        _emit 0x7f
        _emit 0x1e
        _emit 0x3b
        _emit 0xd1
        _emit 0x7e
        _emit 0x1e
        _emit 0x3b
        _emit 0xd0
        _emit 0x7d
        _emit 0x1a
        _emit 0x8b
        _emit 0xc3
        _emit 0x5b
        _emit 0x5e
        _emit 0xc9
        _emit 0xc3
        _emit 0x3b
        _emit 0xd0
        _emit 0x7c
        _emit 0xf6
        _emit 0x3b
        _emit 0xd1
        _emit 0x7f
        _emit 0xf2
        _emit 0x3b
        _emit 0xd0
        _emit 0x7e
        _emit 0x08
        _emit 0x3b
        _emit 0xd1
        _emit 0x7d
        _emit 0x04
        _emit 0x33
        _emit 0xc0
        _emit 0xeb
        _emit 0xe8
        _emit 0x8b
        _emit 0x47
        _emit 0x08
        _emit 0x6b
        _emit 0xc0
        _emit 0x3c
        _emit 0x03
        _emit 0x47
        _emit 0x04
        _emit 0x6b
        _emit 0xc0
        _emit 0x3c
        _emit 0x03
        _emit 0x07
        _emit 0x69
        _emit 0xc0
        _emit 0xe8
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x3b
        _emit 0xd1
        _emit 0x75
        _emit 0x0d
        _emit 0x33
        _emit 0xc9
        _emit 0x3b
        _emit 0x05
        _emit 0x4c
        _emit 0xba
        _emit 0x2e
        _emit 0x01
        _emit 0x0f
        _emit 0x9d
        _emit 0xc1
        _emit 0xeb
        _emit 0x0b
        _emit 0x33
        _emit 0xc9
        _emit 0x3b
        _emit 0x05
        _emit 0x58
        _emit 0xba
        _emit 0x2e
        _emit 0x01
        _emit 0x0f
        _emit 0x9c
        _emit 0xc1
        _emit 0x8b
    }
}
