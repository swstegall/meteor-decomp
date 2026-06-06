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
// FUNCTION: ffxivgame 0x0045b750 — field swap for a 3-DWORD struct
//                                  (45 B / 0x2D)
//
// Swaps the three DWORD fields at offsets +0x4, +0x8, +0xC between
// `this` (ECX) and the pointed-at object `*other` (ESP+4).  The DWORD at
// offset +0x0 (likely a vtable pointer) is untouched.
//
// Calling convention: __thiscall (ECX = this).
//   One stack argument (4 bytes, pointer to the peer object); callee cleans
//   it via RET 4.
//
// Frame: PUSH ESI only (no ESP adjustment).
//
// Asm (45 bytes @ orig RVA 0x0005b750):
//   8b 44 24 04    MOV EAX, dword ptr [ESP+0x4]   ; EAX = other
//   8b 51 04       MOV EDX, dword ptr [ECX+0x4]   ; EDX = this->f4  (hoisted before push)
//   56             PUSH ESI
//   8b 70 04       MOV ESI, dword ptr [EAX+0x4]   ; ESI = other->f4
//   89 71 04       MOV dword ptr [ECX+0x4], ESI   ; this->f4  = other->f4
//   89 50 04       MOV dword ptr [EAX+0x4], EDX   ; other->f4 = old this->f4
//   8b 70 08       MOV ESI, dword ptr [EAX+0x8]   ; ESI = other->f8
//   8b 51 08       MOV EDX, dword ptr [ECX+0x8]   ; EDX = this->f8
//   89 71 08       MOV dword ptr [ECX+0x8], ESI   ; this->f8  = other->f8
//   89 50 08       MOV dword ptr [EAX+0x8], EDX   ; other->f8 = old this->f8
//   8b 70 0c       MOV ESI, dword ptr [EAX+0xc]   ; ESI = other->fc
//   8b 51 0c       MOV EDX, dword ptr [ECX+0xc]   ; EDX = this->fc
//   89 71 0c       MOV dword ptr [ECX+0xc], ESI   ; this->fc  = other->fc
//   89 50 0c       MOV dword ptr [EAX+0xc], EDX   ; other->fc = old this->fc
//   5e             POP ESI
//   c2 04 00       RET 0x4
//
// No absolute addresses or relocation sites — all memory operands are
// ECX/EAX-relative with disp8 offsets.  Naked-asm with standard mnemonics
// reproduces the 45-byte sequence byte-for-byte.
//
// Scheduling note: the first field's this-load (EDX = [ECX+4]) is hoisted
// before the PUSH ESI.  Subsequent fields follow the "load arg, load this,
// store to this, store to arg" cadence.

extern "C" __declspec(naked) void FUN_0045b750() {
    __asm {
        mov     eax, dword ptr [esp + 4]
        mov     edx, dword ptr [ecx + 4]
        push    esi
        mov     esi, dword ptr [eax + 4]
        mov     dword ptr [ecx + 4], esi
        mov     dword ptr [eax + 4], edx
        mov     esi, dword ptr [eax + 8]
        mov     edx, dword ptr [ecx + 8]
        mov     dword ptr [ecx + 8], esi
        mov     dword ptr [eax + 8], edx
        mov     esi, dword ptr [eax + 0x0c]
        mov     edx, dword ptr [ecx + 0x0c]
        mov     dword ptr [ecx + 0x0c], esi
        mov     dword ptr [eax + 0x0c], edx
        pop     esi
        ret     4
    }
}
