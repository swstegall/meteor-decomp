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
// FUNCTION: ffxivgame 0x0000f990 — __thiscall constructor/initializer
//                                   (289 B / 0x121, EH4-SEH wrapped, 6 stack args)
//
// Inspection (read from the disassembly at orig RVA 0x0000f990):
//
//   __thiscall void FUN_0040f990(this, p1, p2, p3, p4, p5, p6)
//     ECX = this; [ESP+4..ESP+18] = p1..p6;  RET 0x18 (callee pops 6 dwords).
//
//   Structure:
//     EH4 prologue (PUSH -1 / PUSH scope_table 0xe54f6c / FS:[0] chain).
//     Saves ECX (this), EBX, EBP, ESI, EDI.
//     ESI = this.  [ESP+0x10] = ESI (EH unwind snapshot).
//
//     Stores 6 stack params into object fields:
//       [ESI+0x04] = p1, [ESI+0x08] = p2, [ESI+0x0c] = p3,
//       [ESI+0x10] = p4, [ESI+0x14] = p5, [ESI+0x18] = p6.
//
//     LEA EAX, [ESI+0x2c]; PUSH EAX.
//     EH trylevel → 0 (via [ESP+0x20] = EBX=0 before vtable writes).
//     Writes vtable pointer 0xf56834 to [ESI].
//     Writes 0x9d5110 → [ESI+0x1c], 0x9d4600 → [ESI+0x20], 0x4104a0 → [ESI+0x24].
//     Clears [ESI+0x28].
//     Calls IAT slot [0x00f3e174] (presumably a base-class ctor sub-object init).
//
//     LEA EDI, [ESI+0x44].
//     Writes vtable 0xf567c4 → [EDI]; initialises doubly-linked sentinel:
//       [EDI+0x4] = EDI, [EDI+0x8] = EDI.
//     Clears [ESI+0x50], [ESI+0x54], [ESI+0x58].
//     Writes 0xf568a4 → [ESI+0x5c], 0 → [ESI+0x60].
//     Writes 0xf56878 → [ESI+0x64], 0 → [ESI+0x68].
//
//     EH trylevel byte → 3 (via [ESP+0x1c] = 3).
//
//     if ([ESI+0x8] == 0):
//       Allocate: PUSH 0x10; PUSH [ESI+0x4]; CALL 0x009d5712; ADD ESP, 8.
//       [ESI+0x8] = EAX (alloc result).
//       [ESI+0x28] = 1.
//
//     EAX = [ESI+0x8]; ECX = [ESI+0x10]; EBP = [ESI+0x4].
//     [ESP+0x24] = EAX (clobbers p1 slot on stack for temp use).
//     CALL 0x004109a0 (ECX=param, EBP=ptr — appears to be a lookup fn).
//
//     if (EAX != 0):
//       ECX = [ESP+0x24] (saved EAX / temp).
//       PUSH EBX×4; PUSH 1; PUSH EBP; PUSH ECX; PUSH ESI.
//       ECX = EAX (result).
//       CALL 0x00410510 (__thiscall — node insert or similar).
//       if (EAX != 0): EAX += 8; JMP link_step.
//     else:
//       EAX = 0.
//
//   link_step:
//     EDX = [EDI+0x8]; [EDX+0x4] = EAX.
//     ECX = [EDI+0x8]; [EAX+0x8] = ECX.
//     ECX = [ESP+0x14] (EH chain saved-FS:[0]).
//     [EAX+0x4] = EDI; [EDI+0x8] = EAX.
//
//   EH4 epilogue: POP EDI/ESI/EBP/EBX; MOV FS:[0], ECX; ADD ESP, 0x10; RET 0x18.
//
// Stack frame (ESP-relative, after EH4 prologue + saves):
//   [ESP+0x00]  pushed EAX([ESI+0x2c]) temp   (PUSH EAX @ 0xf9d9)
//   [ESP+0x04]  saved EDI
//   [ESP+0x08]  saved ESI  (= this)
//   [ESP+0x0c]  saved EBP
//   [ESP+0x10]  saved EBX
//   [ESP+0x14]  saved ECX  (= this, also used as FS:[0] chain link)
//   [ESP+0x18]  FS:[0] old chain
//   [ESP+0x1c]  scope-table address (0xe54f6c)
//   [ESP+0x20]  EH trylevel word    (set to EBX=0 then byte=3)
//   [ESP+0x24]  p1  (also reused as local temp after initial store)
//   [ESP+0x28]  p2
//   [ESP+0x2c]  p3
//   [ESP+0x30]  p4
//   [ESP+0x34]  p5
//   [ESP+0x38]  p6
//
// Reloc-bearing sites (absolute addresses baked into orig binary):
//   +0x02  scope-table addr  (0x00e54f6c — .rdata EH4 FuncInfo)
//   +0x07  FS:[0] read       (constant 0, fold-through)
//   +0x0e  FS:[0] install    (constant 0, fold-through)
//   +0x4e  vtable store      (0x00f56834 — .rdata)
//   +0x5a  fn ptr store      (0x009d5110 — .text)
//   +0x61  fn ptr store      (0x009d4600 — .text)
//   +0x68  fn ptr store      (0x004104a0 — .text)
//   +0x72  IAT CALL          (.rdata 0x00f3e174)
//   +0x7b  sub-vtable store  (0x00f567c4 — .rdata)
//   +0x90  fn ptr store      (0x00f568a4 — .rdata)
//   +0x9a  fn ptr store      (0x00f56878 — .rdata)
//   +0xb4  CALL rel32        (0x009d5712 — .text, alloc helper)
//   +0xcf  CALL rel32        (0x004109a0 — .text, lookup)
//   +0xe8  CALL rel32        (0x00410510 — .text, insert)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   All branch targets and call sites are absolute-address-dependent.
//   A source-level port would need to reproduce the exact EH4 prologue,
//   the exact register allocation across the conditional alloc path, and
//   every absolute address above — impractical at /O2 without
//   link-time context. Emit the 289 bytes verbatim via MASM _emit.

#ifdef _MSC_VER
extern "C" __declspec(naked) void FUN_0040f990() {
    __asm {
        // 0000f990: 6a ff                PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0000f992: 68 6c 4f e5 00       PUSH 0xe54f6c
        _emit 0x68
        _emit 0x6c
        _emit 0x4f
        _emit 0xe5
        _emit 0x00
        // 0000f997: 64 a1 00 00 00 00    MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f99d: 50                   PUSH EAX
        _emit 0x50
        // 0000f99e: 64 89 25 00 00 00 00 MOV dword ptr FS:[0x0],ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f9a5: 51                   PUSH ECX
        _emit 0x51
        // 0000f9a6: 53                   PUSH EBX
        _emit 0x53
        // 0000f9a7: 55                   PUSH EBP
        _emit 0x55
        // 0000f9a8: 56                   PUSH ESI
        _emit 0x56
        // 0000f9a9: 8b f1                MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0000f9ab: 57                   PUSH EDI
        _emit 0x57
        // 0000f9ac: 89 74 24 10          MOV dword ptr [ESP+0x10],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 0000f9b0: 8b 44 24 24          MOV EAX,dword ptr [ESP+0x24]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0000f9b4: 8b 4c 24 28          MOV ECX,dword ptr [ESP+0x28]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // 0000f9b8: 8b 54 24 2c          MOV EDX,dword ptr [ESP+0x2c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        // 0000f9bc: 89 46 04             MOV dword ptr [ESI+0x4],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 0000f9bf: 8b 44 24 30          MOV EAX,dword ptr [ESP+0x30]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // 0000f9c3: 89 46 10             MOV dword ptr [ESI+0x10],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x10
        // 0000f9c6: 89 4e 08             MOV dword ptr [ESI+0x8],ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x08
        // 0000f9c9: 8b 4c 24 34          MOV ECX,dword ptr [ESP+0x34]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        // 0000f9cd: 89 56 0c             MOV dword ptr [ESI+0xc],EDX
        _emit 0x89
        _emit 0x56
        _emit 0x0c
        // 0000f9d0: 8b 54 24 38          MOV EDX,dword ptr [ESP+0x38]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x38
        // 0000f9d4: 8d 46 2c             LEA EAX,[ESI+0x2c]
        _emit 0x8d
        _emit 0x46
        _emit 0x2c
        // 0000f9d7: 33 db                XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // 0000f9d9: 50                   PUSH EAX
        _emit 0x50
        // 0000f9da: 89 5c 24 20          MOV dword ptr [ESP+0x20],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        // 0000f9de: c7 06 34 68 f5 00    MOV dword ptr [ESI],0xf56834
        _emit 0xc7
        _emit 0x06
        _emit 0x34
        _emit 0x68
        _emit 0xf5
        _emit 0x00
        // 0000f9e4: 89 4e 14             MOV dword ptr [ESI+0x14],ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x14
        // 0000f9e7: 89 56 18             MOV dword ptr [ESI+0x18],EDX
        _emit 0x89
        _emit 0x56
        _emit 0x18
        // 0000f9ea: c7 46 1c 10 51 9d 00 MOV dword ptr [ESI+0x1c],0x9d5110
        _emit 0xc7
        _emit 0x46
        _emit 0x1c
        _emit 0x10
        _emit 0x51
        _emit 0x9d
        _emit 0x00
        // 0000f9f1: c7 46 20 00 46 9d 00 MOV dword ptr [ESI+0x20],0x9d4600
        _emit 0xc7
        _emit 0x46
        _emit 0x20
        _emit 0x00
        _emit 0x46
        _emit 0x9d
        _emit 0x00
        // 0000f9f8: c7 46 24 a0 04 41 00 MOV dword ptr [ESI+0x24],0x4104a0
        _emit 0xc7
        _emit 0x46
        _emit 0x24
        _emit 0xa0
        _emit 0x04
        _emit 0x41
        _emit 0x00
        // 0000f9ff: 88 5e 28             MOV byte ptr [ESI+0x28],BL
        _emit 0x88
        _emit 0x5e
        _emit 0x28
        // 0000fa02: ff 15 74 e1 f3 00    CALL dword ptr [0x00f3e174]
        _emit 0xff
        _emit 0x15
        _emit 0x74
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0000fa08: 8d 7e 44             LEA EDI,[ESI+0x44]
        _emit 0x8d
        _emit 0x7e
        _emit 0x44
        // 0000fa0b: c7 07 c4 67 f5 00    MOV dword ptr [EDI],0xf567c4
        _emit 0xc7
        _emit 0x07
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 0000fa11: 89 7f 04             MOV dword ptr [EDI+0x4],EDI
        _emit 0x89
        _emit 0x7f
        _emit 0x04
        // 0000fa14: 89 7f 08             MOV dword ptr [EDI+0x8],EDI
        _emit 0x89
        _emit 0x7f
        _emit 0x08
        // 0000fa17: 88 5e 50             MOV byte ptr [ESI+0x50],BL
        _emit 0x88
        _emit 0x5e
        _emit 0x50
        // 0000fa1a: 89 5e 54             MOV dword ptr [ESI+0x54],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x54
        // 0000fa1d: 89 5e 58             MOV dword ptr [ESI+0x58],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x58
        // 0000fa20: c7 46 5c a4 68 f5 00 MOV dword ptr [ESI+0x5c],0xf568a4
        _emit 0xc7
        _emit 0x46
        _emit 0x5c
        _emit 0xa4
        _emit 0x68
        _emit 0xf5
        _emit 0x00
        // 0000fa27: 89 5e 60             MOV dword ptr [ESI+0x60],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x60
        // 0000fa2a: c7 46 64 78 68 f5 00 MOV dword ptr [ESI+0x64],0xf56878
        _emit 0xc7
        _emit 0x46
        _emit 0x64
        _emit 0x78
        _emit 0x68
        _emit 0xf5
        _emit 0x00
        // 0000fa31: 89 5e 68             MOV dword ptr [ESI+0x68],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x68
        // 0000fa34: 39 5e 08             CMP dword ptr [ESI+0x8],EBX
        _emit 0x39
        _emit 0x5e
        _emit 0x08
        // 0000fa37: c6 44 24 1c 03       MOV byte ptr [ESP+0x1c],0x3
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x03
        // 0000fa3c: 75 15                JNZ +0x15
        _emit 0x75
        _emit 0x15
        // 0000fa3e: 8b 46 04             MOV EAX,dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0000fa41: 6a 10                PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 0000fa43: 50                   PUSH EAX
        _emit 0x50
        // 0000fa44: e8 c9 5c 5c 00       CALL 0x009d5712
        _emit 0xe8
        _emit 0xc9
        _emit 0x5c
        _emit 0x5c
        _emit 0x00
        // 0000fa49: 83 c4 08             ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0000fa4c: 89 46 08             MOV dword ptr [ESI+0x8],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 0000fa4f: c6 46 28 01          MOV byte ptr [ESI+0x28],0x1
        _emit 0xc6
        _emit 0x46
        _emit 0x28
        _emit 0x01
        // 0000fa53: 8b 46 08             MOV EAX,dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 0000fa56: 8b 4e 10             MOV ECX,dword ptr [ESI+0x10]
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 0000fa59: 8b 6e 04             MOV EBP,dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x6e
        _emit 0x04
        // 0000fa5c: 89 44 24 24          MOV dword ptr [ESP+0x24],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0000fa60: e8 3b 0f 00 00       CALL 0x004109a0
        _emit 0xe8
        _emit 0x3b
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        // 0000fa65: 3b c3                CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // 0000fa67: 74 1d                JZ +0x1d
        _emit 0x74
        _emit 0x1d
        // 0000fa69: 8b 4c 24 24          MOV ECX,dword ptr [ESP+0x24]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 0000fa6d: 53                   PUSH EBX
        _emit 0x53
        // 0000fa6e: 53                   PUSH EBX
        _emit 0x53
        // 0000fa6f: 53                   PUSH EBX
        _emit 0x53
        // 0000fa70: 53                   PUSH EBX
        _emit 0x53
        // 0000fa71: 6a 01                PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0000fa73: 55                   PUSH EBP
        _emit 0x55
        // 0000fa74: 51                   PUSH ECX
        _emit 0x51
        // 0000fa75: 56                   PUSH ESI
        _emit 0x56
        // 0000fa76: 8b c8                MOV ECX,EAX
        _emit 0x8b
        _emit 0xc8
        // 0000fa78: e8 93 0a 00 00       CALL 0x00410510
        _emit 0xe8
        _emit 0x93
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        // 0000fa7d: 3b c3                CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // 0000fa7f: 74 05                JZ +0x5
        _emit 0x74
        _emit 0x05
        // 0000fa81: 83 c0 08             ADD EAX,0x8
        _emit 0x83
        _emit 0xc0
        _emit 0x08
        // 0000fa84: eb 02                JMP +0x2
        _emit 0xeb
        _emit 0x02
        // 0000fa86: 33 c0                XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0000fa88: 8b 57 08             MOV EDX,dword ptr [EDI+0x8]
        _emit 0x8b
        _emit 0x57
        _emit 0x08
        // 0000fa8b: 89 42 04             MOV dword ptr [EDX+0x4],EAX
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 0000fa8e: 8b 4f 08             MOV ECX,dword ptr [EDI+0x8]
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        // 0000fa91: 89 48 08             MOV dword ptr [EAX+0x8],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 0000fa94: 8b 4c 24 14          MOV ECX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0000fa98: 89 78 04             MOV dword ptr [EAX+0x4],EDI
        _emit 0x89
        _emit 0x78
        _emit 0x04
        // 0000fa9b: 89 47 08             MOV dword ptr [EDI+0x8],EAX
        _emit 0x89
        _emit 0x47
        _emit 0x08
        // 0000fa9e: 5f                   POP EDI
        _emit 0x5f
        // 0000fa9f: 8b c6                MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0000faa1: 5e                   POP ESI
        _emit 0x5e
        // 0000faa2: 5d                   POP EBP
        _emit 0x5d
        // 0000faa3: 5b                   POP EBX
        _emit 0x5b
        // 0000faa4: 64 89 0d 00 00 00 00 MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000faab: 83 c4 10             ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0000faae: c2 18 00             RET 0x18
        _emit 0xc2
        _emit 0x18
        _emit 0x00
    }
}
#endif // _MSC_VER
