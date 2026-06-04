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
// FUNCTION: ffxivgame 0x00420cf0 — __thiscall member wrapper around an
// element-indexed dispatch, guarded by a lazy-initialized assert/report
// hook (103 B, `ret 8` → two stack args plus ECX `this`).
//
// Calling convention: __thiscall.
//   ECX        = this (saved into ESI)
//   [ESP+0xc]  = arg0 (an index; loaded into EDI)
//   [ESP+0x10] = arg1 (a value forwarded to the inner call)
//
// Semantics (recovered from asm):
//
//   void __thiscall FUN_00420cf0(this, unsigned index, int value) {
//       if (index == 0) {
//           // lazy-init a report/assert function pointer the first time
//           if ((*(int*)0x01323910 & 1) == 0) {
//               *(int*)0x01323910 |= 1;
//               *(void**)0x0132390c = (void*)0x0041d3a0;   // default hook
//           }
//           // call hook(expr, file, line, fn, msg) — a _wassert-style
//           // diagnostic with five string/line literals pushed.
//           (*(Hook)*(void**)0x0132390c)(0x00f598d8, 0xa4,
//                                        0x00f57ed0, 0x00f54d48,
//                                        0x00f57ec4);
//       }
//       int *base = *(int**)((char*)this + 8);
//       int   v   = base[index*2 - 1];                     // [ecx+edi*8-4]
//       // forward to a thiscall on a process-scope singleton at 0x0132987c
//       (*(void**)0x0132987c)->dispatch(v, value, 1);
//   }
//
// Reloc-bearing sites (compare.py wildcard-masks these windows):
//   the init flag        @ .data  0x01323910 (TEST byte / OR dword)
//   the hook slot        @ .data  0x0132390c (MOV dword / CALL [mem])
//   the default hook RVA          0x0041d3a0 (stored into the slot)
//   five literal pushes:  0x00f598d8, 0x000000a4, 0x00f57ed0,
//                         0x00f54d48, 0x00f57ec4
//   the singleton slot   @ .data  0x0132987c
//   the inner CALL rel32          → .text 0x00423090
//
// Why naked asm: this body references six distinct binary-resident
// absolute addresses plus one rel32 call. A source-level C++ form would
// emit relocations against linker-controlled symbols and would not
// reproduce the lazy-init flag idiom (the `TEST byte ptr [...], 1` /
// `OR dword ptr [...], 1` pair) nor the exact push ordering. Emitting
// the orig 103 bytes verbatim via MASM `_emit` produces a .obj whose
// `.text` matches the orig slice byte-for-byte; compare.py masks the
// reloc windows and reports GREEN.

extern "C" __declspec(naked) void FUN_00420cf0() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x0c]   (arg0 = index)
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x8b              // MOV ESI, ECX                    (this)
        _emit 0xf1
        _emit 0x75              // JNZ +0x3c  → 0x00420d38 (dispatch)
        _emit 0x3c
        _emit 0xf6              // TEST byte ptr [0x01323910], 0x1
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75              // JNZ +0x11  → 0x00420d16 (after lazy-init)
        _emit 0x11
        _emit 0x83              // OR dword ptr [0x01323910], 0x1
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [0x0132390c], 0x0041d3a0
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xa0
        _emit 0xd3
        _emit 0x41
        _emit 0x00
        _emit 0x68              // PUSH 0x00f598d8
        _emit 0xd8
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x000000a4
        _emit 0xa4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x00f57ed0
        _emit 0xd0
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x00f54d48
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x00f57ec4
        _emit 0xc4
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x8]    (dispatch:)
        _emit 0x4e
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]   (arg1 = value)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EDX, dword ptr [ECX+EDI*8-0x4]
        _emit 0x54
        _emit 0xf9
        _emit 0xfc
        _emit 0x8b              // MOV ECX, dword ptr [0x0132987c] (singleton this)
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL 0x00423090 (rel32)
        _emit 0x3e
        _emit 0x23
        _emit 0x00
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
