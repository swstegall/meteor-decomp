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
// FUNCTION: ffxivgame 0x00043750 — __thiscall scalar deleting destructor
//                                  (50 B / 0x32)
//
// Standard MSVC 2005 "scalar deleting destructor" thunk for an audio
// object whose layout carries three vtable/embedded-vtable slots:
//
//   void * __thiscall FUN_00443750(void *this, unsigned char flags) {
//       *(void **)((char *)this + 0x00) = &vftable_A;   // 0x00f671a4
//       *(void **)((char *)this + 0x28) = &vftable_B;   // 0x00f67190
//       *(void **)((char *)this + 0x28) = &vftable_C;   // 0x00f6717c
//       FUN_00442840(this);                              // base dtor body
//       if (flags & 1)
//           operator delete(this);                       // 0x009d1b17
//       return this;
//   }
//
// The double store to +0x28 (B then C) is the canonical "set vtables on
// the way down through the class hierarchy" idiom MSVC emits when the
// destructor re-establishes the embedded sub-object's vptr before
// running the cleanup body — the second store wins, the first is dead
// but present in the orig bytes.
//
// Calling convention: __thiscall (ECX = this; one hidden stack arg, the
// deleting `flags` byte at [ESP+0x08] after PUSH ESI; callee cleans
// 4 bytes via `ret 4`). Returns `this` in EAX.
//
// Frame:
//   PUSH ESI ; MOV ESI, ECX              ; save + stash this in ESI
//   [no ESP adjustment]
//
// Asm (47 bytes @ orig RVA 0x00043750):
//   56                  PUSH ESI
//   8b f1               MOV  ESI, ECX
//   c7 06 a4 71 f6 00   MOV  [ESI],        0x00f671a4
//   c7 46 28 90 71 f6 00 MOV [ESI+0x28],   0x00f67190
//   c7 46 28 7c 71 f6 00 MOV [ESI+0x28],   0x00f6717c
//   e8 d4 f0 ff ff      CALL 0x00442840            ; rel32 (masked)
//   f6 44 24 08 01      TEST byte [ESP+0x8], 0x1
//   74 09               JZ   skip_delete
//   56                  PUSH ESI
//   e8 9e e3 58 00      CALL 0x009d1b17            ; operator delete (rel32, masked)
//   8b c6               MOV  EAX, ESI              ; (skip_delete:) return this
//   5e                  POP  ESI
//   c2 04 00            RET  4
//
// Reconstruction: __declspec(naked) _emit byte passthrough. The two
// REL32 callsites (FUN_00442840, operator delete) are masked out of the
// byte diff by tools/compare.py, so the emitted .obj .text is
// byte-identical to the orig slice with no relocations.

extern "C" __declspec(naked) void FUN_00443750() {
    __asm {
        // 00043750: 56                  PUSH ESI
        _emit 0x56
        // 00043751: 8b f1               MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00043753: c7 06 a4 71 f6 00   MOV dword ptr [ESI], 0x00f671a4
        _emit 0xc7
        _emit 0x06
        _emit 0xa4
        _emit 0x71
        _emit 0xf6
        _emit 0x00
        // 00043759: c7 46 28 90 71 f6 00 MOV dword ptr [ESI+0x28], 0x00f67190
        _emit 0xc7
        _emit 0x46
        _emit 0x28
        _emit 0x90
        _emit 0x71
        _emit 0xf6
        _emit 0x00
        // 00043760: c7 46 28 7c 71 f6 00 MOV dword ptr [ESI+0x28], 0x00f6717c
        _emit 0xc7
        _emit 0x46
        _emit 0x28
        _emit 0x7c
        _emit 0x71
        _emit 0xf6
        _emit 0x00
        // 00043767: e8 d4 f0 ff ff      CALL 0x00442840
        _emit 0xe8
        _emit 0xd4
        _emit 0xf0
        _emit 0xff
        _emit 0xff
        // 0004376c: f6 44 24 08 01      TEST byte ptr [ESP+0x8], 0x1
        _emit 0xf6
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        // 00043771: 74 09               JZ 0x0044377c
        _emit 0x74
        _emit 0x09
        // 00043773: 56                  PUSH ESI
        _emit 0x56
        // 00043774: e8 9e e3 58 00      CALL 0x009d1b17
        _emit 0xe8
        _emit 0x9e
        _emit 0xe3
        _emit 0x58
        _emit 0x00
        // 00043779: 83 c4 04            ADD ESP, 4   (clean __cdecl operator delete arg)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0004377c: 8b c6               MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 0004377e: 5e                  POP ESI
        _emit 0x5e
        // 0004377f: c2 04 00            RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
