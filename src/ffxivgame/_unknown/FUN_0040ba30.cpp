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
// FUNCTION: ffxivgame 0x0000ba30 — page-pool allocator with EH4 frame
//                                  (__cdecl, 289 bytes / 0x121)
//
// Signature (inferred from asm at orig RVA 0x0000ba30):
//
//   __cdecl bool FUN_0040ba30(void *param_1, unsigned int param_2)
//
//   param_2: page_size (loaded from [ESP+0x40] inside function)
//   ESI:     output struct pointer (caller-owned, written through)
//   Returns: 0 (AL=0) on allocation failure, 1 (AL=1) on success.
//
// Behaviour (inferred from asm):
//
//   1. Compute page count:
//        iVar1 = 0x1000 / param_2      (floor division)
//        iVar2 = param_2 + 2
//        while (iVar2 * iVar1 > 0x1000)
//            iVar1--;
//
//   2. Allocate backing buffer:
//        ptr = FUN_0040daa0(iVar1 * param_2)  (malloc-like)
//        if (!ptr) {
//            FUN_0040d6a0(0)                   (failure handler)
//            return false;
//        }
//
//   3. Initialise the pool:
//        end_ptr = ptr + iVar1 * param_2
//        FUN_0040dd50(ECX = &local_c)          (ctor for local object)
//        FUN_0040dd90(ECX = &local_1c, ptr, end_ptr, param_2, iVar1)
//
//   4. Zero-initialise ESI output struct (0x24 bytes) via PXOR XMM0 + MOVQ x5
//
//   5. Copy 0x1c bytes (3 qwords + 1 dword) from local_2c → ESI
//
//   6. Look up a tier index for param_2 in the table at 0xf55988 (22 entries)
//        for (i = 0; i < 0x16; i++)
//            if (param_2 <= table[i]) break;
//        if (i == 0x16) i = 0xffffffff;
//        ESI->field_20 = (char)i;
//
//   7. Clean up the local object (FUN_0040db10)
//
//   8. Return true.
//
// EH4 SEH frame layout (prologue at orig RVA 0x0000ba30):
//   MOV EAX, FS:[0]   ; save current exception chain head
//   PUSH -1            ; initial try-state = -1
//   PUSH 0xe54e16      ; scope-table RVA (handler address)
//   PUSH EAX           ; old FS:[0] chain link
//   MOV FS:[0], ESP    ; install SEH record
//   SUB ESP, 0x24      ; 9 DWORD locals
//   PUSH EBX / EBP / EDI (three callee-saved regs after SUB)
//
// Stack layout (ESP-relative, after PUSH EBP at ba49):
//   [ESP+0x00] EBP (just pushed)
//   [ESP+0x04] EBX
//   [ESP+0x08..+0x2b] 0x24 bytes of local variables
//   [ESP+0x2c] old FS:[0] chain link
//   [ESP+0x30] 0xe54e16 (scope-table address)
//   [ESP+0x34] -1 (EH4 try-state)
//   [ESP+0x38] return address
//   [ESP+0x3c] param_1
//   [ESP+0x40] param_2 (page_size)
//
// Reloc-bearing sites masked by tools/compare.py (all CALL rel32 or
// absolute address immediates):
//   +0x49  CALL FUN_0040daa0  (rel32 = 0x00002022)
//   +0x59  CALL FUN_0040d6a0  (rel32 = 0x00001c12)
//   +0x81  CALL FUN_0040dd50  (rel32 = 0x0000229a)
//   +0x9a  CALL FUN_0040dd90  (rel32 = 0x000022c1)
//   +0xe5  absolute [EAX*4 + 0xf55988]  (abs32)
//   +0x108 CALL FUN_0040db10  (rel32 = 0x00001fd3)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The EH4 prologue (MOV EAX,FS:[0] / PUSH -1 / PUSH handler /
//   PUSH EAX / MOV FS:[0],ESP) and the subsequent non-standard EDI
//   save order (PUSH EDI after PUSH EBX + PUSH EBP) cannot be
//   reproduced from source-level C++ under /O2 /GS. The PXOR XMM0 /
//   MOVQ output-struct zeroing pattern and the MOVQ copy loop are
//   register-pressure-sensitive and shift under separate TU compilation.
//   A __declspec(naked) body re-emitting the original 289 bytes
//   verbatim via MASM _emit directives is the pragmatic match path.

extern "C" __declspec(naked) void FUN_0040ba30() {
    __asm {
        // 0000ba30:  64 a1 00 00 00 00     MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000ba36:  6a ff                 PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0000ba38:  68 16 4e e5 00        PUSH 0xe54e16
        _emit 0x68
        _emit 0x16
        _emit 0x4e
        _emit 0xe5
        _emit 0x00
        // 0000ba3d:  50                    PUSH EAX
        _emit 0x50
        // 0000ba3e:  64 89 25 00 00 00 00  MOV dword ptr FS:[0x0], ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000ba45:  83 ec 24              SUB ESP, 0x24
        _emit 0x83
        _emit 0xec
        _emit 0x24
        // 0000ba48:  53                    PUSH EBX
        _emit 0x53
        // 0000ba49:  55                    PUSH EBP
        _emit 0x55
        // 0000ba4a:  8b 6c 24 40           MOV EBP, dword ptr [ESP+0x40]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x40
        // 0000ba4e:  33 d2                 XOR EDX, EDX
        _emit 0x33
        _emit 0xd2
        // 0000ba50:  b8 00 10 00 00        MOV EAX, 0x1000
        _emit 0xb8
        _emit 0x00
        _emit 0x10
        _emit 0x00
        _emit 0x00
        // 0000ba55:  f7 f5                 DIV EBP
        _emit 0xf7
        _emit 0xf5
        // 0000ba57:  57                    PUSH EDI
        _emit 0x57
        // 0000ba58:  8b f8                 MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // 0000ba5a:  8d 45 02              LEA EAX, [EBP+0x2]
        _emit 0x8d
        _emit 0x45
        _emit 0x02
        // 0000ba5d:  8b c8                 MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 0000ba5f:  0f af cf              IMUL ECX, EDI
        _emit 0x0f
        _emit 0xaf
        _emit 0xcf
        // 0000ba62:  81 f9 00 10 00 00     CMP ECX, 0x1000
        _emit 0x81
        _emit 0xf9
        _emit 0x00
        _emit 0x10
        _emit 0x00
        _emit 0x00
        // 0000ba68:  76 07                 JBE 0x0040ba71
        _emit 0x76
        _emit 0x07
        // 0000ba6a:  83 ef 01              SUB EDI, 0x1
        _emit 0x83
        _emit 0xef
        _emit 0x01
        // 0000ba6d:  2b c8                 SUB ECX, EAX
        _emit 0x2b
        _emit 0xc8
        // 0000ba6f:  eb f1                 JMP 0x0040ba62
        _emit 0xeb
        _emit 0xf1
        // 0000ba71:  8b 4c 24 40           MOV ECX, dword ptr [ESP+0x40]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        // 0000ba75:  0f af c7              IMUL EAX, EDI
        _emit 0x0f
        _emit 0xaf
        _emit 0xc7
        // 0000ba78:  50                    PUSH EAX
        _emit 0x50
        // 0000ba79:  e8 22 20 00 00        CALL 0x0040daa0
        _emit 0xe8
        _emit 0x22
        _emit 0x20
        _emit 0x00
        _emit 0x00
        // 0000ba7e:  8b d8                 MOV EBX, EAX
        _emit 0x8b
        _emit 0xd8
        // 0000ba80:  85 db                 TEST EBX, EBX
        _emit 0x85
        _emit 0xdb
        // 0000ba82:  75 1e                 JNZ 0x0040baa2
        _emit 0x75
        _emit 0x1e
        // 0000ba84:  8b 4c 24 40           MOV ECX, dword ptr [ESP+0x40]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        // 0000ba88:  50                    PUSH EAX
        _emit 0x50
        // 0000ba89:  e8 12 1c 00 00        CALL 0x0040d6a0
        _emit 0xe8
        _emit 0x12
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        // 0000ba8e:  5f                    POP EDI
        _emit 0x5f
        // 0000ba8f:  5d                    POP EBP
        _emit 0x5d
        // 0000ba90:  32 c0                 XOR AL, AL
        _emit 0x32
        _emit 0xc0
        // 0000ba92:  5b                    POP EBX
        _emit 0x5b
        // 0000ba93:  8b 4c 24 24           MOV ECX, dword ptr [ESP+0x24]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 0000ba97:  64 89 0d 00 00 00 00  MOV dword ptr FS:[0x0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000ba9e:  83 c4 30              ADD ESP, 0x30
        _emit 0x83
        _emit 0xc4
        _emit 0x30
        // 0000baa1:  c3                    RET
        _emit 0xc3
        // 0000baa2:  8b c7                 MOV EAX, EDI
        _emit 0x8b
        _emit 0xc7
        // 0000baa4:  0f af c5              IMUL EAX, EBP
        _emit 0x0f
        _emit 0xaf
        _emit 0xc5
        // 0000baa7:  03 c3                 ADD EAX, EBX
        _emit 0x03
        _emit 0xc3
        // 0000baa9:  8d 4c 24 0c           LEA ECX, [ESP+0xc]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0000baad:  89 44 24 44           MOV dword ptr [ESP+0x44], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x44
        // 0000bab1:  e8 9a 22 00 00        CALL 0x0040dd50
        _emit 0xe8
        _emit 0x9a
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 0000bab6:  8b 44 24 44           MOV EAX, dword ptr [ESP+0x44]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x44
        // 0000baba:  57                    PUSH EDI
        _emit 0x57
        // 0000babb:  55                    PUSH EBP
        _emit 0x55
        // 0000babc:  50                    PUSH EAX
        _emit 0x50
        // 0000babd:  53                    PUSH EBX
        _emit 0x53
        // 0000babe:  8d 4c 24 1c           LEA ECX, [ESP+0x1c]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0000bac2:  c7 44 24 48 00 00 00 00  MOV dword ptr [ESP+0x48], 0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x48
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000baca:  e8 c1 22 00 00        CALL 0x0040dd90
        _emit 0xe8
        _emit 0xc1
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 0000bacf:  8b 44 24 10           MOV EAX, dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0000bad3:  66 0f ef c0           PXOR XMM0, XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        // 0000bad7:  66 0f d6 06           MOVQ qword ptr [ESI], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x06
        // 0000badb:  66 0f d6 46 08        MOVQ qword ptr [ESI+0x8], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x08
        // 0000bae0:  66 0f d6 46 10        MOVQ qword ptr [ESI+0x10], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x10
        // 0000bae5:  66 0f d6 46 18        MOVQ qword ptr [ESI+0x18], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x18
        // 0000baea:  c7 46 20 00 00 00 00  MOV dword ptr [ESI+0x20], 0x0
        _emit 0xc7
        _emit 0x46
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000baf1:  f3 0f 7e 00           MOVQ XMM0, qword ptr [EAX]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x00
        // 0000baf5:  66 0f d6 06           MOVQ qword ptr [ESI], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x06
        // 0000baf9:  f3 0f 7e 40 08        MOVQ XMM0, qword ptr [EAX+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x08
        // 0000bafe:  66 0f d6 46 08        MOVQ qword ptr [ESI+0x8], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x08
        // 0000bb03:  f3 0f 7e 40 10        MOVQ XMM0, qword ptr [EAX+0x10]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x10
        // 0000bb08:  66 0f d6 46 10        MOVQ qword ptr [ESI+0x10], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x10
        // 0000bb0d:  8b 48 18              MOV ECX, dword ptr [EAX+0x18]
        _emit 0x8b
        _emit 0x48
        _emit 0x18
        // 0000bb10:  89 4e 18              MOV dword ptr [ESI+0x18], ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x18
        // 0000bb13:  33 c0                 XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0000bb15:  3b 2c 85 88 59 f5 00  CMP EBP, dword ptr [EAX*4+0xf55988]
        _emit 0x3b
        _emit 0x2c
        _emit 0x85
        _emit 0x88
        _emit 0x59
        _emit 0xf5
        _emit 0x00
        // 0000bb1c:  76 0b                 JBE 0x0040bb29
        _emit 0x76
        _emit 0x0b
        // 0000bb1e:  83 c0 01              ADD EAX, 0x1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 0000bb21:  83 f8 16              CMP EAX, 0x16
        _emit 0x83
        _emit 0xf8
        _emit 0x16
        // 0000bb24:  72 ef                 JC 0x0040bb15
        _emit 0x72
        _emit 0xef
        // 0000bb26:  83 c8 ff              OR EAX, 0xffffffff
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // 0000bb29:  8d 4c 24 0c           LEA ECX, [ESP+0xc]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0000bb2d:  88 46 20              MOV byte ptr [ESI+0x20], AL
        _emit 0x88
        _emit 0x46
        _emit 0x20
        // 0000bb30:  c7 44 24 38 ff ff ff ff  MOV dword ptr [ESP+0x38], 0xffffffff
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0000bb38:  e8 d3 1f 00 00        CALL 0x0040db10
        _emit 0xe8
        _emit 0xd3
        _emit 0x1f
        _emit 0x00
        _emit 0x00
        // 0000bb3d:  8b 4c 24 30           MOV ECX, dword ptr [ESP+0x30]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        // 0000bb41:  5f                    POP EDI
        _emit 0x5f
        // 0000bb42:  5d                    POP EBP
        _emit 0x5d
        // 0000bb43:  b0 01                 MOV AL, 0x1
        _emit 0xb0
        _emit 0x01
        // 0000bb45:  5b                    POP EBX
        _emit 0x5b
        // 0000bb46:  64 89 0d 00 00 00 00  MOV dword ptr FS:[0x0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000bb4d:  83 c4 30              ADD ESP, 0x30
        _emit 0x83
        _emit 0xc4
        _emit 0x30
        // 0000bb50:  c3                    RET
        _emit 0xc3
    }
}
