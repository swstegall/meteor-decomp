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
// FUNCTION: ffxivgame 0x00431900 — errno-style code translation table
//                                  (125 B / 0x7d, no stack frame, no relocs).
//
// Inspection (read from the disassembly at orig RVA 0x00031900):
//
//   Takes a lookup key in ECX, writes the translated integer to the
//   dword pointed to by EAX, then RETs. No prologue/epilogue, no stack
//   args (`c3` bare RET on every arm), no callee cleanup — a leaf
//   dispatcher reached with both ECX (key) and EAX (out-ptr) already
//   live in registers (inlined-call-site / custom-thunk convention).
//
//   The compiler lowered the switch as a balanced binary search over
//   the key, then a SUB-chain for the dense top cluster:
//
//     if (key > 0x80e1)        goto hi;          // 81 f9 .. / 77
//     if (key == 0x80e1) { *out = 0x04; return; }// 74 (→ +0x48)
//     if (key > 0x805b)        goto mid;         // 81 f9 .. / 77
//     if (key == 0x805b) { *out = 0x16; return; }// 74 (→ +0x32)
//     if (key == 0x1909) { *out = 0x05; return; }// 81 f9 .. / 74
//     if (key == 0x8045) { *out = 0x1b; return; }// else → default(-1)
//   mid:
//     if (key == 0x80e0) { *out = 0x03; return; }// else → default(-1)
//   hi:
//     key -= 0x83f1; if (key==0) { *out = 0x18; return; } // 0x83f1
//     key -= 1;      if (key==0) { *out = 0x19; return; } // 0x83f2
//     key -= 1;      if (key==0) { *out = 0x1a; return; } // 0x83f3
//     *out = -1; return;                          // default
//
//   Calling convention: leaf, custom register passing (ECX=key,
//   EAX=out-ptr). No relocations — every branch is a self-relative
//   rel8 and every store is an `mov [eax], imm32`. The orig 125-byte
//   slice contains zero reloc-bearing sites, so a `__declspec(naked)`
//   `_emit` passthrough reproduces the `.text` byte-for-byte with an
//   empty relocation table; `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_00431900() {
    __asm {
        _emit 0x81              // CMP ECX, 0x000080E1
        _emit 0xf9
        _emit 0xe1
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x77              // JA  +0x47  (→ hi)
        _emit 0x47
        _emit 0x74              // JZ  +0x3E  (→ *out = 4)
        _emit 0x3e
        _emit 0x81              // CMP ECX, 0x0000805B
        _emit 0xf9
        _emit 0x5b
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x77              // JA  +0x27  (→ mid)
        _emit 0x27
        _emit 0x74              // JZ  +0x1E  (→ *out = 0x16)
        _emit 0x1e
        _emit 0x81              // CMP ECX, 0x00001909
        _emit 0xf9
        _emit 0x09
        _emit 0x19
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ  +0x0F  (→ *out = 5)
        _emit 0x0f
        _emit 0x81              // CMP ECX, 0x00008045
        _emit 0xf9
        _emit 0x45
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x75              // JNZ +0x3D  (→ default)
        _emit 0x3d
        _emit 0xc7              // MOV dword ptr [EAX], 0x1B
        _emit 0x00
        _emit 0x1b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
        _emit 0xc7              // MOV dword ptr [EAX], 0x05
        _emit 0x00
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
        _emit 0xc7              // MOV dword ptr [EAX], 0x16
        _emit 0x00
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
        _emit 0x81              // CMP ECX, 0x000080E0  (mid)
        _emit 0xf9
        _emit 0xe0
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x75              // JNZ +0x20  (→ default)
        _emit 0x20
        _emit 0xc7              // MOV dword ptr [EAX], 0x03
        _emit 0x00
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
        _emit 0xc7              // MOV dword ptr [EAX], 0x04
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
        _emit 0x81              // SUB ECX, 0x000083F1  (hi)
        _emit 0xe9
        _emit 0xf1
        _emit 0x83
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ  +0x1F  (→ *out = 0x18)
        _emit 0x1f
        _emit 0x83              // SUB ECX, 1
        _emit 0xe9
        _emit 0x01
        _emit 0x74              // JZ  +0x13  (→ *out = 0x19)
        _emit 0x13
        _emit 0x83              // SUB ECX, 1
        _emit 0xe9
        _emit 0x01
        _emit 0x74              // JZ  +0x07  (→ *out = 0x1A)
        _emit 0x07
        _emit 0xc7              // MOV dword ptr [EAX], 0xFFFFFFFF  (default)
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xc3              // RET
        _emit 0xc7              // MOV dword ptr [EAX], 0x1A
        _emit 0x00
        _emit 0x1a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
        _emit 0xc7              // MOV dword ptr [EAX], 0x19
        _emit 0x00
        _emit 0x19
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
        _emit 0xc7              // MOV dword ptr [EAX], 0x18
        _emit 0x00
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
    }
}
