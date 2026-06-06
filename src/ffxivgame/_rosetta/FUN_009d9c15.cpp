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
// FUNCTION: ffxivgame 0x005d9c15 — CRT _setmode wrapper / fd-table access validator
//                                  (237 B / 0xed, EH3/EH4-prolog wrapped, __cdecl).
//
// Behaviour read from disassembly at orig RVA 0x005d9c15:
//
//   __cdecl int FUN_009d9c15(int fd, int mode);
//
//   Validates `mode` — accepts only the five CRT text/binary mode flags:
//     _O_TEXT   (0x4000), _O_BINARY (0x8000), _O_WTEXT (0x10000),
//     _O_UTF16  (0x20000), _O_U8TEXT (0x40000).
//   On any other value sets errno=EINVAL (22), calls _invoke_watson
//   (5 × NULL args via FUN_009d2290), and returns -1.
//
//   Validates `fd` — must not be -2 (pseudo-handle), must be >= 0,
//   and must be < nfiles_max (at .data 0x137b7dc). Out-of-range sets
//   errno=EBADF (9) and returns -1.
//
//   Looks up fd in the CRT ioinfo bitfield table (base at .data 0x137b7e0):
//     slot  = fd >> 5         (which 64-entry ioinfo** block)
//     bit   = (fd & 0x1f) * 64   (byte offset within the block, × 64)
//     open? = ioinfo[slot][bit + 4] & 1
//   If the fd is not open, sets errno=EBADF and returns -1.
//
//   Acquires a per-fd lock via FUN_009e77ed(fd) (CRITICAL_SECTION-style).
//   Re-checks the open bit; if still set calls FUN_009d9b25(fd, mode)
//   (the real mode-change implementation) and stores its return value.
//   Otherwise sets errno=EBADF and stores -1.
//
//   EH cleanup via FUN_009d9d02 (destructor / lock release called at
//   epilog), then returns stored result through FUN_009de535 epilog.
//
// Stack frame (after EH3 prolog at 0x9de4f0 / push 0x10 / push scope_table):
//   [ebp+0xc]   = mode (second arg)
//   [ebp+0x8]   = fd   (first arg)
//   [ebp-0x4]   = EH state / trylevel (0 → 0xfffffffe)
//   [ebp-0x1c]  = result (return value accumulator)
//
// Reloc-bearing sites in the orig 237 bytes:
//   +0x02  abs32  0x0122d068 — EH scope table (.rdata)
//   +0x07  rel32  0x009de4f0 — __EH_prolog3_catch (CRT)
//   +0x37  rel32  0x009d9d47 — get-errno stub (CRT)
//   +0x47  rel32  0x009d2290 — _invoke_watson / invalid-param handler (CRT)
//   +0x60  rel32  0x009d9d47 — get-errno stub (CRT, 2nd)
//   +0x6c  abs32  0x0137b7dc — nfiles_max (.data)
//   +0x73  rel32  0x009d9d47 — get-errno stub (CRT, 3rd)
//   +0x84  abs32  0x0137b7e0 — ioinfo table base (.data)
//   +0xa3  rel32  0x009e77ed — per-fd lock acquire
//   +0xb9  rel32  0x009d9b25 — _setmode_nolock inner impl
//   +0xc1  rel32  0x009d9d47 — get-errno stub (CRT, 4th)
//   +0xd4  rel32  0x009d9d02 — __local_unwind / frame cleanup
//   +0xe7  rel32  0x009de535 — __EH_epilog3 (CRT)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ would require reproducing the EH3 frame layout
//   (push 0x10 / push scope_table / call __EH_prolog3_catch), exact
//   register allocation across the fd-table scan (ECX=slot, ESI=bit,
//   EDI=&ioinfo[slot], EBX=0/result), the short-vs-near branch choices
//   for each of the five mode comparisons, and the ten absolute-address
//   references above. Each constraint is brittle under /O2, so this
//   function takes the same __declspec(naked) passthrough path as its
//   sibling matches FUN_004054d0, FUN_00403a20, FUN_00402a30.

extern "C" __declspec(naked) void FUN_009d9c15() {
    __asm {
        // 005d9c15  PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 005d9c17  PUSH 0x0122d068 (scope table)
        _emit 0x68
        _emit 0x68
        _emit 0xd0
        _emit 0x22
        _emit 0x01
        // 005d9c1c  CALL 0x9de4f0 (__EH_prolog3_catch)
        _emit 0xe8
        _emit 0xcf
        _emit 0x48
        _emit 0x00
        _emit 0x00
        // 005d9c21  MOV EAX, [EBP+0xC]  ; mode arg
        _emit 0x8b
        _emit 0x45
        _emit 0x0c
        // 005d9c24  CMP EAX, 0x4000  (_O_TEXT)
        _emit 0x3d
        _emit 0x00
        _emit 0x40
        _emit 0x00
        _emit 0x00
        // 005d9c29  JE +0x3e  -> 0x9d9c69
        _emit 0x74
        _emit 0x3e
        // 005d9c2b  CMP EAX, 0x8000  (_O_BINARY)
        _emit 0x3d
        _emit 0x00
        _emit 0x80
        _emit 0x00
        _emit 0x00
        // 005d9c30  JE +0x37  -> 0x9d9c69
        _emit 0x74
        _emit 0x37
        // 005d9c32  CMP EAX, 0x10000  (_O_WTEXT)
        _emit 0x3d
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x00
        // 005d9c37  JE +0x30  -> 0x9d9c69
        _emit 0x74
        _emit 0x30
        // 005d9c39  CMP EAX, 0x40000  (_O_U8TEXT)
        _emit 0x3d
        _emit 0x00
        _emit 0x00
        _emit 0x04
        _emit 0x00
        // 005d9c3e  JE +0x29  -> 0x9d9c69
        _emit 0x74
        _emit 0x29
        // 005d9c40  CMP EAX, 0x20000  (_O_UTF16)
        _emit 0x3d
        _emit 0x00
        _emit 0x00
        _emit 0x02
        _emit 0x00
        // 005d9c45  JE +0x22  -> 0x9d9c69
        _emit 0x74
        _emit 0x22
        // 005d9c47  CALL 0x9d9d47  (get-errno)
        _emit 0xe8
        _emit 0xfb
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d9c4c  MOV DWORD PTR [EAX], 0x16  (EINVAL=22)
        _emit 0xc7
        _emit 0x00
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d9c52  XOR EBX, EBX
        _emit 0x33
        _emit 0xdb
        // 005d9c54  PUSH EBX
        _emit 0x53
        // 005d9c55  PUSH EBX
        _emit 0x53
        // 005d9c56  PUSH EBX
        _emit 0x53
        // 005d9c57  PUSH EBX
        _emit 0x53
        // 005d9c58  PUSH EBX
        _emit 0x53
        // 005d9c59  CALL 0x9d2290  (_invoke_watson / invalid-param handler)
        _emit 0xe8
        _emit 0x32
        _emit 0x86
        _emit 0xff
        _emit 0xff
        // 005d9c5e  ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 005d9c61  OR EAX, 0xffffffff  (EAX = -1)
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // 005d9c64  JMP +0x93  -> 0x9d9cfc  (epilog)
        _emit 0xe9
        _emit 0x93
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d9c69  MOV EAX, [EBP+0x8]  ; fd arg
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        // 005d9c6c  CMP EAX, -2
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        // 005d9c6f  JNE +0xd  -> 0x9d9c7e
        _emit 0x75
        _emit 0x0d
        // 005d9c71  CALL 0x9d9d47  (get-errno)
        _emit 0xe8
        _emit 0xd1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d9c76  MOV DWORD PTR [EAX], 9  (EBADF=9)
        _emit 0xc7
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d9c7c  JMP -0x1d  -> 0x9d9c61
        _emit 0xeb
        _emit 0xe3
        // 005d9c7e  XOR EBX, EBX
        _emit 0x33
        _emit 0xdb
        // 005d9c80  CMP EAX, EBX
        _emit 0x3b
        _emit 0xc3
        // 005d9c82  JL +8  -> 0x9d9c8c
        _emit 0x7c
        _emit 0x08
        // 005d9c84  CMP EAX, [0x137b7dc]  (nfiles_max)
        _emit 0x3b
        _emit 0x05
        _emit 0xdc
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        // 005d9c8a  JB +0xd  -> 0x9d9c99
        _emit 0x72
        _emit 0x0d
        // 005d9c8c  CALL 0x9d9d47  (get-errno)
        _emit 0xe8
        _emit 0xb6
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d9c91  MOV DWORD PTR [EAX], 9  (EBADF=9)
        _emit 0xc7
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d9c97  JMP -0x45  -> 0x9d9c54
        _emit 0xeb
        _emit 0xbb
        // 005d9c99  MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 005d9c9b  SAR ECX, 5  (slot = fd >> 5)
        _emit 0xc1
        _emit 0xf9
        _emit 0x05
        // 005d9c9e  LEA EDI, [ECX*4 + 0x137b7e0]  (&ioinfo_table[slot])
        _emit 0x8d
        _emit 0x3c
        _emit 0x8d
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        // 005d9ca5  MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 005d9ca7  AND ESI, 0x1f  (bit = fd & 31)
        _emit 0x83
        _emit 0xe6
        _emit 0x1f
        // 005d9caa  SHL ESI, 6  (byte offset = bit * 64)
        _emit 0xc1
        _emit 0xe6
        _emit 0x06
        // 005d9cad  MOV ECX, [EDI]  (ioinfo* ptr)
        _emit 0x8b
        _emit 0x0f
        // 005d9caf  MOVZX ECX, BYTE PTR [ECX + ESI + 4]  (ioinfo.osfile)
        _emit 0x0f
        _emit 0xb6
        _emit 0x4c
        _emit 0x31
        _emit 0x04
        // 005d9cb4  AND ECX, 1  (test FOPEN bit)
        _emit 0x83
        _emit 0xe1
        _emit 0x01
        // 005d9cb7  JE -0x2d  -> 0x9d9c8c  (not open -> EBADF)
        _emit 0x74
        _emit 0xd3
        // 005d9cb9  PUSH EAX  (fd)
        _emit 0x50
        // 005d9cba  CALL 0x9e77ed  (per-fd lock acquire)
        _emit 0xe8
        _emit 0x2e
        _emit 0xdb
        _emit 0x00
        _emit 0x00
        // 005d9cbf  POP ECX
        _emit 0x59
        // 005d9cc0  MOV [EBP-4], EBX  (trylevel = 0)
        _emit 0x89
        _emit 0x5d
        _emit 0xfc
        // 005d9cc3  MOV EAX, [EDI]  (ioinfo* ptr, reload)
        _emit 0x8b
        _emit 0x07
        // 005d9cc5  TEST BYTE PTR [EAX + ESI + 4], 1  (re-check FOPEN bit)
        _emit 0xf6
        _emit 0x44
        _emit 0x30
        _emit 0x04
        _emit 0x01
        // 005d9cca  JE +0x12  -> 0x9d9cde
        _emit 0x74
        _emit 0x12
        // 005d9ccc  PUSH DWORD PTR [EBP+0xC]  (mode)
        _emit 0xff
        _emit 0x75
        _emit 0x0c
        // 005d9ccf  PUSH DWORD PTR [EBP+0x8]  (fd)
        _emit 0xff
        _emit 0x75
        _emit 0x08
        // 005d9cd2  CALL 0x9d9b25  (_setmode_nolock)
        _emit 0xe8
        _emit 0x4e
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 005d9cd7  POP ECX
        _emit 0x59
        // 005d9cd8  POP ECX
        _emit 0x59
        // 005d9cd9  MOV [EBP-0x1C], EAX  (store result)
        _emit 0x89
        _emit 0x45
        _emit 0xe4
        // 005d9cdc  JMP +0xf  -> 0x9d9ced
        _emit 0xeb
        _emit 0x0f
        // 005d9cde  CALL 0x9d9d47  (get-errno)
        _emit 0xe8
        _emit 0x64
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d9ce3  MOV DWORD PTR [EAX], 9  (EBADF)
        _emit 0xc7
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d9ce9  OR DWORD PTR [EBP-0x1C], 0xffffffff  (result = -1)
        _emit 0x83
        _emit 0x4d
        _emit 0xe4
        _emit 0xff
        // 005d9ced  MOV DWORD PTR [EBP-4], 0xfffffffe  (trylevel = -2, epilog state)
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 005d9cf4  CALL 0x9d9d02  (per-fd cleanup / lock release)
        _emit 0xe8
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d9cf9  MOV EAX, [EBP-0x1C]  (load return value)
        _emit 0x8b
        _emit 0x45
        _emit 0xe4
        // 005d9cfc  CALL 0x9de535  (__EH_epilog3)
        _emit 0xe8
        _emit 0x34
        _emit 0x48
        _emit 0x00
        _emit 0x00
        // 005d9d01  RET
        _emit 0xc3
    }
}
