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
// FUNCTION: ffxivgame 0x00438850 — __thiscall trampoline that builds a
//                                  5-field stack-local record and dispatches
//                                  to FUN_00435840 (62 B / 0x3e).
//
// Calling convention: __thiscall (ECX = this; 4 DWORD stack args; callee
//   cleans 0x10 via `ret 0x10`).
//
// Description (recovered from asm at RVA 0x00038850):
//
//   Allocates a 20-byte (0x14) stack frame to hold a 5-field local struct
//   at [ESP+0].  Fields are filled from the four stack args and from
//   this->field4 ([ECX+0x4]), then FUN_00435840 is called as a __thiscall
//   method on that struct with this->field4 forwarded as the sole extra arg.
//
//   Local struct layout (relative to the original ESP after SUB ESP,0x14):
//     +0x00  void*  data0  = 0x00f64950   (written after PUSH, via LEA window)
//     +0x04  DWORD  f4     = arg1
//     +0x08  DWORD  f8     = arg3
//     +0x0c  DWORD  fC     = arg4
//     +0x10  DWORD  f10    = arg2
//
//   Instruction flow:
//     SUB ESP,0x14            — open 20-byte frame
//     EAX = arg1; EDX = arg3
//     [ESP+0x04] = EAX        — f4 <- arg1
//     EAX = arg4
//     [ESP+0x0c] = EAX        — fC <- arg4
//     EAX = this->field4      — [ECX+0x04]
//     [ESP+0x08] = EDX        — f8 <- arg3
//     EDX = arg2
//     PUSH EAX                — forward this->field4 as lone stack arg to FUN_00435840
//     ECX = &(old [ESP+0x00]) — ECX = &local_struct (via LEA [newESP+0x4])
//     [ESP+0x04] = 0xf64950   — data0 <- 0x00f64950 (accessed through PUSH window)
//     [ESP+0x14] = EDX        — f10 <- arg2         (accessed through PUSH window)
//     CALL FUN_00435840       — dispatch
//     ADD ESP,0x14            — discard frame + PUSH slot
//     RET 0x10                — __thiscall cleanup of 4 stack args
//
// Why naked asm: MSVC 2005 initialises the struct fields out of natural
//   declaration order (arg1, arg4, arg3, data0, arg2) while interleaving
//   the load of this->field4 into EAX for the upcoming PUSH.  Reproducing
//   that exact register-allocation sequence from C++ source is unreliable;
//   naked asm locks in the byte-identical encoding.
//
// Reloc-bearing sites (compare.py wildcard-masks each 4-byte window):
//   +0x27  MOV [ESP+0x4], imm32 0x00f64950 — .rdata pointer, DIR32
//   +0x33  CALL rel32 -> FUN_00435840       — REL32

extern "C" void FUN_00435840();   // __thiscall helper: dispatches via vtable slot 0x190

extern "C" __declspec(naked) void FUN_00438850() {
    __asm {
        sub     esp, 0x14
        mov     eax, dword ptr [esp + 0x18]
        mov     edx, dword ptr [esp + 0x20]
        mov     dword ptr [esp + 0x4], eax
        mov     eax, dword ptr [esp + 0x24]
        mov     dword ptr [esp + 0xc], eax
        mov     eax, dword ptr [ecx + 0x4]
        mov     dword ptr [esp + 0x8], edx
        mov     edx, dword ptr [esp + 0x1c]
        push    eax
        lea     ecx, [esp + 0x4]
        mov     dword ptr [esp + 0x4], 0x00f64950
        mov     dword ptr [esp + 0x14], edx
        call    FUN_00435840
        add     esp, 0x14
        ret     0x10
    }
}
