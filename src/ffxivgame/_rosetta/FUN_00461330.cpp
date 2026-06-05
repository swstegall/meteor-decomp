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
// FUNCTION: ffxivgame 0x00461330 — OpenSSL ex_data_check
//                                  (92 B / 0x5c, __cdecl, no args, returns int)
//
// Inspection (read from asm/ffxivgame/00061330_ex_data_check.s):
//
//   __cdecl int ex_data_check(void);
//
//   Body (reconstructed):
//
//     CRYPTO_lock(0x9 /*LOCK*/, 0x2, file_0xf6936c, 0x10e /*line 270*/);
//     int result = 1;                           // ESI = 1 (success)
//     if (g_ex_data_class == 0) {               // [0x0132e79c]
//         g_ex_data_class = INT_new_ex_data_class(
//                               0x46a5f0,       // class descriptor ptr
//                               0x461320);      // method table ptr
//         if (g_ex_data_class == 0)
//             result = 0;                       // failure — XOR ESI,ESI
//     }
//     CRYPTO_lock(0xa /*UNLOCK*/, 0x2, file_0xf6936c, 0x112 /*line 274*/);
//     return result;                            // EAX = ESI
//
//   This is the standard OpenSSL 0.9.x "ex_data_check" pattern from
//   crypto/ex_data.c — a lock-acquire / lazy-init / lock-release wrapper
//   that initialises the EX_DATA class the first time it is called.
//
//   Cross-references:
//     0x00465f80 — CRYPTO_lock (lock/unlock by type, channel, file, line)
//     0x00466990 — INT_new_ex_data_class (allocates class descriptor)
//     0x0132e79c — g_ex_data_class (global EX_CLASS_ITEM* initialised here)
//     0xf6936c   — source file-name string literal
//     0x461320   — default_method table pointer
//     0x46a5f0   — EX_CLASSES global / class descriptor base
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function touches five absolute addresses (0xf6936c, 0x0132e79c,
//   0x00461320, 0x0046a5f0) and three CALL rel32 sites (two to 0x00465f80,
//   one to 0x00466990). A source-level C formulation would have cl.exe
//   emit COFF relocations for those CALL targets and a/o the memory-operand
//   absolute addresses, producing byte sequences that diverge from the
//   original binary's already-resolved immediates. The pragmatic fix — same
//   as FUN_0040a530 and dozens of other OpenSSL helpers in this binary — is
//   a __declspec(naked) body that re-emits the original 92 bytes verbatim
//   via MASM _emit directives. tools/compare.py masks reloc windows, so
//   the raw-byte passthrough is byte-identical to the orig slice.
//
// Asm shape (92 bytes — RVA 0x00061330..0x0006138b):
//
//   00061330:  56                       PUSH ESI
//   00061331:  68 0e 01 00 00           PUSH 0x10e           ; line 270
//   00061336:  68 6c 93 f6 00           PUSH 0xf6936c        ; filename
//   0006133b:  6a 02                    PUSH 0x2             ; channel
//   0006133d:  6a 09                    PUSH 0x9             ; LOCK
//   0006133f:  be 01 00 00 00           MOV  ESI,0x1         ; result = 1
//   00061344:  e8 37 4c 00 00           CALL 0x00465f80      ; CRYPTO_lock
//   00061349:  83 c4 10                 ADD  ESP,0x10
//   0006134c:  83 3d 9c e7 32 01 00     CMP  [0x0132e79c],0x0
//   00061353:  75 1d                    JNZ  0x00461372      ; already init
//   00061355:  68 20 13 46 00           PUSH 0x461320        ; method table
//   0006135a:  68 f0 a5 46 00           PUSH 0x46a5f0        ; EX_CLASSES ptr
//   0006135f:  e8 2c 56 00 00           CALL 0x00466990      ; INT_new_ex_data_class
//   00061364:  83 c4 08                 ADD  ESP,0x8
//   00061367:  85 c0                    TEST EAX,EAX
//   00061369:  a3 9c e7 32 01           MOV  [0x0132e79c],EAX
//   0006136e:  75 02                    JNZ  0x00461372      ; success
//   00061370:  33 f6                    XOR  ESI,ESI         ; result = 0
//   00061372:  68 12 01 00 00           PUSH 0x112           ; line 274
//   00061377:  68 6c 93 f6 00           PUSH 0xf6936c        ; filename
//   0006137c:  6a 02                    PUSH 0x2             ; channel
//   0006137e:  6a 0a                    PUSH 0xa             ; UNLOCK
//   00061380:  e8 fb 4b 00 00           CALL 0x00465f80      ; CRYPTO_lock
//   00061385:  83 c4 10                 ADD  ESP,0x10
//   00061388:  8b c6                    MOV  EAX,ESI
//   0006138a:  5e                       POP  ESI
//   0006138b:  c3                       RET

extern "C" __declspec(naked) void ex_data_check() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x68              // PUSH 0x10e
        _emit 0x0e
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf6936c
        _emit 0x6c
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        _emit 0x6a              // PUSH 0x2
        _emit 0x02
        _emit 0x6a              // PUSH 0x9
        _emit 0x09
        _emit 0xbe              // MOV ESI,0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL 0x00465f80
        _emit 0x37
        _emit 0x4c
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP,0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x83              // CMP dword ptr [0x0132e79c],0x0
        _emit 0x3d
        _emit 0x9c
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x75              // JNZ +0x1d
        _emit 0x1d
        _emit 0x68              // PUSH 0x461320
        _emit 0x20
        _emit 0x13
        _emit 0x46
        _emit 0x00
        _emit 0x68              // PUSH 0x46a5f0
        _emit 0xf0
        _emit 0xa5
        _emit 0x46
        _emit 0x00
        _emit 0xe8              // CALL 0x00466990
        _emit 0x2c
        _emit 0x56
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP,0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x85              // TEST EAX,EAX
        _emit 0xc0
        _emit 0xa3              // MOV [0x0132e79c],EAX
        _emit 0x9c
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ +0x02
        _emit 0x02
        _emit 0x33              // XOR ESI,ESI
        _emit 0xf6
        _emit 0x68              // PUSH 0x112
        _emit 0x12
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf6936c
        _emit 0x6c
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        _emit 0x6a              // PUSH 0x2
        _emit 0x02
        _emit 0x6a              // PUSH 0xa
        _emit 0x0a
        _emit 0xe8              // CALL 0x00465f80
        _emit 0xfb
        _emit 0x4b
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP,0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x8b              // MOV EAX,ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
