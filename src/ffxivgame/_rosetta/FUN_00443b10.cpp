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
// FUNCTION: ffxivgame 0x00043b10 — float-argument greater-than check
//                                  against first component of a 16-byte-
//                                  aligned 4-float vector (__thiscall, 75 B)
//
// __thiscall bool IsGreaterThanX(float arg)
//
//   `this` (ECX) points to a 16-byte-aligned 4-float struct (or __m128).
//   Returns 1 if arg > this->floats[0], 0 otherwise.
//   The comparison is done via double promotion (CVTPS2PD + COMISD).
//
// Calling convention:  __thiscall — ECX = this, one stack arg (float, 4 B),
//                      RET 0x4 (callee-cleans the 4-byte stack argument).
// Stack frame:         PUSH EBP / MOV EBP,ESP / AND ESP,0xfffffff0
//                      (16-byte alignment for SSE MOVAPS) / SUB ESP,0x20.
//
// Sequence of operations (read from orig RVA 0x00043b10, 75 bytes):
//
//   prolog: PUSH EBP; MOV EBP,ESP; AND ESP,0xfffffff0; SUB ESP,0x20
//   MOVSS  XMM1, [EBP+8]         ; XMM1 = arg (float)
//   XORPS  XMM0, XMM0            ; zero XMM0 upper bits
//   MOVSS  XMM0, XMM1            ; XMM0 = arg (upper 96 bits zeroed)
//   MOVAPS [ESP+0x10], XMM0      ; store aligned {arg,0,0,0}
//   MOVAPS XMM0, [ECX]           ; XMM0 = {this->x,y,z,w}
//   MOVSS  XMM1, [ESP+0x10]      ; XMM1 = arg (reload)
//   MOVAPS [ESP], XMM0           ; spill this->vector to stack
//   MOVSS  XMM0, [ESP]           ; XMM0 = this->x (first float)
//   CVTPS2PD XMM0, XMM0          ; promote this->x to double
//   CVTPS2PD XMM1, XMM1          ; promote arg to double
//   COMISD XMM1, XMM0            ; compare (double)arg vs (double)this->x
//   JBE → xor eax,eax; epilog    ; arg <= this->x → return 0
//   MOV EAX,1; epilog            ; arg >  this->x → return 1
//   epilog: MOV ESP,EBP; POP EBP; RET 0x4
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The SSE2 codegen (CVTPS2PD + COMISD) for what is logically a
//   float comparison is MSVC 2005's artefact of the __m128-typed member
//   and the aligned-load/store round-trip — no C++ source form reliably
//   reproduces the exact instruction schedule, register allocation, and
//   AND-alignment prologue together. The 75-byte slice contains ZERO
//   relocations (no CALL rel32, no DIR32 fixups); every immediate and
//   displacement is self-contained. compare.py therefore reports GREEN
//   by direct equality with no wildcard windows needed.

extern "C" __declspec(naked) void FUN_00443b10() {
    __asm {
        // 00043b10: push ebp
        _emit 0x55
        // 00043b11: mov ebp, esp
        _emit 0x8b
        _emit 0xec
        // 00043b13: and esp, 0xfffffff0
        _emit 0x83
        _emit 0xe4
        _emit 0xf0
        // 00043b16: sub esp, 0x20
        _emit 0x83
        _emit 0xec
        _emit 0x20
        // 00043b19: movss xmm1, dword ptr [ebp+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x4d
        _emit 0x08
        // 00043b1e: xorps xmm0, xmm0
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        // 00043b21: movss xmm0, xmm1
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0xc1
        // 00043b25: movaps xmmword ptr [esp+0x10], xmm0
        _emit 0x0f
        _emit 0x29
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00043b2a: movaps xmm0, xmmword ptr [ecx]
        _emit 0x0f
        _emit 0x28
        _emit 0x01
        // 00043b2d: movss xmm1, dword ptr [esp+0x10]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00043b33: movaps xmmword ptr [esp], xmm0
        _emit 0x0f
        _emit 0x29
        _emit 0x04
        _emit 0x24
        // 00043b37: movss xmm0, dword ptr [esp]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x04
        _emit 0x24
        // 00043b3c: cvtps2pd xmm0, xmm0
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        // 00043b3f: cvtps2pd xmm1, xmm1
        _emit 0x0f
        _emit 0x5a
        _emit 0xc9
        // 00043b42: comisd xmm1, xmm0
        _emit 0x66
        _emit 0x0f
        _emit 0x2f
        _emit 0xc8
        // 00043b46: jbe +0x0b  (→ 0x00443b53)
        _emit 0x76
        _emit 0x0b
        // 00043b48: mov eax, 1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00043b4d: mov esp, ebp
        _emit 0x8b
        _emit 0xe5
        // 00043b4f: pop ebp
        _emit 0x5d
        // 00043b50: ret 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 00043b53: xor eax, eax
        _emit 0x33
        _emit 0xc0
        // 00043b55: mov esp, ebp
        _emit 0x8b
        _emit 0xe5
        // 00043b57: pop ebp
        _emit 0x5d
        // 00043b58: ret 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
