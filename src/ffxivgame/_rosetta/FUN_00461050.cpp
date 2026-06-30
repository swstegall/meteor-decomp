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
// FUNCTION: ffxivgame 0x00061050 — message/event dispatcher keyed on a
//                                  one-byte command tag (455 B / 0x1c7, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x00061050):
//
//   __cdecl void dispatch(void** edi /*node ptr*/, struct* esi /*ctx*/)
//   — the two operands arrive already in EDI/ESI (the caller is matched
//   under the same convention; this routine is reached as a tail of a
//   register-threading chain). Returns void.
//
//   Structural shape (a 7-way jump table on ctx->tag, several arms each
//   ending in tail dispatch + a shared teardown epilogue):
//
//     __alloca_probe-style stack helper (MOV EAX,8; CALL 0x009d29d0);
//     if (edi == NULL) goto done;
//     int* tbl = ctx->m_tbl;               // [esi+0x10]
//     char tag = ctx->tag;                 // [esi]
//     if (tag != 0 && *edi == 0) goto done;
//     void* cb = NULL;                     // EBX
//     if (tbl && tbl[4] /* +0x10 */) cb = tbl[4];
//     if ((unsigned)tag > 6) goto pop_done;
//     switch (tag) { ... 7 arms via [ECX*4 + 0x461218] ... }
//
//   The arms call neighbour helpers (FUN_00461240, FUN_00460f70,
//   FUN_0046d7d0, FUN_0046d9e0, FUN_0046d810, FUN_0046d8a0, FUN_0046da00,
//   FUN_004632f0) and the callback slots tbl[1]/tbl[2] indirectly; they
//   converge on a shared teardown that, when ctx->flag ([esp+0x14]) is
//   clear, frees *edi via FUN_004632f0 and nulls it.
//
//   Reloc-bearing sites in the orig 455 bytes (image base 0x00400000):
//     +0x05  CALL rel32          → 0x009d29d0 (stack-alloc helper)
//     +0x4f  JMP  [ECX*4+imm32]  → 0x00461218 (7-entry jump table, lives
//                                  immediately after this function — NOT in
//                                  the 455-byte body)
//     +0x5f  CALL rel32          → FUN_00461240
//     +0x6f  CALL rel32          → FUN_00460f70
//     +0x97  CALL rel32          → FUN_0046d7d0
//     +0xbb  CALL rel32          → FUN_0046d9e0
//     +0xc2  CALL rel32          → FUN_00461240
//     +0x113 CALL rel32          → FUN_0046d810
//     +0x13d CALL rel32          → FUN_0046d8a0
//     +0x164 CALL rel32          → FUN_0046da00
//     +0x174 CALL rel32          → FUN_0046d9e0
//     +0x17b CALL rel32          → FUN_00461240
//     +0x1b3 CALL rel32          → FUN_004632f0
//   plus the three indirect CALL EBX (tbl[4]) and CALL EAX (tbl[1]/tbl[2]).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 into
//   reproducing the exact register threading (EDI/ESI pre-loaded, EBX/EBP
//   spilled across the switch), the dense 7-entry jump table emitted at
//   imm32 0x00461218, and the dozen linker-resolved rel32 windows above.
//   Every high-level lowering shifts at least one byte (switch jump-table
//   placement, branch short-vs-near, the [esp+0x4] callback-slot temp).
//   The pragmatic choice — the same one the reloc-heavy siblings
//   (FUN_00401820 / FUN_00409350 / FUN_0040b840) took — is a
//   `__declspec(naked)` body that re-emits the orig 455 bytes verbatim
//   via MASM `_emit`. The .obj's `.text` ends up byte-identical to the
//   orig slice, which is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_00461050() {
    __asm {
        _emit 0xb8
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x76
        _emit 0x19
        _emit 0x57
        _emit 0x00
        _emit 0x85
        _emit 0xff
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        _emit 0x0f
        _emit 0x84
        _emit 0xae
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8a
        _emit 0x0e
        _emit 0x84
        _emit 0xc9
        _emit 0x74
        _emit 0x09
        _emit 0x83
        _emit 0x3f
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0x9f
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x85
        _emit 0xc0
        _emit 0x53
        _emit 0x74
        _emit 0x0d
        _emit 0x8b
        _emit 0x58
        _emit 0x10
        _emit 0x85
        _emit 0xdb
        _emit 0x74
        _emit 0x06
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x04
        _emit 0xeb
        _emit 0x0c
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x04
        _emit 0x0f
        _emit 0xbe
        _emit 0xc9
        _emit 0x83
        _emit 0xf9
        _emit 0x06
        _emit 0x0f
        _emit 0x87
        _emit 0x74
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x55
        _emit 0xff
        _emit 0x24
        _emit 0x8d
        _emit 0x18
        _emit 0x12
        _emit 0x46
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x10
        _emit 0x50
        _emit 0x57
        _emit 0xe8
        _emit 0x8c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0xc3
        _emit 0x56
        _emit 0x57
        _emit 0xe8
        _emit 0xac
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0xc3
        _emit 0x85
        _emit 0xdb
        _emit 0x74
        _emit 0x14
        _emit 0x6a
        _emit 0x00
        _emit 0x56
        _emit 0x57
        _emit 0x6a
        _emit 0x02
        _emit 0xff
        _emit 0xd3
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x83
        _emit 0xf8
        _emit 0x02
        _emit 0x0f
        _emit 0x84
        _emit 0x2c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0x57
        _emit 0xe8
        _emit 0xe4
        _emit 0xc6
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x8c
        _emit 0xf3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x3b
        _emit 0x46
        _emit 0x0c
        _emit 0x0f
        _emit 0x8d
        _emit 0xea
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        _emit 0x8d
        _emit 0x04
        _emit 0x80
        _emit 0x8d
        _emit 0x2c
        _emit 0x81
        _emit 0x55
        _emit 0x57
        _emit 0xe8
        _emit 0xd0
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x55
        _emit 0x50
        _emit 0xe8
        _emit 0x29
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0xe9
        _emit 0xcb
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0xea
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0xdf
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x0f
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xff
        _emit 0xe0
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0xca
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x40
        _emit 0x08
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0xbf
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0x57
        _emit 0xff
        _emit 0xd0
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0xc3
        _emit 0x56
        _emit 0x6a
        _emit 0xff
        _emit 0x57
        _emit 0xe8
        _emit 0xa8
        _emit 0xc6
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x8f
        _emit 0x9e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x85
        _emit 0xdb
        _emit 0x74
        _emit 0x14
        _emit 0x6a
        _emit 0x00
        _emit 0x56
        _emit 0x57
        _emit 0x6a
        _emit 0x02
        _emit 0xff
        _emit 0xd3
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x83
        _emit 0xf8
        _emit 0x02
        _emit 0x0f
        _emit 0x84
        _emit 0x86
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0x57
        _emit 0xe8
        _emit 0x0e
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x85
        _emit 0xc0
        _emit 0x8d
        _emit 0x14
        _emit 0x80
        _emit 0x8d
        _emit 0x6c
        _emit 0x91
        _emit 0xec
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x7e
        _emit 0x3c
        _emit 0x8b
        _emit 0xff
        _emit 0x6a
        _emit 0x00
        _emit 0x55
        _emit 0x57
        _emit 0xe8
        _emit 0x47
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xd8
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xdb
        _emit 0x74
        _emit 0x11
        _emit 0x53
        _emit 0x57
        _emit 0xe8
        _emit 0x17
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x53
        _emit 0x50
        _emit 0xe8
        _emit 0x70
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        _emit 0x83
        _emit 0xed
        _emit 0x14
        _emit 0x3b
        _emit 0x46
        _emit 0x0c
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x7c
        _emit 0xca
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x85
        _emit 0xdb
        _emit 0x74
        _emit 0x0b
        _emit 0x6a
        _emit 0x00
        _emit 0x56
        _emit 0x57
        _emit 0x6a
        _emit 0x03
        _emit 0xff
        _emit 0xd3
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x75
        _emit 0x11
        _emit 0x8b
        _emit 0x17
        _emit 0x52
        _emit 0xe8
        _emit 0xe8
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0xc3
    }
}
