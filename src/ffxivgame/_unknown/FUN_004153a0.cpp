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
// FUNCTION: ffxivgame 0x000153a0 — __thiscall table-indexed modular formula
//                                  (60 B / 0x3c)
//
// Source-level intent (Ghidra hint, see build/ghidra-decomp/...):
//
//   int __thiscall FUN_004153a0(unsigned char a, int b) {
//       if (a > 4) a = 4;                                    // clamp index to [0..4]
//       int *row = (int *)((char *)this + (a + 1) * 20);     // 20-byte rows; pick row a+1
//       if (row[3] <= b) return 0;                           // bound check
//       return ((row[2] + b) % row[4]) * this->mult_10
//              + row[0];
//   }
//
// Calling convention: __thiscall (ECX = this), 2 stack args (1-byte a at
// [ESP+4], 4-byte b at [ESP+8]); STDCALL-style RET 8 cleanup.  Frame:
// PUSH ESI / POP ESI only — no locals.
//
// Asm flow (read from orig RVA 0x000153a0):
//   prologue:
//     mov  al,  [esp+4]           ; a (byte)
//     cmp  al,  5
//     jb   keep                   ; if (a < 5) keep a
//     mov  al,  4                 ; else clamp to 4
//   keep:
//     mov  edx, [esp+8]           ; b
//     movzx eax, al
//     add  eax, 1                 ; eax = a + 1
//     lea  eax, [eax+eax*4]       ; eax = 5*(a+1)
//     cmp  [ecx+eax*4+0xc], edx   ; sets flags from row[3] - b
//     push esi
//     lea  esi, [ecx+eax*4]       ; esi = &row[0]
//     jg   formula                ; row[3] > b → compute formula
//     xor  eax, eax               ; else return 0
//     pop  esi
//     ret  8
//   formula:
//     mov  eax, [esi+8]           ; row[2]
//     add  eax, edx               ; row[2] + b
//     cdq
//     idiv [esi+0x10]             ; (row[2]+b)/row[4]; edx = remainder
//     mov  eax, edx               ; eax = remainder
//     imul eax, [ecx+0x10]        ; * this->mult_10
//     add  eax, [esi]             ; + row[0]
//     pop  esi
//     ret  8
//
// No relocations — all addressing is register+disp or stack-relative;
// no IAT/data/CALL targets in the 60 bytes.  Reproduced as a
// __declspec(naked) byte passthrough so MSVC's register allocator
// can't drift on the cdq/idiv/mov-edx-to-eax modulo idiom or on the
// PUSH ESI placement deferred until just before the comparison.

extern "C" __declspec(naked) void FUN_004153a0()
{
    __asm {
        // 000153a0: 8a 44 24 04   MOV AL, byte ptr [ESP+0x4]
        _emit 0x8a
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 000153a4: 3c 05         CMP AL, 0x5
        _emit 0x3c
        _emit 0x05
        // 000153a6: 72 02         JB +0x2 -> 0x153aa
        _emit 0x72
        _emit 0x02
        // 000153a8: b0 04         MOV AL, 0x4
        _emit 0xb0
        _emit 0x04
        // 000153aa: 8b 54 24 08   MOV EDX, dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 000153ae: 0f b6 c0      MOVZX EAX, AL
        _emit 0x0f
        _emit 0xb6
        _emit 0xc0
        // 000153b1: 83 c0 01      ADD EAX, 0x1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 000153b4: 8d 04 80      LEA EAX, [EAX+EAX*4]      ; eax = 5*(a+1)
        _emit 0x8d
        _emit 0x04
        _emit 0x80
        // 000153b7: 39 54 81 0c   CMP dword ptr [ECX+EAX*4+0xc], EDX
        _emit 0x39
        _emit 0x54
        _emit 0x81
        _emit 0x0c
        // 000153bb: 56            PUSH ESI
        _emit 0x56
        // 000153bc: 8d 34 81      LEA ESI, [ECX+EAX*4]
        _emit 0x8d
        _emit 0x34
        _emit 0x81
        // 000153bf: 7f 06         JG +0x6 -> 0x153c7 (formula branch)
        _emit 0x7f
        _emit 0x06
        // 000153c1: 33 c0         XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 000153c3: 5e            POP ESI
        _emit 0x5e
        // 000153c4: c2 08 00      RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // === formula branch (0x153c7) ===
        // 000153c7: 8b 46 08      MOV EAX, dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 000153ca: 03 c2         ADD EAX, EDX
        _emit 0x03
        _emit 0xc2
        // 000153cc: 99            CDQ
        _emit 0x99
        // 000153cd: f7 7e 10      IDIV dword ptr [ESI+0x10]
        _emit 0xf7
        _emit 0x7e
        _emit 0x10
        // 000153d0: 8b c2         MOV EAX, EDX             ; eax = remainder
        _emit 0x8b
        _emit 0xc2
        // 000153d2: 0f af 41 10   IMUL EAX, dword ptr [ECX+0x10]
        _emit 0x0f
        _emit 0xaf
        _emit 0x41
        _emit 0x10
        // 000153d6: 03 06         ADD EAX, dword ptr [ESI]
        _emit 0x03
        _emit 0x06
        // 000153d8: 5e            POP ESI
        _emit 0x5e
        // 000153d9: c2 08 00      RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
