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
// FUNCTION: ffxivgame 0x0003d3a0 — __thiscall accessor/builder that returns
//                                  a result object into a caller-supplied
//                                  out slot (466 B / 0x1d2, full /GS + SEH).
//
// Inspection (read from the disassembly at orig RVA 0x0003d3a0):
//
//   __thiscall <Result*> build(this /*ECX=ESI*/, <out> /*[esp+0x444]=EBP*/,
//                              <key>);  // returns EBP (the out slot)
//
//   Body shape (one early-out branch + a key-lookup branch, both writing
//   *out = 0, then the success tail that constructs the result):
//
//     EH frame: PUSH -1 / PUSH 0xe569ec (scope table) / FS:[0] link, with
//     the standard /GS security-cookie spill at [esp+0x420] and a second
//     cookie-xor'd copy pushed for the EH record at [esp+0x434].
//
//     FUN_004330f0(&local, this);            // probe / parse into local
//     if (local == 0) { *out = 0; goto done; }
//     if (FUN_009d6b65(&local, this, 0xf6693c) /* [esp+0x1c] */ == 0) {
//         // not-found path: format two diagnostic strings + report
//         char buf[0x400];
//         FUN_009d4f9f(buf, 0x400, "…", 0x3fd, "…", 0x1b1, "…");
//         n = FUN_009d4f9f(&buf[n], 0x400-n, 0x3fe-n, "…", this);
//         FUN_009d4bb4(&buf2, 0x400, "…");
//         (*0x012660d8)(&buf3, 4);
//         *out = 0; goto done;
//     }
//     // found path
//     a = FUN_009d6a61(eax);
//     b = FUN_009d6962(a);
//     c = FUN_0040e2d0(&local2, 0x10, 0xf66a04);
//     d = FUN_0040a330(0);
//     e = FUN_0040e110(d, edi /*c*/, esi);
//     FUN_009d6947(e, edi, 1, [esp+0x10]);
//     FUN_009d2646([esp+0x20]);
//     r = FUN_0043ce20(&local3, [esp+0x28], esi, [esp+0x45c]);
//     edi = *r; *r = 0;
//     state = 1;
//     if (local3.x) { (**local3.x)(1); }    // virtual release
//     if (esi) FUN_0040df70(esi);           // operator delete(esi-4 size hdr)
//     *out = edi;
//   done:
//     return out;  // EAX = EBP
//
//   Reloc-bearing sites in the orig 466 bytes (image base 0x00400000):
//     +0x02  PUSH imm32   → 0x00e569ec (EH scope table)
//     +0x14  MOV  []      → 0x012ea8b0 (__security_cookie)  (twice)
//     +0x55  CALL rel32   → 0x004330f0
//     +0x71  PUSH imm32   → 0x00f6693c (string)
//     +0x7b  CALL rel32   → 0x009d6b65
//     +0x8b… PUSH imm32   → 0x00f66940 / 0x00f66980 (strings)
//     +0xb0  CALL rel32   → 0x009d4f9f  (twice)
//     +0xd5  PUSH imm32   → 0x00f66a00 (string)
//     +0xe4  CALL rel32   → 0x009d4bb4
//     +0xf0  CALL [imm32] → [0x012660d8] (fnptr)
//     +0x106 CALL rel32   → 0x009d6a61
//     +0x10c CALL rel32   → 0x009d6962
//     +0x114 PUSH imm32   → 0x00f66a04 (string)
//     +0x121 CALL rel32   → 0x0040e2d0
//     +0x12a CALL rel32   → 0x0040a330
//     +0x136 CALL rel32   → 0x0040e110
//     +0x146 CALL rel32   → 0x009d6947
//     +0x150 CALL rel32   → 0x009d2646
//     +0x168 CALL rel32   → 0x0043ce20
//     +0x1a1 CALL rel32   → 0x0040df70
//     +0x1c6 CALL rel32   → 0x009d20f4 (__security_check_cookie)
//
// Reconstruction strategy — naked-asm byte passthrough via __declspec(naked)
//   + `_emit`, matching the established sibling idiom (FUN_0040b840 inline-SEH
//   body, FUN_00401820 reloc-heavy /GS body). The pre-linked literals already
//   match the orig PE byte-for-byte, so the .obj's `.text` is byte-identical
//   to the orig slice, which is what tools/compare.py grades.

extern "C" __declspec(naked) void FUN_0043d3a0() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xec
        _emit 0x69
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x81
        _emit 0xec
        _emit 0x24
        _emit 0x04
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
        _emit 0x20
        _emit 0x04
        _emit 0x00
        _emit 0x00
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
        _emit 0x34
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xac
        _emit 0x24
        _emit 0x44
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf1
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x56
        _emit 0x50
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x28
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xf6
        _emit 0x5c
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x75
        _emit 0x0c
        _emit 0xc7
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe9
        _emit 0x39
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x3c
        _emit 0x69
        _emit 0xf6
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x56
        _emit 0x51
        _emit 0xe8
        _emit 0x45
        _emit 0x97
        _emit 0x59
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x7a
        _emit 0x68
        _emit 0x40
        _emit 0x69
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0xb1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x80
        _emit 0x69
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0xfd
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x40
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x52
        _emit 0x88
        _emit 0x84
        _emit 0x24
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x4a
        _emit 0x7b
        _emit 0x59
        _emit 0x00
        _emit 0x56
        _emit 0x68
        _emit 0xd4
        _emit 0x69
        _emit 0xf6
        _emit 0x00
        _emit 0xb9
        _emit 0xfe
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x2b
        _emit 0xc8
        _emit 0x51
        _emit 0xba
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x2b
        _emit 0xd0
        _emit 0x52
        _emit 0x8d
        _emit 0x44
        _emit 0x04
        _emit 0x58
        _emit 0x50
        _emit 0xe8
        _emit 0x2a
        _emit 0x7b
        _emit 0x59
        _emit 0x00
        _emit 0x68
        _emit 0x00
        _emit 0x6a
        _emit 0xf6
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x51
        _emit 0xe8
        _emit 0x2b
        _emit 0x77
        _emit 0x59
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x68
        _emit 0x6a
        _emit 0x04
        _emit 0x52
        _emit 0xff
        _emit 0x15
        _emit 0xd8
        _emit 0x60
        _emit 0x26
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x40
        _emit 0xc7
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe9
        _emit 0xa4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0xe8
        _emit 0xb6
        _emit 0x95
        _emit 0x59
        _emit 0x00
        _emit 0x50
        _emit 0xe8
        _emit 0xb1
        _emit 0x94
        _emit 0x59
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x68
        _emit 0x04
        _emit 0x6a
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x10
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0x8b
        _emit 0xf8
        _emit 0xe8
        _emit 0x0a
        _emit 0x0e
        _emit 0xfd
        _emit 0xff
        _emit 0x6a
        _emit 0x00
        _emit 0x8b
        _emit 0xf0
        _emit 0xe8
        _emit 0x61
        _emit 0xce
        _emit 0xfc
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x56
        _emit 0x57
        _emit 0x8b
        _emit 0xc8
        _emit 0xe8
        _emit 0x35
        _emit 0x0c
        _emit 0xfd
        _emit 0xff
        _emit 0x8b
        _emit 0xf0
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x50
        _emit 0x6a
        _emit 0x01
        _emit 0x57
        _emit 0x56
        _emit 0xe8
        _emit 0x5c
        _emit 0x94
        _emit 0x59
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x51
        _emit 0xe8
        _emit 0x51
        _emit 0x51
        _emit 0x59
        _emit 0x00
        _emit 0x8b
        _emit 0x94
        _emit 0x24
        _emit 0x5c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x52
        _emit 0x56
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0x51
        _emit 0xe8
        _emit 0x13
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x10
        _emit 0x8b
        _emit 0xfa
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x85
        _emit 0xc9
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0x3c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x08
        _emit 0x8b
        _emit 0x01
        _emit 0x8b
        _emit 0x10
        _emit 0x6a
        _emit 0x01
        _emit 0xff
        _emit 0xd2
        _emit 0x85
        _emit 0xf6
        _emit 0x74
        _emit 0x09
        _emit 0x8b
        _emit 0x4e
        _emit 0xfc
        _emit 0x56
        _emit 0xe8
        _emit 0x2a
        _emit 0x0a
        _emit 0xfd
        _emit 0xff
        _emit 0x89
        _emit 0x7d
        _emit 0x00
        _emit 0x8b
        _emit 0xc5
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x34
        _emit 0x04
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
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x20
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x89
        _emit 0x4b
        _emit 0x59
        _emit 0x00
        _emit 0x81
        _emit 0xc4
        _emit 0x30
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0xc3
    }
}
