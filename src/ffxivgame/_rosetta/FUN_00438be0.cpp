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
// FUNCTION: ffxivgame 0x00438be0 — __thiscall wrapper that stages a 3-DWORD
//   local struct on the stack and forwards it to the virtual-call gate
//   FUN_00435ef0 (48 B / 0x30).
//
// Layout (inferred from asm):
//   this (ECX on entry):
//     +0x04  DWORD  field4   (forwarded as single stack arg to FUN_00435ef0)
//
//   Stack args (callee-cleaned by RET 0x8 — 2 DWORDs):
//     [ESP+0x04] : DWORD *  arg1   (pointer; *arg1 placed in local[1])
//     [ESP+0x08] : DWORD    arg2   (placed directly in local[2])
//
//   12-byte local struct (allocated via SUB ESP, 0xc):
//     [local+0x0] = 0x00f649d0   (type-tag / class-descriptor constant)
//     [local+0x4] = *arg1        (value read through arg1 pointer)
//     [local+0x8] = arg2
//
// Call to FUN_00435ef0 (__thiscall, one stack arg, ret 4):
//   ECX = &local_struct   (LEA ECX, [ESP+0x4] after PUSH ECX)
//   [ESP+0x0] = this->field4  (pushed before the LEA, cleaned by callee ret 4)
//
// Calling convention: __thiscall / 2 stack args / callee cleans via RET 0x8.
// Frame: SUB ESP, 0xc (12-byte local) + PUSH ECX (arg staging).
//
// Asm (48 bytes @ orig RVA 0x00038be0):
//   83 ec 0c                    SUB  ESP, 0xc
//   8b 44 24 10                 MOV  EAX, [ESP+0x10]       ; arg1 (ptr)
//   8b 49 04                    MOV  ECX, [ECX+0x4]        ; this->field4
//   8b 10                       MOV  EDX, [EAX]            ; *arg1
//   8b 44 24 14                 MOV  EAX, [ESP+0x14]       ; arg2
//   51                          PUSH ECX                   ; stage this->field4
//   8d 4c 24 04                 LEA  ECX, [ESP+0x4]        ; ECX = &local
//   c7 44 24 04 d0 49 f6 00     MOV  [ESP+0x4], 0xf649d0   ; local[0] = type tag
//   89 54 24 08                 MOV  [ESP+0x8], EDX        ; local[1] = *arg1
//   89 44 24 0c                 MOV  [ESP+0xc], EAX        ; local[2] = arg2
//   e8 e6 d2 ff ff              CALL FUN_00435ef0           (REL32 — reloc)
//   83 c4 0c                    ADD  ESP, 0xc              ; clean locals
//   c2 08 00                    RET  0x8                   ; clean 2 DWORD args
//
// Reconstruction: __declspec(naked) inline asm. The specific scheduling
// (field4 loaded into ECX before PUSH, then overwritten by LEA) and the
// [ESP+N] offsets after the PUSH cannot be reliably reproduced by
// source-level decomp without knowing the exact struct types.
// The REL32 CALL target is a reloc site; tools/compare.py masks it.

extern "C" void FUN_00435ef0(void);

extern "C" __declspec(naked) void FUN_00438be0() {
    __asm {
        sub     esp, 0xc
        mov     eax, dword ptr [esp + 0x10]
        mov     ecx, dword ptr [ecx + 0x4]
        mov     edx, dword ptr [eax]
        mov     eax, dword ptr [esp + 0x14]
        push    ecx
        lea     ecx, [esp + 0x4]
        mov     dword ptr [esp + 0x4], 0x00f649d0
        mov     dword ptr [esp + 0x8], edx
        mov     dword ptr [esp + 0xc], eax
        call    FUN_00435ef0
        add     esp, 0xc
        ret     8
    }
}
