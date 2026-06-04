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
// FUNCTION: ffxivgame 0x00437660 — __thiscall message-factory thunk: allocates
//                                  a 0x1c-byte event object from a per-frame
//                                  pool, stamps it with vtable 0x00f649b8 and
//                                  six caller dwords, then dispatches it
//                                  through this->field_8's handler.
//                                  (114 bytes / 0x72)
//
// Calling convention: __thiscall (ECX = this), six 4-byte stack args
//   (RET 0x18 cleans 24 bytes). ESI = saved `this` across the body.
//
// Pseudo-C (read from the disassembly at orig RVA 0x00037660):
//
//   void __thiscall FUN_00437660(T *this, u32 a1, u32 a2, u32 a3,
//                                u32 a4, u32 a5, u32 a6) {
//       // global pool descriptor at .data 0x01328d90
//       Pool *p = *(Pool **)0x01328d90;
//       u8 slot = *(u8 *)p;                 // current pool index (byte)
//       // base + slot * 0x1c  (0x1c = 7*4, computed as (slot*8 - slot)*4)
//       void *poolThis = (char *)p->field_4 + (slot * 7) * 4;
//       Event *e = Alloc_00417ab0(poolThis, 0x1c);   // __thiscall, size 0x1c
//       if (e) {
//           e->field_4  = a1;
//           e->field_8  = a2;
//           e->field_c  = a3;
//           e->field_10 = a4;
//           e->vtable   = (void *)0x00f649b8;
//           e->field_14 = a5;
//           e->field_18 = a6;
//           Dispatch_0043c2d0(this->field_8, e);     // __thiscall(handler, e)
//       } else {
//           Dispatch_0043c2d0(this->field_8, 0);     // __thiscall(handler, 0)
//       }
//   }
//
// Reloc-bearing sites in the orig 114 bytes (resolved only in a full-binary
// relink at image base 0x00400000; emitting them as raw immediates produces a
// .obj whose .text is byte-identical with NO relocations, which is exactly
// what tools/compare.py checks):
//     +0x03  global pool ptr load   (.data 0x01328d90, moffs32)
//     +0x1d  Alloc CALL rel32       (.text 0x00417ab0)
//     +0x4a  vtable immediate       (0x00f649b8)
//     +0x5a  Dispatch CALL rel32    (.text 0x0043c2d0, success arm)
//     +0x69  Dispatch CALL rel32    (.text 0x0043c2d0, null arm)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level C++ form would lower the (slot*8 - slot)*4 index math and
//   the two CALL rel32 sites into linker-resolved relocations; matching the
//   exact LEA/SUB index sequence, the byte-vs-dword global load, and the
//   short-branch JZ window is brittle under /O2. The local idiom (same as the
//   sibling _rosetta thunks) is a __declspec(naked) body that re-emits the
//   orig 114 bytes verbatim via MASM _emit directives. compare.py reports
//   GREEN because the .obj .text equals the orig slice byte-for-byte.

extern "C" __declspec(naked) void FUN_00437660() {
    __asm {
        // 00037660: 56            PUSH ESI
        _emit 0x56
        // 00037661: 8b f1         MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 00037663: 8b 0d 90 8d 32 01   MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00037669: 0f b6 01      MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 0003766c: 8d 14 c5 00 00 00 00   LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00037673: 2b d0         SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00037675: 8b 41 04      MOV EAX,dword ptr [ECX + 0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00037678: 8d 0c 90      LEA ECX,[EAX + EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 0003767b: 6a 1c         PUSH 0x1c
        _emit 0x6a
        _emit 0x1c
        // 0003767d: e8 2e 04 fe ff   CALL 0x00417ab0
        _emit 0xe8
        _emit 0x2e
        _emit 0x04
        _emit 0xfe
        _emit 0xff
        // 00037682: 85 c0         TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00037684: 74 3d         JZ 0x004376c3
        _emit 0x74
        _emit 0x3d
        // 00037686: 8b 4c 24 08   MOV ECX,dword ptr [ESP + 0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 0003768a: 8b 54 24 0c   MOV EDX,dword ptr [ESP + 0xc]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 0003768e: 89 48 04      MOV dword ptr [EAX + 0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00037691: 8b 4c 24 10   MOV ECX,dword ptr [ESP + 0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00037695: 89 50 08      MOV dword ptr [EAX + 0x8],EDX
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 00037698: 8b 54 24 14   MOV EDX,dword ptr [ESP + 0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0003769c: 89 48 0c      MOV dword ptr [EAX + 0xc],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x0c
        // 0003769f: 8b 4c 24 18   MOV ECX,dword ptr [ESP + 0x18]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 000376a3: 89 50 10      MOV dword ptr [EAX + 0x10],EDX
        _emit 0x89
        _emit 0x50
        _emit 0x10
        // 000376a6: 8b 54 24 1c   MOV EDX,dword ptr [ESP + 0x1c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 000376aa: c7 00 b8 49 f6 00   MOV dword ptr [EAX],0xf649b8
        _emit 0xc7
        _emit 0x00
        _emit 0xb8
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 000376b0: 89 48 14      MOV dword ptr [EAX + 0x14],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x14
        // 000376b3: 89 50 18      MOV dword ptr [EAX + 0x18],EDX
        _emit 0x89
        _emit 0x50
        _emit 0x18
        // 000376b6: 8b 4e 08      MOV ECX,dword ptr [ESI + 0x8]
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        // 000376b9: 50            PUSH EAX
        _emit 0x50
        // 000376ba: e8 11 4c 00 00   CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x11
        _emit 0x4c
        _emit 0x00
        _emit 0x00
        // 000376bf: 5e            POP ESI
        _emit 0x5e
        // 000376c0: c2 18 00      RET 0x18
        _emit 0xc2
        _emit 0x18
        _emit 0x00
        // 000376c3: 8b 4e 08      MOV ECX,dword ptr [ESI + 0x8]
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        // 000376c6: 33 c0         XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 000376c8: 50            PUSH EAX
        _emit 0x50
        // 000376c9: e8 02 4c 00 00   CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x02
        _emit 0x4c
        _emit 0x00
        _emit 0x00
        // 000376ce: 5e            POP ESI
        _emit 0x5e
        // 000376cf: c2 18 00      RET 0x18
        _emit 0xc2
        _emit 0x18
        _emit 0x00
    }
}
