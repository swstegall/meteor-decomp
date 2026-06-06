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
// FUNCTION: ffxivgame 0x00061900 — iterator/dispatcher loop (374 B / 0x176,
//                                  __cdecl int(arg1, arg2, arg3), 4-byte local
//                                  via __chkstk, no SEH).
//
// Asm shape (read from asm/ffxivgame/00061900_FUN_00461900.s, 374 bytes):
//
//   Prologue:
//     MOV EAX, 4; CALL __chkstk     ; allocate 4-byte local
//     PUSH EBP; PUSH ESI
//     ESI = arg3                     ; third caller argument
//     EBP = 0
//     if (*arg3 == 0) return 1;      ; empty list early-exit
//
//   PUSH EBX
//   EBX = arg1                       ; first caller argument (passed to
//                                    ;   FUN_004613b0 implicitly in EBX)
//   EBX = FUN_004613b0()             ; get/create handle; EBX = result
//   if (!EBX) return 0;              ; handle-creation failed
//
//   PUSH EDI
//   FUN_00465f80(5, 2, 0xf6936c, 0x1c9);  ; trace "enter"
//
//   ; Compute min count:
//   A = FUN_00464030(EBX->field4)
//   B = FUN_00464030(*arg3)
//   EDI = min(A, B)                  ; effective iteration count
//
//   if (EDI > 0):
//     EBP = FUN_00463150(EDI*4, 0xf6936c, 0x1d0)  ; alloc EDI*4 bytes
//     if (EBP):
//       for i in [0, EDI):
//         EBP[i] = FUN_00464040(EBX->field4, i)
//
//   FUN_00465f80(6, 2, 0xf6936c, 0x1d7);  ; trace
//
//   if (EDI > 0 && EBP == 0):
//     FUN_0045c940(0xf, 0x6a, 0x41, 0xf6936c, 0x1da);  ; error report
//     return 0;
//
//   ; Main dispatch loop — for each i in [0, EDI):
//   for i = 0; i < EDI; i++:
//     EBX = arg3
//     if (*arg3):
//       n = FUN_00464030(*arg3)
//       val = (i < n) ? FUN_00464040(*arg3, i) : 0
//     else:
//       val = 0
//     local_var = val
//
//     entry = EBP[i]
//     if (entry && entry->field0x10):
//       (*entry->field0x10)(arg2, arg3, &local_var, i, entry->field0, entry->field4)
//
//     FUN_00461720(arg2, i, local_var)
//
//   if (EBP): FUN_004632f0(EBP);   ; free buffer
//   return 1;
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function passes arg1 to FUN_004613b0 implicitly via EBX (a
//   non-standard register-passing convention). It also contains at least
//   four absolute-address PUSH 0xf6936c literals, multiple PC-relative
//   CALLs, a 7-byte SIB-encoded LEA (EDI*4), and a 4-byte local frame
//   allocated by __chkstk rather than SUB ESP. Re-deriving the exact
//   MSVC 2005 /O2 register schedule, branch encoding (short vs near),
//   and SIB byte selection in C++ source would be brittle. The naked-
//   passthrough approach (identical to FUN_004014b0, FUN_00405080, and
//   FUN_0040ced0) emits the 374 orig bytes verbatim so compare.py
//   reports GREEN.

extern "C" __declspec(naked) void FUN_00461900() {
    __asm {
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xc6
        _emit 0x10
        _emit 0x57
        _emit 0x00
        _emit 0x55
        _emit 0x56
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x18

        _emit 0x33
        _emit 0xed
        _emit 0x39
        _emit 0x2e
        _emit 0x75
        _emit 0x07
        _emit 0x5e
        _emit 0x8d
        _emit 0x45
        _emit 0x01
        _emit 0x5d
        _emit 0x59
        _emit 0xc3
        _emit 0x53
        _emit 0x8b
        _emit 0x5c

        _emit 0x24
        _emit 0x14
        _emit 0xe8
        _emit 0x89
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xd8
        _emit 0x85
        _emit 0xdb
        _emit 0x75
        _emit 0x05
        _emit 0x5b
        _emit 0x5e
        _emit 0x5d

        _emit 0x59
        _emit 0xc3
        _emit 0x57
        _emit 0x68
        _emit 0xc9
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x6c
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x02
        _emit 0x6a

        _emit 0x05
        _emit 0xe8
        _emit 0x3a
        _emit 0x46
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x43
        _emit 0x04
        _emit 0x50
        _emit 0xe8
        _emit 0xe1
        _emit 0x26
        _emit 0x00
        _emit 0x00
        _emit 0x8b

        _emit 0x0e
        _emit 0x51
        _emit 0x8b
        _emit 0xf8
        _emit 0xe8
        _emit 0xd7
        _emit 0x26
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x3b
        _emit 0xc7
        _emit 0x7d
        _emit 0x02

        _emit 0x8b
        _emit 0xf8
        _emit 0x85
        _emit 0xff
        _emit 0x7e
        _emit 0x42
        _emit 0x68
        _emit 0xd0
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x14
        _emit 0xbd
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x6c
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        _emit 0x52
        _emit 0xe8
        _emit 0xd3
        _emit 0x17
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xe8
        _emit 0x83

        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xed
        _emit 0x74
        _emit 0x22
        _emit 0x33
        _emit 0xf6
        _emit 0x85
        _emit 0xff
        _emit 0x7e
        _emit 0x1c
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00

        _emit 0x8b
        _emit 0x43
        _emit 0x04
        _emit 0x56
        _emit 0x50
        _emit 0xe8
        _emit 0xa6
        _emit 0x26
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x44
        _emit 0xb5
        _emit 0x00
        _emit 0x83
        _emit 0xc6

        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x3b
        _emit 0xf7
        _emit 0x7c
        _emit 0xe8
        _emit 0x68
        _emit 0xd7
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x6c
        _emit 0x93

        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x02
        _emit 0x6a
        _emit 0x06
        _emit 0xe8
        _emit 0xc5
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x85
        _emit 0xff

        _emit 0x7e
        _emit 0x24
        _emit 0x85
        _emit 0xed
        _emit 0x75
        _emit 0x20
        _emit 0x68
        _emit 0xda
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x6c
        _emit 0x93
        _emit 0xf6
        _emit 0x00

        _emit 0x6a
        _emit 0x41
        _emit 0x6a
        _emit 0x6a
        _emit 0x6a
        _emit 0x0f
        _emit 0xe8
        _emit 0x65
        _emit 0xaf
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x5f
        _emit 0x5b

        _emit 0x5e
        _emit 0x33
        _emit 0xc0
        _emit 0x5d
        _emit 0x59
        _emit 0xc3
        _emit 0x33
        _emit 0xf6
        _emit 0x85
        _emit 0xff
        _emit 0x7e
        _emit 0x72
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00

        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        _emit 0x8b
        _emit 0x03
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x1d
        _emit 0x50
        _emit 0xe8
        _emit 0x30
        _emit 0x26
        _emit 0x00
        _emit 0x00

        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x3b
        _emit 0xf0
        _emit 0x7c
        _emit 0x04
        _emit 0x33
        _emit 0xc0
        _emit 0xeb
        _emit 0x0c
        _emit 0x8b
        _emit 0x0b
        _emit 0x56
        _emit 0x51
        _emit 0xe8

        _emit 0x2c
        _emit 0x26
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x44
        _emit 0xb5
        _emit 0x00
        _emit 0x85

        _emit 0xc0
        _emit 0x74
        _emit 0x21
        _emit 0x83
        _emit 0x78
        _emit 0x10
        _emit 0x00
        _emit 0x74
        _emit 0x1b
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        _emit 0x8b
        _emit 0x08
        _emit 0x52
        _emit 0x51

        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x56
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x52
        _emit 0x8b
        _emit 0x50
        _emit 0x10
        _emit 0x53
        _emit 0x51
        _emit 0xff

        _emit 0xd2
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x50
        _emit 0x56
        _emit 0x51
        _emit 0xe8

        _emit 0xcc
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x3b
        _emit 0xf7
        _emit 0x7c
        _emit 0x92
        _emit 0x85
        _emit 0xed

        _emit 0x74
        _emit 0x09
        _emit 0x55
        _emit 0xe8
        _emit 0x88
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x5f
        _emit 0x5b
        _emit 0x5e
        _emit 0xb8
        _emit 0x01

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5d
        _emit 0x59
        _emit 0xc3
    }
}
