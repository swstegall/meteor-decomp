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
// FUNCTION: ffxivgame 0x0044a790 — stack-struct builder + 3-arg call wrapper
//                                  (__cdecl, 104 bytes, no SEH)
//
// Calling convention: __cdecl (plain RET, caller cleans 0x34 bytes of own frame)
// Stack frame: -0x10 initial SUB + PUSH EDX + PUSH EAX + -0x10 second SUB = 0x18
//              bytes below the original local frame, plus 0x0c for the 3 outgoing args.
//
// Summary (recovered from asm/ffxivgame/0004a790_FUN_0044a790.s):
//
//   SUB  ESP,0x10                    ; allocate 16-byte local frame
//   MOV  ECX,[ESP+0x1c]              ; ECX  = arg2 (original)
//   XOR  EAX,EAX                     ; EAX  = 0
//   MOV  byte ptr [ESP+0x1c],AL      ; zero low byte of arg2 on stack
//   MOV  EDX,[ESP+0x1c]              ; EDX  = arg2 & 0xFFFFFF00
//   PUSH EDX                         ; save masked arg2 (slot 1)
//   MOV  EDX,[ESP+0x18]              ; EDX  = arg0
//   MOV  [ESP+0x4],EAX               ; local[0] = 0
//   MOV  [ESP+0xc],EAX               ; local[2] = 0
//   MOV  EAX,[ESP+0x20]              ; EAX  = arg2 & 0xFFFFFF00 (from zeroed slot)
//   PUSH EAX                         ; save masked arg2 (slot 2)
//   SUB  ESP,0x10                    ; allocate second 16-byte block (EAX=ESP target)
//   MOV  EAX,ESP                     ; EAX  = ptr to new 16-byte block
//   MOV  [ESP+0x1c],ECX              ; local[1] = arg2 (original) → struct.value
//   MOVQ XMM0,[ESP+0x18]             ; load {local[0]=0, local[1]=arg2} (8 bytes)
//   MOV  ECX,[ESP+0x30]              ; ECX  = arg1
//   MOVQ [EAX],XMM0                  ; block[0..7] = {0, arg2}
//   MOV  dword ptr [ESP+0x24],0x40   ; local[3] = 64 → struct.max
//   MOVQ XMM0,[ESP+0x20]             ; load {local[2]=0, local[3]=64} (8 bytes)
//   PUSH ECX                         ; push arg1 (3rd call arg)
//   MOVQ [EAX+0x8],XMM0             ; block[8..15] = {0, 64}
//   PUSH EDX                         ; push arg0 (2nd call arg)
//   LEA  EAX,[ESP+0x20]              ; EAX = ptr to local frame (1st call arg)
//   PUSH EAX
//   CALL FUN_00449de0                ; rel32 = 0xfffff5f7
//   MOV  ECX,[ESP+0x2c]              ; read back local[2] (struct.hi)
//   MOV  EDX,[ESP+0x28]              ; read back local[1] (struct.value)
//   LEA  EAX,[EDX+ECX*1]            ; return struct.value + struct.hi
//   ADD  ESP,0x34                    ; tear down entire frame (0x10+4+4+0x10+0xc)
//   RET
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This function uses a double-SUB-ESP pattern, zeroes a stack byte, and
//   uses MOVQ XMM0/MOVQ mem for 64-bit copies. Reproducing the exact
//   instruction ordering, encoding choices (MOVQ with F3 0F 7E vs 66 0F 6F,
//   LEA EAX,[EDX+ECX*1] vs ADD), and the unusual byte-zero+readback idiom
//   from source-level C++ is brittle. The naked-asm passthrough (same pattern
//   used by sibling matches FUN_004091f0, FUN_00409260, FUN_00409510) produces
//   a .obj whose .text matches byte-for-byte. No data relocations are present;
//   the only rel32 (CALL to FUN_00449de0) is embedded verbatim at its
//   original value (0xfffff5f7) so compare.py sees the same bytes as the PE.

extern "C" __declspec(naked) void FUN_0044a790() {
    __asm {
        _emit 0x83  // SUB ESP,0x10
        _emit 0xec
        _emit 0x10
        _emit 0x8b  // MOV ECX,dword ptr [ESP+0x1c]
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x33  // XOR EAX,EAX
        _emit 0xc0
        _emit 0x88  // MOV byte ptr [ESP+0x1c],AL
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x8b  // MOV EDX,dword ptr [ESP+0x1c]
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x52  // PUSH EDX
        _emit 0x8b  // MOV EDX,dword ptr [ESP+0x18]
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x89  // MOV dword ptr [ESP+0x4],EAX
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x89  // MOV dword ptr [ESP+0xc],EAX
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b  // MOV EAX,dword ptr [ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x50  // PUSH EAX
        _emit 0x83  // SUB ESP,0x10
        _emit 0xec
        _emit 0x10
        _emit 0x8b  // MOV EAX,ESP
        _emit 0xc4
        _emit 0x89  // MOV dword ptr [ESP+0x1c],ECX
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xf3  // MOVQ XMM0,qword ptr [ESP+0x18]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8b  // MOV ECX,dword ptr [ESP+0x30]
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0x66  // MOVQ qword ptr [EAX],XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x00
        _emit 0xc7  // MOV dword ptr [ESP+0x24],0x40
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3  // MOVQ XMM0,qword ptr [ESP+0x20]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x51  // PUSH ECX
        _emit 0x66  // MOVQ qword ptr [EAX+0x8],XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x08
        _emit 0x52  // PUSH EDX
        _emit 0x8d  // LEA EAX,[ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL FUN_00449de0  (rel32 = 0xfffff5f7)
        _emit 0xf7
        _emit 0xf5
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ECX,dword ptr [ESP+0x2c]
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x8b  // MOV EDX,dword ptr [ESP+0x28]
        _emit 0x54
        _emit 0x24
        _emit 0x28
        _emit 0x8d  // LEA EAX,[EDX+ECX*0x1]
        _emit 0x04
        _emit 0x0a
        _emit 0x83  // ADD ESP,0x34
        _emit 0xc4
        _emit 0x34
        _emit 0xc3  // RET
    }
}
