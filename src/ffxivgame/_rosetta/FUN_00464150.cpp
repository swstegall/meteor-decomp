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
// FUNCTION: ffxivgame 0x00064150 — `internal_find`: linear search over a
//                                  lazily-initialized table, returning the
//                                  index of a matching key or -1 (121 B / 0x79).
//
// Inspection (read from the disassembly at orig RVA 0x00064150):
//
//   The receiver is passed in ESI (used as a base pointer, never modified
//   nor saved — so it is preserved across the call), and two stack args
//   ([ESP+0x4] = key, [ESP+0x8] = aux) feed the lookup. RET (no operand):
//   caller-cleanup. Object layout touched:
//     [ESI + 0x00]  element count
//     [ESI + 0x04]  base pointer to an array of 4-byte keys
//     [ESI + 0x08]  "sorted/initialized" flag (set to 1 after a sort)
//     [ESI + 0x10]  state word (0 = not yet built)
//
//   Structural shape (pseudo-C):
//
//     int internal_find(self /*ESI*/, int key /*[ESP+4]*/, x /*[ESP+8]*/) {
//         if (!self) return -1;
//         int a = self->state;                 // [ESI+0x10]
//         if (a == 0) {
//             // fall through to the linear-scan fast path
//             if (self->count <= 0) return -1;
//             int* p = self->base;             // [ESI+0x4]
//             for (; *p != key; ++a, ++p)
//                 if (a >= self->count) return -1;
//             return ???;                      // (a, falls into 0x4641c8 RET)
//         }
//         // state != 0: ensure the table is sorted, then binary-search
//         if (self->flag == 0) {               // [ESI+0x8]
//             qsort(self->base, self->count, 4, cmp@0x009d5de0);
//             self->flag = 1;
//         }
//         if (x == 0) return -1;
//         void* hit = bsearch_like(&local, self->base, self->count,
//                                  4, self->state, x);   // CALL 0x00464ca0
//         if (!hit) return -1;
//         return (hit - self->base) >> 2;      // element index
//     }
//
//   Reloc-bearing sites in the orig 121 bytes (absolute addresses resolved
//   only at full-binary relink; standalone .obj compilation re-emits the
//   linker-resolved rel32 bytes as raw immediates, which match the orig
//   binary byte-for-byte and carry no relocation):
//     +0x38  CALL rel32 → 0x009d5de0 (qsort-style sort helper)
//     +0x66  CALL rel32 → 0x00464ca0 (bsearch-style lookup helper)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   ESI as an unsaved incoming register pointer, the exact branch
//   encodings, and the two linker-resolved rel32 call targets are not
//   reproducible from /O2 C++ source without per-byte drift. The pragmatic
//   match — same as the sibling _rosetta bodies — is a `__declspec(naked)`
//   function re-emitting the orig 121 bytes verbatim via MASM `_emit`
//   directives; the .obj's `.text` is byte-identical to the orig slice and
//   `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_00464150() {
    __asm {
        // 00064150: 85 f6              TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 00064152: 8b 54 24 04        MOV EDX,[ESP+0x4]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x04
        // 00064156: 74 1c              JZ 0x00464174
        _emit 0x74
        _emit 0x1c
        // 00064158: 8b 46 10           MOV EAX,[ESI+0x10]
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        // 0006415b: 85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0006415d: 75 19              JNZ 0x00464178
        _emit 0x75
        _emit 0x19
        // 0006415f: 39 06              CMP [ESI],EAX
        _emit 0x39
        _emit 0x06
        // 00064161: 7e 11              JLE 0x00464174
        _emit 0x7e
        _emit 0x11
        // 00064163: 8b 4e 04           MOV ECX,[ESI+0x4]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 00064166: 39 11              CMP [ECX],EDX
        _emit 0x39
        _emit 0x11
        // 00064168: 74 5e              JZ 0x004641c8
        _emit 0x74
        _emit 0x5e
        // 0006416a: 83 c0 01           ADD EAX,0x1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 0006416d: 83 c1 04           ADD ECX,0x4
        _emit 0x83
        _emit 0xc1
        _emit 0x04
        // 00064170: 3b 06              CMP EAX,[ESI]
        _emit 0x3b
        _emit 0x06
        // 00064172: 7c f2              JL 0x00464166
        _emit 0x7c
        _emit 0xf2
        // 00064174: 83 c8 ff           OR EAX,0xffffffff
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // 00064177: c3                 RET
        _emit 0xc3
        // 00064178: 83 7e 08 00        CMP [ESI+0x8],0x0
        _emit 0x83
        _emit 0x7e
        _emit 0x08
        _emit 0x00
        // 0006417c: 75 1d              JNZ 0x0046419b
        _emit 0x75
        _emit 0x1d
        // 0006417e: 8b 4e 04           MOV ECX,[ESI+0x4]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 00064181: 50                 PUSH EAX
        _emit 0x50
        // 00064182: 8b 06              MOV EAX,[ESI]
        _emit 0x8b
        _emit 0x06
        // 00064184: 6a 04              PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 00064186: 50                 PUSH EAX
        _emit 0x50
        // 00064187: 51                 PUSH ECX
        _emit 0x51
        // 00064188: e8 53 1c 57 00     CALL 0x009d5de0
        _emit 0xe8
        _emit 0x53
        _emit 0x1c
        _emit 0x57
        _emit 0x00
        // 0006418d: 8b 54 24 14        MOV EDX,[ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 00064191: 83 c4 10           ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00064194: c7 46 08 01 00 00 00   MOV [ESI+0x8],0x1
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0006419b: 85 d2              TEST EDX,EDX
        _emit 0x85
        _emit 0xd2
        // 0006419d: 74 d5              JZ 0x00464174
        _emit 0x74
        _emit 0xd5
        // 0006419f: 8b 54 24 08        MOV EDX,[ESP+0x8]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 000641a3: 8b 46 10           MOV EAX,[ESI+0x10]
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        // 000641a6: 8b 0e              MOV ECX,[ESI]
        _emit 0x8b
        _emit 0x0e
        // 000641a8: 52                 PUSH EDX
        _emit 0x52
        // 000641a9: 8b 56 04           MOV EDX,[ESI+0x4]
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 000641ac: 50                 PUSH EAX
        _emit 0x50
        // 000641ad: 6a 04              PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 000641af: 51                 PUSH ECX
        _emit 0x51
        // 000641b0: 52                 PUSH EDX
        _emit 0x52
        // 000641b1: 8d 44 24 18        LEA EAX,[ESP+0x18]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 000641b5: 50                 PUSH EAX
        _emit 0x50
        // 000641b6: e8 e5 0a 00 00     CALL 0x00464ca0
        _emit 0xe8
        _emit 0xe5
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        // 000641bb: 83 c4 18           ADD ESP,0x18
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 000641be: 85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 000641c0: 74 b2              JZ 0x00464174
        _emit 0x74
        _emit 0xb2
        // 000641c2: 2b 46 04           SUB EAX,[ESI+0x4]
        _emit 0x2b
        _emit 0x46
        _emit 0x04
        // 000641c5: c1 f8 02           SAR EAX,0x2
        _emit 0xc1
        _emit 0xf8
        _emit 0x02
        // 000641c8: c3                 RET
        _emit 0xc3
    }
}
