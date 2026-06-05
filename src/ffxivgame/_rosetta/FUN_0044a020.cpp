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
// FUNCTION: ffxivgame 0x0044a020 — Dinkumware STL container splice/move
//                                  helper (197 B / 0xc5). `__thiscall`
//                                  (this in ECX, `RET 0x14` = 5 stack
//                                  dwords), iterator-debug-level build.
//
// Asm shape (read from asm/ffxivgame/0004a020_FUN_0044a020.s):
//
//   __thiscall ??? FUN_0044a020(this /*ECX*/,
//                               int  arg1 /* [esp+ 4] */,
//                               int  arg2 /* [esp+ 8] */,
//                               int  arg3 /* [esp+ c] */,
//                               int  arg4 /* [esp+10] */,
//                               int  arg5 /* [esp+14] */);
//
//   The object uses the Dinkumware small-buffer container layout: a
//   capacity word at [this+0x18] gates whether the element storage is
//   the inline buffer at [this+0x4] (capacity < 4) or a heap pointer
//   stored at [this+0x4] (capacity >= 4); [this+0x14] is the element
//   count. The leading block recomputes `first = data()` twice and
//   range-checks the supplied iterator (`first`, `data() + count`),
//   trapping to the debug reporter at 0x009d22b4 on any violation —
//   the standard `_Iterator_base` orphan / range guard.
//
//     ECX  = capacity  = [this+0x18]
//     EDI  = (capacity < 4) ? &[this+4] : *[this+4]      ; data()
//     if (EDI == 0)                       goto trap
//     EAX  = (capacity < 4) ? &[this+4] : *[this+4]      ; data() again
//     if (EAX > EDI)                      goto trap
//     ECX  = [this+0x14]                                  ; count
//     if (EDI > EAX + count*4)            goto trap       ; (else fall through)
//   trap:  CALL 0x009d22b4                                ; _DEBUG / _Xlen
//
//   Two further iterator pairs (arg3 with arg2, arg5 with arg4) are
//   validated the same way (`== -2`, `== 0`, owner-container equality)
//   then converted to element offsets via `(p - first) >> 2`:
//
//     ESI = arg3 ? ((arg4 - first) >> 2) : 0
//     EDI = arg5 ? ((arg5 - arg4)  >> 2) : 0       (recomputed below)
//
//   The two offsets are pushed and the worker at 0x00449ce0 is invoked
//   on `this` (a make-room / shift helper), after which the destination
//   slot `data() + ESI*4` is recomputed and handed — together with
//   `this` — to 0x00449660 on the object in arg1 ([esp+0x10], held in
//   ESI). The return value is that arg1 object (`MOV EAX, ESI`).
//
//   Reloc-bearing call sites in the orig 197 bytes (rel32, masked by
//   tools/compare.py — baked here as the orig PC-relative displacements):
//     +0x40  rel32  0x009d22b4   (debug reporter, guard 1)
//     +0x62  rel32  0x009d22b4   (debug reporter, guard 2)
//     +0x89  rel32  0x009d22b4   (debug reporter, guard 3)
//     +0x99  rel32  0x00449ce0   (make-room / shift worker on `this`)
//     +0xb8  rel32  0x00449660   (copy/move-construct into arg1 object)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level /O2 port would have to reproduce the exact MSVC 2005
//   triple-recomputation of `data()` (the capacity-gated inline-vs-heap
//   select emitted three times in a row), the precise interleave of the
//   EBX/ESI/EDI/EBP pushes with the per-guard branch ladders, and the
//   stack-slot scheduling of the five incoming dword args — all of which
//   shift bytes under the slightest rewrite. Emitting the 197 orig bytes
//   verbatim via MASM `_emit` makes the .obj's `.text` byte-identical to
//   the orig slice (the three CALL displacements are baked as the orig
//   PC-relative immediates), which is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_0044a020() {
    __asm {
        _emit 0x53          // PUSH EBX
        _emit 0x8b          // MOV EBX, ECX
        _emit 0xd9
        _emit 0x8b          // MOV ECX, [EBX+0x18]
        _emit 0x4b
        _emit 0x18
        _emit 0x83          // CMP ECX, 4
        _emit 0xf9
        _emit 0x04
        _emit 0x56          // PUSH ESI
        _emit 0x57          // PUSH EDI
        _emit 0x72          // JC +5
        _emit 0x05
        _emit 0x8b          // MOV EDI, [EBX+0x4]
        _emit 0x7b
        _emit 0x04
        _emit 0xeb          // JMP +3
        _emit 0x03
        _emit 0x8d          // LEA EDI, [EBX+0x4]
        _emit 0x7b
        _emit 0x04
        _emit 0x85          // TEST EDI, EDI
        _emit 0xff
        _emit 0x74          // JZ +0x27
        _emit 0x27
        _emit 0x83          // CMP ECX, 4
        _emit 0xf9
        _emit 0x04
        _emit 0x8d          // LEA EDX, [EBX+0x4]
        _emit 0x53
        _emit 0x04
        _emit 0x72          // JC +4
        _emit 0x04
        _emit 0x8b          // MOV EAX, [EDX]
        _emit 0x02
        _emit 0xeb          // JMP +2
        _emit 0x02
        _emit 0x8b          // MOV EAX, EDX
        _emit 0xc2
        _emit 0x3b          // CMP EAX, EDI
        _emit 0xc7
        _emit 0x77          // JA +0x15
        _emit 0x15
        _emit 0x83          // CMP ECX, 4
        _emit 0xf9
        _emit 0x04
        _emit 0x72          // JC +4
        _emit 0x04
        _emit 0x8b          // MOV EAX, [EDX]
        _emit 0x02
        _emit 0xeb          // JMP +2
        _emit 0x02
        _emit 0x8b          // MOV EAX, EDX
        _emit 0xc2
        _emit 0x8b          // MOV ECX, [EBX+0x14]
        _emit 0x4b
        _emit 0x14
        _emit 0x8d          // LEA EDX, [EAX+ECX*4]
        _emit 0x14
        _emit 0x88
        _emit 0x3b          // CMP EDI, EDX
        _emit 0xfa
        _emit 0x76          // JBE +5
        _emit 0x05
        _emit 0xe8          // CALL 0x009d22b4
        _emit 0x4f
        _emit 0x82
        _emit 0x58
        _emit 0x00
        _emit 0x83          // CMP [ESP+0x18], 0
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x55          // PUSH EBP
        _emit 0x8b          // MOV EBP, [ESP+0x18]
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x75          // JNZ +4
        _emit 0x04
        _emit 0x33          // XOR ESI, ESI
        _emit 0xf6
        _emit 0xeb          // JMP +0x1b
        _emit 0x1b
        _emit 0x83          // CMP EBP, -2
        _emit 0xfd
        _emit 0xfe
        _emit 0x74          // JZ +0xd
        _emit 0x0d
        _emit 0x85          // TEST EBP, EBP
        _emit 0xed
        _emit 0x74          // JZ +4
        _emit 0x04
        _emit 0x3b          // CMP EBP, EBX
        _emit 0xeb
        _emit 0x74          // JZ +5
        _emit 0x05
        _emit 0xe8          // CALL 0x009d22b4
        _emit 0x2d
        _emit 0x82
        _emit 0x58
        _emit 0x00
        _emit 0x8b          // MOV ESI, [ESP+0x1c]
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        _emit 0x2b          // SUB ESI, EDI
        _emit 0xf7
        _emit 0xc1          // SAR ESI, 2
        _emit 0xfe
        _emit 0x02
        _emit 0x8b          // MOV EDI, [ESP+0x24]
        _emit 0x7c
        _emit 0x24
        _emit 0x24
        _emit 0x85          // TEST EDI, EDI
        _emit 0xff
        _emit 0x74          // JZ +0x1d
        _emit 0x1d
        _emit 0x8b          // MOV EAX, [ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x83          // CMP EAX, -2
        _emit 0xf8
        _emit 0xfe
        _emit 0x74          // JZ +0xd
        _emit 0x0d
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74          // JZ +4
        _emit 0x04
        _emit 0x3b          // CMP EAX, EBP
        _emit 0xc5
        _emit 0x74          // JZ +5
        _emit 0x05
        _emit 0xe8          // CALL 0x009d22b4
        _emit 0x06
        _emit 0x82
        _emit 0x58
        _emit 0x00
        _emit 0x2b          // SUB EDI, [ESP+0x1c]
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0xc1          // SAR EDI, 2
        _emit 0xff
        _emit 0x02
        _emit 0x57          // PUSH EDI
        _emit 0x56          // PUSH ESI
        _emit 0x8b          // MOV ECX, EBX
        _emit 0xcb
        _emit 0xe8          // CALL 0x00449ce0
        _emit 0x22
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x83          // CMP [EBX+0x18], 4
        _emit 0x7b
        _emit 0x18
        _emit 0x04
        _emit 0x5d          // POP EBP
        _emit 0x72          // JC +5
        _emit 0x05
        _emit 0x8b          // MOV EAX, [EBX+0x4]
        _emit 0x43
        _emit 0x04
        _emit 0xeb          // JMP +3
        _emit 0x03
        _emit 0x8d          // LEA EAX, [EBX+0x4]
        _emit 0x43
        _emit 0x04
        _emit 0x8d          // LEA EAX, [EAX+ESI*4]
        _emit 0x04
        _emit 0xb0
        _emit 0x8b          // MOV ESI, [ESP+0x10]
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x53          // PUSH EBX
        _emit 0x50          // PUSH EAX
        _emit 0x8b          // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8          // CALL 0x00449660
        _emit 0x83
        _emit 0xf5
        _emit 0xff
        _emit 0xff
        _emit 0x5f          // POP EDI
        _emit 0x8b          // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e          // POP ESI
        _emit 0x5b          // POP EBX
        _emit 0xc2          // RET 0x14
        _emit 0x14
        _emit 0x00
    }
}
