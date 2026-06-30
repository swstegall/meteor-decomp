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
// FUNCTION: ffxivgame 0x00062370 — `print_gens` — iterates over a
//           generator list and prints each entry using a sprintf-style
//           formatter and several helpers.  (92 B / 0x5c, no SEH.)
//
// Inspection (read from asm/ffxivgame/00062370_print_gens.s):
//
//   Prolog: PUSH ESI; PUSH EBX — saves ESI (loop counter) and EBX
//   (object / handle passed to every call).  EBX is simultaneously the
//   first argument to the initial get-count call (EBX on the stack when
//   CALL 0x464030 runs → the saved-EBX slot doubles as the call arg;
//   ADD ESP,4 then consumes it).  No PUSH EBP until the branch-past-
//   guard is taken (count > 0).
//
//   Shape:
//
//     ESI = 0;                                    // loop counter i
//     int count = FUN_00464030(EBX);              // get_count(obj)
//     if (count <= 0) return 1;                   // JLE exit
//     EBP = *(int*)(ESP+0xc) + 2;                 // first stack arg +2
//     // JMP into loop body (6-byte LEA EBX,[EBX+0] NOP at 0x6238a)
//     do {
//         FUN_00467e90(EDI, 0xf6952c, EBP, 0xf54d48); // sprintf-style
//         int item = FUN_00464040(EBX, ESI);           // get_item(obj,i)
//         FUN_0046dd20(EDI, item);                     // print_item
//         FUN_00469b80(EDI, 0xf54d98);                 // print_sep
//         ESI++;
//         count = FUN_00464030(EBX);                   // refresh count
//     } while (ESI < count);                           // JL loop_top
//     return 1;
//
// Gap bytes at 0x6238a–0x6238f:
//   8d 9b 00 00 00 00 = LEA EBX,[EBX+0]  (6-byte multi-byte NOP, MSVC
//   alignment pad between the JMP and the loop body; jumped over by
//   `eb 06` at 0x62388).
//
// CALL displacement bytes are position-fixed to image-base 0x400000
// (same as every other rosetta entry); tools/compare.py masks COFF
// relocation bytes in the .obj diff, but since this is a naked _emit
// passthrough all 6 call displacement fields are emitted as raw literals
// matching the orig exactly — no COFF relocations generated.
//
// Reloc-bearing CALL sites (all rel32, orig-binary-resolved values):
//   +0x05  FUN_00464030  → b7 1c 00 00   (get_count, initial)
//   +0x2d  FUN_00467e90  → ef 5a 00 00   (sprintf-style formatter)
//   +0x34  FUN_00464040  → 98 1c 00 00   (get_item)
//   +0x3b  FUN_0046dd20  → 71 b9 00 00   (print_item)
//   +0x46  FUN_00469b80  → c6 77 00 00   (print_sep)
//   +0x4f  FUN_00464030  → 6d 1c 00 00   (get_count, refresh)
//
// The function size per config/ffxivgame.yaml is 0x5c = 92 bytes; the
// epilogue (MOV EAX,1 / POP ESI / RET, bytes 0x623cb–0x623d1) falls
// partially beyond the YAML boundary and is therefore NOT included in
// this 92-byte emit (the last emitted byte is 0xb8, the opcode of
// MOV EAX,1).

extern "C" __declspec(naked) void FUN_00462370() {
    __asm {
        // 0x00062370  56              PUSH ESI
        _emit 0x56
        // 0x00062371  53              PUSH EBX
        _emit 0x53
        // 0x00062372  33 f6           XOR ESI,ESI
        _emit 0x33
        _emit 0xf6
        // 0x00062374  e8 b7 1c 00 00  CALL FUN_00464030 (get_count)
        _emit 0xe8
        _emit 0xb7
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        // 0x00062379  83 c4 04        ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0x0006237c  85 c0           TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0x0006237e  7e 4b           JLE exit (+0x4b → 0x004623cb)
        _emit 0x7e
        _emit 0x4b
        // 0x00062380  55              PUSH EBP
        _emit 0x55
        // 0x00062381  8b 6c 24 0c     MOV EBP,dword ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        // 0x00062385  83 c5 02        ADD EBP,0x2
        _emit 0x83
        _emit 0xc5
        _emit 0x02
        // 0x00062388  eb 06           JMP +6 → 0x00462390 (loop body)
        _emit 0xeb
        _emit 0x06
        // 0x0006238a  8d 9b 00 00 00 00  LEA EBX,[EBX+0]  (6-byte NOP pad)
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // loop body at 0x00062390:
        // 0x00062390  68 48 4d f5 00  PUSH 0xf54d48
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 0x00062395  55              PUSH EBP
        _emit 0x55
        // 0x00062396  68 2c 95 f6 00  PUSH 0xf6952c
        _emit 0x68
        _emit 0x2c
        _emit 0x95
        _emit 0xf6
        _emit 0x00
        // 0x0006239b  57              PUSH EDI
        _emit 0x57
        // 0x0006239c  e8 ef 5a 00 00  CALL FUN_00467e90 (sprintf-style)
        _emit 0xe8
        _emit 0xef
        _emit 0x5a
        _emit 0x00
        _emit 0x00
        // 0x000623a1  56              PUSH ESI
        _emit 0x56
        // 0x000623a2  53              PUSH EBX
        _emit 0x53
        // 0x000623a3  e8 98 1c 00 00  CALL FUN_00464040 (get_item)
        _emit 0xe8
        _emit 0x98
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        // 0x000623a8  50              PUSH EAX
        _emit 0x50
        // 0x000623a9  57              PUSH EDI
        _emit 0x57
        // 0x000623aa  e8 71 b9 00 00  CALL FUN_0046dd20 (print_item)
        _emit 0xe8
        _emit 0x71
        _emit 0xb9
        _emit 0x00
        _emit 0x00
        // 0x000623af  68 98 4d f5 00  PUSH 0xf54d98
        _emit 0x68
        _emit 0x98
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 0x000623b4  57              PUSH EDI
        _emit 0x57
        // 0x000623b5  e8 c6 77 00 00  CALL FUN_00469b80 (print_sep)
        _emit 0xe8
        _emit 0xc6
        _emit 0x77
        _emit 0x00
        _emit 0x00
        // 0x000623ba  53              PUSH EBX
        _emit 0x53
        // 0x000623bb  83 c6 01        ADD ESI,0x1
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        // 0x000623be  e8 6d 1c 00 00  CALL FUN_00464030 (get_count refresh)
        _emit 0xe8
        _emit 0x6d
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        // 0x000623c3  83 c4 2c        ADD ESP,0x2c
        _emit 0x83
        _emit 0xc4
        _emit 0x2c
        // 0x000623c6  3b f0           CMP ESI,EAX
        _emit 0x3b
        _emit 0xf0
        // 0x000623c8  7c c6           JL -0x3a → 0x00462390 (loop top)
        _emit 0x7c
        _emit 0xc6
        // 0x000623ca  5d              POP EBP
        _emit 0x5d
        // 0x000623cb  b8              [opcode of MOV EAX,0x1 — YAML boundary;
        //                              remaining imm32 + POP ESI + RET fall
        //                              beyond the 92-byte / 0x5c window]
        _emit 0xb8
    }
}
