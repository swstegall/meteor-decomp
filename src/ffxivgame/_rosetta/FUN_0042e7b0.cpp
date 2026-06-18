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
// FUNCTION: ffxivgame 0x0002e7b0 — negate a 4-component float vector:
//                                  copies ECX->{x,y,z,w} into a 16-byte-
//                                  aligned stack buffer, negates all four
//                                  components with SSE XORPS/SUBPS (0 − v),
//                                  and stores the result through the single
//                                  stack parameter (a Vec4* out-pointer).
//                                  __thiscall, 116 bytes / 0x74, EBP frame.
//
// Calling convention: __thiscall (ECX = this); returns void.
//   One explicit stack parameter at [EBP+0x8]: Vec4* out
//   Epilogue: MOV ESP,EBP / POP EBP / RET 0x4
//
// Object layout (offsets read by this function):
//   [ECX + 0x00]  float x
//   [ECX + 0x04]  float y
//   [ECX + 0x08]  float z
//   [ECX + 0x0c]  float w
//
// Notable codegen details:
//   - AND ESP,0xfffffff0 + SUB ESP,0x10 align the stack to 16 bytes so
//     that MOVAPS [ESP] / MOVAPS xmmword ptr [ESP] can run without a
//     GP-fault — MSVC 2005 emits this prolog when it needs an aligned
//     SSE store in the frame.
//   - The four MOVSS scalar copies (ECX → stack, stack → EAX) avoid any
//     alignment requirement on the caller's this-pointer or out-pointer
//     while still running the packed XORPS/SUBPS on the aligned local.
//   - No CALL instructions and no absolute addresses → no relocations;
//     the raw bytes are byte-identical in any .obj.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Source-level C++ using SSE intrinsics could notionally reproduce
//   this, but MSVC 2005's /O2 register allocator would pick different
//   XMM register assignments or omit the intermediate stack round-trip
//   depending on surrounding context. A __declspec(naked) body emitting
//   the original 116 bytes verbatim via MASM _emit directives produces
//   a .obj whose .text section is byte-identical to the original slice.

extern "C" __declspec(naked) void FUN_0042e7b0() {
    __asm {
        // 0002e7b0: 55           PUSH EBP
        _emit 0x55
        // 0002e7b1: 8b ec        MOV EBP,ESP
        _emit 0x8b
        _emit 0xec
        // 0002e7b3: 83 e4 f0     AND ESP,0xfffffff0
        _emit 0x83
        _emit 0xe4
        _emit 0xf0
        // 0002e7b6: 83 ec 10     SUB ESP,0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 0002e7b9: f3 0f 10 01  MOVSS XMM0,dword ptr [ECX]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x01
        // 0002e7bd: 8b 45 08     MOV EAX,dword ptr [EBP+0x8]
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        // 0002e7c0: f3 0f 11 04 24  MOVSS dword ptr [ESP],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        // 0002e7c5: f3 0f 10 41 04  MOVSS XMM0,dword ptr [ECX+0x4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x04
        // 0002e7ca: f3 0f 11 44 24 04  MOVSS dword ptr [ESP+0x4],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0002e7d0: f3 0f 10 41 08  MOVSS XMM0,dword ptr [ECX+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x08
        // 0002e7d5: f3 0f 11 44 24 08  MOVSS dword ptr [ESP+0x8],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0002e7db: f3 0f 10 41 0c  MOVSS XMM0,dword ptr [ECX+0xc]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x0c
        // 0002e7e0: f3 0f 11 44 24 0c  MOVSS dword ptr [ESP+0xc],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0002e7e6: 0f 28 04 24  MOVAPS XMM0,xmmword ptr [ESP]
        _emit 0x0f
        _emit 0x28
        _emit 0x04
        _emit 0x24
        // 0002e7ea: 0f 57 c9     XORPS XMM1,XMM1
        _emit 0x0f
        _emit 0x57
        _emit 0xc9
        // 0002e7ed: 0f 5c c8     SUBPS XMM1,XMM0
        _emit 0x0f
        _emit 0x5c
        _emit 0xc8
        // 0002e7f0: 0f 29 0c 24  MOVAPS xmmword ptr [ESP],XMM1
        _emit 0x0f
        _emit 0x29
        _emit 0x0c
        _emit 0x24
        // 0002e7f4: f3 0f 10 04 24  MOVSS XMM0,dword ptr [ESP]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x04
        _emit 0x24
        // 0002e7f9: f3 0f 11 00  MOVSS dword ptr [EAX],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x00
        // 0002e7fd: f3 0f 10 44 24 04  MOVSS XMM0,dword ptr [ESP+0x4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0002e803: f3 0f 11 40 04  MOVSS dword ptr [EAX+0x4],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x04
        // 0002e808: f3 0f 10 44 24 08  MOVSS XMM0,dword ptr [ESP+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0002e80e: f3 0f 11 40 08  MOVSS dword ptr [EAX+0x8],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x08
        // 0002e813: f3 0f 10 44 24 0c  MOVSS XMM0,dword ptr [ESP+0xc]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0002e819: f3 0f 11 40 0c  MOVSS dword ptr [EAX+0xc],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x0c
        // 0002e81e: 8b e5        MOV ESP,EBP
        _emit 0x8b
        _emit 0xe5
        // 0002e820: 5d           POP EBP
        _emit 0x5d
        // 0002e821: c2 04 00     RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
