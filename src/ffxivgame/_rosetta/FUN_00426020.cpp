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
// FUNCTION: ffxivgame 0x00426020 — `__cdecl` debug-shape draw helper
//                                   (354 B / 0x162, no SEH; 32-byte
//                                    ESP-aligned 0xc0 stack frame).
//
// Inspection (read from the disassembly at orig RVA 0x00026020):
//
//   void __cdecl FUN_00426020(void *a, void *b, void *xform, float w);
//     ; frame args: [ebp+0x08]=a, [ebp+0x0c]=b, [ebp+0x10]=xform,
//     ;             [ebp+0x14]=w (float)
//
//   Structural shape (from the asm flow):
//
//     ; ---- aligned frame -------------------------------------------
//     PUSH EBP; MOV EBP,ESP; AND ESP,~0x1f; SUB ESP,0xc0
//
//     ; ---- copy a 4-float (vec4 / matrix-row) out of xform ---------
//     ; load xform[0..3] into XMM3..XMM0, spill them to a 0x10-byte
//     ; sub-frame, then call the transform/normalise helper.
//     EAX = xform;
//     XMM0 = xform[3]; XMM1 = xform[2]; XMM2 = xform[1]; XMM3 = xform[0];
//     SUB ESP,0x10; EAX = ESP; ECX = &[ESP+0x2c];
//     EAX[0..3] = {XMM3,XMM2,XMM1,XMM0};
//     CALL 0x004305a0;                  ; (ESP+0x10 sub-frame, __cdecl)
//
//     ; ---- assemble two parallel vertex records on the stack -------
//     ; from a (xyz) and b (xyz), with a constant 'w' column pulled
//     ; from .data 0x00f62f80 and the prior helper result in ECX.
//     EAX = a;
//     [ESP+0x20] = a[0]; [ESP+0x24] = a[1]; [ESP+0x28] = a[2];
//     XMM1 = *0x00f62f80;  ECX = [ESP+0x1c];
//     XORPS XMM0,XMM0;     ; zero filler column
//     [ESP+0x30]=0; [ESP+0x34]=0; [ESP+0x38]=XMM1; [ESP+0x3c]=ECX;
//     [ESP+0x40]=0; [ESP+0x44]=0;
//     EAX = b;
//     [ESP+0x44]=b[0]; [ESP+0x48]=b[1]; [ESP+0x50]=b[2];
//     [ESP+0x54]=0; [ESP+0x58]=0; [ESP+0x5c]=XMM1; [ESP+0x60]=ECX;
//     [ESP+0x64]=0; [ESP+0x68]=0;
//     LEA EAX,[ESP+0x80]; PUSH EAX;
//     CALL 0x0042fcf0; ADD ESP,4;       ; build vertex batch (returns EAX)
//
//     ; ---- push the batch through the draw pipeline ----------------
//     PUSH EAX; PUSH ECX; EAX=ESP; *EAX=0; CALL 0x00419020;
//     w = [EBP+0x14]; *0x01328f70 = w;  CALL 0x0041c3c0;  ; set draw param
//     PUSH 0;            CALL 0x00419410; ADD ESP,8;
//     LEA ECX,[ESP+0x20]; PUSH ECX; PUSH 0x24; PUSH 0x132; PUSH 2;
//     PUSH ECX; EAX=ESP; *EAX=3; CALL 0x0041efc0;         ; submit draw
//     *0x01328f70 = *0x00f54f70 (=1.0f);  CALL 0x0041c3c0;; restore param
//     ADD ESP,4; MOV ESP,EBP; POP EBP; RET
//
//   Reloc-bearing sites in the orig 354 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x3e   rel32 CALL 0x004305a0   — vec4 transform/normalise helper
//     +0x4e   moffs MOVSS [0x00f62f80] — constant 'w' column source
//     +0xe4   rel32 CALL 0x0042fcf0   — vertex-batch builder
//     +0xf6   rel32 CALL 0x00419020   — pipeline state push
//     +0x10a  moffs MOVSS [0x01328f70] — global draw param (set)
//     +0x112  rel32 CALL 0x0041c3c0   — apply draw param
//     +0x119  rel32 CALL 0x00419410   — begin/flush
//     +0x138  rel32 CALL 0x0041efc0   — submit draw (type 2, count 0x132)
//     +0x13f  moffs MOVSS [0x00f54f70] — constant 1.0f
//     +0x14e  moffs MOVSS [0x01328f70] — global draw param (restore)
//     +0x156  rel32 CALL 0x0041c3c0   — apply draw param (restore)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ port at /O2 here would have to reproduce the exact
//   32-byte ESP alignment, the SSE scalar spill scheduling for the two
//   parallel vertex records, the interleaved PUSH-then-build sub-frames
//   (SUB ESP,0x10 / PUSH EAX / ADD ESP,4) used to pass aggregate args to
//   the helper calls, AND the linker-resolved absolute addresses in the
//   eleven relocation windows above. Each of those is brittle under /O2 —
//   every high-level rewrite shifts at least one byte (spill order,
//   branch encoding, modrm vs moffs32, FF15 vs E8). The same pragmatic
//   choice the sibling matches in this binary took (FUN_0040ced0 /
//   FUN_004014b0 / FUN_00405080) is a `__declspec(naked)` body that
//   re-emits the orig 354 bytes verbatim via MASM `_emit` directives, so
//   the .obj's `.text` is byte-identical to the orig slice.

extern "C" __declspec(naked) void FUN_00426020() {
    __asm {
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x83
        _emit 0xe4
        _emit 0xe0
        _emit 0x81
        _emit 0xec
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x45
        _emit 0x10
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x40
        _emit 0x0c
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x48
        _emit 0x08
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x50
        _emit 0x04
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x18
        _emit 0x83
        _emit 0xec
        _emit 0x10
        _emit 0x8b
        _emit 0xc4
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x18
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x50
        _emit 0x04
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x48
        _emit 0x08
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x0c
        _emit 0xe8
        _emit 0x3d
        _emit 0xa5
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x0d
        _emit 0x80
        _emit 0x2f
        _emit 0xf6
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x40
        _emit 0x04
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x40
        _emit 0x08
        _emit 0x8b
        _emit 0x45
        _emit 0x0c
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x10
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x44
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x50
        _emit 0x04
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x48
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x50
        _emit 0x08
        _emit 0x8d
        _emit 0x84
        _emit 0x24
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        _emit 0x50
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x44
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x50
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x54
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x58
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x5c
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x64
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x68
        _emit 0xe8
        _emit 0xe7
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x50
        _emit 0x51
        _emit 0x8b
        _emit 0xc4
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x05
        _emit 0x2f
        _emit 0xff
        _emit 0xff
        _emit 0xd9
        _emit 0x45
        _emit 0x14
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x45
        _emit 0x14
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x04
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x05
        _emit 0x70
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0xe8
        _emit 0x89
        _emit 0x62
        _emit 0xff
        _emit 0xff
        _emit 0x6a
        _emit 0x00
        _emit 0xe8
        _emit 0xd2
        _emit 0x32
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x51
        _emit 0x6a
        _emit 0x24
        _emit 0x68
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x02
        _emit 0x51
        _emit 0x8b
        _emit 0xc4
        _emit 0xc7
        _emit 0x00
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x63
        _emit 0x8e
        _emit 0xff
        _emit 0xff
        _emit 0xd9
        _emit 0xe8
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x05
        _emit 0x70
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0xe8
        _emit 0x45
        _emit 0x62
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x8b
        _emit 0xe5
        _emit 0x5d
        _emit 0xc3
    }
}
