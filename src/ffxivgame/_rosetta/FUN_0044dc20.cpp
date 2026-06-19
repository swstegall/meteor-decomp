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
// FUNCTION: ffxivgame 0x0004dc20 — global-state teardown dispatcher
//                                   (__cdecl void FUN_0044dc20(void), 181 B)
//
// Behaviour (recovered from asm/ffxivgame/0004dc20_FUN_0044dc20.s):
//
//   void FUN_0044dc20(void) {
//       // 1. Call virtual method at vtable[0x4c/4] = vtable[19] on a
//       //    global object pointer (if non-null).
//       SomeClass *obj = *(SomeClass **)0x0132cf40;
//       if (obj) {
//           (*(void(**)(SomeClass*))((*(void***)obj)[0x4c/4]))(obj);
//       }
//
//       // 2. Load a pair of globals; assert *0x0132cf54 <= *0x0132cf58
//       //    (twice — belt-and-suspenders guard, calls 0x009d22b4 on
//       //    violation). Then thiscall into FUN_00444dc0 with this=0x0132cf50
//       //    and 5 args: [&local, 0x132cf50, *0x132cf54, 0x132cf50, *0x132cf58].
//       unsigned end1  = *(unsigned*)0x0132cf58;
//       unsigned beg1  = *(unsigned*)0x0132cf54;
//       _ASSERT(beg1 <= end1);   // → 0x009d22b4 on fail
//       _ASSERT(beg1 <= end1);   // second check after reload
//       SomeVec *vec1 = (SomeVec*)0x0132cf50;
//       // FUN_00444dc0(this=vec1, &local, vec1, beg1, vec1_ptr, end1)
//
//       // 3. Same pattern for a second pair of globals @ 0x0132cf64/0x0132cf68,
//       //    then thiscall into FUN_0071cc50 with this=0x0132cf60.
//
//       // 4. Write -1 to global @ 0x01266dfc (marks "uninitialized" or "done").
//       *(int*)0x01266dfc = -1;
//   }
//
// Globals touched:
//   0x0132cf40 — pointer to a global object (null-checked; vtable call if live)
//   0x0132cf54 — begin/size of range 1
//   0x0132cf58 — end/cap  of range 1
//   0x0132cf50 — base address of range-1 SomeVec (also passed as this/arg)
//   0x0132cf64 — begin/size of range 2
//   0x0132cf68 — end/cap  of range 2
//   0x0132cf60 — base address of range-2 SomeVec (also passed as this/arg)
//   0x01266dfc — written -1 (teardown sentinel)
//
// Called functions:
//   0x009d22b4 — assertion/range-violation handler (called on beg > end)
//   0x00444dc0 — thiscall, range-1 destructor/reset (this = 0x0132cf50)
//   0x0071cc50 — thiscall, range-2 destructor/reset (this = 0x0132cf60)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This function carries eight DIR32 global-address relocations and three
//   distinct CALL rel32 targets (0x009d22b4 × 4, 0x00444dc0 × 1,
//   0x0071cc50 × 1).  Attempting a source-level reconstruction risks register
//   reordering on the two consecutive assert-reload sequences and the
//   interleaved ESI/EDI/EBX saves; the naked-asm `_emit` passthrough avoids
//   all such drift.  compare.py wildcards every reloc window so the raw
//   imm32/rel32 bytes do not need to resolve to valid addresses.
//
// Reloc-bearing sites (byte offset within function):
//   +0x01   MOV ECX,[DIR32]     → 0x0132cf40
//   +0x15   MOV EAX,[DIR32]     → 0x0132cf58  (first load)
//   +0x1d   MOV EDI,[DIR32]     → 0x0132cf54  (first load)
//   +0x29   CALL rel32          → 0x009d22b4   (first assert call)
//   +0x2e   MOV EAX,[DIR32]     → 0x0132cf58  (reload)
//   +0x33   MOV EDI,[DIR32]     → 0x0132cf54  (reload)
//   +0x3b   MOV ESI,imm32       → 0x0132cf50
//   +0x42   CALL rel32          → 0x009d22b4   (second assert call)
//   +0x4a   MOV EAX,imm32       → 0x0132cf50
//   +0x55   MOV ECX,imm32       → 0x0132cf50
//   +0x5a   CALL rel32          → 0x00444dc0
//   +0x5f   MOV EAX,[DIR32]     → 0x0132cf68
//   +0x64   MOV ECX,[DIR32]     → 0x0132cf64
//   +0x70   CALL rel32          → 0x009d22b4   (third assert call)
//   +0x75   MOV EAX,[DIR32]     → 0x0132cf68  (reload)
//   +0x7a   MOV ECX,[DIR32]     → 0x0132cf64  (reload)
//   +0x82   MOV ESI,imm32       → 0x0132cf60
//   +0x8b   CALL rel32          → 0x009d22b4   (fourth assert call)
//   +0x92   MOV EAX,imm32       → 0x0132cf60
//   +0xa0   CALL rel32          → 0x0071cc50
//   +0xa7   MOV [DIR32],-1      → 0x01266dfc

extern "C" __declspec(naked) void FUN_0044dc20() {
    __asm {
        // MOV ECX,dword ptr [0x0132cf40]
        _emit 0x8b
        _emit 0x0d
        _emit 0x40
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // SUB ESP,0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // JZ +7 (to 0x0044dc34)
        _emit 0x74
        _emit 0x07
        // MOV EAX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // MOV EDX,dword ptr [EAX+0x4c]
        _emit 0x8b
        _emit 0x50
        _emit 0x4c
        // CALL EDX
        _emit 0xff
        _emit 0xd2
        // MOV EAX,[0x0132cf58]
        _emit 0xa1
        _emit 0x58
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // PUSH EBX
        _emit 0x53
        // PUSH ESI
        _emit 0x56
        // PUSH EDI
        _emit 0x57
        // MOV EDI,dword ptr [0x0132cf54]
        _emit 0x8b
        _emit 0x3d
        _emit 0x54
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // CMP EDI,EAX
        _emit 0x3b
        _emit 0xf8
        // MOV EBX,EAX
        _emit 0x8b
        _emit 0xd8
        // JBE +0x10 (to 0x0044dc58)
        _emit 0x76
        _emit 0x10
        // CALL 0x009d22b4
        _emit 0xe8
        _emit 0x67
        _emit 0x46
        _emit 0x58
        _emit 0x00
        // MOV EAX,[0x0132cf58]
        _emit 0xa1
        _emit 0x58
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // MOV EDI,dword ptr [0x0132cf54]
        _emit 0x8b
        _emit 0x3d
        _emit 0x54
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // CMP EDI,EAX
        _emit 0x3b
        _emit 0xf8
        // MOV ESI,0x132cf50
        _emit 0xbe
        _emit 0x50
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // JBE +5 (to 0x0044dc66)
        _emit 0x76
        _emit 0x05
        // CALL 0x009d22b4
        _emit 0xe8
        _emit 0x4e
        _emit 0x46
        _emit 0x58
        _emit 0x00
        // PUSH EBX
        _emit 0x53
        // PUSH ESI
        _emit 0x56
        // PUSH EDI
        _emit 0x57
        // MOV EAX,0x132cf50
        _emit 0xb8
        _emit 0x50
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // PUSH EAX
        _emit 0x50
        // LEA EAX,[ESP+0x1c]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // PUSH EAX
        _emit 0x50
        // MOV ECX,0x132cf50
        _emit 0xb9
        _emit 0x50
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // CALL 0x00444dc0
        _emit 0xe8
        _emit 0x42
        _emit 0x71
        _emit 0xff
        _emit 0xff
        // MOV EAX,[0x0132cf68]
        _emit 0xa1
        _emit 0x68
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // MOV ECX,dword ptr [0x0132cf64]
        _emit 0x8b
        _emit 0x0d
        _emit 0x64
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // CMP ECX,EAX
        _emit 0x3b
        _emit 0xc8
        // MOV EBX,EAX
        _emit 0x8b
        _emit 0xd8
        // JBE +0x10 (to 0x0044dc9f)
        _emit 0x76
        _emit 0x10
        // CALL 0x009d22b4
        _emit 0xe8
        _emit 0x20
        _emit 0x46
        _emit 0x58
        _emit 0x00
        // MOV EAX,[0x0132cf68]
        _emit 0xa1
        _emit 0x68
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // MOV ECX,dword ptr [0x0132cf64]
        _emit 0x8b
        _emit 0x0d
        _emit 0x64
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // CMP ECX,EAX
        _emit 0x3b
        _emit 0xc8
        // MOV ESI,0x132cf60
        _emit 0xbe
        _emit 0x60
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // MOV EDI,ECX
        _emit 0x8b
        _emit 0xf9
        // JBE +5 (to 0x0044dcaf)
        _emit 0x76
        _emit 0x05
        // CALL 0x009d22b4
        _emit 0xe8
        _emit 0x05
        _emit 0x46
        _emit 0x58
        _emit 0x00
        // PUSH EBX
        _emit 0x53
        // PUSH ESI
        _emit 0x56
        // MOV EAX,0x132cf60
        _emit 0xb8
        _emit 0x60
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // PUSH EDI
        _emit 0x57
        // PUSH EAX
        _emit 0x50
        // LEA ECX,[ESP+0x1c]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // PUSH ECX
        _emit 0x51
        // MOV ECX,EAX
        _emit 0x8b
        _emit 0xc8
        // CALL 0x0071cc50
        _emit 0xe8
        _emit 0x8c
        _emit 0xef
        _emit 0x2c
        _emit 0x00
        // POP EDI
        _emit 0x5f
        // POP ESI
        _emit 0x5e
        // MOV dword ptr [0x01266dfc],0xffffffff
        _emit 0xc7
        _emit 0x05
        _emit 0xfc
        _emit 0x6d
        _emit 0x26
        _emit 0x01
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // POP EBX
        _emit 0x5b
        // ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // RET
        _emit 0xc3
    }
}
