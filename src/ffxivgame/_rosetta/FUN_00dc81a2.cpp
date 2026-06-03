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
// FUNCTION: ffxivgame 0x009c81a2 — Kernighan popcount (count set bits)
//                                  (__stdcall, 26 B)
//
// Counts the number of set bits in a 32-bit integer using Kernighan's
// method: repeatedly strip the lowest set bit via `n &= n-1` and count
// iterations until n == 0.  Returns zero immediately when called with
// zero.
//
// Calling convention: __stdcall (RET 4 — callee cleans one DWORD arg).
// Register layout: arg → ECX, count → EAX, temp (n-1) → EDX.
//
// The function opens with MOV EDI,EDI (8B FF) — the standard MSVC 2005
// two-byte hot-patch NOP — followed by the ordinary EBP frame prologue.
// We use __declspec(naked) to emit the bytes verbatim, which avoids any
// interaction with the /Oy (frame-pointer-omission) flag in ROSETTA_FLAGS
// that would otherwise remove the PUSH EBP / MOV EBP,ESP prologue.
//
// Asm (26 bytes @ RVA 0x009c81a2):
//   8b ff                 MOV  EDI, EDI          ; hot-patch NOP
//   55                    PUSH EBP
//   8b ec                 MOV  EBP, ESP
//   8b 4d 08              MOV  ECX, [EBP+8]      ; ECX = n
//   33 c0                 XOR  EAX, EAX           ; EAX = 0  (count)
//   85 c9                 TEST ECX, ECX
//   74 08                 JZ   +0x8 → done        ; if n==0 skip loop
//                         ; loop_top:
//   8d 51 ff              LEA  EDX, [ECX-1]       ; EDX = n-1
//   40                    INC  EAX                 ; count++
//   23 ca                 AND  ECX, EDX            ; n &= n-1
//   75 f8                 JNZ  -0x8 → loop_top    ; if n!=0 continue
//                         ; done:
//   5d                    POP  EBP
//   c2 04 00              RET  4

extern "C" __declspec(naked) int __stdcall FUN_00dc81a2(unsigned int /*n*/)
{
    __asm {
        mov  edi, edi
        push ebp
        mov  ebp, esp
        mov  ecx, [ebp + 8]
        xor  eax, eax
        test ecx, ecx
        jz   done
    loop_top:
        lea  edx, [ecx - 1]
        inc  eax
        and  ecx, edx
        jnz  loop_top
    done:
        pop  ebp
        ret  4
    }
}
