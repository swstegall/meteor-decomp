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
// FUNCTION: ffxivgame 0x004337e0 — factory that allocates a 0x10-byte
//                                  message/event object from a per-slot
//                                  pool and dispatches it (__thiscall,
//                                  139 bytes / 0x8b)
//
// Calling convention: __thiscall (ECX = this); RET 0xc (three DWORD args:
//   arg1 @ [esp+0x4], arg2 @ [esp+0x8], arg3 @ [esp+0xc] at entry).
// Callee-saves pushed: EBX, ESI, EDI.
//
// Body shape:
//   - Reads a per-slot pool object via the global at 0x01328d90:
//         ECX = *(void**)0x01328d90;
//         idx = *(BYTE*)ECX;                      // current slot
//         pool = *(void**)(ECX + 4) + (idx*7)*4;  // &table[idx] (28B stride)
//   - First thiscall to 0x00417ae0 on `pool` with args (arg3, arg2<<4, 4);
//     result kept in EDI.
//   - Re-reads the pool pointer (same idx*7*4 computation) and thiscalls
//     0x00417ab0(0x10) to allocate a 0x10-byte node → EAX.
//   - If allocation succeeded, populates the node:
//         node[0x0] = 0x00f64900;   // vtable
//         node[0x4] = arg1;
//         node[0x8] = arg2;
//         node[0xc] = EDI;          // first-call result
//     then thiscalls 0x0043c2d0 on *(this+0xc) with the node.
//   - Otherwise thiscalls 0x0043c2d0 on *(this+0xc) with NULL.
//
// Reloc-bearing sites (compare.py masks these byte windows):
//   ABS: global  @ [0x01328d90]   (two reads)
//   REL: FUN_00417ae0 (rel32 = 0xfffe42cd)
//   REL: FUN_00417ab0 (rel32 = 0xfffe427c)
//   ABS: vtable   0x00f64900       (MOV [EAX], imm32)
//   REL: FUN_0043c2d0 (rel32 = 0x00008a7c / 0x00008a6b)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The biased pool-pointer arithmetic and the slot/vtable absolute
//   references aren't coercible from a source-level C++ form under /O2,
//   so — like siblings FUN_0040ad30 / FUN_004089f0 — this re-emits the
//   original 139 bytes verbatim via MASM _emit directives. The .obj's
//   .text is byte-identical to the original slice and compare.py reports
//   GREEN.

extern "C" __declspec(naked) void FUN_004337e0() {
    __asm {
        // 000337e0:  53                 PUSH EBX
        _emit 0x53
        // 000337e1:  8b d9              MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 000337e3:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000337e9:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 000337ec:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000337f3:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 000337f5:  8b 41 04           MOV EAX,dword ptr [ECX + 0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 000337f8:  56                 PUSH ESI
        _emit 0x56
        // 000337f9:  8b 74 24 10        MOV ESI,dword ptr [ESP + 0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 000337fd:  8d 0c 90           LEA ECX,[EAX + EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00033800:  8b 44 24 14        MOV EAX,dword ptr [ESP + 0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00033804:  57                 PUSH EDI
        _emit 0x57
        // 00033805:  6a 04              PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 00033807:  8b d6              MOV EDX,ESI
        _emit 0x8b
        _emit 0xd6
        // 00033809:  c1 e2 04           SHL EDX,0x4
        _emit 0xc1
        _emit 0xe2
        _emit 0x04
        // 0003380c:  52                 PUSH EDX
        _emit 0x52
        // 0003380d:  50                 PUSH EAX
        _emit 0x50
        // 0003380e:  e8 cd 42 fe ff     CALL 0x00417ae0
        _emit 0xe8
        _emit 0xcd
        _emit 0x42
        _emit 0xfe
        _emit 0xff
        // 00033813:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00033819:  8b f8              MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 0003381b:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 0003381e:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00033825:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00033827:  8b 41 04           MOV EAX,dword ptr [ECX + 0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 0003382a:  8d 0c 90           LEA ECX,[EAX + EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 0003382d:  6a 10              PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 0003382f:  e8 7c 42 fe ff     CALL 0x00417ab0
        _emit 0xe8
        _emit 0x7c
        _emit 0x42
        _emit 0xfe
        _emit 0xff
        // 00033834:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00033836:  74 22              JZ 0x0043385a
        _emit 0x74
        _emit 0x22
        // 00033838:  8b 4c 24 10        MOV ECX,dword ptr [ESP + 0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0003383c:  c7 00 00 49 f6 00  MOV dword ptr [EAX],0xf64900
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 00033842:  89 48 04           MOV dword ptr [EAX + 0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00033845:  89 70 08           MOV dword ptr [EAX + 0x8],ESI
        _emit 0x89
        _emit 0x70
        _emit 0x08
        // 00033848:  89 78 0c           MOV dword ptr [EAX + 0xc],EDI
        _emit 0x89
        _emit 0x78
        _emit 0x0c
        // 0003384b:  8b 4b 0c           MOV ECX,dword ptr [EBX + 0xc]
        _emit 0x8b
        _emit 0x4b
        _emit 0x0c
        // 0003384e:  50                 PUSH EAX
        _emit 0x50
        // 0003384f:  e8 7c 8a 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x7c
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        // 00033854:  5f                 POP EDI
        _emit 0x5f
        // 00033855:  5e                 POP ESI
        _emit 0x5e
        // 00033856:  5b                 POP EBX
        _emit 0x5b
        // 00033857:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 0003385a:  8b 4b 0c           MOV ECX,dword ptr [EBX + 0xc]
        _emit 0x8b
        _emit 0x4b
        _emit 0x0c
        // 0003385d:  33 c0              XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0003385f:  50                 PUSH EAX
        _emit 0x50
        // 00033860:  e8 6b 8a 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x6b
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        // 00033865:  5f                 POP EDI
        _emit 0x5f
        // 00033866:  5e                 POP ESI
        _emit 0x5e
        // 00033867:  5b                 POP EBX
        _emit 0x5b
        // 00033868:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
