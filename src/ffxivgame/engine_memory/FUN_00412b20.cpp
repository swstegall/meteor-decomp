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
// FUNCTION: ffxivgame 0x00012b20 — __thiscall two-way vtable-slot-1 dispatcher:
//           compare [this+0x24] with (this-4); if equal, tail-call through
//           this->member_0x14's vtable slot 1; else if byte [this+0x1d] != 0,
//           tail-call through this->member_0x18's vtable slot 1; else return 0.
//           (37 bytes)
//
// Asm (37 bytes @ orig RVA 0x00012b20):
//   8D 41 FC          LEA  EAX, [ECX-4]
//   39 40 28          CMP  dword ptr [EAX+0x28], EAX  ; [ecx+0x24] == ecx-4?
//   75 0A             JNZ  SHORT label2
//   8B 49 14          MOV  ECX, [ECX+0x14]
//   8B 01             MOV  EAX, [ECX]
//   8B 50 04          MOV  EDX, [EAX+4]
//   FF E2             JMP  EDX
// label2:
//   80 79 1D 00       CMP  BYTE PTR [ECX+0x1D], 0
//   74 0A             JZ   SHORT label3
//   8B 49 18          MOV  ECX, [ECX+0x18]
//   8B 01             MOV  EAX, [ECX]
//   8B 50 04          MOV  EDX, [EAX+4]
//   FF E2             JMP  EDX
// label3:
//   33 C0             XOR  EAX, EAX
//   C3                RET

extern "C" __declspec(naked) void FUN_00412b20()
{
    __asm {
        lea  eax, [ecx-4]
        cmp  [eax+28h], eax
        jnz  short label2
        mov  ecx, [ecx+14h]
        mov  eax, [ecx]
        mov  edx, [eax+4]
        jmp  edx
label2:
        cmp  byte ptr [ecx+1dh], 0
        jz   short label3
        mov  ecx, [ecx+18h]
        mov  eax, [ecx]
        mov  edx, [eax+4]
        jmp  edx
label3:
        xor  eax, eax
        ret
    }
}
