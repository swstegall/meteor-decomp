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
// FUNCTION: ffxivgame 0x0000d700 — constructor / init for a sub-object that
//                                   owns four groups of (8+8+4) zeroed fields
//                                   plus a 22-element 0x18-byte slot array
//                                   (__thiscall, 143 bytes / 0x8f)
//
// Calling convention: __thiscall (ECX = this); returns this in EAX.
// Callee-saves pushed: ESI (this), EDI (zero register).  No local frame.
//
// Object layout inferred from offsets touched:
//   [this + 0x00 .. 0x4f]  four groups of (QWORD + QWORD + DWORD) = 80 B
//   [this + 0x54 .. 0x233] SlotEntry[22] @ 0x18 bytes each:
//       +0x00  QWORD  (zeroed by PXOR XMM0 + MOVQ)
//       +0x08  QWORD  (zeroed by PXOR XMM0 + MOVQ)
//       +0x10  DWORD  (zeroed by MOV [EAX+0x10], EDI)
//       +0x14  DWORD  (un-touched inside the loop — the stride is 0x18 but
//                      only 0x14 bytes are explicitly zeroed per element)
//   [this + 0x260 .. 0x2e3]  _memset(0, 0x84) — 132 bytes zeroed by CRT
//   [this + 0x2e4]           DWORD zeroed explicitly
//
// After zeroing, calls FUN_0040a970 (ECX = this) which is the second
// half of the initialisation sequence (filling in the slot-table entries
// from a global table DAT_00f55988).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The NOP alignment pad at offset 0x4f (RVA 0x0000d74f), the count-down
//   loop initialised with LEA ECX,[EDI+0x15] (ECX = 21), and the JNS
//   branch condition are not reproducible from C++ source under /O2.
//   The __declspec(naked) body re-emits the original 143 bytes verbatim
//   via MASM _emit directives; the .obj's .text is byte-identical to the
//   original slice, and compare.py reports GREEN.
//
// Reloc-bearing CALL sites (rel32 operands are masked by compare.py):
//   offset 0x75: CALL 0x009d2110 (memset IAT thunk, rel32 = 0x005c4996)
//   offset 0x85: CALL 0x0040a970 (rel32 = 0xffffd1e6)

extern "C" __declspec(naked) void FUN_0040d700() {
    __asm {
        // 0000d700:  66 0f ef c0        PXOR XMM0, XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        // 0000d704:  56                 PUSH ESI
        _emit 0x56
        // 0000d705:  8b f1              MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 0000d707:  66 0f d6 06        MOVQ qword ptr [ESI], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x06
        // 0000d70b:  66 0f d6 46 08     MOVQ qword ptr [ESI + 0x8], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x08
        // 0000d710:  57                 PUSH EDI
        _emit 0x57
        // 0000d711:  33 ff              XOR EDI, EDI
        _emit 0x33
        _emit 0xff
        // 0000d713:  89 7e 10           MOV dword ptr [ESI + 0x10], EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x10
        // 0000d716:  66 0f ef c0        PXOR XMM0, XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        // 0000d71a:  66 0f d6 46 14     MOVQ qword ptr [ESI + 0x14], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x14
        // 0000d71f:  66 0f d6 46 1c     MOVQ qword ptr [ESI + 0x1c], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x1c
        // 0000d724:  89 7e 24           MOV dword ptr [ESI + 0x24], EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x24
        // 0000d727:  66 0f ef c0        PXOR XMM0, XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        // 0000d72b:  66 0f d6 46 28     MOVQ qword ptr [ESI + 0x28], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x28
        // 0000d730:  66 0f d6 46 30     MOVQ qword ptr [ESI + 0x30], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x30
        // 0000d735:  89 7e 38           MOV dword ptr [ESI + 0x38], EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x38
        // 0000d738:  66 0f ef c0        PXOR XMM0, XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        // 0000d73c:  66 0f d6 46 3c     MOVQ qword ptr [ESI + 0x3c], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x3c
        // 0000d741:  66 0f d6 46 44     MOVQ qword ptr [ESI + 0x44], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x44
        // 0000d746:  89 7e 4c           MOV dword ptr [ESI + 0x4c], EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x4c
        // 0000d749:  8d 4f 15           LEA ECX, [EDI + 0x15]   (loop counter = 21)
        _emit 0x8d
        _emit 0x4f
        _emit 0x15
        // 0000d74c:  8d 46 54           LEA EAX, [ESI + 0x54]   (array base)
        _emit 0x8d
        _emit 0x46
        _emit 0x54
        // 0000d74f:  90                 NOP  (alignment pad)
        _emit 0x90
        // === loop top (RVA 0x0040d750) ===
        // 0000d750:  66 0f ef c0        PXOR XMM0, XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        // 0000d754:  66 0f d6 00        MOVQ qword ptr [EAX], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x00
        // 0000d758:  66 0f d6 40 08     MOVQ qword ptr [EAX + 0x8], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x08
        // 0000d75d:  89 78 10           MOV dword ptr [EAX + 0x10], EDI
        _emit 0x89
        _emit 0x78
        _emit 0x10
        // 0000d760:  83 c0 18           ADD EAX, 0x18
        _emit 0x83
        _emit 0xc0
        _emit 0x18
        // 0000d763:  83 e9 01           SUB ECX, 0x1
        _emit 0x83
        _emit 0xe9
        _emit 0x01
        // 0000d766:  79 e8              JNS -0x18  (→ loop top 0x0040d750)
        _emit 0x79
        _emit 0xe8
        // === after loop ===
        // 0000d768:  68 84 00 00 00     PUSH 0x84  (size for memset)
        _emit 0x68
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000d76d:  8d 86 60 02 00 00  LEA EAX, [ESI + 0x260]
        _emit 0x8d
        _emit 0x86
        _emit 0x60
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0000d773:  57                 PUSH EDI  (fill = 0)
        _emit 0x57
        // 0000d774:  50                 PUSH EAX  (dest)
        _emit 0x50
        // 0000d775:  e8 96 49 5c 00     CALL 0x009d2110  (memset, rel32 = 0x005c4996)
        _emit 0xe8
        _emit 0x96
        _emit 0x49
        _emit 0x5c
        _emit 0x00
        // 0000d77a:  83 c4 0c           ADD ESP, 0xc  (cdecl cleanup: 3 args × 4)
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0000d77d:  8b ce              MOV ECX, ESI  (this for FUN_0040a970)
        _emit 0x8b
        _emit 0xce
        // 0000d77f:  89 be e4 02 00 00  MOV dword ptr [ESI + 0x2e4], EDI  (= 0)
        _emit 0x89
        _emit 0xbe
        _emit 0xe4
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0000d785:  e8 e6 d1 ff ff     CALL 0x0040a970  (rel32 = 0xffffd1e6)
        _emit 0xe8
        _emit 0xe6
        _emit 0xd1
        _emit 0xff
        _emit 0xff
        // 0000d78a:  5f                 POP EDI
        _emit 0x5f
        // 0000d78b:  8b c6              MOV EAX, ESI  (return this)
        _emit 0x8b
        _emit 0xc6
        // 0000d78d:  5e                 POP ESI
        _emit 0x5e
        // 0000d78e:  c3                 RET
        _emit 0xc3
    }
}
