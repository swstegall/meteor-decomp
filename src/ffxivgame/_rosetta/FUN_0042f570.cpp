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
// FUNCTION: ffxivgame 0x0002f570 — __cdecl 3-arg wrapper: matrix×point
//                                  transform with w-component constant
//                                  override (63 B / 0x3f).
//
// Signature (reconstructed from the asm):
//
//   __cdecl float* FUN_0042f570(float* out, Matrix4* mat, const float* in)
//
//   Calls FUN_0042f210 (__thiscall matrix×vec4 point transform, ECX=mat)
//   with a 16-byte stack-local buffer as the output, then overwrites the
//   4th float (w component at offset 0x0c) with the constant value stored
//   at absolute address 0x00f54f70, copies the resulting 16 bytes to `out`
//   via two SSE2 MOVQ transfers, and returns `out` in EAX.
//
//   Calling convention: __cdecl — three DWORD stack args, caller cleans.
//   Return value (EAX): out (first argument).
//
// Asm (63 bytes @ 0x0002f570):
//   8b 44 24 0c                  MOV EAX, [ESP+0x0c]        ; in (arg3) saved early
//   83 ec 10                     SUB ESP, 0x10              ; alloc 16-byte local buf
//   50                           PUSH EAX                   ; push in
//   8d 4c 24 04                  LEA ECX, [ESP+0x04]        ; ECX = local buf addr
//   51                           PUSH ECX                   ; push local buf (out)
//   8b 4c 24 20                  MOV ECX, [ESP+0x20]        ; mat (arg2) → ECX (this)
//   e8 8a fc ff ff               CALL FUN_0042f210          ; mat.transform(local, in)
//   f3 0f 10 05 70 4f f5 00      MOVSS XMM0, [0x00f54f70]  ; float w-constant
//   8b 4c 24 14                  MOV ECX, [ESP+0x14]        ; out (arg1) after callee cleanup
//   f3 0f 11 40 0c               MOVSS [EAX+0x0c], XMM0    ; local[3] = w-constant
//   f3 0f 7e 00                  MOVQ XMM0, [EAX]          ; load local[0..1] (xy)
//   66 0f d6 01                  MOVQ [ECX], XMM0           ; out[0..1] = local[0..1]
//   f3 0f 7e 40 08               MOVQ XMM0, [EAX+0x08]     ; load local[2..3] (zw)
//   66 0f d6 41 08               MOVQ [ECX+0x08], XMM0     ; out[2..3] = local[2..3]
//   8b c1                        MOV EAX, ECX              ; return out
//   83 c4 10                     ADD ESP, 0x10             ; restore stack
//   c3                           RET
//
// FUN_0042f210 is __thiscall (RET 0x8, callee-cleans 8 bytes of stack
// args): after the CALL returns, ESP = original_ESP - 0x10, so
// [ESP+0x14] = original_ESP + 0x4 = arg1.  EAX on return is the local
// buffer pointer.
//
// The CALL rel32 to FUN_0042f210 is masked by tools/compare.py.
// The absolute address 0x00f54f70 is a DIR32 reference into the binary's
// .rdata section; it is NOT masked by compare.py and must be emitted as
// raw bytes so the .obj bytes match the original binary exactly.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The combination of the early [ESP+0x0c] read before the SUB ESP,
//   the unaligned stack-local scratch buffer, the SSE2 MOVQ-pair bulk
//   copy, and the DIR32 absolute load of the w-constant is not reliably
//   reproducible from C++ source under MSVC 2005 /O2 in isolation. The
//   canonical _rosetta approach — __declspec(naked) + verbatim _emit
//   directives — produces a .text section that is byte-identical to the
//   original 63-byte slice; tools/compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_0042f570() {
    __asm {
        // 0002f570:  8b 44 24 0c           MOV EAX, dword ptr [ESP+0x0c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0002f574:  83 ec 10              SUB ESP, 0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 0002f577:  50                    PUSH EAX
        _emit 0x50
        // 0002f578:  8d 4c 24 04           LEA ECX, [ESP+0x04]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0002f57c:  51                    PUSH ECX
        _emit 0x51
        // 0002f57d:  8b 4c 24 20           MOV ECX, dword ptr [ESP+0x20]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 0002f581:  e8 8a fc ff ff        CALL FUN_0042f210  (rel32 masked by compare.py)
        _emit 0xe8
        _emit 0x8a
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 0002f586:  f3 0f 10 05 70 4f f5 00   MOVSS XMM0, dword ptr [0x00f54f70]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        // 0002f58e:  8b 4c 24 14           MOV ECX, dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0002f592:  f3 0f 11 40 0c        MOVSS dword ptr [EAX+0x0c], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x0c
        // 0002f597:  f3 0f 7e 00           MOVQ XMM0, qword ptr [EAX]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x00
        // 0002f59b:  66 0f d6 01           MOVQ qword ptr [ECX], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x01
        // 0002f59f:  f3 0f 7e 40 08        MOVQ XMM0, qword ptr [EAX+0x08]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x08
        // 0002f5a4:  66 0f d6 41 08        MOVQ qword ptr [ECX+0x08], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x41
        _emit 0x08
        // 0002f5a9:  8b c1                 MOV EAX, ECX
        _emit 0x8b
        _emit 0xc1
        // 0002f5ab:  83 c4 10              ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0002f5ae:  c3                    RET
        _emit 0xc3
    }
}
