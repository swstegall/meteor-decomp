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
// FUNCTION: ffxivgame 0x0040de80 — __thiscall circular-buffer push: bucket
//                                  a value into a short[] ring via signed
//                                  integer division (55 bytes).
//
// Inferred layout of the inner struct (this->field_04):
//   struct InnerBlock {
//       int      base;       // +0x00  subtracted from arg1 before division
//       short*   array;      // +0x04  pointer to int16_t ring buffer
//       // gap                // +0x08..+0x09
//       short    index;      // +0x0a  current write position (16-bit)
//       short    width;      // +0x0c  bucket width / divisor (16-bit)
//       // gap                // +0x0e..+0x13
//       int      total;      // +0x14  running count of pushes (32-bit)
//   };
//   struct Container {
//       // ...
//       InnerBlock *block;   // +0x04
//   };
//
// Calling convention: __thiscall (ECX = this; one DWORD stack arg;
//   callee cleans 4 bytes via `ret 4`).
//
// Frame: PUSH EBX + PUSH ESI (callee-saves); no local stack allocation.
//
// Quirks pinned by naked asm:
//   1. arg1 is loaded into EAX from [ESP+4] BEFORE the PUSH EBX/ESI
//      saves — the register already holds the value across those pushes.
//   2. this->block is accessed via EDX (ADD [EDX+0x14]) before the
//      saves, then re-loaded into ESI after them; using two registers
//      for the same pointer is the scheduler's way of avoiding a reload
//      into a live register.
//   3. `MOV EDX, ESI` is emitted as an alias before `SUB EAX, [EDX]`
//      even though `SUB EAX, [ESI]` would be equivalent — MSVC 2005
//      scheduler did not fold them.
//   4. At the end, this->block is re-read from ECX (not reused from ESI,
//      which was clobbered to hold the array pointer) for the final
//      `ADD word ptr [ECX+0xa], 1`.
//   5. Both increments use `ADD [...], 1` (short 83-encoded form), not
//      `INC [...]` — consistent with MSVC 2005 generating ADD for
//      non-result-used field increments.

extern "C" __declspec(naked) void FUN_0040de80() {
    __asm {
        mov     eax, dword ptr [esp + 4]
        test    eax, eax
        jz      early_ret
        mov     edx, dword ptr [ecx + 4]
        add     dword ptr [edx + 0x14], 1
        push    ebx
        push    esi
        mov     esi, dword ptr [ecx + 4]
        movzx   ebx, word ptr [esi + 0xc]
        mov     edx, esi
        sub     eax, dword ptr [edx]
        cdq
        idiv    ebx
        movzx   edx, word ptr [esi + 0xa]
        mov     esi, dword ptr [esi + 4]
        mov     word ptr [esi + edx*2], ax
        mov     ecx, dword ptr [ecx + 4]
        add     word ptr [ecx + 0xa], 1
        pop     esi
        pop     ebx
    early_ret:
        ret     4
    }
}
