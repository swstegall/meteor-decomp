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
// FUNCTION: ffxivgame 0x005e69cc — register-convention leaf that tests
//                                  bit 0x80000 of EAX (caller-supplied),
//                                  returns 7 if set, otherwise adds a
//                                  global double to ST(0) and returns 1
//                                  (25 B / 0x19).
//
// Calling convention: non-standard (EAX used as input at entry, bare RET).
// Written as __declspec(naked) to reproduce the exact byte sequence.
//
// Asm (25 bytes):
//   a9 00 00 08 00       TEST EAX, 0x80000
//   74 06                JZ   +6  (→ 0x009e69d9)
//   b8 07 00 00 00       MOV  EAX, 7
//   c3                   RET
//   dc 05 70 87 08 01    FADD qword ptr [0x01088770]   ; adds global double to ST(0)
//   b8 01 00 00 00       MOV  EAX, 1
//   c3                   RET

extern "C" double g_double_01088770;

extern "C" __declspec(naked) void FUN_009e69cc() {
    __asm {
        test    eax, 0x80000
        jz      skip
        mov     eax, 7
        ret
    skip:
        fadd    qword ptr [g_double_01088770]
        mov     eax, 1
        ret
    }
}
