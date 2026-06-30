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
// FUNCTION: ffxivgame 0x00069ac0 — OpenSSL _BIO_write
//                                  (__cdecl int BIO_write(BIO *, const void *, int), 178 B)
//
// Standard OpenSSL BIO write dispatcher. Calling convention: __cdecl.
// Register allocation (from asm):
//   ESI = b        (BIO *,      arg1)
//   EDI = callback (long (*)(BIO *, int, const char *, int, long, long), from b->callback)
//   EBX = inl      (int,        arg3)
//   EBP = in       (const void *, arg2)
//
// Control flow:
//   1. if (b == NULL) return 0;
//   2. Load b->method (EAX) and b->callback (EDI).
//      if (!b->method || !b->method->bwrite) → BIO_R_UNSUPPORTED_METHOD error (→ step 8).
//   3. Load arg3 (inl→EBX) and arg2 (in→EBP).
//      if (callback != NULL) → pre-write callback(b, BIO_CB_WRITE=3, in, inl, 0L, 1L).
//         if returned i <= 0, return i.
//   4. if (!b->init) → BIO_R_UNINITIALIZED error, return -2.
//   5. i = b->method->bwrite(b, in, inl).
//      if (i > 0) b->num_write += i.
//   6. if (callback != NULL) → post-write callback(b, BIO_CB_WRITE|BIO_CB_RETURN=0x83,
//                                                   in, inl, 0L, (long)i).
//   7. return i.
//   8. ERR_put_error(ERR_LIB_BIO=0x20, BIO_F_BIO_WRITE=0x71,
//                   BIO_R_UNSUPPORTED_METHOD=0x79, "bio_lib.c", 0xe9=233);
//      return -2.
//
// OpenSSL constants observed:
//   ERR_LIB_BIO           = 0x20 = 32
//   BIO_F_BIO_WRITE       = 0x71 = 113
//   BIO_R_UNSUPPORTED_METHOD = 0x79 = 121    (line 233 = 0xe9 in bio_lib.c)
//   BIO_R_UNINITIALIZED   = 0x78 = 120       (line 243 = 0xf3 in bio_lib.c)
//   BIO_CB_WRITE          = 0x03
//   BIO_CB_WRITE|BIO_CB_RETURN = 0x83
//
// BIO struct offsets used:
//   +0x00  BIO_METHOD *method
//   +0x04  long (*callback)(BIO *, int, const char *, int, long, long)
//   +0x0C  int init
//   +0x34  unsigned long num_write
//
// BIO_METHOD struct offsets used:
//   +0x08  int (*bwrite)(BIO *, const char *, int)
//
// Reloc-bearing sites (4-byte windows wildcarded by compare.py):
//   +0x48   PUSH imm32 → 0xf791a0  ("bio_lib.c" string, DIR32)
//   +0x4d   CALL rel32 → 0x0045c940  (ERR_put_error)
//   +0x97   PUSH imm32 → 0xf791a0  ("bio_lib.c" string, DIR32)
//   +0x9c   CALL rel32 → 0x0045c940  (ERR_put_error)

extern "C" __declspec(naked) void FUN_00469ac0() {
    __asm {
        // +0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x8]    (b = arg1)
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75              // JNZ +0x04   (b != NULL → continue)
        _emit 0x04
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET          (return 0)
        // +0x0d
        _emit 0x8b              // MOV EAX, dword ptr [ESI]        (b->method)
        _emit 0x06
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESI+0x4]    (b->callback)
        _emit 0x7e
        _emit 0x04
        _emit 0x74              // JZ  +0x7b   (method==NULL → error path)
        _emit 0x7b
        _emit 0x83              // CMP dword ptr [EAX+0x8], 0      (bwrite == NULL?)
        _emit 0x78
        _emit 0x08
        _emit 0x00
        _emit 0x74              // JZ  +0x75   (bwrite==NULL → error path)
        _emit 0x75
        // +0x1d
        _emit 0x85              // TEST EDI, EDI                   (callback != NULL?)
        _emit 0xff
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x18]   (inl = arg3)
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x18]   (in = arg2)
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x74              // JZ  +0x12   (no callback → skip pre-call)
        _emit 0x12
        // +0x2b  pre-write callback: cb(b, BIO_CB_WRITE=3, in, inl, 0L, 1L)
        _emit 0x6a              // PUSH 0x1    (arg6 = 1L)
        _emit 0x01
        _emit 0x6a              // PUSH 0x0    (arg5 = 0L)
        _emit 0x00
        _emit 0x53              // PUSH EBX    (arg4 = inl)
        _emit 0x55              // PUSH EBP    (arg3 = in)
        _emit 0x6a              // PUSH 0x3    (arg2 = BIO_CB_WRITE)
        _emit 0x03
        _emit 0x56              // PUSH ESI    (arg1 = b)
        _emit 0xff              // CALL EDI
        _emit 0xd7
        _emit 0x83              // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7e              // JLE +0x23   (i <= 0 → early return with i)
        _emit 0x23
        // +0x3d  check b->init
        _emit 0x83              // CMP dword ptr [ESI+0xc], 0      (b->init)
        _emit 0x7e
        _emit 0x0c
        _emit 0x00
        _emit 0x75              // JNZ +0x22   (init set → proceed to bwrite)
        _emit 0x22
        // +0x43  BIO_R_UNINITIALIZED error: ERR_put_error(32, 113, 120, file, 243)
        _emit 0x68              // PUSH 0xf3   (arg5 = line 243 = 0xf3)
        _emit 0xf3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH file   (arg4 = "bio_lib.c", DIR32 reloc)
        _emit 0xa0
        _emit 0x91
        _emit 0xf7
        _emit 0x00
        _emit 0x6a              // PUSH 0x78   (arg3 = BIO_R_UNINITIALIZED = 120)
        _emit 0x78
        _emit 0x6a              // PUSH 0x71   (arg2 = BIO_F_BIO_WRITE = 113)
        _emit 0x71
        _emit 0x6a              // PUSH 0x20   (arg1 = ERR_LIB_BIO = 32)
        _emit 0x20
        _emit 0xe8              // CALL ERR_put_error  (rel32 → 0x0045c940)
        _emit 0x28
        _emit 0x2e
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xb8              // MOV EAX, 0xfffffffe  (= -2)
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // +0x60  common epilogue (also reached from JLE path above)
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        // +0x65  do_write: i = b->method->bwrite(b, in, inl)
        _emit 0x8b              // MOV EAX, dword ptr [ESI]        (b->method)
        _emit 0x06
        _emit 0x8b              // MOV ECX, dword ptr [EAX+0x8]   (bwrite fn ptr)
        _emit 0x48
        _emit 0x08
        _emit 0x53              // PUSH EBX    (arg3 = inl)
        _emit 0x55              // PUSH EBP    (arg2 = in)
        _emit 0x56              // PUSH ESI    (arg1 = b)
        _emit 0xff              // CALL ECX    (b->method->bwrite)
        _emit 0xd1
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7e              // JLE +0x03   (i <= 0 → skip num_write update)
        _emit 0x03
        _emit 0x01              // ADD dword ptr [ESI+0x34], EAX  (b->num_write += i)
        _emit 0x46
        _emit 0x34
        _emit 0x85              // TEST EDI, EDI                   (callback != NULL?)
        _emit 0xff
        _emit 0x74              // JZ  -0x1d   (no callback → common epilogue)
        _emit 0xe3
        // +0x7d  post-write callback: cb(b, BIO_CB_WRITE|BIO_CB_RETURN=0x83, in, inl, 0L, i)
        _emit 0x50              // PUSH EAX    (arg6 = i)
        _emit 0x6a              // PUSH 0x0    (arg5 = 0L)
        _emit 0x00
        _emit 0x53              // PUSH EBX    (arg4 = inl)
        _emit 0x55              // PUSH EBP    (arg3 = in)
        _emit 0x68              // PUSH 0x83   (arg2 = BIO_CB_WRITE|BIO_CB_RETURN = 0x83)
        _emit 0x83
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI    (arg1 = b)
        _emit 0xff              // CALL EDI    (callback)
        _emit 0xd7
        _emit 0x83              // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        // +0x92  error path: BIO_R_UNSUPPORTED_METHOD (method/bwrite == NULL)
        //        only EDI and ESI were pushed at this point
        _emit 0x68              // PUSH 0xe9   (arg5 = line 233 = 0xe9)
        _emit 0xe9
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH file   (arg4 = "bio_lib.c", DIR32 reloc)
        _emit 0xa0
        _emit 0x91
        _emit 0xf7
        _emit 0x00
        _emit 0x6a              // PUSH 0x79   (arg3 = BIO_R_UNSUPPORTED_METHOD = 121)
        _emit 0x79
        _emit 0x6a              // PUSH 0x71   (arg2 = BIO_F_BIO_WRITE = 113)
        _emit 0x71
        _emit 0x6a              // PUSH 0x20   (arg1 = ERR_LIB_BIO = 32)
        _emit 0x20
        _emit 0xe8              // CALL ERR_put_error  (rel32 → 0x0045c940)
        _emit 0xd9
        _emit 0x2d
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x5f              // POP EDI
        _emit 0xb8              // MOV EAX, 0xfffffffe  (= -2)
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
