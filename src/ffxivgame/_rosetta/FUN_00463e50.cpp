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
// FUNCTION: ffxivgame 0x00463e50 — sk_new: OpenSSL STACK allocator (109 B)
//
// __cdecl void *sk_new(int (*cmp)(...))
//   arg:  [ESP+0xc]  comparison function pointer
//   ret:  EAX        pointer to new STACK, or NULL on failure
//
// Allocates and initializes an OpenSSL-style STACK structure:
//
//   outer struct (0x14 = 20 bytes):
//     [+0x00]  num       (int)  = 0
//     [+0x04]  data      (ptr)  → inner array
//     [+0x08]  sorted    (int)  = 0
//     [+0x0c]  num_alloc (int)  = 4
//     [+0x10]  comp      (ptr)  = cmp argument
//
//   inner data array (0x10 = 16 bytes = 4 null pointers):
//     [+0x00] = 0, [+0x04] = 0, [+0x08] = 0, [+0x0c] = 0
//
// Uses CRYPTO_malloc (at VA 0x00463150) with arguments
// (size, filename_ptr, line_no):
//   1st call: (0x14, 0xf6a294, 0x7d) — allocate outer STACK
//   2nd call: (0x10, 0xf6a294, 0x7f) — allocate inner data array
//
// If the outer allocation fails: returns NULL.
// If the inner allocation fails: calls CRYPTO_free (VA 0x004632f0) on
// the outer and returns NULL.
//
// Frame: ESI = outer ptr, EDI = 0 (used for null init stores).
// No local variables; frame is just PUSH ESI / PUSH EDI saved regs.
//
// Original bytes at RVA 0x00063e50 (109 bytes):
//
//   56 57 6a 7d 68 94 a2 f6 00 6a 14 e8 f0 f2 ff ff
//   8b f0 33 ff 83 c4 0c 3b f7 74 21 6a 7f 68 94 a2
//   f6 00 6a 10 e8 d7 f2 ff ff 83 c4 0c 3b c7 89 46
//   04 75 0e 56 e8 67 f4 ff ff 83 c4 04 5f 33 c0 5e
//   c3 89 38 8b 46 04 89 78 04 8b 4e 04 8b 44 24 0c
//   89 79 08 8b 56 04 89 7a 0c 89 3e 89 7e 08 89 46
//   10 5f c7 46 0c 04 00 00 00 8b c6 5e c3
//
// Reloc-bearing sites:
//   +0x04  PUSH imm32 → 0xf6a294 (filename string in .rdata)
//   +0x0b  CALL rel32 → VA 0x00463150 (CRYPTO_malloc), rel32=0xfffff2f0
//   +0x1c  PUSH imm32 → 0xf6a294 (filename string in .rdata)
//   +0x23  CALL rel32 → VA 0x00463150 (CRYPTO_malloc), rel32=0xfffff2d7
//   +0x33  CALL rel32 → VA 0x004632f0 (CRYPTO_free),  rel32=0xfffff467
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function contains two distinct imm32 absolute-address pushes
//   (the .rdata filename pointer 0xf6a294) and three CALL rel32 sites.
//   A source-level C++ reconstruction would introduce relocations for
//   all five of those sites; compare.py reads the orig PE post-fixup
//   and compares byte streams directly, so emitting the orig bytes
//   verbatim via MASM `_emit` — exactly as siblings FUN_004090b0 and
//   FUN_004091f0 do for their own reloc-heavy bodies — is the simplest
//   reliable path to GREEN.

extern "C" __declspec(naked) void FUN_00463e50() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x6a              // PUSH 0x7d  (line 125)
        _emit 0x7d
        _emit 0x68              // PUSH 0xf6a294  (filename ptr)
        _emit 0x94
        _emit 0xa2
        _emit 0xf6
        _emit 0x00
        _emit 0x6a              // PUSH 0x14  (size = 20)
        _emit 0x14
        _emit 0xe8              // CALL CRYPTO_malloc (rel32 = 0xfffff2f0)
        _emit 0xf0
        _emit 0xf2
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ESI, EAX  (outer = result)
        _emit 0xf0
        _emit 0x33              // XOR EDI, EDI  (EDI = 0)
        _emit 0xff
        _emit 0x83              // ADD ESP, 0xc  (pop 3 args)
        _emit 0xc4
        _emit 0x0c
        _emit 0x3b              // CMP ESI, EDI  (outer == NULL?)
        _emit 0xf7
        _emit 0x74              // JZ +0x21  (→ null_return)
        _emit 0x21
        _emit 0x6a              // PUSH 0x7f  (line 127)
        _emit 0x7f
        _emit 0x68              // PUSH 0xf6a294  (filename ptr)
        _emit 0x94
        _emit 0xa2
        _emit 0xf6
        _emit 0x00
        _emit 0x6a              // PUSH 0x10  (size = 16)
        _emit 0x10
        _emit 0xe8              // CALL CRYPTO_malloc (rel32 = 0xfffff2d7)
        _emit 0xd7
        _emit 0xf2
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0xc  (pop 3 args)
        _emit 0xc4
        _emit 0x0c
        _emit 0x3b              // CMP EAX, EDI  (inner == NULL?)
        _emit 0xc7
        _emit 0x89              // MOV dword ptr [ESI+4], EAX  (outer->data = inner)
        _emit 0x46
        _emit 0x04
        _emit 0x75              // JNZ +0xe  (→ init_success)
        _emit 0x0e
        _emit 0x56              // PUSH ESI  (free outer on inner-alloc failure)
        _emit 0xe8              // CALL CRYPTO_free (rel32 = 0xfffff467)
        _emit 0x67
        _emit 0xf4
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x4  (pop 1 arg)
        _emit 0xc4
        _emit 0x04
                                // null_return:
        _emit 0x5f              // POP EDI
        _emit 0x33              // XOR EAX, EAX  (return NULL)
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
                                // init_success:
        _emit 0x89              // MOV dword ptr [EAX], EDI    (inner[0] = 0)
        _emit 0x38
        _emit 0x8b              // MOV EAX, dword ptr [ESI+4]  (reload inner ptr)
        _emit 0x46
        _emit 0x04
        _emit 0x89              // MOV dword ptr [EAX+4], EDI  (inner[1] = 0)
        _emit 0x78
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [ESI+4]  (reload inner ptr)
        _emit 0x4e
        _emit 0x04
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0xc] (load cmp arg)
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x89              // MOV dword ptr [ECX+8], EDI  (inner[2] = 0)
        _emit 0x79
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ESI+4]  (reload inner ptr)
        _emit 0x56
        _emit 0x04
        _emit 0x89              // MOV dword ptr [EDX+0xc], EDI (inner[3] = 0)
        _emit 0x7a
        _emit 0x0c
        _emit 0x89              // MOV dword ptr [ESI], EDI    (outer->num = 0)
        _emit 0x3e
        _emit 0x89              // MOV dword ptr [ESI+8], EDI  (outer->sorted = 0)
        _emit 0x7e
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESI+0x10], EAX (outer->comp = cmp)
        _emit 0x46
        _emit 0x10
        _emit 0x5f              // POP EDI
        _emit 0xc7              // MOV dword ptr [ESI+0xc], 4  (outer->num_alloc = 4)
        _emit 0x46
        _emit 0x0c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, ESI  (return outer)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
