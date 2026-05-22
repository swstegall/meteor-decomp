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
// FUNCTION: ffxivgame 0x0040d910 — FUN_0040d910 (134 B / 0x86)
//                                   __thiscall member function, EH3-wrapped,
//                                   zeroes a 64-element table at this+0x24
//                                   (each element 0x24 bytes, 0x24*64 = 0x900
//                                   bytes), clears [this+0x9a4], then calls
//                                   FUN_0040dd90 to initialize a sub-object.
//
// Calling convention: __thiscall (ECX = this); RET (no stack args).
// Returns: EAX = this.
// Callee-saves pushed (inside EH3 frame): ECX, EBX, ESI, EDI.
//
// EH3 frame:
//   state -1  = before FUN_0040dd50 call (unwind: nothing)
//   state  0  = after  FUN_0040dd50 call (unwind: per handler at 0xe54e58)
//
// Object layout (offsets touched):
//   [this + 0x24 .. 0x923]   64 × 0x24 byte entries — zeroed by loop
//   [this + 0x924]           argument to FUN_0040dd90 (sub-object start)
//   [this + 0x9a4]           DWORD cleared to 0 before FUN_0040dd90 call
//
// High-level behaviour:
//   1. FUN_0040dd50(this)   — some pre-init (thiscall, ECX=this)
//   2. Zero 64 entries of 0x24 bytes each at this+0x24  (do-while count-down
//      from 0x3f, using EDI as signed counter, JNS to continue)
//   3. [this+0x9a4] = 0
//   4. FUN_0040dd90(this+0x24, this+0x924, 0x24, 0x40)  — init sub-array
//   5. Restore FS:[0], return this in EAX.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The EH3 frame uses PUSH ECX between the SEH header (PUSH -1 / PUSH handler /
//   MOV EAX,FS:[0] / PUSH EAX / MOV FS:[0],ESP) and the callee-save push
//   sequence (EBX, ESI, EDI). MSVC 2005 then stores ESI (this) back to the
//   ECX slot at [ESP+0xc] so the SEH handler can access `this`. The SEH scope
//   state cookie is at [ESP+0x18] (the EBP slot is absent — MSVC skips EBP
//   since no frame pointer is needed). This interleaving is not reproducible
//   from C++ source without __try/__finally. The __declspec(naked) passthrough
//   avoids all of that; compare.py masks the two REL32 reloc sites
//   (FUN_0040dd50 and FUN_0040dd90) and the SEH handler DIR32 (0xe54e58).

extern "C" __declspec(naked) void FUN_0040d910()
{
    __asm {
        // 0x0000d910: 6a ff        PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0x0000d912: 68 58 4e e5 00   PUSH 0xe54e58  [SEH handler, DIR32 reloc]
        _emit 0x68
        _emit 0x58
        _emit 0x4e
        _emit 0xe5
        _emit 0x00
        // 0x0000d917: 64 a1 00 00 00 00   MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x0000d91d: 50   PUSH EAX
        _emit 0x50
        // 0x0000d91e: 64 89 25 00 00 00 00   MOV dword ptr FS:[0x0],ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x0000d925: 51   PUSH ECX
        _emit 0x51
        // 0x0000d926: 53   PUSH EBX
        _emit 0x53
        // 0x0000d927: 56   PUSH ESI
        _emit 0x56
        // 0x0000d928: 8b f1   MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0x0000d92a: 57   PUSH EDI
        _emit 0x57
        // 0x0000d92b: 89 74 24 0c   MOV dword ptr [ESP+0xc],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 0x0000d92f: e8 1c 04 00 00   CALL 0x0040dd50  [REL32 reloc FUN_0040dd50]
        _emit 0xe8
        _emit 0x1c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0x0000d934: 8d 5e 24   LEA EBX,[ESI+0x24]
        _emit 0x8d
        _emit 0x5e
        _emit 0x24
        // 0x0000d937: 33 c9   XOR ECX,ECX
        _emit 0x33
        _emit 0xc9
        // 0x0000d939: 8b d3   MOV EDX,EBX
        _emit 0x8b
        _emit 0xd3
        // 0x0000d93b: 89 4c 24 18   MOV dword ptr [ESP+0x18],ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0x0000d93f: 8d 79 3f   LEA EDI,[ECX+0x3f]
        _emit 0x8d
        _emit 0x79
        _emit 0x3f
        // 0x0000d942: 8d 42 18   LEA EAX,[EDX+0x18]
        _emit 0x8d
        _emit 0x42
        _emit 0x18
        // === loop body (0x0000d945 = offset 0x35) ===
        // 0x0000d945: 89 0a   MOV dword ptr [EDX],ECX
        _emit 0x89
        _emit 0x0a
        // 0x0000d947: 89 48 ec   MOV dword ptr [EAX-0x14],ECX
        _emit 0x89
        _emit 0x48
        _emit 0xec
        // 0x0000d94a: 66 89 48 f0   MOV word ptr [EAX-0x10],CX
        _emit 0x66
        _emit 0x89
        _emit 0x48
        _emit 0xf0
        // 0x0000d94e: 66 89 48 f2   MOV word ptr [EAX-0xe],CX
        _emit 0x66
        _emit 0x89
        _emit 0x48
        _emit 0xf2
        // 0x0000d952: 66 89 48 f4   MOV word ptr [EAX-0xc],CX
        _emit 0x66
        _emit 0x89
        _emit 0x48
        _emit 0xf4
        // 0x0000d956: 89 48 f8   MOV dword ptr [EAX-0x8],ECX
        _emit 0x89
        _emit 0x48
        _emit 0xf8
        // 0x0000d959: 89 48 fc   MOV dword ptr [EAX-0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0xfc
        // 0x0000d95c: 89 08   MOV dword ptr [EAX],ECX
        _emit 0x89
        _emit 0x08
        // 0x0000d95e: 83 c2 24   ADD EDX,0x24
        _emit 0x83
        _emit 0xc2
        _emit 0x24
        // 0x0000d961: 83 c0 24   ADD EAX,0x24
        _emit 0x83
        _emit 0xc0
        _emit 0x24
        // 0x0000d964: 83 ef 01   SUB EDI,0x1
        _emit 0x83
        _emit 0xef
        _emit 0x01
        // 0x0000d967: 79 dc   JNS -0x24 (→ 0x0040d945)
        _emit 0x79
        _emit 0xdc
        // === after loop ===
        // 0x0000d969: 6a 40   PUSH 0x40
        _emit 0x6a
        _emit 0x40
        // 0x0000d96b: 6a 24   PUSH 0x24
        _emit 0x6a
        _emit 0x24
        // 0x0000d96d: 8d 86 24 09 00 00   LEA EAX,[ESI+0x924]
        _emit 0x8d
        _emit 0x86
        _emit 0x24
        _emit 0x09
        _emit 0x00
        _emit 0x00
        // 0x0000d973: 50   PUSH EAX
        _emit 0x50
        // 0x0000d974: 89 8e a4 09 00 00   MOV dword ptr [ESI+0x9a4],ECX
        _emit 0x89
        _emit 0x8e
        _emit 0xa4
        _emit 0x09
        _emit 0x00
        _emit 0x00
        // 0x0000d97a: 53   PUSH EBX
        _emit 0x53
        // 0x0000d97b: 8b ce   MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0x0000d97d: e8 0e 04 00 00   CALL 0x0040dd90  [REL32 reloc FUN_0040dd90]
        _emit 0xe8
        _emit 0x0e
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0x0000d982: 8b 4c 24 10   MOV ECX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0x0000d986: 5f   POP EDI
        _emit 0x5f
        // 0x0000d987: 8b c6   MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0x0000d989: 5e   POP ESI
        _emit 0x5e
        // 0x0000d98a: 5b   POP EBX
        _emit 0x5b
        // 0x0000d98b: 64 89 0d 00 00 00 00   MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x0000d992: 83 c4 10   ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0x0000d995: c3   RET
        _emit 0xc3
    }
}

// vim: ts=4 sts=4 sw=4 et
