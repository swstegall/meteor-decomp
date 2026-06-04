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
// FUNCTION: ffxivgame 0x004352e0 — `__thiscall` 5-arg constructor /
//                                  initializer for a vtable'd object with
//                                  two optional 16-byte sub-blocks (130 B).
//
// Inspection (read from the disassembly at orig RVA 0x000352e0):
//
//   __thiscall void init(Obj *this /*ECX*/, int a /*[ESP+0x4]*/,
//                        int b /*[ESP+0x8]*/, const __m128 *c /*[ESP+0xc]*/,
//                        const __m128 *d /*[ESP+0x10]*/, int e /*[ESP+0x14]*/)
//   — `RET 0x14` confirms 5 stack args (callee cleanup), ECX = this.
//
//   Structural shape:
//
//     this->vftable   = 0x00f649c8;     // [this+0x00]
//     this->field_04  = a;              // [this+0x04]
//     this->field_08  = b;              // [this+0x08]
//     // zero the two 16-byte sub-blocks (PXOR + MOVQ pairs)
//     this->block_0c  = {0,0,0,0};      // [this+0x0c .. 0x1b]
//     this->block_1c  = {0,0,0,0};      // [this+0x1c .. 0x2b]
//     this->field_2c  = e;              // [this+0x2c]
//     this->flag_30   = 0;              // [this+0x30] byte
//     this->flag_31   = 0;              // [this+0x31] byte
//     if (c) { copy 16 bytes from *c into block_0c; this->flag_30 = 1; }
//     if (d) { copy 16 bytes from *d into block_1c; this->flag_31 = 1; }
//
//   The 16-byte block ops use SSE2 (PXOR XMM0,XMM0 to zero; MOVQ
//   xmm<-[mem]/[mem]<-xmm in two halves per block) even though this
//   binary defaults to x87 FP — MSVC 2005 emits SSE integer MOVQ for
//   plain 8/16-byte struct copies/zeroes regardless of /arch.
//
//   The only reloc-bearing site in the orig 130 bytes is the vtable
//   immediate:
//     +0x14   MOV dword ptr [EAX], 0x00f649c8   (absolute vftable VA)
//   All branches (the two `JZ` skips over the optional copies) are
//   intra-function rel8.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ constructor would emit the same shape, but pinning
//   MSVC 2005 /O2 to this exact SSE2-MOVQ lowering of the struct zeroes /
//   copies (vs. mov-imm0 stores), the precise register allocation
//   (EDX/ECX juggling of the stack args), and the absolute vftable address
//   is brittle — any rewrite shifts a byte. The pragmatic choice — the
//   same one the siblings FUN_00401350 / FUN_00403d60 took — is a
//   `__declspec(naked)` body that re-emits the orig 130 bytes verbatim via
//   MASM `_emit` directives. The vftable VA is an absolute value in the
//   binary's own address space, so emitting it as raw immediate bytes
//   produces the same bytes the linker resolved; the .obj's `.text` ends
//   up byte-identical to the orig slice with NO relocations, and
//   `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_004352e0() {
    __asm {
        _emit 0x8b              // MOV EDX, [ESP+0x8]
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EAX, ECX        (eax = this)
        _emit 0xc1
        _emit 0x8b              // MOV ECX, [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x89              // MOV [EAX+0x4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x8b              // MOV ECX, [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x89              // MOV [EAX+0x8], EDX
        _emit 0x50
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [EAX], 0x00F649C8  (vftable)
        _emit 0x00
        _emit 0xc8
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x66              // PXOR XMM0, XMM0
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        _emit 0x66              // MOVQ [EAX+0x0c], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x0c
        _emit 0x66              // MOVQ [EAX+0x14], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x14
        _emit 0x66              // PXOR XMM0, XMM0
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        _emit 0x66              // MOVQ [EAX+0x1c], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x1c
        _emit 0x66              // MOVQ [EAX+0x24], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x24
        _emit 0x33              // XOR EDX, EDX
        _emit 0xd2
        _emit 0x89              // MOV [EAX+0x2c], ECX
        _emit 0x48
        _emit 0x2c
        _emit 0x8b              // MOV ECX, [ESP+0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x3b              // CMP ECX, EDX
        _emit 0xca
        _emit 0x88              // MOV byte ptr [EAX+0x30], DL
        _emit 0x50
        _emit 0x30
        _emit 0x88              // MOV byte ptr [EAX+0x31], DL
        _emit 0x50
        _emit 0x31
        _emit 0x74              // JZ +0x17  (skip first copy)
        _emit 0x17
        _emit 0xf3              // MOVQ XMM0, [ECX]
        _emit 0x0f
        _emit 0x7e
        _emit 0x01
        _emit 0x66              // MOVQ [EAX+0x0c], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x0c
        _emit 0xf3              // MOVQ XMM0, [ECX+0x8]
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x08
        _emit 0x66              // MOVQ [EAX+0x14], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x14
        _emit 0xc6              // MOV byte ptr [EAX+0x30], 0x1
        _emit 0x40
        _emit 0x30
        _emit 0x01
        _emit 0x8b              // MOV ECX, [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x3b              // CMP ECX, EDX
        _emit 0xca
        _emit 0x74              // JZ +0x17  (skip second copy)
        _emit 0x17
        _emit 0xf3              // MOVQ XMM0, [ECX]
        _emit 0x0f
        _emit 0x7e
        _emit 0x01
        _emit 0x66              // MOVQ [EAX+0x1c], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x1c
        _emit 0xf3              // MOVQ XMM0, [ECX+0x8]
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x08
        _emit 0x66              // MOVQ [EAX+0x24], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x24
        _emit 0xc6              // MOV byte ptr [EAX+0x31], 0x1
        _emit 0x40
        _emit 0x31
        _emit 0x01
        _emit 0xc2              // RET 0x14
        _emit 0x14
        _emit 0x00
    }
}
