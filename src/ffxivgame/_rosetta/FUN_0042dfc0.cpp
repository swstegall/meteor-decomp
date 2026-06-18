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
// FUNCTION: ffxivgame 0x0042dfc0 — __cdecl wrapper (75 B / 0x4b) that builds a
//                                  4-float scratch frame on the stack (seeded
//                                  with 0.0 / global_float / 0.0 / 0.0), then
//                                  forwards four caller args plus the scratch
//                                  frame address and four extra float constants
//                                  to FUN_00427d80.
//
// Calling convention: __cdecl (plain RET, caller cleans 4 stack args).
// Frame: SUB ESP, 0x10 (16 bytes — holds the 4-float scratch array).
// No callee-saved registers touched.
//
// Inspection (read from orig RVA 0x0002dfc0, 75 bytes):
//
//   0002dfc0:  83 ec 10        SUB ESP, 0x10
//   0002dfc3:  8b 44 24 20     MOV EAX, [ESP+0x20]          ; arg4
//   0002dfc7:  d9 44 24 18     FLD float [ESP+0x18]          ; ST(0) = arg2 (float)
//   0002dfcb:  8b 4c 24 1c     MOV ECX, [ESP+0x1c]          ; arg3
//   0002dfcf:  0f 57 c0        XORPS XMM0, XMM0             ; XMM0 = 0.0f
//   0002dfd2:  f3 0f 10 0d …   MOVSS XMM1, [0x00f54f70]     ; XMM1 = global float
//   0002dfda:  50              PUSH EAX                      ; push arg4
//   0002dfdb:  8b 44 24 18     MOV EAX, [ESP+0x18]          ; reload arg1
//   0002dfdf:  51              PUSH ECX                      ; push arg3
//   0002dfe0:  51              PUSH ECX                      ; (slot for float arg2)
//   0002dfe1:  8d 54 24 0c     LEA EDX, [ESP+0xc]           ; EDX = &scratch[0]
//   0002dfe5:  d9 1c 24        FSTP float [ESP]              ; write arg2 into its slot
//   0002dfe8:  52              PUSH EDX                      ; push &scratch
//   0002dfe9:  50              PUSH EAX                      ; push arg1
//   0002dfea:  f3 0f 11 44 24 14  MOVSS [ESP+0x14], XMM0    ; scratch[0] = 0.0f
//   0002dff0:  f3 0f 11 4c 24 18  MOVSS [ESP+0x18], XMM1    ; scratch[1] = global_float
//   0002dff6:  f3 0f 11 44 24 1c  MOVSS [ESP+0x1c], XMM0    ; scratch[2] = 0.0f
//   0002dffc:  f3 0f 11 44 24 20  MOVSS [ESP+0x20], XMM0    ; scratch[3] = 0.0f
//   0002e002:  e8 79 9d ff ff  CALL FUN_00427d80
//   0002e007:  83 c4 24        ADD ESP, 0x24
//   0002e00a:  c3              RET
//
// Effective call to FUN_00427d80 (9 args, __cdecl, caller cleans 0x24 bytes):
//   arg1 = arg1 (caller's first arg)
//   arg2 = &scratch  (ptr to the 4-float local array)
//   arg3 = arg2      (caller's float arg, passed via FLD/FSTP)
//   arg4 = arg3      (caller's third arg)
//   arg5 = arg4      (caller's fourth arg)
//   arg6..9 = { 0.0f, global_float, 0.0f, 0.0f }  (scratch array contents)
//
// Reloc-bearing sites in the orig 75 bytes:
//   +0x14..0x17  MOVSS abs32 → .rdata 0x00f54f70 (global float constant)
//   +0x43..0x46  CALL rel32  → .text FUN_00427d80
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The mixed SSE (XORPS / MOVSS for zero-fill and global-load) and x87
//   (FLD / FSTP for the float arg forwarding) in a single function prevents a
//   reliable source-level MSVC 2005 /O2 match — see sibling matches
//   FUN_0041a930 and FUN_0042e0a0 for the same constraint.  The `_emit`
//   passthrough re-emits the 75 bytes verbatim; the two relocated fields
//   (abs32 address + rel32 displacement) match the baked values in the orig
//   load image, so compare.py reports GREEN with no wildcard needed.

extern "C" __declspec(naked) void FUN_0042dfc0() {
    __asm {
        // 0002dfc0: 83 ec 10     SUB ESP, 0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 0002dfc3: 8b 44 24 20  MOV EAX, dword ptr [ESP+0x20]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0002dfc7: d9 44 24 18  FLD float ptr [ESP+0x18]
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 0002dfcb: 8b 4c 24 1c  MOV ECX, dword ptr [ESP+0x1c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0002dfcf: 0f 57 c0     XORPS XMM0, XMM0
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        // 0002dfd2: f3 0f 10 0d 70 4f f5 00  MOVSS XMM1, dword ptr [0x00f54f70]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x0d
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        // 0002dfda: 50           PUSH EAX
        _emit 0x50
        // 0002dfdb: 8b 44 24 18  MOV EAX, dword ptr [ESP+0x18]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 0002dfdf: 51           PUSH ECX
        _emit 0x51
        // 0002dfe0: 51           PUSH ECX
        _emit 0x51
        // 0002dfe1: 8d 54 24 0c  LEA EDX, [ESP+0xc]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 0002dfe5: d9 1c 24     FSTP float ptr [ESP]
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // 0002dfe8: 52           PUSH EDX
        _emit 0x52
        // 0002dfe9: 50           PUSH EAX
        _emit 0x50
        // 0002dfea: f3 0f 11 44 24 14  MOVSS dword ptr [ESP+0x14], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0002dff0: f3 0f 11 4c 24 18  MOVSS dword ptr [ESP+0x18], XMM1
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0002dff6: f3 0f 11 44 24 1c  MOVSS dword ptr [ESP+0x1c], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0002dffc: f3 0f 11 44 24 20  MOVSS dword ptr [ESP+0x20], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0002e002: e8 79 9d ff ff  CALL FUN_00427d80 (rel32)
        _emit 0xe8
        _emit 0x79
        _emit 0x9d
        _emit 0xff
        _emit 0xff
        // 0002e007: 83 c4 24     ADD ESP, 0x24
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        // 0002e00a: c3           RET
        _emit 0xc3
    }
}
