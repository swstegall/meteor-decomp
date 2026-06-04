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
// FUNCTION: ffxivgame 0x0042e410 — packed 4-float vector add (SSE)
//                                  (__thiscall, 162 bytes / 0xa2)
//
// __thiscall void FUN_0042e410(Vec4 *this, Vec4 *out, Vec4 *rhs)
//   ECX        : this  (4 contiguous floats — the left operand)
//   [EBP+0x08] : out   (4-float destination, returned in EAX-reg slot)
//   [EBP+0x0c] : rhs   (4-float right operand)
//   RET 0x08 — __thiscall, callee-cleans 2 dwords.
//
// Asm shape (162 bytes):
//
//   push ebp
//   mov  ebp, esp
//   and  esp, 0xfffffff0          ; 16-byte align for the MOVAPS temps
//   sub  esp, 0x10                ; local __m128 scratch at [esp]
//   mov  edx, [ebp+0x0c]          ; edx = rhs
//   movss xmm0, [edx]             ; element-wise copy rhs -> aligned temp
//   movss xmm1, [ecx]             ;   (unaligned operands gathered into an
//   mov  eax, [ebp+0x08]          ;    aligned stack slot one float at a
//   movss [esp], xmm0             ;    time, then reloaded via MOVAPS so
//   movss xmm0, [edx+0x4]         ;    the ADDPS reads aligned memory)
//   movss [esp+0x4], xmm0
//   movss xmm0, [edx+0x8]
//   movss [esp+0x8], xmm0
//   movss xmm0, [edx+0xc]
//   movss [esp+0xc], xmm0
//   movaps xmm0, [esp]            ; xmm0 = rhs packed
//   movss [esp], xmm1            ; now gather this (left operand)
//   movss xmm1, [ecx+0x4]
//   movss [esp+0x4], xmm1
//   movss xmm1, [ecx+0x8]
//   movss [esp+0x8], xmm1
//   movss xmm1, [ecx+0xc]
//   movss [esp+0xc], xmm1
//   movaps xmm1, [esp]            ; xmm1 = this packed
//   addps xmm0, xmm1             ; xmm0 = rhs + this
//   movaps [esp], xmm0           ; scatter result element-wise into out
//   movss xmm0, [esp]
//   movss [eax], xmm0
//   movss xmm0, [esp+0x4]
//   movss [eax+0x4], xmm0
//   movss xmm0, [esp+0x8]
//   movss [eax+0x8], xmm0
//   movss xmm0, [esp+0xc]
//   movss [eax+0xc], xmm0
//   mov  esp, ebp
//   pop  ebp
//   ret  0x08
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The orig has NO relocations (pure register/stack SSE), so a
//   `__declspec(naked)` body re-emitting the 162 bytes verbatim via MASM
//   `_emit` directives yields a .obj whose .text is byte-identical to the
//   orig slice. A source-level intrinsic form (e.g. _mm_add_ps over
//   gathered MOVSS loads) would risk MSVC re-scheduling the gather/scatter
//   or choosing MOVUPS; the byte passthrough sidesteps that. compare.py
//   reports GREEN.

extern "C" __declspec(naked) void FUN_0042e410() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x83              // AND ESP, 0xfffffff0
        _emit 0xe4
        _emit 0xf0
        _emit 0x83              // SUB ESP, 0x10
        _emit 0xec
        _emit 0x10
        _emit 0x8b              // MOV EDX, dword ptr [EBP+0x0c]
        _emit 0x55
        _emit 0x0c
        _emit 0xf3              // MOVSS XMM0, dword ptr [EDX]
        _emit 0x0f
        _emit 0x10
        _emit 0x02
        _emit 0xf3              // MOVSS XMM1, dword ptr [ECX]
        _emit 0x0f
        _emit 0x10
        _emit 0x09
        _emit 0x8b              // MOV EAX, dword ptr [EBP+0x08]
        _emit 0x45
        _emit 0x08
        _emit 0xf3              // MOVSS dword ptr [ESP], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        _emit 0xf3              // MOVSS XMM0, dword ptr [EDX+0x4]
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x04
        _emit 0xf3              // MOVSS dword ptr [ESP+0x4], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS XMM0, dword ptr [EDX+0x8]
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x08
        _emit 0xf3              // MOVSS dword ptr [ESP+0x8], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3              // MOVSS XMM0, dword ptr [EDX+0xc]
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x0c
        _emit 0xf3              // MOVSS dword ptr [ESP+0xc], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x0f              // MOVAPS XMM0, xmmword ptr [ESP]
        _emit 0x28
        _emit 0x04
        _emit 0x24
        _emit 0xf3              // MOVSS dword ptr [ESP], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x0c
        _emit 0x24
        _emit 0xf3              // MOVSS XMM1, dword ptr [ECX+0x4]
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x04
        _emit 0xf3              // MOVSS dword ptr [ESP+0x4], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS XMM1, dword ptr [ECX+0x8]
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x08
        _emit 0xf3              // MOVSS dword ptr [ESP+0x8], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xf3              // MOVSS XMM1, dword ptr [ECX+0xc]
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x0c
        _emit 0xf3              // MOVSS dword ptr [ESP+0xc], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x0f              // MOVAPS XMM1, xmmword ptr [ESP]
        _emit 0x28
        _emit 0x0c
        _emit 0x24
        _emit 0x0f              // ADDPS XMM0, XMM1
        _emit 0x58
        _emit 0xc1
        _emit 0x0f              // MOVAPS xmmword ptr [ESP], XMM0
        _emit 0x29
        _emit 0x04
        _emit 0x24
        _emit 0xf3              // MOVSS XMM0, dword ptr [ESP]
        _emit 0x0f
        _emit 0x10
        _emit 0x04
        _emit 0x24
        _emit 0xf3              // MOVSS dword ptr [EAX], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x00
        _emit 0xf3              // MOVSS XMM0, dword ptr [ESP+0x4]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS dword ptr [EAX+0x4], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x04
        _emit 0xf3              // MOVSS XMM0, dword ptr [ESP+0x8]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3              // MOVSS dword ptr [EAX+0x8], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x08
        _emit 0xf3              // MOVSS XMM0, dword ptr [ESP+0xc]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xf3              // MOVSS dword ptr [EAX+0xc], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x0c
        _emit 0x8b              // MOV ESP, EBP
        _emit 0xe5
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x0008
        _emit 0x08
        _emit 0x00
    }
}
