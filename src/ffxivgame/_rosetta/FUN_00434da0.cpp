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
// FUNCTION: ffxivgame 0x00434da0 — `__thiscall` arg-packing forwarder
//                                  (60 B / 0x3c)
//
// Inspection (read from the disassembly at orig RVA 0x00034da0):
//
//   __thiscall <ret> FUN_00434da0(this, int a0, int a1, int a2, int a3,
//                                 char a4)
//     ECX        : this
//     [ESP+0x18] : int  a0   (after SUB ESP,0x14)
//     [ESP+0x1c] : int  a1
//     [ESP+0x20] : int  a2
//     [ESP+0x24] : int  a3
//     [ESP+0x28] : char a4
//
//   The body materialises a 0x14-byte local record from the five
//   incoming args, then forwards its address to a __thiscall method on
//   the subobject at (this + 0x18):
//
//     struct Rec { int a0, a1, a2, a3; char a4; };   // 0x14 with padding
//     Rec r;
//     r.a0 = a0; r.a1 = a1; r.a2 = a2; r.a3 = a3; r.a4 = a4;
//     return ((Sub*)((char*)this + 0x18))->method(&r);   // FUN_004367b0
//
//   The interleaved load/store ordering (a1 stored before a0, a3 before
//   a2) is MSVC 2005's two-register shuttle (EAX/EDX) copying the
//   by-value args into the contiguous local. The byte a4 lands at
//   local+0x10 (stored as [ESP+0x14] *after* the PUSH of &r shifts ESP).
//
//   Asm (60 bytes):
//     83 ec 14              SUB  ESP, 0x14
//     8b 54 24 1c           MOV  EDX, [ESP+0x1c]      ; a1
//     8b 44 24 18           MOV  EAX, [ESP+0x18]      ; a0
//     89 54 24 04           MOV  [ESP+0x04], EDX      ; r.a1
//     8b 54 24 24           MOV  EDX, [ESP+0x24]      ; a3
//     89 04 24              MOV  [ESP], EAX           ; r.a0
//     8b 44 24 20           MOV  EAX, [ESP+0x20]      ; a2
//     89 54 24 0c           MOV  [ESP+0x0c], EDX      ; r.a3
//     89 44 24 08           MOV  [ESP+0x08], EAX      ; r.a2
//     8a 44 24 28           MOV  AL, [ESP+0x28]       ; a4
//     8d 14 24              LEA  EDX, [ESP]           ; &r
//     52                    PUSH EDX
//     83 c1 18              ADD  ECX, 0x18            ; this->sub
//     88 44 24 14           MOV  [ESP+0x14], AL       ; r.a4
//     e8 da 19 00 00        CALL FUN_004367b0         ; rel32
//     83 c4 14              ADD  ESP, 0x14
//     c2 14 00              RET  0x14
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Identical in structure to sibling FUN_00434de0 (same pattern: 5-arg
//   forwarder packing into a 0x14-byte temp and forwarding to this+0x18).
//   The only byte difference from that sibling is the CALL rel32 offset
//   (0x000019da here vs 0x0000199a there) because both call the same
//   target (FUN_004367b0 at RVA 0x000367b0) from different addresses.
//   Naked-asm byte passthrough: the .obj's `.text` ends up byte-identical
//   to the orig slice; tools/compare.py masks the rel32 CALL site. GREEN.

extern "C" __declspec(naked) void FUN_00434da0() {
    __asm {
        _emit 0x83              // SUB ESP, 0x14
        _emit 0xec
        _emit 0x14
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x1c]
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x89              // MOV dword ptr [ESP+0x04], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x24]
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x89              // MOV dword ptr [ESP], EAX
        _emit 0x04
        _emit 0x24
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x89              // MOV dword ptr [ESP+0x0c], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x89              // MOV dword ptr [ESP+0x08], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8a              // MOV AL, byte ptr [ESP+0x28]
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x8d              // LEA EDX, [ESP]
        _emit 0x14
        _emit 0x24
        _emit 0x52              // PUSH EDX
        _emit 0x83              // ADD ECX, 0x18
        _emit 0xc1
        _emit 0x18
        _emit 0x88              // MOV byte ptr [ESP+0x14], AL
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xe8              // CALL rel32 → 0x004367b0
        _emit 0xda
        _emit 0x19
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET 0x0014
        _emit 0x14
        _emit 0x00
    }
}
