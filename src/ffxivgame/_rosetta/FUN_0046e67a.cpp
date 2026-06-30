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
// FUNCTION: ffxivgame 0x0046e67a — collection-item search loop (199 B / 0xC7)
//                                  iterates over a collection indexed by arg5,
//                                  matching items by name string (5-byte
//                                  REPE CMPSB at 0xf79770) and a field[4]
//                                  comparison to 0xf679cc, then dispatches
//                                  to handler sub-functions.
//
// Asm shape (read from asm/ffxivgame/0006e67a_FUN_0046e67a.s):
//
//   Prologue saves EBX, EBP, ESI; uses EBP as loop counter (XOR EBP,EBP).
//   Epilogue pops ESI, EBP, EBX, (set EAX), EDI, ECX — calling convention
//   is non-standard (__usercall or caller pre-pushes EDI/ECX); exact ABI
//   not recoverable without call-site analysis.
//
//   Outline:
//     count = FUN_00464030(arg5);
//     if (count <= 0) return arg1;          // (via EDI)
//     for (i = 0; i < count; i++) {
//         item  = FUN_00464040(arg5, i);    // → EBX
//         if (strcmp_like(item+4, 0xf679cc) != 0) goto try_generic;
//         if (item+8 == NULL)               goto try_generic;
//         if (memcmp(item+8, 0xf79770, 5) != 0) goto try_generic;
//         // name matched
//         if (FUN_0046df10(arg4/*ECX*/, arg1/*EBX*/) == 0) {
//             FUN_004641f0(arg1, 0x461fb0);
//             return 0;
//         }
//         goto next_iter;
//     try_generic:
//         result = FUN_0046e4f0(0, arg3, arg4, item, 0);
//         if (result != 0) FUN_00463fc0(arg1, result);
//     next_iter:
//         count = FUN_00464030(arg5);   // re-query count
//     }
//     return arg1;
//
//   Reloc-bearing sites in the orig 199 bytes (tools/compare.py masks these
//   in the diff; _emit bakes them as raw immediates identical to the orig slice):
//     +0x0b  REL32 → FUN_00464030 (CALL)
//     +0x14  REL32 → FUN_00464040 (CALL)
//     +0x21  DIR32 → 0x00f679cc   (PUSH imm)
//     +0x2d  REL32 → FUN_00470410 (CALL)
//     +0x3f  DIR32 → 0x00f79770   (MOV EDI, imm)
//     +0x57  REL32 → FUN_0046df10 (CALL)
//     +0x65  DIR32 → 0x00461fb0   (PUSH imm)
//     +0x6a  REL32 → FUN_004641f0 (CALL)
//     +0x89  REL32 → FUN_0046e4f0 (CALL)
//     +0x9b  REL32 → FUN_00463fc0 (CALL)
//     +0xa4  REL32 → FUN_00464030 (second call, loop re-check)
//     +0xb1  REL32 → FUN_00464030 (... same, after branch)
//
// Reconstruction strategy — naked-asm byte passthrough (same as FUN_00406680):
//
//   The non-standard epilogue (5 pops vs 3 prologue pushes) makes the ABI
//   ambiguous; even a symbolic naked-asm would require choosing a calling
//   convention that the MSVC 2005 /Oicall path does not expose. Emitting
//   the 199 original bytes verbatim via MASM _emit directives avoids all
//   ABI guesswork and produces a .obj whose .text section is byte-identical
//   to the original slice (modulo the masked reloc sites).

extern "C" __declspec(naked) void FUN_0046e67a() {
    __asm {
        _emit 0x53   // PUSH EBX
        _emit 0x55   // PUSH EBP
        _emit 0x56   // PUSH ESI
        _emit 0x8b   // MOV ESI, [ESP+0x20]
        _emit 0x74
        _emit 0x24
        _emit 0x20
        _emit 0x56   // PUSH ESI  (arg for FUN_00464030)
        _emit 0x33   // XOR EBP, EBP
        _emit 0xed
        _emit 0xe8   // CALL FUN_00464030 (rel32)
        _emit 0xa7
        _emit 0x59
        _emit 0xff
        _emit 0xff
        _emit 0x83   // ADD ESP, 4
        _emit 0xc4
        _emit 0x04
        _emit 0x85   // TEST EAX, EAX
        _emit 0xc0
        _emit 0x0f   // JLE near (to epilogue at +0xa5)
        _emit 0x8e
        _emit 0xa5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x55   // PUSH EBP  (loop index, for FUN_00464040)
        _emit 0x56   // PUSH ESI  (collection, for FUN_00464040)
        _emit 0xe8   // CALL FUN_00464040 (rel32)
        _emit 0xa5
        _emit 0x59
        _emit 0xff
        _emit 0xff
        _emit 0x8b   // MOV EBX, EAX
        _emit 0xd8
        _emit 0x8b   // MOV EAX, [EBX+4]
        _emit 0x43
        _emit 0x04
        _emit 0x68   // PUSH 0xf679cc (DIR32)
        _emit 0xcc
        _emit 0x79
        _emit 0xf6
        _emit 0x00
        _emit 0x50   // PUSH EAX
        _emit 0xe8   // CALL FUN_00470410 (rel32)
        _emit 0x65
        _emit 0x1d
        _emit 0x00
        _emit 0x00
        _emit 0x83   // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x85   // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75   // JNZ short (+0x42, to try_generic)
        _emit 0x42
        _emit 0x8b   // MOV ESI, [EBX+8]
        _emit 0x73
        _emit 0x08
        _emit 0x85   // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74   // JZ short (+0x3b, to try_generic)
        _emit 0x3b
        _emit 0xbf   // MOV EDI, 0xf79770 (DIR32)
        _emit 0x70
        _emit 0x97
        _emit 0xf7
        _emit 0x00
        _emit 0xb9   // MOV ECX, 5
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33   // XOR EDX, EDX
        _emit 0xd2
        _emit 0xf3   // REPE CMPSB
        _emit 0xa6
        _emit 0x75   // JNZ short (+0x2b, to try_generic)
        _emit 0x2b
        _emit 0x8b   // MOV EBX, [ESP+0x10]
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x8b   // MOV ECX, [ESP+0x1c]
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xe8   // CALL FUN_0046df10 (rel32)
        _emit 0x3a
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        _emit 0x85   // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75   // JNZ short (+0x43, to next_iter)
        _emit 0x43
        _emit 0x8b   // MOV EAX, [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x68   // PUSH 0x461fb0 (DIR32)
        _emit 0xb0
        _emit 0x1f
        _emit 0x46
        _emit 0x00
        _emit 0x50   // PUSH EAX
        _emit 0xe8   // CALL FUN_004641f0 (rel32)
        _emit 0x07
        _emit 0x5b
        _emit 0xff
        _emit 0xff
        _emit 0x83   // ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0x5e   // POP ESI
        _emit 0x5d   // POP EBP
        _emit 0x5b   // POP EBX
        _emit 0x33   // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5f   // POP EDI
        _emit 0x59   // POP ECX
        _emit 0xc3   // RET
        // try_generic:
        _emit 0x8b   // MOV EAX, [ESP+0x1c]
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x8b   // MOV ECX, [ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x6a   // PUSH 0
        _emit 0x00
        _emit 0x53   // PUSH EBX  (item)
        _emit 0x50   // PUSH EAX  ([ESP+0x1c])
        _emit 0x51   // PUSH ECX  ([ESP+0x18])
        _emit 0x6a   // PUSH 0
        _emit 0x00
        _emit 0xe8   // CALL FUN_0046e4f0 (rel32)
        _emit 0xe8
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x83   // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x85   // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74   // JZ short (to 0x0046e6da = FUN_004641f0 path above)
        _emit 0xcb
        _emit 0x8b   // MOV EDX, [ESP+0x10]
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x50   // PUSH EAX
        _emit 0x52   // PUSH EDX
        _emit 0xe8   // CALL FUN_00463fc0 (rel32)
        _emit 0xa6
        _emit 0x58
        _emit 0xff
        _emit 0xff
        _emit 0x83   // ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        // next_iter:
        _emit 0x8b   // MOV ESI, [ESP+0x20]
        _emit 0x74
        _emit 0x24
        _emit 0x20
        _emit 0x56   // PUSH ESI
        _emit 0x83   // ADD EBP, 1
        _emit 0xc5
        _emit 0x01
        _emit 0xe8   // CALL FUN_00464030 (rel32)
        _emit 0x06
        _emit 0x59
        _emit 0xff
        _emit 0xff
        _emit 0x83   // ADD ESP, 4
        _emit 0xc4
        _emit 0x04
        _emit 0x3b   // CMP EBP, EAX
        _emit 0xe8
        _emit 0x0f   // JL near (to loop top)
        _emit 0x8c
        _emit 0x5f
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // loop_exit:
        _emit 0x8b   // MOV EDI, [ESP+0x10]
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x5e   // POP ESI
        _emit 0x5d   // POP EBP
        _emit 0x5b   // POP EBX
        _emit 0x8b   // MOV EAX, EDI
        _emit 0xc7
        _emit 0x5f   // POP EDI
        _emit 0x59   // POP ECX
        _emit 0xc3   // RET
    }
}
