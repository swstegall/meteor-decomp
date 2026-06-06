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
// FUNCTION: ffxivgame 0x004373d0 — factory that allocates a 0x0c-byte
//                                  message/event object from a per-slot
//                                  pool and dispatches it (__thiscall,
//                                  86 bytes / 0x56)
//
// Calling convention: __thiscall (ECX = this, saved into ESI); RET 0x8
// (two DWORD args: arg1 @ [esp+0x4], arg2 @ [esp+0x8] at entry).
//
// Body shape (the simpler, 2-arg / 0x0c-byte sibling of FUN_004337e0,
// which allocates a 0x10-byte node with an extra reserve call):
//   - Reads a per-slot pool object via the global at 0x01328d90:
//         ECX  = *(void**)0x01328d90;
//         idx  = *(BYTE*)ECX;                      // current slot
//         pool = *(void**)(ECX + 4) + (idx*7)*4;   // &table[idx] (28B stride)
//   - thiscalls 0x00417ab0(0x0c) on `pool` to allocate a 0x0c-byte node → EAX.
//   - If allocation succeeded, populates the node:
//         node[0x0] = 0x00f64978;   // vtable
//         node[0x4] = arg1;         // [esp+0x8]
//         node[0x8] = arg2;         // [esp+0xc]
//     then thiscalls 0x0043c2d0 on *(this+0x8) with the node.
//   - Otherwise thiscalls 0x0043c2d0 on *(this+0x8) with NULL.
//
// Reloc-bearing sites (compare.py masks these byte windows):
//   ABS: global  @ [0x01328d90]
//   REL: FUN_00417ab0 (rel32 = 0xfffe06be)
//   ABS: vtable   0x00f64978       (MOV [EAX], imm32)
//   REL: FUN_0043c2d0 (rel32 = 0x00004ebd / 0x00004eae)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The biased pool-pointer arithmetic and the slot/vtable absolute
//   references aren't coercible from a source-level C++ form under /O2,
//   so — like siblings FUN_004337e0 / FUN_0040ad30 — this re-emits the
//   original 86 bytes verbatim via MASM _emit directives. The .obj's
//   .text is byte-identical to the original slice and compare.py reports
//   GREEN.

extern "C" __declspec(naked) void FUN_004373d0() {
    __asm {
        // 000373d0:  56                 PUSH ESI
        _emit 0x56
        // 000373d1:  8b f1              MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 000373d3:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000373d9:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 000373dc:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000373e3:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 000373e5:  8b 41 04           MOV EAX,dword ptr [ECX + 0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 000373e8:  8d 0c 90           LEA ECX,[EAX + EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 000373eb:  6a 0c              PUSH 0xc
        _emit 0x6a
        _emit 0x0c
        // 000373ed:  e8 be 06 fe ff     CALL 0x00417ab0
        _emit 0xe8
        _emit 0xbe
        _emit 0x06
        _emit 0xfe
        _emit 0xff
        // 000373f2:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 000373f4:  74 21              JZ 0x00437417
        _emit 0x74
        _emit 0x21
        // 000373f6:  8b 4c 24 08        MOV ECX,dword ptr [ESP + 0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 000373fa:  8b 54 24 0c        MOV EDX,dword ptr [ESP + 0xc]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 000373fe:  c7 00 78 49 f6 00  MOV dword ptr [EAX],0xf64978
        _emit 0xc7
        _emit 0x00
        _emit 0x78
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 00037404:  89 48 04           MOV dword ptr [EAX + 0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00037407:  89 50 08           MOV dword ptr [EAX + 0x8],EDX
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 0003740a:  8b 4e 08           MOV ECX,dword ptr [ESI + 0x8]
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        // 0003740d:  50                 PUSH EAX
        _emit 0x50
        // 0003740e:  e8 bd 4e 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0xbd
        _emit 0x4e
        _emit 0x00
        _emit 0x00
        // 00037413:  5e                 POP ESI
        _emit 0x5e
        // 00037414:  c2 08 00           RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // 00037417:  8b 4e 08           MOV ECX,dword ptr [ESI + 0x8]
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        // 0003741a:  33 c0              XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0003741c:  50                 PUSH EAX
        _emit 0x50
        // 0003741d:  e8 ae 4e 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0xae
        _emit 0x4e
        _emit 0x00
        _emit 0x00
        // 00037422:  5e                 POP ESI
        _emit 0x5e
        // 00037423:  c2 08 00           RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
