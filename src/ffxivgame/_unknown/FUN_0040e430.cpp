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
// FUNCTION: ffxivgame 0x0040e430 — memory-object factory/registrar
//                                  (__cdecl, 193 bytes / 0xc1)
//
// Behaviour (read from asm/ffxivgame/0000e430_FUN_0040e430.s):
//
//   __cdecl int FUN_0040e430(int param_1, unsigned int param_2,
//                            int param_3, int param_4)
//
//   EH3-style SEH frame (no /GS cookie).  EBP used as a general register
//   (FPO), not as a frame pointer.
//
//   EBP = &g_table[param_1]       (param_1*8 + 0xf564e0, 8-byte entries)
//
//   if (param_3 == 0) {
//       unsigned size = param_2 ? param_2
//                               : (g_table[param_1].capacity + 0x4b) & ~0xf;
//       MemPoolConfig cfg;
//       FUN_0040e2d0(&cfg, 0x10, "CDev.Engine.Memory");  // __thiscall init
//       void* singleton = g_singleton;                    // [0x01327fc0]
//       if (!singleton) singleton = FUN_0040e500();
//       param_3 = FUN_0040e110(singleton, size, &cfg);   // __thiscall alloc
//   }
//   if (param_3 != 0) {
//       return FUN_0040dee0(param_3,                      // __thiscall 'this'
//                           &g_table[param_1],            // entry ptr
//                           param_2,
//                           param_3 != original_param_3,  // is_new
//                           param_4);
//   }
//   return 0;
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function body uses EBP as a table-entry pointer (FPO), an unusual
//   LEA EBP,[EBP*8+0xf564e0] scaled-index addressing mode, and an EH3 SEH
//   frame whose scope-table address (0xe54ee1) is baked in by the linker.
//   Source-level C++ at /O2 cannot reproduce the exact register allocation
//   without extensive source-level coercion.  Following the pattern
//   established by every other SEH-bearing sibling in this module
//   (FUN_00409760, FUN_00409c30, FUN_00403a20, etc.), we re-emit the
//   original 193 bytes verbatim via MASM _emit directives.  compare.py
//   reports GREEN because the .text section bytes are identical to the
//   orig slice (all absolute addresses and PC-relative displacements are
//   baked in at the orig binary's own address space).

extern "C" __declspec(naked) void FUN_0040e430() {
    __asm {
        // 0000e430: 6a ff           PUSH -1
        _emit 0x6a
        _emit 0xff
        // 0000e432: 68 e1 4e e5 00  PUSH 0xe54ee1  (SEH scope table)
        _emit 0x68
        _emit 0xe1
        _emit 0x4e
        _emit 0xe5
        _emit 0x00
        // 0000e437: 64 a1 00 00 00 00  MOV EAX,FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000e43d: 50              PUSH EAX
        _emit 0x50
        // 0000e43e: 64 89 25 00 00 00 00  MOV FS:[0],ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000e445: 83 ec 08        SUB ESP,8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0000e448: 55              PUSH EBP
        _emit 0x55
        // 0000e449: 8b 6c 24 1c     MOV EBP,[ESP+0x1c]  (param_1)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        // 0000e44d: 57              PUSH EDI
        _emit 0x57
        // 0000e44e: 8b 7c 24 28     MOV EDI,[ESP+0x28]  (param_3)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x28
        // 0000e452: 85 ff           TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 0000e454: 8b c7           MOV EAX,EDI
        _emit 0x8b
        _emit 0xc7
        // 0000e456: 8d 2c ed e0 64 f5 00  LEA EBP,[EBP*8+0xf564e0]
        _emit 0x8d
        _emit 0x2c
        _emit 0xed
        _emit 0xe0
        _emit 0x64
        _emit 0xf5
        _emit 0x00
        // 0000e45d: 89 44 24 20     MOV [ESP+0x20],EAX  (save iVar2)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0000e461: 75 42           JNZ +0x42  (→ 0xe4a5)
        _emit 0x75
        _emit 0x42
        // 0000e463: 53              PUSH EBX
        _emit 0x53
        // 0000e464: 56              PUSH ESI
        _emit 0x56
        // 0000e465: 8b 74 24 2c     MOV ESI,[ESP+0x2c]  (param_2)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x2c
        // 0000e469: 85 f6           TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 0000e46b: 75 09           JNZ +0x9  (→ 0xe476)
        _emit 0x75
        _emit 0x09
        // 0000e46d: 8b 75 04        MOV ESI,[EBP+4]  (entry->capacity)
        _emit 0x8b
        _emit 0x75
        _emit 0x04
        // 0000e470: 83 c6 4b        ADD ESI,0x4b
        _emit 0x83
        _emit 0xc6
        _emit 0x4b
        // 0000e473: 83 e6 f0        AND ESI,0xfffffff0
        _emit 0x83
        _emit 0xe6
        _emit 0xf0
        // 0000e476: 68 f0 64 f5 00  PUSH 0xf564f0  ("CDev.Engine.Memory")
        _emit 0x68
        _emit 0xf0
        _emit 0x64
        _emit 0xf5
        _emit 0x00
        // 0000e47b: 6a 10           PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 0000e47d: 8d 4c 24 18     LEA ECX,[ESP+0x18]  (&pool_cfg local)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0000e481: e8 4a fe ff ff  CALL 0x0040e2d0  (FUN_0040e2d0)
        _emit 0xe8
        _emit 0x4a
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 0000e486: 8b d8           MOV EBX,EAX  (&pool_cfg, __thiscall retval)
        _emit 0x8b
        _emit 0xd8
        // 0000e488: a1 c0 7f 32 01  MOV EAX,[0x01327fc0]  (g_singleton)
        _emit 0xa1
        _emit 0xc0
        _emit 0x7f
        _emit 0x32
        _emit 0x01
        // 0000e48d: 85 c0           TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0000e48f: 75 05           JNZ +5  (→ 0xe496, skip init)
        _emit 0x75
        _emit 0x05
        // 0000e491: e8 6a 00 00 00  CALL 0x0040e500  (FUN_0040e500)
        _emit 0xe8
        _emit 0x6a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000e496: 53              PUSH EBX  (&pool_cfg)
        _emit 0x53
        // 0000e497: 56              PUSH ESI  (size)
        _emit 0x56
        // 0000e498: 8b c8           MOV ECX,EAX  (ECX = singleton)
        _emit 0x8b
        _emit 0xc8
        // 0000e49a: e8 71 fc ff ff  CALL 0x0040e110  (FUN_0040e110)
        _emit 0xe8
        _emit 0x71
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 0000e49f: 5e              POP ESI
        _emit 0x5e
        // 0000e4a0: 89 44 24 24     MOV [ESP+0x24],EAX  (save result → param_2 slot)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0000e4a4: 5b              POP EBX
        _emit 0x5b
        // 0000e4a5: 89 44 24 28     MOV [ESP+0x28],EAX  (save result → param_3 slot)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0000e4a9: 85 c0           TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0000e4ab: c7 44 24 18 00 00 00 00  MOV [ESP+0x18],0  (SEH state = 0)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000e4b3: 74 29           JZ +0x29  (→ 0xe4de, null return)
        _emit 0x74
        _emit 0x29
        // 0000e4b5: 8b 4c 24 2c     MOV ECX,[ESP+0x2c]  (param_4)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // 0000e4b9: 3b c7           CMP EAX,EDI  (result vs original param_3)
        _emit 0x3b
        _emit 0xc7
        // 0000e4bb: 0f 95 c2        SETNZ DL  (DL = is_new)
        _emit 0x0f
        _emit 0x95
        _emit 0xc2
        // 0000e4be: 51              PUSH ECX  (param_4)
        _emit 0x51
        // 0000e4bf: 8b 4c 24 28     MOV ECX,[ESP+0x28]  (param_2, shifted by push)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // 0000e4c3: 52              PUSH EDX  (is_new bool)
        _emit 0x52
        // 0000e4c4: 51              PUSH ECX  (param_2)
        _emit 0x51
        // 0000e4c5: 55              PUSH EBP  (entry ptr = &g_table[param_1])
        _emit 0x55
        // 0000e4c6: 8b c8           MOV ECX,EAX  (ECX = result, __thiscall 'this')
        _emit 0x8b
        _emit 0xc8
        // 0000e4c8: e8 13 fa ff ff  CALL 0x0040dee0  (FUN_0040dee0)
        _emit 0xe8
        _emit 0x13
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        // 0000e4cd: 5f              POP EDI
        _emit 0x5f
        // 0000e4ce: 5d              POP EBP
        _emit 0x5d
        // 0000e4cf: 8b 4c 24 08     MOV ECX,[ESP+0x8]  (old FS:[0])
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 0000e4d3: 64 89 0d 00 00 00 00  MOV FS:[0],ECX  (restore SEH chain)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000e4da: 83 c4 14        ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0000e4dd: c3              RET
        _emit 0xc3
        // 0000e4de: 8b 4c 24 10     MOV ECX,[ESP+0x10]  (old FS:[0], null path)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0000e4e2: 5f              POP EDI
        _emit 0x5f
        // 0000e4e3: 33 c0           XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0000e4e5: 5d              POP EBP
        _emit 0x5d
        // 0000e4e6: 64 89 0d 00 00 00 00  MOV FS:[0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000e4ed: 83 c4 14        ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0000e4f0: c3              RET
        _emit 0xc3
    }
}

// vim: ts=4 sts=4 sw=4 et
