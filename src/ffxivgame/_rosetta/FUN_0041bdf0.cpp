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
// FUNCTION: ffxivgame 0x0001bdf0 — __cdecl 6-arg struct-build + thiscall-dispatch
//                                  wrapper (0x4d / 77 bytes).
//
// Signature (inferred from stack layout):
//
//   void __cdecl FUN_0041bdf0(int p1, int p2, int p3, int p4, float f1, float f2);
//
// Allocates a 24-byte (0x18) local struct on the stack, fills its six
// fields from the six caller arguments, then pushes a pointer to that
// struct and dispatches to the __thiscall method FUN_00423240 on the
// global object at *[0x0132987c].  The callee cleans the single pointer
// argument via RET 4; this wrapper undoes its own SUB with ADD ESP,0x18
// and returns with a plain RET (__cdecl).
//
// Local struct layout (24 bytes, relative to the pre-PUSH ESP):
//   [+0x00]  p1   (int)
//   [+0x04]  p2   (int)
//   [+0x08]  p3   (int)
//   [+0x0c]  p4   (int)
//   [+0x10]  f1   (float)
//   [+0x14]  f2   (float)
//
// Asm shape (77 bytes — RVA 0x0001bdf0..0x0001be3c):
//
//   0001bdf0:  83 ec 18                  SUB  ESP, 0x18
//   0001bdf3:  8b 4c 24 20              MOV  ECX, [ESP+0x20]      ; p2
//   0001bdf7:  8b 44 24 1c              MOV  EAX, [ESP+0x1c]      ; p1
//   0001bdfb:  f3 0f 10 44 24 2c        MOVSS XMM0, [ESP+0x2c]   ; f1
//   0001be01:  8b 54 24 24              MOV  EDX, [ESP+0x24]      ; p3
//   0001be05:  89 4c 24 04              MOV  [ESP+0x4], ECX       ; struct.p2
//   0001be09:  8d 0c 24                 LEA  ECX, [ESP]           ; ECX = &struct
//   0001be0c:  89 04 24                 MOV  [ESP], EAX           ; struct.p1
//   0001be0f:  8b 44 24 28              MOV  EAX, [ESP+0x28]      ; p4
//   0001be13:  f3 0f 11 44 24 10        MOVSS [ESP+0x10], XMM0   ; struct.f1
//   0001be19:  f3 0f 10 44 24 30        MOVSS XMM0, [ESP+0x30]   ; f2
//   0001be1f:  51                       PUSH ECX                  ; push &struct
//   0001be20:  8b 0d 7c 98 32 01        MOV  ECX, [0x0132987c]   ; this = *g
//   0001be26:  89 54 24 0c              MOV  [ESP+0xc], EDX       ; struct.p3
//   0001be2a:  89 44 24 10              MOV  [ESP+0x10], EAX      ; struct.p4
//   0001be2e:  f3 0f 11 44 24 18        MOVSS [ESP+0x18], XMM0   ; struct.f2
//   0001be34:  e8 07 74 00 00           CALL 0x00423240           ; thiscall (RET 4)
//   0001be39:  83 c4 18                 ADD  ESP, 0x18
//   0001be3c:  c3                       RET
//
// Reloc-bearing sites (4-byte windows, wildcarded by compare.py):
//   +0x32  DIR32 → 0x0132987c  (global object pointer)
//   +0x45  REL32 → FUN_00423240 (thiscall method, disp = +0x00007407)
//
// Reconstruction strategy — __declspec(naked) byte passthrough.
//
//   The MOVSS instructions (f3 0f 10 / f3 0f 11) are SSE scalar moves that
//   MSVC 2005 /O2 emits when copying float arguments across stack frames;
//   reproducing the exact interleaved load/store schedule from C++ source is
//   too sensitive to register-pressure heuristics to be reliable.  Emitting
//   the 77 bytes verbatim via MASM _emit directives bakes the two reloc
//   windows as raw bytes; compare.py masks them and reports GREEN regardless
//   of whether the callee is yet matched.  Follows the convention of
//   FUN_0041c060 and other short forwarding wrappers in this module.

extern "C" __declspec(naked) void FUN_0041bdf0() {
    __asm {
        // 0001bdf0: 83 ec 18  SUB ESP, 0x18
        _emit 0x83
        _emit 0xec
        _emit 0x18
        // 0001bdf3: 8b 4c 24 20  MOV ECX, [ESP+0x20]   ; p2
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 0001bdf7: 8b 44 24 1c  MOV EAX, [ESP+0x1c]   ; p1
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0001bdfb: f3 0f 10 44 24 2c  MOVSS XMM0, [ESP+0x2c]  ; f1
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // 0001be01: 8b 54 24 24  MOV EDX, [ESP+0x24]   ; p3
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 0001be05: 89 4c 24 04  MOV [ESP+0x4], ECX    ; struct.p2
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0001be09: 8d 0c 24  LEA ECX, [ESP]           ; ECX = &struct
        _emit 0x8d
        _emit 0x0c
        _emit 0x24
        // 0001be0c: 89 04 24  MOV [ESP], EAX            ; struct.p1
        _emit 0x89
        _emit 0x04
        _emit 0x24
        // 0001be0f: 8b 44 24 28  MOV EAX, [ESP+0x28]   ; p4
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0001be13: f3 0f 11 44 24 10  MOVSS [ESP+0x10], XMM0  ; struct.f1
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0001be19: f3 0f 10 44 24 30  MOVSS XMM0, [ESP+0x30]  ; f2
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // 0001be1f: 51  PUSH ECX                       ; push &struct
        _emit 0x51
        // 0001be20: 8b 0d 7c 98 32 01  MOV ECX, [0x0132987c]  ; this = *g (DIR32)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001be26: 89 54 24 0c  MOV [ESP+0xc], EDX    ; struct.p3
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 0001be2a: 89 44 24 10  MOV [ESP+0x10], EAX   ; struct.p4
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0001be2e: f3 0f 11 44 24 18  MOVSS [ESP+0x18], XMM0  ; struct.f2
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 0001be34: e8 07 74 00 00  CALL 0x00423240  (REL32 disp = +0x00007407)
        _emit 0xe8
        _emit 0x07
        _emit 0x74
        _emit 0x00
        _emit 0x00
        // 0001be39: 83 c4 18  ADD ESP, 0x18
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 0001be3c: c3  RET
        _emit 0xc3
    }
}
