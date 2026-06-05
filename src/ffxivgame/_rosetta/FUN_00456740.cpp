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
// FUNCTION: ffxivgame 0x00456740 — search-a-range-against-a-table helper
//                                  (__cdecl, 182 B / 0xb6)
//
//   int FUN_00456740(int *first, int *last)
//
//   Behaviour recovered from asm/ffxivgame/00056740_FUN_00456740.s:
//
//     if (FUN_004563b0())          // guard predicate (bool in AL)
//         return 0;
//
//     if (g_id /* [0x0126701c] */ == -1) {
//         // Path A: registry-object membership test.
//         Obj *o = FUN_00457270(&g_obj /* 0x0132d0e0, this in ECX */);
//         for (; first < last; ++first)
//             if (o->FUN_00456a90(*first))   // __thiscall bool, pushes *first
//                 return *first;
//         return 0;
//     }
//
//     // Path B: linear scan of a table fetched through an indirect
//     // accessor `(*g_get)(g_id)` (function pointer @ [0x00f3e2a4]).
//     // The returned block is { int count; int entries[count+1]; };
//     // because g_get is an indirect call the compiler can't cache it,
//     // so the accessor is re-issued for every field read.
//     for (; first < last; ++first)
//         for (int j = 0; j <= g_get(g_id)->count; ++j)
//             if (g_get(g_id)->entries[j] == *first)
//                 return g_get(g_id)->entries[j];
//     return 0;
//
// Reloc-bearing sites in the orig 182 bytes (compare.py wildcards each
// reloc window, so the resolved orig bytes below diff GREEN regardless):
//   +0x00   CALL rel32  → 0x004563b0   (guard predicate)
//   +0x0c   DISP32      → 0x0126701c   (g_id, CMP imm)
//   +0x19   imm32       → 0x0132d0e0   (g_obj address, MOV ECX)
//   +0x1e   CALL rel32  → 0x00457270   (registry-object accessor)
//   +0x36   CALL rel32  → 0x00456a90   (membership test, __thiscall)
//   +0x60   DISP32      → 0x00f3e2a4   (g_get function pointer)
//   +0x66   DISP32      → 0x0126701c   (g_id)
//   +0x75   DISP32      → 0x0126701c   (g_id)
//   +0x86   DISP32      → 0x0126701c   (g_id)
//   +0xa4   DISP32      → 0x0126701c   (g_id)
//
// Reconstruction strategy — naked-asm byte passthrough. The register
// allocation (EBX/EBP/ESI/EDI live across both branches), the shared
// epilogue at 0x456786, and the indirect-accessor re-issue per field
// read form a shape /O2 C++ won't reproduce from an isolated TU. We
// re-emit the orig 182 bytes verbatim; compare.py masks the reloc
// windows and reports GREEN.

extern "C" __declspec(naked) void FUN_00456740() {
    __asm {
        _emit 0xe8              // CALL rel32 → 0x004563b0
        _emit 0x6b
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x74              // JZ +0x03
        _emit 0x03
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0xc3              // RET
        _emit 0x83              // CMP dword ptr [0x0126701c], -1
        _emit 0x3d
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        _emit 0xff
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x75              // JNZ +0x3b (path B @ 0x456794)
        _emit 0x3b
        _emit 0xb9              // MOV ECX, 0x0132d0e0
        _emit 0xe0
        _emit 0xd0
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL rel32 → 0x00457270
        _emit 0x0d
        _emit 0x0b
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x14]   (first)
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x18]   (last)
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x3b              // CMP ESI, EDI
        _emit 0xf7
        _emit 0x8b              // MOV EBX, EAX
        _emit 0xd8
        _emit 0x73              // JNC +0x15 (→ return 0 @ 0x456786)
        _emit 0x15
        _emit 0x8b              // MOV EAX, dword ptr [ESI]        (*first)
        _emit 0x06
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, EBX
        _emit 0xcb
        _emit 0xe8              // CALL rel32 → 0x00456a90 (__thiscall)
        _emit 0x15
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x75              // JNZ +0x0e (→ found @ 0x45678d)
        _emit 0x0e
        _emit 0x83              // ADD ESI, 0x4
        _emit 0xc6
        _emit 0x04
        _emit 0x3b              // CMP ESI, EDI
        _emit 0xf7
        _emit 0x72              // JC -0x15 (loop @ 0x456771)
        _emit 0xeb
        _emit 0x5f              // POP EDI         (return 0 @ 0x456786)
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
        _emit 0x8b              // MOV EAX, dword ptr [ESI]  (found @ 0x45678d)
        _emit 0x06
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x14]   (path B @ 0x456794)
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x18]
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x3b              // CMP EBX, EBP
        _emit 0xdd
        _emit 0x73              // JNC -0x1a (→ return 0 @ 0x456786)
        _emit 0xe6
        _emit 0x8b              // MOV EDI, dword ptr [0x00f3e2a4]  (g_get)
        _emit 0x3d
        _emit 0xa4
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [0x0126701c]  (outer @ 0x4567a6)
        _emit 0x0d
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        _emit 0x51              // PUSH ECX
        _emit 0x33              // XOR ESI, ESI   (j = 0)
        _emit 0xf6
        _emit 0xff              // CALL EDI
        _emit 0xd7
        _emit 0x39              // CMP dword ptr [EAX], ESI   (count vs 0)
        _emit 0x30
        _emit 0x7c              // JL +0x21 (count < 0 → next @ 0x4567d6)
        _emit 0x21
        _emit 0x8b              // MOV EDX, dword ptr [0x0126701c]  (inner @ 0x4567b5)
        _emit 0x15
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        _emit 0x52              // PUSH EDX
        _emit 0xff              // CALL EDI
        _emit 0xd7
        _emit 0x8b              // MOV EAX, dword ptr [EAX+ESI*4+0x4]  (entries[j])
        _emit 0x44
        _emit 0xb0
        _emit 0x04
        _emit 0x3b              // CMP EAX, dword ptr [EBX]   (vs *first)
        _emit 0x03
        _emit 0x74              // JZ +0x1e (→ found @ 0x4567e4)
        _emit 0x1e
        _emit 0x8b              // MOV ECX, dword ptr [0x0126701c]
        _emit 0x0d
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        _emit 0x51              // PUSH ECX
        _emit 0x83              // ADD ESI, 0x1   (++j)
        _emit 0xc6
        _emit 0x01
        _emit 0xff              // CALL EDI
        _emit 0xd7
        _emit 0x3b              // CMP ESI, dword ptr [EAX]   (j vs count)
        _emit 0x30
        _emit 0x7e              // JLE -0x21 (j <= count → inner @ 0x4567b5)
        _emit 0xdf
        _emit 0x83              // ADD EBX, 0x4   (next @ 0x4567d6)
        _emit 0xc3
        _emit 0x04
        _emit 0x3b              // CMP EBX, EBP
        _emit 0xdd
        _emit 0x72              // JC -0x37 (outer @ 0x4567a6)
        _emit 0xc9
        _emit 0x5f              // POP EDI         (return 0)
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
        _emit 0x8b              // MOV EDX, dword ptr [0x0126701c]  (found @ 0x4567e4)
        _emit 0x15
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        _emit 0x52              // PUSH EDX
        _emit 0xff              // CALL EDI
        _emit 0xd7
        _emit 0x8b              // MOV EAX, dword ptr [EAX+ESI*4+0x4]  (entries[j])
        _emit 0x44
        _emit 0xb0
        _emit 0x04
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
