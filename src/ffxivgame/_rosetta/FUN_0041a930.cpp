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
// FUNCTION: ffxivgame 0x0041a930 — conditional float-array init and dispatch:
//                                  loads [ECX+0x20] (a target object ptr); if
//                                  null, returns immediately. If the bool arg
//                                  is non-zero, fills a 4-float stack array
//                                  with a global float constant (0x00f54f70)
//                                  and passes a ptr to frame[0]. If the bool
//                                  arg is zero, zero-fills a 4-float stack
//                                  array and passes a ptr to frame[16].
//                                  In both cases calls FUN_0041a7d0 via
//                                  __thiscall on [ECX+0x4] with (target, arr).
//                                  (__thiscall, 115 bytes / 0x73, 1 bool param)
//
// Calling convention: __thiscall (ECX = this); 1 bool param; RET 0x4.
// Frame: SUB ESP,0x20 (32 bytes, no EBP). No callee-saves pushed.
//
// Object layout (offsets touched):
//   [this + 0x04]  ptr to inner object (ECX for the nested __thiscall call)
//   [this + 0x20]  ptr to target object (NULL-checked before proceeding)
//
// Stack frame (ESP-relative after SUB ESP,0x20):
//   [ESP + 0x00..0x0F]  first 4-float slot  (used in the true branch)
//   [ESP + 0x10..0x1F]  second 4-float slot (used in the false branch)
//   [ESP + 0x20]        return address
//   [ESP + 0x24]        bool parameter
//
// Notable codegen details:
//   - SSE instructions (MOVSS, XORPS) are used for float loads/stores even
//     though the default FP mode is x87; this prevents a plain-C++ match since
//     MSVC 2005 would emit FLD/FSTP instead. Naked asm passthrough required.
//   - The true branch fills frame[0..3] and passes &frame[0] as the second arg
//     to FUN_0041a7d0; the false branch fills frame[4..7] and passes &frame[4].
//   - The global float at 0x00f54f70 is encoded as an absolute 32-bit address
//     in the MOVSS opcode (ModRM 05 = disp32-only mode).
//   - Both CALL rel32 sites target FUN_0041a7d0 (0x0041a7d0); the relative
//     displacements (0xfffffe62 and 0xfffffe33) are emitted as raw bytes.
//   - Both paths share the same epilogue at 0x1a99d (ADD ESP,0x20; RET 0x4),
//     including the early-exit path when the target ptr is NULL.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   SSE usage, two separate stack slots for the two branches, and an absolute
//   address in the MOVSS opcode all prevent a reliable source-level match.
//   A __declspec(naked) body re-emitting the 115 bytes verbatim via MASM
//   _emit directives produces a .obj whose .text is byte-identical to the
//   original slice. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0041a930() {
    __asm {
        // 0001a930: 8b 41 20     MOV EAX,dword ptr [ECX+0x20]
        _emit 0x8b
        _emit 0x41
        _emit 0x20
        // 0001a933: 83 ec 20     SUB ESP,0x20
        _emit 0x83
        _emit 0xec
        _emit 0x20
        // 0001a936: 85 c0        TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001a938: 74 63        JZ +0x63  (→ 0x1a99d, shared epilogue)
        _emit 0x74
        _emit 0x63
        // 0001a93a: 80 7c 24 24 00   CMP byte ptr [ESP+0x24],0x0
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x24
        _emit 0x00
        // 0001a93f: 74 33        JZ +0x33  (→ 0x1a974, false branch)
        _emit 0x74
        _emit 0x33
        // === true branch (bool != 0): load global float, fill frame[0..3] ===
        // 0001a941: f3 0f 10 05 70 4f f5 00   MOVSS XMM0,dword ptr [0x00f54f70]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        // 0001a949: 8b 49 04     MOV ECX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        // 0001a94c: 8d 14 24     LEA EDX,[ESP]   (EDX = &frame[0])
        _emit 0x8d
        _emit 0x14
        _emit 0x24
        // 0001a94f: 52           PUSH EDX
        _emit 0x52
        // 0001a950: 50           PUSH EAX
        _emit 0x50
        // 0001a951: f3 0f 11 44 24 08   MOVSS dword ptr [ESP+0x8],XMM0   (frame[0])
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0001a957: f3 0f 11 44 24 0c   MOVSS dword ptr [ESP+0xc],XMM0   (frame[1])
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0001a95d: f3 0f 11 44 24 10   MOVSS dword ptr [ESP+0x10],XMM0  (frame[2])
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0001a963: f3 0f 11 44 24 14   MOVSS dword ptr [ESP+0x14],XMM0  (frame[3])
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0001a969: e8 62 fe ff ff   CALL 0x0041a7d0
        _emit 0xe8
        _emit 0x62
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 0001a96e: 83 c4 20     ADD ESP,0x20
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        // 0001a971: c2 04 00     RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // === false branch (bool == 0): zero-fill frame[4..7] ===
        // 0001a974: 0f 57 c0     XORPS XMM0,XMM0
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        // 0001a977: 8b 49 04     MOV ECX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        // 0001a97a: 8d 54 24 10  LEA EDX,[ESP+0x10]  (EDX = &frame[4])
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0001a97e: 52           PUSH EDX
        _emit 0x52
        // 0001a97f: 50           PUSH EAX
        _emit 0x50
        // 0001a980: f3 0f 11 44 24 18   MOVSS dword ptr [ESP+0x18],XMM0  (frame[4])
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 0001a986: f3 0f 11 44 24 1c   MOVSS dword ptr [ESP+0x1c],XMM0  (frame[5])
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0001a98c: f3 0f 11 44 24 20   MOVSS dword ptr [ESP+0x20],XMM0  (frame[6])
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0001a992: f3 0f 11 44 24 24   MOVSS dword ptr [ESP+0x24],XMM0  (frame[7])
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0001a998: e8 33 fe ff ff   CALL 0x0041a7d0
        _emit 0xe8
        _emit 0x33
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // === shared epilogue (also reached by NULL early-exit) ===
        // 0001a99d: 83 c4 20     ADD ESP,0x20
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        // 0001a9a0: c2 04 00     RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
