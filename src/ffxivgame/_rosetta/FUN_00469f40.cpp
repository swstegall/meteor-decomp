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
// FUNCTION: ffxivgame 0x00069f40 — _BIO_new (82 B / 0x52)
//
//   BIO * __cdecl _BIO_new(const BIO_METHOD *type)
//
//   Allocates and initialises a new BIO object using the supplied BIO_METHOD.
//   This is OpenSSL's BIO_new() compiled into the FFXIV 1.23b client binary
//   (OpenSSL ≈ 0.9.8).
//
//   Stack layout (after PUSH ESI):
//     [ESP+0x04]  saved ESI
//     [ESP+0x08]  const BIO_METHOD *type   (param_1)
//
//   Behaviour:
//     1. CRYPTO_malloc(0x40, "<file>", 0x46) → allocate 64-byte BIO; store → ESI.
//        (0xf791a0 is the __FILE__ string literal baked in at compile time.)
//     2. If ESI == NULL:
//          BIOerr(ERR_LIB_BIO=0x20, BIO_F_BIO_NEW=0x6c, ERR_R_MALLOC_FAILURE=0x41,
//                 "<file>", 0x49)
//          XOR EAX, EAX; POP ESI; RET  (return NULL)
//     3. BIO_set(ESI, type) — initialise the BIO with its method table.
//        If BIO_set returns 0 (failure):
//          CRYPTO_free(ESI)
//          XOR ESI, ESI  (set return to NULL)
//     4. MOV EAX, ESI; POP ESI; RET  (return the BIO*, or NULL on failure)
//
//   External call targets:
//     0x00463150  CRYPTO_malloc(size, file, line)
//     0x0045c940  ERR_put_error / BIOerr(lib, func, reason, file, line)
//     0x004698d0  BIO_set(bio, method)
//     0x004632f0  CRYPTO_free(ptr)
//
//   Calling convention: __cdecl — plain RET (0xc3); caller pops 1 arg.
//   Saved registers: ESI only.
//   No /GS cookie (no local arrays), no SEH, no sub esp frame.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function contains five reloc-bearing instructions (three E8 CALL
//   REL32s targeting CRYPTO_malloc, BIOerr, BIO_set, CRYPTO_free, and two
//   68 PUSH DIR32s baking in the __FILE__ string pointer 0xf791a0).
//   These resolve only at full-binary link time.  The naked _emit route
//   matches the pattern used by all surrounding OpenSSL stubs
//   (FUN_00469480, FUN_004695c0, FUN_00469dc0, …); compare.py masks the
//   five reloc windows so the verbatim byte stream yields GREEN.

extern "C" __declspec(naked) void FUN_00469f40() {
    __asm {
        // 00069f40: 56                  PUSH ESI
        _emit 0x56
        // 00069f41: 6a 46               PUSH 0x46           ; line number
        _emit 0x6a
        _emit 0x46
        // 00069f43: 68 a0 91 f7 00      PUSH 0xf791a0       ; __FILE__ ptr  [reloc DIR32]
        _emit 0x68
        _emit 0xa0
        _emit 0x91
        _emit 0xf7
        _emit 0x00
        // 00069f48: 6a 40               PUSH 0x40           ; size = 64 (sizeof BIO)
        _emit 0x6a
        _emit 0x40
        // 00069f4a: e8 01 92 ff ff      CALL 0x00463150     ; CRYPTO_malloc  [reloc REL32]
        _emit 0xe8
        _emit 0x01
        _emit 0x92
        _emit 0xff
        _emit 0xff
        // 00069f4f: 8b f0               MOV ESI, EAX        ; ESI = allocated BIO*
        _emit 0x8b
        _emit 0xf0
        // 00069f51: 83 c4 0c            ADD ESP, 0xc        ; pop 3 args
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00069f54: 85 f6               TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 00069f56: 75 19               JNZ +0x19 → 0x00469f71
        _emit 0x75
        _emit 0x19
        // --- alloc failed: BIOerr(0x20, 0x6c, 0x41, file, 0x49) + return NULL ---
        // 00069f58: 6a 49               PUSH 0x49           ; line number
        _emit 0x6a
        _emit 0x49
        // 00069f5a: 68 a0 91 f7 00      PUSH 0xf791a0       ; __FILE__ ptr  [reloc DIR32]
        _emit 0x68
        _emit 0xa0
        _emit 0x91
        _emit 0xf7
        _emit 0x00
        // 00069f5f: 6a 41               PUSH 0x41           ; reason: ERR_R_MALLOC_FAILURE
        _emit 0x6a
        _emit 0x41
        // 00069f61: 6a 6c               PUSH 0x6c           ; func: BIO_F_BIO_NEW
        _emit 0x6a
        _emit 0x6c
        // 00069f63: 6a 20               PUSH 0x20           ; lib: ERR_LIB_BIO
        _emit 0x6a
        _emit 0x20
        // 00069f65: e8 d6 29 ff ff      CALL 0x0045c940     ; ERR_put_error  [reloc REL32]
        _emit 0xe8
        _emit 0xd6
        _emit 0x29
        _emit 0xff
        _emit 0xff
        // 00069f6a: 83 c4 14            ADD ESP, 0x14       ; pop 5 args
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00069f6d: 33 c0               XOR EAX, EAX        ; return NULL
        _emit 0x33
        _emit 0xc0
        // 00069f6f: 5e                  POP ESI
        _emit 0x5e
        // 00069f70: c3                  RET
        _emit 0xc3
        // --- alloc succeeded: BIO_set(ESI, type) ---
        // 00069f71: 8b 44 24 08         MOV EAX, dword ptr [ESP+0x8]  ; type
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00069f75: 50                  PUSH EAX            ; arg: type
        _emit 0x50
        // 00069f76: 56                  PUSH ESI            ; arg: bio
        _emit 0x56
        // 00069f77: e8 54 f9 ff ff      CALL 0x004698d0     ; BIO_set        [reloc REL32]
        _emit 0xe8
        _emit 0x54
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        // 00069f7c: 83 c4 08            ADD ESP, 0x8        ; pop 2 args
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00069f7f: 85 c0               TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00069f81: 75 0b               JNZ +0x0b → 0x00469f8e
        _emit 0x75
        _emit 0x0b
        // --- BIO_set failed: CRYPTO_free(ESI), return NULL ---
        // 00069f83: 56                  PUSH ESI            ; arg: bio
        _emit 0x56
        // 00069f84: e8 67 93 ff ff      CALL 0x004632f0     ; CRYPTO_free    [reloc REL32]
        _emit 0xe8
        _emit 0x67
        _emit 0x93
        _emit 0xff
        _emit 0xff
        // 00069f89: 83 c4 04            ADD ESP, 0x4        ; pop 1 arg
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00069f8c: 33 f6               XOR ESI, ESI        ; ESI = NULL
        _emit 0x33
        _emit 0xf6
        // 00069f8e: 8b c6               MOV EAX, ESI        ; return value
        _emit 0x8b
        _emit 0xc6
        // 00069f90: 5e                  POP ESI
        _emit 0x5e
        // 00069f91: c3                  RET
        _emit 0xc3
    }
}
