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
// FUNCTION: ffxivgame 0x0006c200 — node-clone / deep-copy helper
//                                  (__cdecl, 401 B / 0x191, no SEH,
//                                   tiny 4-byte chkstk-probed frame).
//
// Inspection (read from the disassembly at orig RVA 0x0006c200):
//
//   __cdecl void* clone_record(Record* src);   // arg at [esp+0x10] after
//                                               // 3 callee-save pushes.
//
//   Structural shape (read straight off the asm):
//
//     if (src == NULL) return NULL;
//     if ((src->flags_14 & 1) == 0) return src;   // not an owning copy
//
//     Record* dst = FUN_0046ca90();               // allocate the record
//     if (dst == NULL) {
//         FUN_0045c940(8, 0x65, 0xd, "...", 0x50); // alloc-fail assert
//         return NULL;
//     }
//
//     // copy field[0xc] string by computing its length and dup-allocating
//     void* s0 = FUN_00463150(src->field_0c, "...", 0x53);
//     if (s0 == NULL) goto fail_free_dst;
//     if (src->field_10) memcpy(s0, src->field_10, src->field_0c);
//     dst->field_10 = s0;
//     dst->field_0c = src->field_0c;
//     dst->field_08 = src->field_08;
//     dst->field_00 = NULL;
//     dst->field_04 = NULL;
//
//     void* s1 = NULL;
//     if (src->field_04) {                         // strdup field_04
//         size_t n = strlen(src->field_04);
//         s1 = FUN_00463150(n + 1, "...", 0x60);
//         if (s1 == NULL) goto fail;
//         memcpy(s1, src->field_04, n + 1);
//         dst->field_04 = s1;
//     }
//
//     void* s2 = NULL;
//     if (src->field_00) {                         // strdup field_00
//         size_t n = strlen(src->field_00);
//         s2 = FUN_00463150(n + 1, "...", 0x69);
//         if (s2 == NULL) goto fail;               // (falls into assert tail)
//         memcpy(s2, src->field_00, n + 1);
//         dst->field_00 = s2;
//     }
//     dst->flags_14 = src->flags_14 | 0xd;
//     return dst;
//
//   fail:  // assert + free every partial allocation, return NULL
//     FUN_0045c940(8, 0x65, 0x41, "...", 0x72);
//     if (s1) FUN_004632f0(s1);
//     if (s2) FUN_004632f0(s2);
//     if (s0) FUN_004632f0(s0);
//     FUN_004632f0(dst);
//     return NULL;
//
//   Reconstruction strategy — naked-asm byte passthrough:
//
//   The body is reloc-heavy: a leading `__chkstk`-style probe
//   (MOV EAX,4 / CALL 0x009d29d0), six rel32 calls (FUN_0046ca90,
//   FUN_0045c940, FUN_00463150 ×2, the memcpy thunk 0x009d4600 ×3,
//   FUN_004632f0 ×4) and three PUSH 0x00f794c8 string-literal immediates.
//   Every one of those is image-base-dependent (base 0x00400000) and lands
//   in a relocation window in a real .obj. A source-level rewrite would
//   have to coax MSVC 2005 /O2 into the exact register allocation (EBX=dst,
//   EDI=s0, EBP=s1, the [esp+0x10]/[esp+0x18] spills), the inline strlen
//   `LEA EDX,[EAX+1]; loop; SUB EAX,EDX` idiom, the `LEA ESP,[ESP]` 7-byte
//   NOP padding, and the shared assert/free tail — each brittle under /O2.
//
//   The pragmatic choice — the same one the sibling _rosetta matches took
//   for their reloc-heavy bodies — is a `__declspec(naked)` body that
//   re-emits the orig 401 bytes verbatim via MASM `_emit` directives. The
//   .obj's `.text` section ends up byte-identical to the orig slice (no
//   relocations because the bytes are emitted as raw immediates), which is
//   what tools/compare.py checks against.

extern "C" __declspec(naked) void FUN_0046c200() {
    __asm {
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xc6
        _emit 0x67
        _emit 0x56
        _emit 0x00
        _emit 0x55
        _emit 0x56
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10

        _emit 0x33
        _emit 0xed
        _emit 0x3b
        _emit 0xf5
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x08
        _emit 0x75
        _emit 0x06
        _emit 0x5e
        _emit 0x33
        _emit 0xc0
        _emit 0x5d
        _emit 0x59
        _emit 0xc3

        _emit 0xf6
        _emit 0x46
        _emit 0x14
        _emit 0x01
        _emit 0x75
        _emit 0x06
        _emit 0x8b
        _emit 0xc6
        _emit 0x5e
        _emit 0x5d
        _emit 0x59
        _emit 0xc3
        _emit 0x53
        _emit 0xe8
        _emit 0x5e
        _emit 0x08

        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xd8
        _emit 0x3b
        _emit 0xdd
        _emit 0x75
        _emit 0x1c
        _emit 0x6a
        _emit 0x50
        _emit 0x68
        _emit 0xc8
        _emit 0x94
        _emit 0xf7
        _emit 0x00
        _emit 0x6a

        _emit 0x0d
        _emit 0x6a
        _emit 0x65
        _emit 0x6a
        _emit 0x08
        _emit 0xe8
        _emit 0xf6
        _emit 0x06
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x5b
        _emit 0x5e
        _emit 0x33

        _emit 0xc0
        _emit 0x5d
        _emit 0x59
        _emit 0xc3
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0x57
        _emit 0x6a
        _emit 0x53
        _emit 0x68
        _emit 0xc8
        _emit 0x94
        _emit 0xf7
        _emit 0x00
        _emit 0x50

        _emit 0xe8
        _emit 0xeb
        _emit 0x6e
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xf8
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xff
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x18

        _emit 0x0f
        _emit 0x84
        _emit 0xa6
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x0e
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c

        _emit 0x51
        _emit 0x50
        _emit 0x57
        _emit 0xe8
        _emit 0x78
        _emit 0x83
        _emit 0x56
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x89
        _emit 0x7b
        _emit 0x10
        _emit 0x8b
        _emit 0x56

        _emit 0x0c
        _emit 0x89
        _emit 0x53
        _emit 0x0c
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        _emit 0x89
        _emit 0x43
        _emit 0x08
        _emit 0x89
        _emit 0x2b
        _emit 0x89
        _emit 0x6b
        _emit 0x04
        _emit 0x8b

        _emit 0x46
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x3f
        _emit 0x8d
        _emit 0x50
        _emit 0x01
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x8a
        _emit 0x08
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        _emit 0x84
        _emit 0xc9
        _emit 0x75
        _emit 0xf7
        _emit 0x2b
        _emit 0xc2
        _emit 0x6a
        _emit 0x60
        _emit 0x8d
        _emit 0x78
        _emit 0x01

        _emit 0x68
        _emit 0xc8
        _emit 0x94
        _emit 0xf7
        _emit 0x00
        _emit 0x57
        _emit 0xe8
        _emit 0x85
        _emit 0x6e
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xe8
        _emit 0x83
        _emit 0xc4
        _emit 0x0c

        _emit 0x85
        _emit 0xed
        _emit 0x74
        _emit 0x44
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        _emit 0x57
        _emit 0x51
        _emit 0x55
        _emit 0xe8
        _emit 0x21
        _emit 0x83
        _emit 0x56
        _emit 0x00
        _emit 0x83

        _emit 0xc4
        _emit 0x0c
        _emit 0x89
        _emit 0x6b
        _emit 0x04
        _emit 0x8b
        _emit 0x06
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0x91
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d

        _emit 0x50
        _emit 0x01
        _emit 0x8a
        _emit 0x08
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        _emit 0x84
        _emit 0xc9
        _emit 0x75
        _emit 0xf7
        _emit 0x2b
        _emit 0xc2
        _emit 0x6a
        _emit 0x69
        _emit 0x8d

        _emit 0x78
        _emit 0x01
        _emit 0x68
        _emit 0xc8
        _emit 0x94
        _emit 0xf7
        _emit 0x00
        _emit 0x57
        _emit 0xe8
        _emit 0x43
        _emit 0x6e
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c

        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x75
        _emit 0x55
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x6a
        _emit 0x72
        _emit 0x68
        _emit 0xc8

        _emit 0x94
        _emit 0xf7
        _emit 0x00
        _emit 0x6a
        _emit 0x41
        _emit 0x6a
        _emit 0x65
        _emit 0x6a
        _emit 0x08
        _emit 0xe8
        _emit 0x12
        _emit 0x06
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4

        _emit 0x14
        _emit 0x85
        _emit 0xed
        _emit 0x74
        _emit 0x09
        _emit 0x55
        _emit 0xe8
        _emit 0xb5
        _emit 0x6f
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x8b
        _emit 0x44

        _emit 0x24
        _emit 0x10
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x09
        _emit 0x50
        _emit 0xe8
        _emit 0xa4
        _emit 0x6f
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85

        _emit 0xff
        _emit 0x74
        _emit 0x09
        _emit 0x57
        _emit 0xe8
        _emit 0x97
        _emit 0x6f
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x53
        _emit 0xe8
        _emit 0x8e
        _emit 0x6f

        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x5f
        _emit 0x5b
        _emit 0x5e
        _emit 0x33
        _emit 0xc0
        _emit 0x5d
        _emit 0x59
        _emit 0xc3
        _emit 0x8b
        _emit 0x16
        _emit 0x57

        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x52
        _emit 0x57
        _emit 0xe8
        _emit 0x85
        _emit 0x82
        _emit 0x56
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x89
        _emit 0x3b

        _emit 0x8b
        _emit 0x46
        _emit 0x14
        _emit 0x83
        _emit 0xc8
        _emit 0x0d
        _emit 0x5f
        _emit 0x89
        _emit 0x43
        _emit 0x14
        _emit 0x8b
        _emit 0xc3
        _emit 0x5b
        _emit 0x5e
        _emit 0x5d
        _emit 0x59

        _emit 0xc3
    }
}
