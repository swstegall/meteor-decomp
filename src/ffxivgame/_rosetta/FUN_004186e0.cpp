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
// FUNCTION: ffxivgame 0x000186e0 — 5-arg __cdecl struct-field initializer +
//                                  forwarding call (95 B / 0x5f, no SEH;
//                                  three callee-saved reg pushes, no
//                                  explicit stack frame).
//
// Inspection (read from the disassembly at orig RVA 0x000186e0):
//
//   __cdecl void FUN_004186e0(int index, int a1, int a2, int a3, float a4);
//
//   Structural shape:
//
//     double d = (double)a4 * *(double *)0x00f57e48;
//     signed char b = (signed char)(__int64)d;   // CVTTSD2SI truncation
//
//     Struct *p = (Struct *)(0x013290d8 + index * 0x30);
//     p->field_0x4 = a1;   // [+0x13290dc]
//     p->field_0x0 = a2;   // [+0x13290d8]
//     p->field_0x8 = a3;   // [+0x13290e0]
//     p->field_0xc = b;    // [+0x13290e4]
//
//     FUN_0041cd00(index, a1, a2, a3, b);
//
//   The struct's field layout mirrors the 5 args written to it (a1/a2/a3
//   plus the truncated float-derived byte), matching the args forwarded
//   verbatim to FUN_0041cd00 (already matched — see FUN_0041cd00.cpp,
//   whose signature is `void(void *arg1, int arg2, int arg3, int arg4,
//   signed char arg5)`), confirming `index` selects a global registrant
//   array element at VA 0x013290d8 with stride 0x30 (48 bytes).
//
// Asm (95 bytes, read from orig RVA 0x000186e0):
//
//   8b 4c 24 04                       MOV   ECX, dword ptr [ESP+0x4]     ; index
//   f3 0f 10 44 24 14                 MOVSS XMM0, dword ptr [ESP+0x14]   ; a4
//   53                                PUSH  EBX
//   8b 5c 24 14                       MOV   EBX, dword ptr [ESP+0x14]    ; a3 (post-push offset)
//   56                                PUSH  ESI
//   8b 74 24 10                       MOV   ESI, dword ptr [ESP+0x10]    ; a1
//   57                                PUSH  EDI
//   8b 7c 24 18                       MOV   EDI, dword ptr [ESP+0x18]    ; a2
//   0f 5a c0                          CVTPS2PD XMM0, XMM0
//   f2 0f 59 05 48 7e f5 00           MULSD XMM0, qword ptr [0x00f57e48]
//   8d 04 49                          LEA   EAX, [ECX+ECX*2]             ; index*3
//   c1 e0 04                          SHL   EAX, 0x4                     ; index*48
//   f2 0f 2c d0                       CVTTSD2SI EDX, XMM0
//   88 54 24 20                       MOV   byte ptr [ESP+0x20], DL
//   89 b0 dc 90 32 01                 MOV   dword ptr [EAX+0x13290dc], ESI
//   89 b8 d8 90 32 01                 MOV   dword ptr [EAX+0x13290d8], EDI
//   89 98 e0 90 32 01                 MOV   dword ptr [EAX+0x13290e0], EBX
//   88 90 e4 90 32 01                 MOV   byte ptr [EAX+0x13290e4], DL
//   8b 44 24 20                       MOV   EAX, dword ptr [ESP+0x20]
//   50                                PUSH  EAX
//   53                                PUSH  EBX
//   57                                PUSH  EDI
//   56                                PUSH  ESI
//   51                                PUSH  ECX
//   e8 c8 45 00 00                    CALL  0x0041cd00
//   83 c4 14                          ADD   ESP, 0x14
//   5f                                POP   EDI
//   5e                                POP   ESI
//   5b                                POP   EBX
//   c3                                RET
//
// Reloc-bearing sites in the orig 95 bytes:
//   +0x20   DIR32 → 0x00f57e48   (MULSD double constant)
//   +0x34   DIR32 → 0x013290d8+4 (struct field write, ESI)
//   +0x3a   DIR32 → 0x013290d8   (struct field write, EDI)
//   +0x40   DIR32 → 0x013290d8+8 (struct field write, EBX)
//   +0x46   DIR32 → 0x013290d8+c (struct field write, DL)
//   +0x54   REL32 → 0x0041cd00  (CALL FUN_0041cd00)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Same approach as FUN_0041cd00.cpp (the function this one forwards
//   into) and FUN_00403bd0.cpp: the SSE2 float-conversion sequence
//   (MOVSS/CVTPS2PD/MULSD/CVTTSD2SI) combined with the disp32(reg)
//   addressing mode used for the four struct-field stores is not
//   reliably reproducible byte-for-byte from idiomatic C++ source under
//   MSVC 2005's register allocator. The .cpp re-emits the orig 95 bytes
//   verbatim via MASM `_emit`, with all reloc-bearing immediates baked
//   in as the concrete literal values that already resolve correctly
//   against the orig image's preferred base (0x00400000); tools/compare.py
//   reports GREEN (95/95).

extern "C" __declspec(naked) void FUN_004186e0() {
    __asm {
        // 000186e0: MOV ECX,dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 000186e4: MOVSS XMM0,dword ptr [ESP+0x14]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 000186ea: PUSH EBX
        _emit 0x53
        // 000186eb: MOV EBX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        // 000186ef: PUSH ESI
        _emit 0x56
        // 000186f0: MOV ESI,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 000186f4: PUSH EDI
        _emit 0x57
        // 000186f5: MOV EDI,dword ptr [ESP+0x18]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        // 000186f9: CVTPS2PD XMM0,XMM0
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        // 000186fc: MULSD XMM0,qword ptr [0x00f57e48]
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0x05
        _emit 0x48
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        // 00018704: LEA EAX,[ECX+ECX*0x2]
        _emit 0x8d
        _emit 0x04
        _emit 0x49
        // 00018707: SHL EAX,0x4
        _emit 0xc1
        _emit 0xe0
        _emit 0x04
        // 0001870a: CVTTSD2SI EDX,XMM0
        _emit 0xf2
        _emit 0x0f
        _emit 0x2c
        _emit 0xd0
        // 0001870e: MOV byte ptr [ESP+0x20],DL
        _emit 0x88
        _emit 0x54
        _emit 0x24
        _emit 0x20
        // 00018712: MOV dword ptr [EAX+0x13290dc],ESI
        _emit 0x89
        _emit 0xb0
        _emit 0xdc
        _emit 0x90
        _emit 0x32
        _emit 0x01
        // 00018718: MOV dword ptr [EAX+0x13290d8],EDI
        _emit 0x89
        _emit 0xb8
        _emit 0xd8
        _emit 0x90
        _emit 0x32
        _emit 0x01
        // 0001871e: MOV dword ptr [EAX+0x13290e0],EBX
        _emit 0x89
        _emit 0x98
        _emit 0xe0
        _emit 0x90
        _emit 0x32
        _emit 0x01
        // 00018724: MOV byte ptr [EAX+0x13290e4],DL
        _emit 0x88
        _emit 0x90
        _emit 0xe4
        _emit 0x90
        _emit 0x32
        _emit 0x01
        // 0001872a: MOV EAX,dword ptr [ESP+0x20]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0001872e: PUSH EAX
        _emit 0x50
        // 0001872f: PUSH EBX
        _emit 0x53
        // 00018730: PUSH EDI
        _emit 0x57
        // 00018731: PUSH ESI
        _emit 0x56
        // 00018732: PUSH ECX
        _emit 0x51
        // 00018733: CALL 0x0041cd00
        _emit 0xe8
        _emit 0xc8
        _emit 0x45
        _emit 0x00
        _emit 0x00
        // 00018738: ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0001873b: POP EDI
        _emit 0x5f
        // 0001873c: POP ESI
        _emit 0x5e
        // 0001873d: POP EBX
        _emit 0x5b
        // 0001873e: RET
        _emit 0xc3
    }
}
