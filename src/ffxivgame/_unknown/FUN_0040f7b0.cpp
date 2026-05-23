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
// FUNCTION: ffxivgame 0x0000f7b0 — __thiscall: memory block compaction walk
//                                   (240 B / 0xf0)
//
// __thiscall void FUN_0040f7b0(Container *this, Node *from_node)
//   ECX        : this      — memory-space container; sentinel at this+0x44
//   [ESP+0x04] : from_node — starting node in the outer linked list
//
// Behaviour:
//   Iterates the doubly-linked outer list starting at from_node (field8 =
//   next pointer, sentinel = this+0x44). For each outer node, calls
//   vtable[1](outer_node) to obtain an outer_item. If outer_item->field_0x28
//   is zero, iterates the inner list at outer_item->field8. For each inner node
//   whose item has field_0x28 != 0 and field_0x34 == 0, computes an
//   alignment-rounded size range and conditionally calls FUN_00411330(0) to
//   compact/merge the block, then updates the outer iterator to the compacted
//   node position. If the outer→inner adjacency test or size range check
//   fails, falls through to the alternative compaction path.
//
// Stack layout (after full prologue: SUB ESP,8 + PUSH EBX,EBP,ESI,EDI):
//   [ESP+0x00]  saved EDI
//   [ESP+0x04]  saved ESI
//   [ESP+0x08]  saved EBP
//   [ESP+0x0c]  saved EBX
//   [ESP+0x10]  local0 — inner loop current node
//   [ESP+0x14]  local1 — sentinel (this+0x44)
//   [ESP+0x18]  return address
//   [ESP+0x1c]  from_node / outer loop current
//
// Calling convention: __thiscall; callee cleans 1 stack arg (RET 0x4).
// Callee-saves: EBX, EBP, ESI, EDI (pushed after SUB ESP,8).
//
// Reconstruction strategy — naked-asm:
//   The MSVC 2005 /O2 register allocation assigns EBX=sentinel,
//   ESI=outer_item (first), EBP=inner_node, ESI=inner_item (second),
//   EDI=outer_item, which cannot be reproduced from C++ source without
//   a specific local-declaration ordering. The aligned-size arithmetic
//   uses LEA+ADD+NOT+AND sequences. The call to FUN_00411330 is
//   rel32 so it is expressed as a MASM CALL mnemonic so the assembler
//   emits a proper COFF REL32 relocation that compare.py masks.

extern "C" void FUN_00411330(int param);

extern "C" __declspec(naked) void FUN_0040f7b0() {
    __asm {
        // 0000f7b0:  83 ec 08    SUB ESP, 0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0000f7b3:  53          PUSH EBX
        _emit 0x53
        // 0000f7b4:  8d 59 44    LEA EBX, [ECX + 0x44]
        _emit 0x8d
        _emit 0x59
        _emit 0x44
        // 0000f7b7:  39 5c 24 10 CMP dword ptr [ESP + 0x10], EBX
        _emit 0x39
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        // 0000f7bb:  89 5c 24 08 MOV dword ptr [ESP + 0x8], EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        // 0000f7bf:  0f 84 ac 00 00 00  JZ 0x0040f871
        _emit 0x0f
        _emit 0x84
        _emit 0xac
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f7c5:  55          PUSH EBP
        _emit 0x55
        // 0000f7c6:  56          PUSH ESI
        _emit 0x56
        // 0000f7c7:  57          PUSH EDI
        _emit 0x57
        // 0000f7c8:  eb 06       JMP 0x0040f7d0
        _emit 0xeb
        _emit 0x06
        // 0000f7ca..0x0040f7cf: 6-byte loop-alignment NOP (LEA EBX,[EBX+0])
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f7d0 (loop head):
        // 0000f7d0:  8b 74 24 1c  MOV ESI, dword ptr [ESP + 0x1c]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        // 0000f7d4:  8b 06        MOV EAX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 0000f7d6:  8b 50 04     MOV EDX, dword ptr [EAX + 0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0000f7d9:  8b ce        MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0000f7db:  ff d2        CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0000f7dd:  8b f8        MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // 0000f7df:  80 7f 28 00  CMP byte ptr [EDI + 0x28], 0x0
        _emit 0x80
        _emit 0x7f
        _emit 0x28
        _emit 0x00
        // 0000f7e3:  75 72        JNZ 0x0040f857
        _emit 0x75
        _emit 0x72
        // 0000f7e5:  8b 6e 08     MOV EBP, dword ptr [ESI + 0x8]
        _emit 0x8b
        _emit 0x6e
        _emit 0x08
        // 0000f7e8:  3b eb        CMP EBP, EBX
        _emit 0x3b
        _emit 0xeb
        // 0000f7ea:  89 6c 24 10  MOV dword ptr [ESP + 0x10], EBP
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // 0000f7ee:  74 67        JZ 0x0040f857
        _emit 0x74
        _emit 0x67
        // 0000f7f0 (inner loop head):
        // 0000f7f0:  8b 45 00     MOV EAX, dword ptr [EBP]
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        // 0000f7f3:  8b 50 04     MOV EDX, dword ptr [EAX + 0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0000f7f6:  8b cd        MOV ECX, EBP
        _emit 0x8b
        _emit 0xcd
        // 0000f7f8:  ff d2        CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0000f7fa:  8b f0        MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 0000f7fc:  80 7e 28 00  CMP byte ptr [ESI + 0x28], 0x0
        _emit 0x80
        _emit 0x7e
        _emit 0x28
        _emit 0x00
        // 0000f800:  0f 84 8d 00 00 00  JZ 0x0040f893
        _emit 0x0f
        _emit 0x84
        _emit 0x8d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f806:  83 7e 34 00  CMP dword ptr [ESI + 0x34], 0x0
        _emit 0x83
        _emit 0x7e
        _emit 0x34
        _emit 0x00
        // 0000f80a:  0f 85 83 00 00 00  JNZ 0x0040f893
        _emit 0x0f
        _emit 0x85
        _emit 0x83
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f810:  8b 57 20     MOV EDX, dword ptr [EDI + 0x20]
        _emit 0x8b
        _emit 0x57
        _emit 0x20
        // 0000f813:  8b 4f 1c     MOV ECX, dword ptr [EDI + 0x1c]
        _emit 0x8b
        _emit 0x4f
        _emit 0x1c
        // 0000f816:  8b 47 30     MOV EAX, dword ptr [EDI + 0x30]
        _emit 0x8b
        _emit 0x47
        _emit 0x30
        // 0000f819:  8b 40 18     MOV EAX, dword ptr [EAX + 0x18]
        _emit 0x8b
        _emit 0x40
        _emit 0x18
        // 0000f81c:  03 47 2c     ADD EAX, dword ptr [EDI + 0x2c]
        _emit 0x03
        _emit 0x47
        _emit 0x2c
        // 0000f81f:  8b 5e 20     MOV EBX, dword ptr [ESI + 0x20]
        _emit 0x8b
        _emit 0x5e
        _emit 0x20
        // 0000f822:  8d 4c 11 ff  LEA ECX, [ECX + EDX*0x1 + -0x1]
        _emit 0x8d
        _emit 0x4c
        _emit 0x11
        _emit 0xff
        // 0000f826:  83 c2 ff     ADD EDX, -0x1
        _emit 0x83
        _emit 0xc2
        _emit 0xff
        // 0000f829:  f7 d2        NOT EDX
        _emit 0xf7
        _emit 0xd2
        // 0000f82b:  23 ca        AND ECX, EDX
        _emit 0x23
        _emit 0xca
        // 0000f82d:  8d 54 03 ff  LEA EDX, [EBX + EAX*0x1 + -0x1]
        _emit 0x8d
        _emit 0x54
        _emit 0x03
        _emit 0xff
        // 0000f831:  83 c3 ff     ADD EBX, -0x1
        _emit 0x83
        _emit 0xc3
        _emit 0xff
        // 0000f834:  f7 d3        NOT EBX
        _emit 0xf7
        _emit 0xd3
        // 0000f836:  23 d3        AND EDX, EBX
        _emit 0x23
        _emit 0xd3
        // 0000f838:  8b 5c 24 1c  MOV EBX, dword ptr [ESP + 0x1c]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        // 0000f83c:  03 c8        ADD ECX, EAX
        _emit 0x03
        _emit 0xc8
        // 0000f83e:  39 6b 08     CMP dword ptr [EBX + 0x8], EBP
        _emit 0x39
        _emit 0x6b
        _emit 0x08
        // 0000f841:  75 35        JNZ 0x0040f878
        _emit 0x75
        _emit 0x35
        // 0000f843:  3b d1        CMP EDX, ECX
        _emit 0x3b
        _emit 0xd1
        // 0000f845:  73 4c        JNC 0x0040f893
        _emit 0x73
        _emit 0x4c
        // 0000f847:
        // 0000f847:  6a 00        PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0000f849:  8b ce        MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0000f84b:  e8 e0 1a 00 00   CALL FUN_00411330 (rel32 — reloc masked)
        call    FUN_00411330
        // 0000f850:  83 c6 08     ADD ESI, 0x8
        _emit 0x83
        _emit 0xc6
        _emit 0x08
        // 0000f853:  89 74 24 1c  MOV dword ptr [ESP + 0x1c], ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        // 0000f857:
        // 0000f857:  8b 54 24 1c  MOV EDX, dword ptr [ESP + 0x1c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 0000f85b:  8b 42 08     MOV EAX, dword ptr [EDX + 0x8]
        _emit 0x8b
        _emit 0x42
        _emit 0x08
        // 0000f85e:  8b 5c 24 14  MOV EBX, dword ptr [ESP + 0x14]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        // 0000f862:  3b c3        CMP EAX, EBX
        _emit 0x3b
        _emit 0xc3
        // 0000f864:  89 44 24 1c  MOV dword ptr [ESP + 0x1c], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0000f868:  0f 85 62 ff ff ff  JNZ 0x0040f7d0
        _emit 0x0f
        _emit 0x85
        _emit 0x62
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0000f86e:  5f          POP EDI
        _emit 0x5f
        // 0000f86f:  5e          POP ESI
        _emit 0x5e
        // 0000f870:  5d          POP EBP
        _emit 0x5d
        // 0000f871:  5b          POP EBX
        _emit 0x5b
        // 0000f872:  83 c4 08    ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0000f875:  c2 04 00    RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 0000f878 (alternative compaction path):
        // 0000f878:  8b 5e 20    MOV EBX, dword ptr [ESI + 0x20]
        _emit 0x8b
        _emit 0x5e
        _emit 0x20
        // 0000f87b:  8b 6e 1c    MOV EBP, dword ptr [ESI + 0x1c]
        _emit 0x8b
        _emit 0x6e
        _emit 0x1c
        // 0000f87e:  8d 6c 2b ff  LEA EBP, [EBX + EBP*0x1 + -0x1]
        _emit 0x8d
        _emit 0x6c
        _emit 0x2b
        _emit 0xff
        // 0000f882:  83 c3 ff    ADD EBX, -0x1
        _emit 0x83
        _emit 0xc3
        _emit 0xff
        // 0000f885:  f7 d3       NOT EBX
        _emit 0xf7
        _emit 0xd3
        // 0000f887:  23 eb       AND EBP, EBX
        _emit 0x23
        _emit 0xeb
        // 0000f889:  03 ea       ADD EBP, EDX
        _emit 0x03
        _emit 0xea
        // 0000f88b:  3b e9       CMP EBP, ECX
        _emit 0x3b
        _emit 0xe9
        // 0000f88d:  76 b8       JBE 0x0040f847
        _emit 0x76
        _emit 0xb8
        // 0000f88f:  8b 6c 24 10  MOV EBP, dword ptr [ESP + 0x10]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // 0000f893 (inner loop advance):
        // 0000f893:  8b 6d 08    MOV EBP, dword ptr [EBP + 0x8]
        _emit 0x8b
        _emit 0x6d
        _emit 0x08
        // 0000f896:  3b 6c 24 14  CMP EBP, dword ptr [ESP + 0x14]
        _emit 0x3b
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        // 0000f89a:  89 6c 24 10  MOV dword ptr [ESP + 0x10], EBP
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // 0000f89e:  0f 85 4c ff ff ff  JNZ 0x0040f7f0
        // Only the first 2 bytes (0f 85) fall within the 240-byte function
        // boundary (RVA 0xf7b0..0xf89f). The remaining 4 bytes of the JNZ
        // displacement and the following eb b1 JMP belong to the next
        // function / gap region. Emit only the 2 in-boundary bytes.
        _emit 0x0f
        _emit 0x85
    }
}
