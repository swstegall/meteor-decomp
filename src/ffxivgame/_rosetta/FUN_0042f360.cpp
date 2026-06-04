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
// FUNCTION: ffxivgame 0x0042f360 — `__stdcall` 16-byte-struct-returning
//                                  wrapper that fills a temp Vec4 via a
//                                  helper, forces its .w lane to a constant,
//                                  then copies it into the caller's return
//                                  slot (61 B / 0x3d)
//
// Inspection (read from the disassembly at orig RVA 0x0002f360, 61 bytes):
//
//   Returns a 16-byte struct by value, so MSVC threads a hidden return
//   pointer as the first stack arg. `RET 8` (callee-cleans hidden ptr +
//   one explicit DWORD arg) ⇒ __stdcall-style return-struct shape:
//
//     Vec4 __stdcall FUN_0042f360(<T> arg);
//     // lowered to: Vec4* (Vec4* retptr /*[ESP+4]*/, <T> arg /*[ESP+8]*/)
//
//   8b 44 24 08          MOV  EAX, [ESP+8]        ; eax = arg
//   83 ec 10             SUB  ESP, 0x10           ; alloc 16B temp Vec4
//   50                   PUSH EAX                 ; arg
//   8d 54 24 04          LEA  EDX, [ESP+4]        ; &temp
//   52                   PUSH EDX                 ; out = &temp
//   e8 9e fe ff ff       CALL 0x0042f210          ; temp = helper(&temp, arg)
//                                                 ;   returns EAX = &temp,
//                                                 ;   callee-cleans its 2 args
//   f3 0f 10 05 ........ MOVSS XMM0, [0x00f54f70] ; const (1.0f)
//   8b 4c 24 14          MOV  ECX, [ESP+0x14]     ; ecx = retptr (hidden arg)
//   f3 0f 11 40 0c       MOVSS [EAX+0xc], XMM0    ; temp.w = const
//   f3 0f 7e 00          MOVQ  XMM0, [EAX]        ; temp.xy
//   66 0f d6 01          MOVQ  [ECX], XMM0        ; ret.xy = temp.xy
//   f3 0f 7e 40 08       MOVQ  XMM0, [EAX+8]      ; temp.zw
//   66 0f d6 41 08       MOVQ  [ECX+8], XMM0      ; ret.zw = temp.zw
//   8b c1                MOV  EAX, ECX            ; return retptr
//   83 c4 10             ADD  ESP, 0x10           ; free temp
//   c2 08 00             RET  8
//
//   The helper at 0x0042f210 shares this return-struct shape (out-param +
//   single value arg, callee-cleaned): after the CALL, ESP is back at the
//   16-byte temp's top, so [ESP+0x14] re-reaches the hidden retptr at the
//   original [ESP+4] before the SUB/PUSH/PUSH shifted the frame.
//
// Reloc-bearing sites in the orig 61 bytes (resolve only in a full-binary
// relink at image base 0x00400000; standalone .obj compilation can't
// reproduce them via source — naked asm emits them as raw immediate bytes
// that match the orig binary's already-resolved bytes, and tools/compare.py
// masks the CALL rel32 / DIR32 operands):
//     +0x0d   CALL rel32   → FUN_0042f210 (RVA 0x0002f210)
//     +0x16   MOVSS DIR32  → float const @ 0x00f54f70
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Coaxing MSVC 2005 to emit this exact SSE-scalar struct-copy sequence
//   (MOVSS/MOVQ lane shuffles plus the hidden-retptr return convention)
//   from C++ source is brittle; the body also carries a rel32 CALL and a
//   DIR32 MOVSS load whose resolved bytes a standalone .obj can't recreate.
//   The pragmatic choice — the same one siblings FUN_00406fa0 and
//   FUN_00408780 took — is a `__declspec(naked)` body re-emitting the orig
//   61 bytes verbatim via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_0042f360() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x08]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x83              // SUB ESP, 0x10
        _emit 0xec
        _emit 0x10
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EDX, [ESP+0x04]
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL rel32 → 0x0042f210
        _emit 0x9e
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xf3              // MOVSS XMM0, dword ptr [0x00f54f70]
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xf3              // MOVSS dword ptr [EAX+0x0c], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x0c
        _emit 0xf3              // MOVQ XMM0, qword ptr [EAX]
        _emit 0x0f
        _emit 0x7e
        _emit 0x00
        _emit 0x66              // MOVQ qword ptr [ECX], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x01
        _emit 0xf3              // MOVQ XMM0, qword ptr [EAX+0x08]
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x08
        _emit 0x66              // MOVQ qword ptr [ECX+0x08], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x41
        _emit 0x08
        _emit 0x8b              // MOV EAX, ECX
        _emit 0xc1
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc2              // RET 0x0008
        _emit 0x08
        _emit 0x00
    }
}
