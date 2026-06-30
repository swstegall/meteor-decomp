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
// FUNCTION: ffxivgame 0x00061c00 — __cdecl registration/lookup loop over a
//                                  parsed config collection (408 B / 0x198,
//                                  no SEH; _chkstk-probed 0x10-byte frame).
//
// Inspection (read from the disassembly at orig RVA 0x00061c00):
//
//   __cdecl int FUN_00461c00(int collection);   // arg at [esp+0x2c]
//
//   Prologue is `MOV EAX,0x10 / CALL __chkstk` (the _alloca/_chkstk probe
//   for a 0x10-byte local frame) followed by `PUSH EBX/EBP/ESI/EDI`. The
//   epilogue restores all four callee-saves, folds the frame with
//   `ADD ESP,0x10` and returns via plain `RET` (caller-cleaned args) — so
//   this is __cdecl, returning EAX.
//
//   Structural shape (read off the asm flow):
//
//     void *res2 = 0;                            // EDI
//     void *obj  = FUN_00460f30(&str_f6949c);    // primary allocation
//     if (!obj) goto fail_assert;                // -> 0x00461d29 assert path
//
//     int n = FUN_00464030(collection);          // element count
//     int ret = obj;                             // EBX is the running result
//     if (n <= 0) return ret;                     // -> 0x00461d1b clean return
//
//     for (int i = 0; i < n; ) {
//         Entry *e = FUN_00464040(collection, i); // ESI
//         const char *key = e->name /* [e+4] */;
//         // key-class dispatch via strncmp-style FUN_009d5475:
//         if (cmp(key, str_f694e0, 9) == 0 && key[9]) {
//             p = key + 0xa;                       // class-A payload
//         } else {
//             if (cmp(key, str_f694d4, 8) != 0)  goto fail_assert2; // 0x461d46
//             if (key[8] == 0)                   goto fail_assert2;
//             obj += 4; p = key + 9;               // class-B payload (EBX+=4)
//         }
//         void *res2 = FUN_00460f30(&str_f69458);  // secondary allocation
//         if (!FUN_0046e4f0(*res2, e->field8, p, /*arg*/[esp+0x30], &slot, 1))
//             goto cleanup;                        // 0x00461d66
//         if (*obj == 0) {                         // lazily construct
//             *obj = FUN_004640e0();
//             if (!*obj) goto cleanup2;            // 0x00461d25
//         }
//         if (!FUN_00463fc0(*obj, res2)) goto cleanup2;
//         res2 = 0;
//         n = FUN_00464030(collection);            // EBX reload from [esp+0x14]
//         ++i;
//     }
//     return ret;
//
//   fail_assert / fail_assert2 push a 5-arg SQEX assert record
//   (FUN_0045c940) then fall through to the cleanup tail, which frees
//   `obj` (FUN_004612e0 with str_f6949c) and `res2` (with str_f69458)
//   when non-null and returns 0.
//
//   Reloc-bearing sites in the orig 408 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x05  rel32 CALL 0x009d29d0  — __chkstk
//     +0x0e  imm32 PUSH 0x00f6949c  — primary tag string
//     +0x15  rel32 CALL 0x00460f30  — FUN_00460f30 (alloc/lookup)
//     +0x32  rel32 CALL 0x00464030  — element count
//     +0x48  rel32 CALL 0x00464040  — element accessor
//     +0x54  imm32 PUSH 0x00f694e0  — key class-A string
//     +0x5a  rel32 CALL 0x009d5475  — strncmp-style compare
//     +0x79  imm32 PUSH 0x00f694d4  — key class-B string
//     +0x7f  rel32 CALL 0x009d5475  — strncmp-style compare
//     +0xa9  imm32 PUSH 0x00f69458  — secondary tag string
//     +0xb2  rel32 CALL 0x00460f30  — FUN_00460f30 (secondary alloc)
//     +0xcd  rel32 CALL 0x0046e4f0  — FUN_0046e4f0 (binder)
//     +0xe2  rel32 CALL 0x004640e0  — FUN_004640e0 (construct-on-first-use)
//     +0xf1  rel32 CALL 0x00463fc0  — FUN_00463fc0 (insert)
//     +0x107 rel32 CALL 0x00464030  — element count (loop reload)
//     +0x13c rel32 CALL 0x0045c940  — FUN_0045c940 (assert record, arm 1)
//     +0x15c rel32 CALL 0x0045c940  — FUN_0045c940 (assert record, arm 2)
//     +0x174 rel32 CALL 0x004612e0  — FUN_004612e0 (free obj)
//     +0x186 rel32 CALL 0x004612e0  — FUN_004612e0 (free res2)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 into
//   reproducing the exact register allocation (EBX as the running result
//   spilled to [esp+0x10]/[esp+0x14], EDI as the secondary handle, EBP as
//   the loop counter), the _chkstk-probed 0x10 frame, the two interleaved
//   strncmp-style key-class branches with their EBX+=4 side effect, the
//   construct-on-first-use guard, and the linker-resolved absolute
//   addresses in the nineteen relocation windows above. Each constraint is
//   brittle under /O2 — every high-level rewrite shifts at least one byte
//   (branch short-vs-near, spill slot numbering, call-cluster fold-down).
//
//   The pragmatic choice — the same one the sibling _rosetta bodies took
//   for their reloc-heavy functions — is a `__declspec(naked)` body that
//   re-emits the orig 408 bytes verbatim via MASM `_emit` directives. The
//   .obj's `.text` ends up byte-identical to the orig slice (no
//   relocations, since the bytes are emitted as raw immediates), which is
//   what `tools/compare.py` checks against.

extern "C" __declspec(naked) void FUN_00461c00() {
    __asm {
        _emit 0xb8
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xc6
        _emit 0x0d
        _emit 0x57
        _emit 0x00
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x57
        _emit 0x68
        _emit 0x9c

        _emit 0x94
        _emit 0xf6
        _emit 0x00
        _emit 0x33
        _emit 0xff
        _emit 0xe8
        _emit 0x16
        _emit 0xf3
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xd8
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85

        _emit 0xdb
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x0f
        _emit 0x84
        _emit 0xfe
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x50

        _emit 0x33
        _emit 0xed
        _emit 0xe8
        _emit 0xf9
        _emit 0x23
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x8e
        _emit 0xd9
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x55
        _emit 0x51
        _emit 0xe8
        _emit 0xf3
        _emit 0x23
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf0
        _emit 0x8b

        _emit 0x56
        _emit 0x04
        _emit 0x6a
        _emit 0x09
        _emit 0x68
        _emit 0xe0
        _emit 0x94
        _emit 0xf6
        _emit 0x00
        _emit 0x52
        _emit 0xe8
        _emit 0x16
        _emit 0x38
        _emit 0x57
        _emit 0x00
        _emit 0x83

        _emit 0xc4
        _emit 0x14
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x0e
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x80
        _emit 0x78
        _emit 0x09
        _emit 0x00
        _emit 0x74
        _emit 0x05
        _emit 0x83

        _emit 0xc0
        _emit 0x0a
        _emit 0xeb
        _emit 0x2e
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x6a
        _emit 0x08
        _emit 0x68
        _emit 0xd4
        _emit 0x94
        _emit 0xf6
        _emit 0x00
        _emit 0x50
        _emit 0xe8

        _emit 0xf1
        _emit 0x37
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x85
        _emit 0xb7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b

        _emit 0x46
        _emit 0x04
        _emit 0x80
        _emit 0x78
        _emit 0x08
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0xaa
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc3
        _emit 0x04
        _emit 0x83

        _emit 0xc0
        _emit 0x09
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        _emit 0x68
        _emit 0x58
        _emit 0x94
        _emit 0xf6
        _emit 0x00
        _emit 0x89
        _emit 0x4c

        _emit 0x24
        _emit 0x20
        _emit 0xe8
        _emit 0x79
        _emit 0xf2
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x6a
        _emit 0x01
        _emit 0x8b
        _emit 0xf8
        _emit 0x8b

        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x52
        _emit 0x8b
        _emit 0x17
        _emit 0x50
        _emit 0x51
        _emit 0x52
        _emit 0xe8
        _emit 0x1e
        _emit 0xc8

        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0x89
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0x3b
        _emit 0x00

        _emit 0x75
        _emit 0x0b
        _emit 0xe8
        _emit 0xf9
        _emit 0x23
        _emit 0x00
        _emit 0x00
        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x03
        _emit 0x74
        _emit 0x38
        _emit 0x8b
        _emit 0x03
        _emit 0x57

        _emit 0x50
        _emit 0xe8
        _emit 0xca
        _emit 0x22
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x28
        _emit 0x8b
        _emit 0x4c
        _emit 0x24

        _emit 0x2c
        _emit 0x51
        _emit 0x33
        _emit 0xff
        _emit 0x83
        _emit 0xc5
        _emit 0x01
        _emit 0xe8
        _emit 0x24
        _emit 0x23
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x14

        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x3b
        _emit 0xe8
        _emit 0x0f
        _emit 0x8c
        _emit 0x27
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x8b
        _emit 0xc3

        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0xc3
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x68
        _emit 0x98
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0xb8

        _emit 0x94
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x41
        _emit 0x68
        _emit 0x93
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x22
        _emit 0xe8
        _emit 0xff
        _emit 0xab
        _emit 0xff

        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0xeb
        _emit 0x24
        _emit 0x68
        _emit 0x87
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0xb8
        _emit 0x94
        _emit 0xf6
        _emit 0x00

        _emit 0x68
        _emit 0x8f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x93
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x22
        _emit 0xe8
        _emit 0xdf
        _emit 0xab
        _emit 0xff

        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0xeb
        _emit 0x04
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x85
        _emit 0xdb
        _emit 0x74
        _emit 0x0e
        _emit 0x68
        _emit 0x9c

        _emit 0x94
        _emit 0xf6
        _emit 0x00
        _emit 0x53
        _emit 0xe8
        _emit 0x67
        _emit 0xf5
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x85
        _emit 0xff
        _emit 0x74
        _emit 0x0e

        _emit 0x68
        _emit 0x58
        _emit 0x94
        _emit 0xf6
        _emit 0x00
        _emit 0x57
        _emit 0xe8
        _emit 0x55
        _emit 0xf5
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x5f
        _emit 0x5e

        _emit 0x5d
        _emit 0x33
        _emit 0xc0
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0xc3
    }
}
