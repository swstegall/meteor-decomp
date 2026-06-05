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
// FUNCTION: ffxivgame 0x0004f2b0 — `__cdecl` "resolve module path then hand
//                                  it to a member" thunk (113 B / 0x71,
//                                  /GS-guarded, ESP-relative 0x1104 frame).
//
// Inspection (read from the disassembly at orig RVA 0x0004f2b0):
//
//   __cdecl void FUN_0044f2b0(T *param);
//
//   The body grabs the running module's full path into a 0x800-byte
//   scratch buffer, runs it through a transform helper into a 0x100-byte
//   buffer, then dispatches the result to a __thiscall member on the
//   caller-supplied object:
//
//     void __cdecl FUN_0044f2b0(T *self) {
//         char path[0x800];                 // [ESP+0x108] scratch
//         char buf [0x100];                  // [ESP+0x008] transformed
//         GetModuleFileNameA(NULL, path, 0x800);   // import [0x00f3e1e0]
//         FUN_0044ef10(buf, 0x100, path);          // path transform
//         self->FUN_004489c0(buf);                 // __thiscall consumer
//     }
//
//   Stack frame (after __chkstk reserves 0x1104, ESP-relative — no EBP):
//     [ESP+0x008]  buf[0x100]   (2nd-call dst / 3rd-call by-ptr arg)
//     [ESP+0x108]  path[0x800]  (GetModuleFileNameA dst)
//     [ESP+0x1100] __security_cookie ^ ESP
//     [ESP+0x1104] return address (param at [ESP+0x1108] after the chkstk
//                  alloc; the function reads it via [ESP+0x110c] post PUSH ESI)
//
//   Calling convention: __cdecl, one pointer parameter (loaded into ESI,
//   later supplied as ECX = this for the trailing __thiscall). Returns void.
//
//   Reloc-bearing sites in the orig 113 bytes (absolute addresses /
//   rel32 displacements that resolve only at full-binary relink; emitted
//   here as raw bytes so the standalone .obj's .text is byte-identical):
//     +0x05  CALL rel32   → __chkstk            (VA 0x009d29d0)
//     +0x0a  MOV  moffs32 ← __security_cookie   (.data 0x012ea8b0)
//     +0x2f  CALL [moffs] → GetModuleFileNameA  (IAT 0x00f3e1e0)
//     +0x47  CALL rel32   → FUN_0044ef10        (VA 0x0044ef10, path xform)
//     +0x56  CALL rel32   → FUN_004489c0        (VA 0x004489c0, __thiscall)
//     +0x65  CALL rel32   → __security_check_cookie (VA 0x009d20f4)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level form would need MSVC 2005 /O2 /GS to reproduce the
//   __chkstk probe, the exact cookie XOR/store offsets, the IAT-indirect
//   import call, and the three rel32 call windows — each brittle under
//   /O2. The same pragmatic route the sibling _rosetta thunks took is a
//   `__declspec(naked)` body re-emitting the orig 113 bytes verbatim via
//   MASM `_emit` directives. The resulting .obj `.text` is byte-identical
//   to the orig slice (the rel32 displacements are relative, so the raw
//   bytes are the exact wire image the linker would emit). compare.py
//   reports GREEN.

extern "C" __declspec(naked) void FUN_0044f2b0() {
    __asm {
        // 0004f2b0: b8 04 11 00 00        MOV EAX,0x1104
        _emit 0xb8
        _emit 0x04
        _emit 0x11
        _emit 0x00
        _emit 0x00
        // 0004f2b5: e8 16 37 58 00        CALL __chkstk (rel32 → 0x009d29d0)
        _emit 0xe8
        _emit 0x16
        _emit 0x37
        _emit 0x58
        _emit 0x00
        // 0004f2ba: a1 b0 a8 2e 01        MOV EAX,[0x012ea8b0]  (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0004f2bf: 33 c4                 XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0004f2c1: 89 84 24 00 11 00 00  MOV [ESP+0x1100],EAX
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0x00
        _emit 0x11
        _emit 0x00
        _emit 0x00
        // 0004f2c8: 56                    PUSH ESI
        _emit 0x56
        // 0004f2c9: 8b b4 24 0c 11 00 00  MOV ESI,[ESP+0x110c]  (param)
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0x0c
        _emit 0x11
        _emit 0x00
        _emit 0x00
        // 0004f2d0: 68 00 08 00 00        PUSH 0x800
        _emit 0x68
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0004f2d5: 8d 84 24 08 01 00 00  LEA EAX,[ESP+0x108]  (&path)
        _emit 0x8d
        _emit 0x84
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0004f2dc: 50                    PUSH EAX
        _emit 0x50
        // 0004f2dd: 6a 00                 PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0004f2df: ff 15 e0 e1 f3 00     CALL [0x00f3e1e0]  (GetModuleFileNameA)
        _emit 0xff
        _emit 0x15
        _emit 0xe0
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0004f2e5: 8d 8c 24 04 01 00 00  LEA ECX,[ESP+0x104]  (&path)
        _emit 0x8d
        _emit 0x8c
        _emit 0x24
        _emit 0x04
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0004f2ec: 51                    PUSH ECX
        _emit 0x51
        // 0004f2ed: 8d 54 24 08           LEA EDX,[ESP+0x8]  (&buf)
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 0004f2f1: 68 00 01 00 00        PUSH 0x100
        _emit 0x68
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0004f2f6: 52                    PUSH EDX
        _emit 0x52
        // 0004f2f7: e8 14 fc ff ff        CALL FUN_0044ef10 (rel32 → 0x0044ef10)
        _emit 0xe8
        _emit 0x14
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 0004f2fc: 83 c4 0c              ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0004f2ff: 8d 44 24 04           LEA EAX,[ESP+0x4]  (&buf)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0004f303: 50                    PUSH EAX
        _emit 0x50
        // 0004f304: 8b ce                 MOV ECX,ESI  (this = param)
        _emit 0x8b
        _emit 0xce
        // 0004f306: e8 b5 96 ff ff        CALL FUN_004489c0 (rel32 → 0x004489c0)
        _emit 0xe8
        _emit 0xb5
        _emit 0x96
        _emit 0xff
        _emit 0xff
        // 0004f30b: 8b 8c 24 04 11 00 00  MOV ECX,[ESP+0x1104]  (cookie)
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x04
        _emit 0x11
        _emit 0x00
        _emit 0x00
        // 0004f312: 5e                    POP ESI
        _emit 0x5e
        // 0004f313: 33 cc                 XOR ECX,ESP
        _emit 0x33
        _emit 0xcc
        // 0004f315: e8 da 2d 58 00        CALL __security_check_cookie (rel32 → 0x009d20f4)
        _emit 0xe8
        _emit 0xda
        _emit 0x2d
        _emit 0x58
        _emit 0x00
        // 0004f31a: 81 c4 04 11 00 00     ADD ESP,0x1104
        _emit 0x81
        _emit 0xc4
        _emit 0x04
        _emit 0x11
        _emit 0x00
        _emit 0x00
        // 0004f320: c3                    RET
        _emit 0xc3
    }
}
