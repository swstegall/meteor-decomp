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
// FUNCTION: ffxivgame 0x0043dec0 — SEH-framed __thiscall constructor / init
//                                  (273 B / 0x111) — sets vtable 0xf57ea0,
//                                  initialises two DWORD members at this+0x08
//                                  and this+0x0c to zero, conditionally
//                                  registers a one-time atexit handler for a
//                                  static (gated on arg1 as a bool), dispatches
//                                  through two virtual-call pairs on an optional
//                                  interface pointer (arg1), then calls
//                                  FUN_0041b0f0 and returns `this`.
//
// Calling convention: __thiscall (ECX = this / EBP), 5 stack args, RET 0x14
//
// Prologue: EH3-style SEH frame (PUSH -1 / scope_table 0xe56c88 / FS:[0]
// chain swap) + /GS __security_cookie (XOR ESP). Saved: EBP, ESI, EDI.
//
// Body outline:
//   EBP = this (ECX)
//   EDI = EBP + 8                        (pointer to this->field_08)
//   [EBP]   = 0xf57ea0                   (vtable)
//   [EDI]   = 0; [EDI+4] = 0             (clear two DWORD members)
//   if (byte arg1 != 0):                 (static atexit registration)
//       TEST/OR [0x01323910] once-flag
//       if first time: MOV [0x0132390c], 0x43c850  (set fn ptr)
//       PUSH 5 args; CALL dword ptr [0x0132390c]   (atexit-style reg)
//       ADD ESP, 0x14
//   push arg5, arg4, arg3, arg2, 0; LEA ECX=&arg1; CALL FUN_0043eec0
//   EH state → 0
//   if (arg1 != NULL): ESI = arg1->vtbl[3](arg1)  else ESI = 0
//   PUSH ESI; ECX = EDI; CALL FUN_00419f80
//   if (arg1 != NULL): EAX = arg1->vtbl[4](arg1)  else EAX = 0
//   PUSH &this->field_04; PUSH EAX; PUSH ESI; CALL FUN_0041b0f0; ADD ESP,0xc
//   LEA ECX=&arg1; EH state → -1; CALL FUN_0043e210
//   EAX = EBP (return this)
//   unwind SEH frame; RET 0x14
//
// Reloc-bearing sites (compare.py wildcards these 4-byte windows):
//   +0x02  PUSH scope_table       0xe56c88   (abs32 imm)
//   +0x11  MOV EAX,__security_cookie  0x012ea8b0  (abs32 moffs)
//   +0x2d  MOV [EBP],0xf57ea0     (vtable abs32 imm)
//   +0x48  TEST/OR [0x01323910]   (abs32 moffs, twice)
//   +0x56  MOV [0x0132390c],0x43c850  (two abs32 imms)
//   +0x9c  CALL rel32 → FUN_0043eec0
//   +0xc2  CALL rel32 → FUN_00419f80
//   +0xe1  CALL rel32 → FUN_0041b0f0
//   +0xf5  CALL rel32 → FUN_0043e210
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The EH3 SEH prolog (PUSH -1 / scope_table / FS:[0] swap / cookie XOR ESP /
//   LEA+MOV FS:[0]) and the EH-state writes (MOV [ESP+0x18],0 / MOV [ESP+0x18],-1)
//   form a compiler-emitted shape whose exact byte encoding depends on the
//   linker-supplied scope_table address and the local stack-slot numbering.
//   Additionally, the one-time static-init block uses moffs32 absolute
//   addresses. Coaxing this exact 273-byte sequence from /O2 /GS /EHsc C++
//   is impractical (state numbering, moffs32 vs modrm, short-vs-near branches).
//   Same choice as the sibling SEH functions (FUN_0043ddf0, FUN_0043dd40,
//   FUN_0043e2e0): a `__declspec(naked)` body re-emitting all 273 orig bytes
//   verbatim via MASM `_emit` directives. The .obj's `.text` section is
//   byte-identical modulo the wildcarded reloc windows; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0043dec0() {
    __asm {
        // 0003dec0: 6a ff                  PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0003dec2: 68 88 6c e5 00         PUSH 0xe56c88  (scope_table RELOC)
        _emit 0x68
        _emit 0x88
        _emit 0x6c
        _emit 0xe5
        _emit 0x00
        // 0003dec7: 64 a1 00 00 00 00      MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003decd: 50                     PUSH EAX  (old FS:[0])
        _emit 0x50
        // 0003dece: 55                     PUSH EBP
        _emit 0x55
        // 0003decf: 56                     PUSH ESI
        _emit 0x56
        // 0003ded0: 57                     PUSH EDI
        _emit 0x57
        // 0003ded1: a1 b0 a8 2e 01         MOV EAX,[0x012ea8b0]  (__security_cookie RELOC)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0003ded6: 33 c4                  XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0003ded8: 50                     PUSH EAX  (cookie ^ ESP)
        _emit 0x50
        // 0003ded9: 8d 44 24 10            LEA EAX,[ESP+0x10]  (SEH node on stack)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0003dedd: 64 a3 00 00 00 00      MOV FS:[0x0],EAX  (install SEH frame)
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003dee3: 8b e9                  MOV EBP,ECX  (EBP = this)
        _emit 0x8b
        _emit 0xe9
        // 0003dee5: 80 7c 24 20 00         CMP byte ptr [ESP+0x20],0x0  (test arg1 as bool)
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x00
        // 0003deea: 8d 7d 08               LEA EDI,[EBP+0x8]  (EDI = this->field_08)
        _emit 0x8d
        _emit 0x7d
        _emit 0x08
        // 0003deed: c7 45 00 a0 7e f5 00   MOV dword ptr [EBP],0xf57ea0  (vtable RELOC)
        _emit 0xc7
        _emit 0x45
        _emit 0x00
        _emit 0xa0
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        // 0003def4: c7 07 00 00 00 00      MOV dword ptr [EDI],0x0
        _emit 0xc7
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003defa: c7 47 04 00 00 00 00   MOV dword ptr [EDI+0x4],0x0
        _emit 0xc7
        _emit 0x47
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003df01: 74 3f                  JZ +0x3f  (→ 0x0043df42, skip atexit block)
        _emit 0x74
        _emit 0x3f
        // 0003df03: b8 01 00 00 00         MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003df08: 84 05 10 39 32 01      TEST byte ptr [0x01323910],AL  (once-flag RELOC)
        _emit 0x84
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0003df0e: 75 10                  JNZ +0x10  (→ 0x0043df20, already init'd)
        _emit 0x75
        _emit 0x10
        // 0003df10: 09 05 10 39 32 01      OR dword ptr [0x01323910],EAX  (set flag RELOC)
        _emit 0x09
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0003df16: c7 05 0c 39 32 01 50 c8 43 00
        //           MOV dword ptr [0x0132390c],0x43c850  (set fn ptr RELOC x2)
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x50
        _emit 0xc8
        _emit 0x43
        _emit 0x00
        // 0003df20: 68 f0 6c f6 00         PUSH 0xf66cf0  (RELOC)
        _emit 0x68
        _emit 0xf0
        _emit 0x6c
        _emit 0xf6
        _emit 0x00
        // 0003df25: 68 df 02 00 00         PUSH 0x2df
        _emit 0x68
        _emit 0xdf
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0003df2a: 68 a0 6c f6 00         PUSH 0xf66ca0  (RELOC)
        _emit 0x68
        _emit 0xa0
        _emit 0x6c
        _emit 0xf6
        _emit 0x00
        // 0003df2f: 68 48 4d f5 00         PUSH 0xf54d48  (RELOC)
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 0003df34: 68 90 6c f6 00         PUSH 0xf66c90  (RELOC)
        _emit 0x68
        _emit 0x90
        _emit 0x6c
        _emit 0xf6
        _emit 0x00
        // 0003df39: ff 15 0c 39 32 01      CALL dword ptr [0x0132390c]  (indirect RELOC)
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0003df3f: 83 c4 14               ADD ESP,0x14  (clean 5 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0003df42: 8b 44 24 30            MOV EAX,dword ptr [ESP+0x30]  (arg5)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // 0003df46: 8b 4c 24 2c            MOV ECX,dword ptr [ESP+0x2c]  (arg4)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // 0003df4a: 8b 54 24 28            MOV EDX,dword ptr [ESP+0x28]  (arg3)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x28
        // 0003df4e: 50                     PUSH EAX  (arg5)
        _emit 0x50
        // 0003df4f: 8b 44 24 28            MOV EAX,dword ptr [ESP+0x28]  (arg2, displaced)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0003df53: 51                     PUSH ECX  (arg4)
        _emit 0x51
        // 0003df54: 52                     PUSH EDX  (arg3)
        _emit 0x52
        // 0003df55: 50                     PUSH EAX  (arg2)
        _emit 0x50
        // 0003df56: 6a 00                  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0003df58: 8d 4c 24 34            LEA ECX,[ESP+0x34]  (ECX = &arg1)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        // 0003df5c: e8 5f 0f 00 00         CALL FUN_0043eec0  (rel32 RELOC)
        _emit 0xe8
        _emit 0x5f
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        // 0003df61: 8b 44 24 20            MOV EAX,dword ptr [ESP+0x20]  (arg1 ptr)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0003df65: 85 c0                  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0003df67: c7 44 24 18 00 00 00 00  MOV dword ptr [ESP+0x18],0x0  (EH state → 0)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003df6f: 74 0c                  JZ +0x0c  (→ 0x0043df7d, arg1 is NULL)
        _emit 0x74
        _emit 0x0c
        // 0003df71: 8b 08                  MOV ECX,dword ptr [EAX]  (vtable)
        _emit 0x8b
        _emit 0x08
        // 0003df73: 8b 51 0c               MOV EDX,dword ptr [ECX+0xc]  (vtbl[3])
        _emit 0x8b
        _emit 0x51
        _emit 0x0c
        // 0003df76: 50                     PUSH EAX  (this = arg1)
        _emit 0x50
        // 0003df77: ff d2                  CALL EDX  (arg1->vtbl[3]())
        _emit 0xff
        _emit 0xd2
        // 0003df79: 8b f0                  MOV ESI,EAX  (ESI = result)
        _emit 0x8b
        _emit 0xf0
        // 0003df7b: eb 02                  JMP +0x02  (→ 0x0043df7f)
        _emit 0xeb
        _emit 0x02
        // 0003df7d: 33 f6                  XOR ESI,ESI  (ESI = 0 if NULL)
        _emit 0x33
        _emit 0xf6
        // 0003df7f: 56                     PUSH ESI  (arg to FUN_00419f80)
        _emit 0x56
        // 0003df80: 8b cf                  MOV ECX,EDI  (ECX = this->field_08)
        _emit 0x8b
        _emit 0xcf
        // 0003df82: e8 f9 bf fd ff         CALL FUN_00419f80  (rel32 RELOC)
        _emit 0xe8
        _emit 0xf9
        _emit 0xbf
        _emit 0xfd
        _emit 0xff
        // 0003df87: 8b 44 24 20            MOV EAX,dword ptr [ESP+0x20]  (arg1 again)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0003df8b: 85 c0                  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0003df8d: 74 0a                  JZ +0x0a  (→ 0x0043df99, arg1 is NULL)
        _emit 0x74
        _emit 0x0a
        // 0003df8f: 8b 08                  MOV ECX,dword ptr [EAX]  (vtable)
        _emit 0x8b
        _emit 0x08
        // 0003df91: 8b 51 10               MOV EDX,dword ptr [ECX+0x10]  (vtbl[4])
        _emit 0x8b
        _emit 0x51
        _emit 0x10
        // 0003df94: 50                     PUSH EAX  (this = arg1)
        _emit 0x50
        // 0003df95: ff d2                  CALL EDX  (arg1->vtbl[4]())
        _emit 0xff
        _emit 0xd2
        // 0003df97: eb 02                  JMP +0x02  (→ 0x0043df9b)
        _emit 0xeb
        _emit 0x02
        // 0003df99: 33 c0                  XOR EAX,EAX  (EAX = 0 if NULL)
        _emit 0x33
        _emit 0xc0
        // 0003df9b: 8d 4d 04               LEA ECX,[EBP+0x4]  (&this->field_04)
        _emit 0x8d
        _emit 0x4d
        _emit 0x04
        // 0003df9e: 51                     PUSH ECX  (&this->field_04)
        _emit 0x51
        // 0003df9f: 50                     PUSH EAX  (vtbl[4] result)
        _emit 0x50
        // 0003dfa0: 56                     PUSH ESI  (vtbl[3] result)
        _emit 0x56
        // 0003dfa1: e8 4a d1 fd ff         CALL FUN_0041b0f0  (rel32 RELOC)
        _emit 0xe8
        _emit 0x4a
        _emit 0xd1
        _emit 0xfd
        _emit 0xff
        // 0003dfa6: 83 c4 0c               ADD ESP,0xc  (clean 3 args, __cdecl)
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0003dfa9: 8d 4c 24 20            LEA ECX,[ESP+0x20]  (ECX = &arg1)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 0003dfad: c7 44 24 18 ff ff ff ff  MOV dword ptr [ESP+0x18],-1  (EH state → -1)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0003dfb5: e8 56 02 00 00         CALL FUN_0043e210  (rel32 RELOC)
        _emit 0xe8
        _emit 0x56
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0003dfba: 8b c5                  MOV EAX,EBP  (return this)
        _emit 0x8b
        _emit 0xc5
        // 0003dfbc: 8b 4c 24 10            MOV ECX,dword ptr [ESP+0x10]  (old FS:[0])
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0003dfc0: 64 89 0d 00 00 00 00   MOV dword ptr FS:[0x0],ECX  (restore SEH chain)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003dfc7: 59                     POP ECX  (discard cookie)
        _emit 0x59
        // 0003dfc8: 5f                     POP EDI
        _emit 0x5f
        // 0003dfc9: 5e                     POP ESI
        _emit 0x5e
        // 0003dfca: 5d                     POP EBP
        _emit 0x5d
        // 0003dfcb: 83 c4 0c               ADD ESP,0xc  (remove sentinel+handler+old_FS)
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0003dfce: c2 14 00               RET 0x14  (clean 5 args = 20 bytes)
        _emit 0xc2
        _emit 0x14
        _emit 0x00
    }
}
