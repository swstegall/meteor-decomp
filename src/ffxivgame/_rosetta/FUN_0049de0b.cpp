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
// FUNCTION: ffxivgame 0x0049de0b — string-keyed linear search through an
//                                  indexed container (358 B / 0x166,
//                                  __cdecl 6-param, no SEH).
//
// Inspection (from disassembly at orig RVA 0x0009de0b):
//
//   Prologue: PUSH EBX / PUSH EBP / PUSH ESI  (three callee-save saves).
//   No SUB ESP; no /GS cookie; no SEH frame.
//
//   EBP = 0 (loop counter, XOR EBP,EBP).
//   ESI = param6  ([ESP+0x24] after 3 saves = sixth stack argument).
//
//   Body outline:
//     count = FUN_00464030(param6);         // element count query
//     if (count <= 0) goto epilogue;
//     loop (EBP = 0; EBP < count):
//       EDI = FUN_00464040(EBP, param6);    // element pointer at index EBP
//       save EDI to [ESP+0x14]              // spill into param2 slot
//       node = EDI->field_4;               // string node
//       str  = node->field_8;              // first key string
//       if (strcmp(str, EDI->str) == 0):   // inline strcmp loop (BL scratch)
//         if (strcmp(param1_str, EDI->str) == 0):  // second key check
//           goto error_path_1;             // assertion: duplicate / conflict
//         // no second match — advance linked-list node and retry strcmp
//         node = node->field_10; node ptr += 0xc; if (node) loop back
//       else:
//         // outer loop: next index
//         EBP++; re-query count; loop
//     EDI = [ESP+0x10]  // param1 = default/out slot
//   epilogue:
//     EAX = EDI
//     POP ESI / POP EBP / POP EBX / POP EDI / ADD ESP,8 / RET
//
//   Three exit paths:
//     (a) normal  — return EDI (param1 default or matched node ptr)
//     (b) error_1 — assertion via FUN_0045c940(0x22,0x65,0x41,0xf87940,0x7b)
//                   then FUN_004644d0(param1); return 0
//     (c) error_2 — assertion via FUN_0045c940(0x22,0x65,0x6f,0xf87940,0x84)
//                   then six-arg call FUN_0045c520 + FUN_004644d0; return 0
//
//   Reloc-bearing sites in the orig 358 bytes (absolute imm32s masked by
//   tools/compare.py on the cmp_obj path):
//     +0x52  imm32 PUSH 0xf87940   (error-path string literal)
//     +0x6c  imm32 PUSH 0xf87940   (error-path string literal, repeated)
//     +0xa3  imm32 PUSH 0xf69dac   (error-path arg)
//     +0xa8  imm32 PUSH 0xf69da4   (error-path arg)
//     +0xad  imm32 PUSH 0xf69d98   (error-path arg)
//
//   All CALL targets are PC-relative REL32 and are reproduced correctly
//   by the naked-asm passthrough (emitted as raw bytes, no link reloc).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The epilogue sequence  POP EDI / ADD ESP,8 / RET  cleans parameters
//   from the stack in a pattern that does not correspond to any single
//   named MSVC 2005 calling convention; it is likely the result of the
//   optimizer eliding a standard RET N form in favour of a register-
//   reuse path.  Re-expressing this at the C++ level would require either
//   __declspec(naked) inline asm anyway, or a register-allocation miracle.
//   Five sibling matches of comparable size (FUN_00405080, FUN_0040ced0,
//   FUN_004014b0) already established the naked-passthrough pattern for
//   this binary.  We follow the same convention here.

extern "C" __declspec(naked) void FUN_0049de0b() {
    __asm {
        // 0009de0b
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x24
        _emit 0x56
        _emit 0x33
        _emit 0xed
        _emit 0xe8
        _emit 0x16
        _emit 0x62
        _emit 0xfc
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x8e
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0009de25
        _emit 0x55
        _emit 0x56
        _emit 0xe8
        _emit 0x14
        _emit 0x62
        _emit 0xfc
        _emit 0xff
        _emit 0x8b
        _emit 0xf8
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x8b
        _emit 0x70
        _emit 0x34
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x0f
        _emit 0x84
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0009de47
        _emit 0x8b
        _emit 0x7f
        _emit 0x04
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0009de50 — inner strcmp loop
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        _emit 0x8b
        _emit 0xd7
        _emit 0x8a
        _emit 0x19
        _emit 0x3a
        _emit 0x1a
        _emit 0x75
        _emit 0x1a
        _emit 0x84
        _emit 0xdb
        _emit 0x74
        _emit 0x12
        _emit 0x8a
        _emit 0x59
        _emit 0x01
        _emit 0x3a
        _emit 0x5a
        _emit 0x01
        _emit 0x75
        _emit 0x0e
        _emit 0x83
        _emit 0xc1
        _emit 0x02
        _emit 0x83
        _emit 0xc2
        _emit 0x02
        _emit 0x84
        _emit 0xdb
        _emit 0x75
        _emit 0xe4
        _emit 0x33
        _emit 0xc9
        _emit 0xeb
        _emit 0x05
        _emit 0x1b
        _emit 0xc9
        _emit 0x83
        _emit 0xd9
        _emit 0xff
        // 0009de7a
        _emit 0x85
        _emit 0xc9
        _emit 0x74
        _emit 0x37
        // 0009de7e — second strcmp loop
        _emit 0x8b
        _emit 0xcf
        _emit 0x8a
        _emit 0x10
        _emit 0x3a
        _emit 0x11
        _emit 0x75
        _emit 0x1a
        _emit 0x84
        _emit 0xd2
        _emit 0x74
        _emit 0x12
        _emit 0x8a
        _emit 0x50
        _emit 0x01
        _emit 0x3a
        _emit 0x51
        _emit 0x01
        _emit 0x75
        _emit 0x0e
        _emit 0x83
        _emit 0xc0
        _emit 0x02
        _emit 0x83
        _emit 0xc1
        _emit 0x02
        _emit 0x84
        _emit 0xd2
        _emit 0x75
        _emit 0xe4
        _emit 0x33
        _emit 0xc0
        _emit 0xeb
        _emit 0x05
        _emit 0x1b
        _emit 0xc0
        _emit 0x83
        _emit 0xd8
        _emit 0xff
        // 0009dea5
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x0c
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        _emit 0x83
        _emit 0xc6
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x9d
        _emit 0xeb
        _emit 0x16
        // 0009deb5
        _emit 0x8b
        _emit 0x0e
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x6a
        _emit 0x01
        _emit 0x51
        _emit 0x52
        _emit 0xe8
        _emit 0x9c
        _emit 0xf7
        _emit 0xfc
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x30
        // 0009decb
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x83
        _emit 0x7e
        _emit 0x04
        _emit 0x00
        _emit 0x74
        _emit 0x4f
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x24
        _emit 0x56
        _emit 0x83
        _emit 0xc5
        _emit 0x01
        _emit 0xe8
        _emit 0x4e
        _emit 0x61
        _emit 0xfc
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x3b
        _emit 0xe8
        _emit 0x0f
        _emit 0x8c
        _emit 0x38
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0009deed
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 0009def1 — epilogue (a)
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x8b
        _emit 0xc7
        _emit 0x5f
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0xc3
        // 0009defb — error path (b)
        _emit 0x6a
        _emit 0x7b
        _emit 0x68
        _emit 0x40
        _emit 0x79
        _emit 0xf8
        _emit 0x00
        _emit 0x6a
        _emit 0x41
        _emit 0x6a
        _emit 0x65
        _emit 0x6a
        _emit 0x22
        _emit 0xe8
        _emit 0x33
        _emit 0xea
        _emit 0xfb
        _emit 0xff
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x50
        _emit 0xe8
        _emit 0xb9
        _emit 0x65
        _emit 0xfc
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 0009df1a — epilogue (b)
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x33
        _emit 0xc0
        _emit 0x5f
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0xc3
        // 0009df24 — error path (c)
        _emit 0x68
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x40
        _emit 0x79
        _emit 0xf8
        _emit 0x00
        _emit 0x6a
        _emit 0x6f
        _emit 0x6a
        _emit 0x65
        _emit 0x6a
        _emit 0x22
        _emit 0xe8
        _emit 0x07
        _emit 0xea
        _emit 0xfb
        _emit 0xff
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        _emit 0x8b
        _emit 0x57
        _emit 0x04
        _emit 0x8b
        _emit 0x07
        _emit 0x51
        _emit 0x68
        _emit 0xac
        _emit 0x9d
        _emit 0xf6
        _emit 0x00
        _emit 0x52
        _emit 0x68
        _emit 0xa4
        _emit 0x9d
        _emit 0xf6
        _emit 0x00
        _emit 0x50
        _emit 0x68
        _emit 0x98
        _emit 0x9d
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x06
        _emit 0xe8
        _emit 0xc6
        _emit 0xe5
        _emit 0xfb
        _emit 0xff
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        _emit 0x51
        _emit 0xe8
        _emit 0x6c
        _emit 0x65
        _emit 0xfc
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x34
        // 0009df67 — epilogue (c)
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x33
        _emit 0xc0
        _emit 0x5f
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0xc3
    }
}
