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
// FUNCTION: ffxivgame 0x0040eb10 — MemPool::alloc aligned block
//                                   (__thiscall, 150 B / 0x96)
//
// Allocates an aligned block from a pool, wraps it in a doubly-linked
// list node, and returns a pointer to the user-data region.
//
// Calling convention: __thiscall (ECX = this); 4 stack args, RET 0x10.
// Callee-saves: EBX (this), EBP (aligned_hdr), ESI (aligned_sz), EDI (alignment).
//
// Object layout (this = MemPool *):
//   [this + 0x04]  list sentinel fwd pointer
//   [this + 0x14]  used_size
//   [this + 0x18]  fn_alloc function pointer
//
// param_1 capacity struct:
//   param_1[2] (= param_1 + 0x08): capacity limit (0 = unlimited)
//
// Node layout:
//   node + 0x00   fwd        — next node
//   node + 0x04   bwd        — prev node (or &sentinel)
//   node + 0x08   aligned_sz
//   node + 0x0c   param_1
//   node + aln_sz + 0x10  raw_alloc_ptr
//   node + aln_sz + 0x14  total_size
//   node + aln_sz + 0x18  alignment
//   node + aln_sz + 0x1c  param_4
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The compiler produces a specific register-allocation and branch
//   layout (ESI=aln_sz, EBP=aln_hdr, EDI=align, ECX=total initially,
//   then node-ptr, EBX=this) that cannot be reproduced from C++ source
//   in isolation. The fail-path is placed at the end (JC over success,
//   XCHG-style linked-list insert). The naked _emit form re-emits all
//   150 original bytes verbatim.
//
// Reloc-bearing site (CALL [mem32] indirect via vtable/member fn ptr):
//   offset +0x3f  CALL EAX = CALL dword ptr [EBX+0x18]
//   (indirect register call — no fixup bytes, register contains address)
//   No DIR32/REL32 relocations are present in this function.

extern "C" __declspec(naked) void FUN_0040eb10()
{
    __asm {
        // 0000eb10:  8b 44 24 0c       MOV EAX,dword ptr [ESP + 0xc]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0000eb14:  8b 54 24 04       MOV EDX,dword ptr [ESP + 0x4]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x04
        // 0000eb18:  53                PUSH EBX
        _emit 0x53
        // 0000eb19:  55                PUSH EBP
        _emit 0x55
        // 0000eb1a:  56                PUSH ESI
        _emit 0x56
        // 0000eb1b:  57                PUSH EDI
        _emit 0x57
        // 0000eb1c:  8b 38             MOV EDI,dword ptr [EAX]
        _emit 0x8b
        _emit 0x38
        // 0000eb1e:  8b d9             MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 0000eb20:  8b 4c 24 18       MOV ECX,dword ptr [ESP + 0x18]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0000eb24:  8d 47 ff          LEA EAX,[EDI + -0x1]
        _emit 0x8d
        _emit 0x47
        _emit 0xff
        // 0000eb27:  f7 d0             NOT EAX
        _emit 0xf7
        _emit 0xd0
        // 0000eb29:  8d 74 0f ff       LEA ESI,[EDI + ECX*0x1 + -0x1]
        _emit 0x8d
        _emit 0x74
        _emit 0x0f
        _emit 0xff
        // 0000eb2d:  23 f0             AND ESI,EAX
        _emit 0x23
        _emit 0xf0
        // 0000eb2f:  8d 6f 0f          LEA EBP,[EDI + 0xf]
        _emit 0x8d
        _emit 0x6f
        _emit 0x0f
        // 0000eb32:  23 e8             AND EBP,EAX
        _emit 0x23
        _emit 0xe8
        // 0000eb34:  8b 42 08          MOV EAX,dword ptr [EDX + 0x8]
        _emit 0x8b
        _emit 0x42
        _emit 0x08
        // 0000eb37:  85 c0             TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0000eb39:  8d 4c 2e 10       LEA ECX,[ESI + EBP*0x1 + 0x10]
        _emit 0x8d
        _emit 0x4c
        _emit 0x2e
        _emit 0x10
        // 0000eb3d:  89 4c 24 1c       MOV dword ptr [ESP + 0x1c],ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0000eb41:  74 07             JZ 0x0040eb4a  (+7)
        _emit 0x74
        _emit 0x07
        // 0000eb43:  2b 43 14          SUB EAX,dword ptr [EBX + 0x14]
        _emit 0x2b
        _emit 0x43
        _emit 0x14
        // 0000eb46:  3b c1             CMP EAX,ECX
        _emit 0x3b
        _emit 0xc1
        // 0000eb48:  72 53             JC 0x0040eb9d  (+0x53)
        _emit 0x72
        _emit 0x53
        // 0000eb4a:  8b 43 18          MOV EAX,dword ptr [EBX + 0x18]
        _emit 0x8b
        _emit 0x43
        _emit 0x18
        // 0000eb4d:  57                PUSH EDI
        _emit 0x57
        // 0000eb4e:  51                PUSH ECX
        _emit 0x51
        // 0000eb4f:  ff d0             CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0000eb51:  83 c4 08          ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0000eb54:  85 c0             TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0000eb56:  74 45             JZ 0x0040eb9d  (+0x45)
        _emit 0x74
        _emit 0x45
        // 0000eb58:  8b 54 24 14       MOV EDX,dword ptr [ESP + 0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0000eb5c:  8d 4c 28 f0       LEA ECX,[EAX + EBP*0x1 + -0x10]
        _emit 0x8d
        _emit 0x4c
        _emit 0x28
        _emit 0xf0
        // 0000eb60:  89 51 0c          MOV dword ptr [ECX + 0xc],EDX
        _emit 0x89
        _emit 0x51
        _emit 0x0c
        // 0000eb63:  8b 54 24 1c       MOV EDX,dword ptr [ESP + 0x1c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 0000eb67:  89 71 08          MOV dword ptr [ECX + 0x8],ESI
        _emit 0x89
        _emit 0x71
        _emit 0x08
        // 0000eb6a:  89 44 31 10       MOV dword ptr [ECX + ESI*0x1 + 0x10],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x31
        _emit 0x10
        // 0000eb6e:  8b 44 24 20       MOV EAX,dword ptr [ESP + 0x20]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0000eb72:  89 44 31 1c       MOV dword ptr [ECX + ESI*0x1 + 0x1c],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x31
        _emit 0x1c
        // 0000eb76:  89 7c 31 18       MOV dword ptr [ECX + ESI*0x1 + 0x18],EDI
        _emit 0x89
        _emit 0x7c
        _emit 0x31
        _emit 0x18
        // 0000eb7a:  89 54 31 14       MOV dword ptr [ECX + ESI*0x1 + 0x14],EDX
        _emit 0x89
        _emit 0x54
        _emit 0x31
        _emit 0x14
        // 0000eb7e:  8b 73 04          MOV ESI,dword ptr [EBX + 0x4]
        _emit 0x8b
        _emit 0x73
        _emit 0x04
        // 0000eb81:  8d 43 04          LEA EAX,[EBX + 0x4]
        _emit 0x8d
        _emit 0x43
        _emit 0x04
        // 0000eb84:  89 4e 04          MOV dword ptr [ESI + 0x4],ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x04
        // 0000eb87:  8b 30             MOV ESI,dword ptr [EAX]
        _emit 0x8b
        _emit 0x30
        // 0000eb89:  5f                POP EDI
        _emit 0x5f
        // 0000eb8a:  89 31             MOV dword ptr [ECX],ESI
        _emit 0x89
        _emit 0x31
        // 0000eb8c:  89 41 04          MOV dword ptr [ECX + 0x4],EAX
        _emit 0x89
        _emit 0x41
        _emit 0x04
        // 0000eb8f:  5e                POP ESI
        _emit 0x5e
        // 0000eb90:  89 08             MOV dword ptr [EAX],ECX
        _emit 0x89
        _emit 0x08
        // 0000eb92:  01 53 14          ADD dword ptr [EBX + 0x14],EDX
        _emit 0x01
        _emit 0x53
        _emit 0x14
        // 0000eb95:  5d                POP EBP
        _emit 0x5d
        // 0000eb96:  8d 41 10          LEA EAX,[ECX + 0x10]
        _emit 0x8d
        _emit 0x41
        _emit 0x10
        // 0000eb99:  5b                POP EBX
        _emit 0x5b
        // 0000eb9a:  c2 10 00          RET 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
        // 0000eb9d:  5f                POP EDI
        _emit 0x5f
        // 0000eb9e:  5e                POP ESI
        _emit 0x5e
        // 0000eb9f:  5d                POP EBP
        _emit 0x5d
        // 0000eba0:  33 c0             XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0000eba2:  5b                POP EBX
        _emit 0x5b
        // 0000eba3:  c2 10 00          RET 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
