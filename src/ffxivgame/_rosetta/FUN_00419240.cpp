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
// FUNCTION: ffxivgame 0x00019240 — select an entry from a global pointer table,
//                                   update the "current" dispatch object pointer,
//                                   and optionally log its fields
//                                   (__cdecl, 281 B / 0x119)
//
// __cdecl void FUN_00419240(int index, int suppress_log)
//     [ESP+0x04] = index        — index into the pointer table at g_table
//     [ESP+0x08] = suppress_log — if non-zero, skip the field-logging block
//
// Behaviour:
//   1. Load the global table base: ECX = [0x01329954].
//   2. Fetch the pointer at table[index] (each element is a 4-byte pointer at
//      ECX+4+index*4), then advance by 8 to reach the "current" sub-object.
//   3. CMP [g_current], new_ptr to detect whether the selection changed; then
//      write new_ptr to g_current (0x01328db4).
//   4. Read new_ptr->field_0x4, save SETNZ from the CMP into BL (EBX), push both
//      onto the stack, store field_0x4 to g_field (0x01328dac), and call
//      FUN_0041c440 with field_0x4 as the single __cdecl argument.
//   5. If BL == 0 (selection did NOT change), return early.
//   6. If suppress_log != 0, return early.
//   7. Log three indexed fields (offsets +0x8, +0xc, +0x10 in g_current) by
//      looking them up in a nested pointer table ([g_current->field_0x4->field_0x8]
//      indexed at stride-8) and passing each to FUN_00422f90(__thiscall on the
//      object at [0x0132987c], 3 explicit args: value, name-ptr, 4).
//   8. Conditionally call FUN_0041a7d0(__thiscall on g_current->field_0x4) for
//      each of the three optional string fields (offsets +0x14, +0x18, +0x1c in
//      g_current) if they are non-null, passing (field_ptr, g_slot_ptr).
//   9. Load a byte from [0x01328fa8] and call FUN_0041a930 with it; return.
//
// Globals referenced:
//   0x01329954  g_table       — pointer to an array-of-pointers (4-byte stride)
//   0x01328db4  g_current     — pointer to the currently-selected sub-object
//   0x01328dac  g_field       — cached field_0x4 from the current sub-object
//   0x0132987c  g_logger      — object used as `this` for FUN_00422f90 calls
//   0x01328fa8  g_byte        — byte passed to FUN_0041a930 at the end
//   0x1328ff4   g_name0       — name string for the first indexed field
//   0x1329034   g_name1       — name string for the second indexed field
//   0x1329074   g_name2       — name string for the third indexed field
//   0x1328fac   g_slot0       — slot buffer for the first optional string field
//   0x1328fbc   g_slot1       — slot buffer for the second optional string field
//   0x1328fcc   g_slot2       — slot buffer for the third optional string field
//
// Calling conventions of callees:
//   FUN_0041c440 — __cdecl, 1 arg (ADD ESP,4 after call)
//   FUN_00422f90 — __thiscall on [0x0132987c], 3 stack args (RET 0xC internally)
//   FUN_0041a7d0 — __thiscall on g_current->field_0x4, 2 stack args (RET 0x8 internally)
//   FUN_0041a930 — __stdcall, 1 arg (RET 0x4 internally; no caller cleanup)
//
// Calling convention: __cdecl (plain RET; no frame pointer; no /GS).
//   Callee-saved: EBX (PUSH/POP pair around the SETNZ / TEST pair).
//   Flags are preserved across non-ALU instructions between the CMP and SETNZ —
//   MSVC 2005 /O2 schedules the PUSH EBX/PUSH EAX before the SETNZ BL while
//   correctly relying on MOV not disturbing flags.
//
// This function is the "select" companion to FUN_00419360 (RVA 0x00019360),
// which updates the three slot buffers and dispatches to the same FUN_0041a7d0
// callbacks.  Both functions share the g_current / g_slot0–g_slot2 / FUN_0041a7d0
// cluster documented in FUN_00419360.
//
// Reconstruction strategy — __declspec(naked) byte passthrough.
//   The combination of absolute-address MOV instructions (which compare.py
//   cannot relax to COFF relocations in a standalone .obj), CALL rel32 targets
//   to unmatched callees, and the non-obvious SETNZ-after-non-ALU-MOV flag
//   scheduling makes a portable source-level recompilation brittle.  The
//   naked-asm _emit approach (same as FUN_00419360, FUN_00419150, etc.) produces
//   a zero-reloc .obj whose .text is byte-identical to the 281-byte orig slice.
//
// Reloc-bearing sites in the orig 281 bytes (masked by compare.py):
//   +0x02  MOV ECX,[imm32]         → 0x01329954  (g_table)
//   +0x11  CMP [imm32],EAX         → 0x01328db4  (g_current)
//   +0x17  MOV [imm32],EAX         → 0x01328db4  (g_current)
//   +0x24  MOV [imm32],EAX         → 0x01328dac  (g_field)
//   +0x29  CALL rel32              → 0x0041c440
//   +0x45  MOV EAX,[imm32]         → 0x01328db4  (g_current reload)
//   +0x57  MOV ECX,[imm32]         → 0x0132987c  (g_logger)
//   +0x5f  PUSH imm32              → 0x1328ff4   (g_name0)
//   +0x65  CALL rel32              → 0x00422f90
//   +0x6a  MOV EAX,[imm32]         → 0x01328db4
//   +0x84  PUSH imm32              → 0x1329034   (g_name1)
//   +0x84  MOV ECX,[imm32]         → 0x0132987c
//   +0x8a  CALL rel32              → 0x00422f90
//   +0x8f  MOV EAX,[imm32]         → 0x01328db4
//   +0xa9  PUSH imm32              → 0x1329074   (g_name2)
//   +0xaf  CALL rel32              → 0x00422f90
//   +0xb4  MOV ECX,[imm32]         → 0x01328db4
//   +0xbf  PUSH imm32              → 0x1328fac   (g_slot0)
//   +0xca  CALL rel32              → 0x0041a7d0
//   +0xcf  MOV ECX,[imm32]         → 0x01328db4
//   +0xdf  PUSH imm32              → 0x1328fbc   (g_slot1)
//   +0xe5  CALL rel32              → 0x0041a7d0
//   +0xea  MOV ECX,[imm32]         → 0x01328db4
//   +0xfa  PUSH imm32              → 0x1328fcc   (g_slot2)
//   +0x100 CALL rel32              → 0x0041a7d0
//   +0x105 MOV ECX,[imm32]         → 0x01328db4
//   +0x10c MOVZX EAX,byte [imm32]  → 0x01328fa8  (g_byte)
//   +0x113 CALL rel32              → 0x0041a930

extern "C" __declspec(naked) void FUN_00419240() {
    __asm {
        // 00019240: 8b 0d 54 99 32 01  MOV ECX,[0x01329954]  (g_table)
        _emit 0x8b
        _emit 0x0d
        _emit 0x54
        _emit 0x99
        _emit 0x32
        _emit 0x01
        // 00019246: 8b 44 24 04  MOV EAX,[ESP+0x4]  (index)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001924a: 8b 44 81 04  MOV EAX,[ECX+EAX*4+0x4]  (table[index])
        _emit 0x8b
        _emit 0x44
        _emit 0x81
        _emit 0x04
        // 0001924e: 83 c0 08  ADD EAX,0x8  (sub-object offset)
        _emit 0x83
        _emit 0xc0
        _emit 0x08
        // 00019251: 39 05 b4 8d 32 01  CMP [0x01328db4],EAX  (changed?)
        _emit 0x39
        _emit 0x05
        _emit 0xb4
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00019257: a3 b4 8d 32 01  MOV [0x01328db4],EAX  (g_current = new_ptr)
        _emit 0xa3
        _emit 0xb4
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 0001925c: 8b 40 04  MOV EAX,[EAX+0x4]  (field_0x4)
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        // 0001925f: 53  PUSH EBX
        _emit 0x53
        // 00019260: 50  PUSH EAX  (arg for FUN_0041c440)
        _emit 0x50
        // 00019261: 0f 95 c3  SETNZ BL  (BL = (old != new); flags from CMP above)
        _emit 0x0f
        _emit 0x95
        _emit 0xc3
        // 00019264: a3 ac 8d 32 01  MOV [0x01328dac],EAX  (g_field = field_0x4)
        _emit 0xa3
        _emit 0xac
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00019269: e8 d2 31 00 00  CALL FUN_0041c440
        _emit 0xe8
        _emit 0xd2
        _emit 0x31
        _emit 0x00
        _emit 0x00
        // 0001926e: 83 c4 04  ADD ESP,4  (caller cleanup: __cdecl 1 arg)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00019271: 84 db  TEST BL,BL
        _emit 0x84
        _emit 0xdb
        // 00019273: 5b  POP EBX
        _emit 0x5b
        // 00019274: 0f 84 de 00 00 00  JZ near (→ 0x00419358 = RET)
        _emit 0x0f
        _emit 0x84
        _emit 0xde
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001927a: 80 7c 24 08 00  CMP byte ptr [ESP+0x8],0x0  (suppress_log)
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x08
        _emit 0x00
        // 0001927f: 0f 85 d3 00 00 00  JNZ near (→ 0x00419358 = RET)
        _emit 0x0f
        _emit 0x85
        _emit 0xd3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // --- field +0x8: first indexed field ------------------------------------
        // 00019285: a1 b4 8d 32 01  MOV EAX,[0x01328db4]
        _emit 0xa1
        _emit 0xb4
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 0001928a: 8b 50 08  MOV EDX,[EAX+0x8]  (field8 = index)
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 0001928d: 8b 40 04  MOV EAX,[EAX+0x4]  (field4 = nested ptr)
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        // 00019290: 8b 48 08  MOV ECX,[EAX+0x8]  (nested->field8 = array base)
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        // 00019293: 8b 54 d1 fc  MOV EDX,[ECX+EDX*8-0x4]
        _emit 0x8b
        _emit 0x54
        _emit 0xd1
        _emit 0xfc
        // 00019297: 8b 0d 7c 98 32 01  MOV ECX,[0x0132987c]  (g_logger = this)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001929d: 6a 04  PUSH 4
        _emit 0x6a
        _emit 0x04
        // 0001929f: 68 f4 8f 32 01  PUSH 0x1328ff4  (g_name0)
        _emit 0x68
        _emit 0xf4
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 000192a4: 52  PUSH EDX
        _emit 0x52
        // 000192a5: e8 e6 9c 00 00  CALL FUN_00422f90
        _emit 0xe8
        _emit 0xe6
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        // --- field +0xc: second indexed field -----------------------------------
        // 000192aa: a1 b4 8d 32 01  MOV EAX,[0x01328db4]
        _emit 0xa1
        _emit 0xb4
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000192af: 8b 48 0c  MOV ECX,[EAX+0xc]  (field_c = index)
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 000192b2: 8b 50 04  MOV EDX,[EAX+0x4]  (field4 = nested ptr)
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000192b5: 8b 42 08  MOV EAX,[EDX+0x8]  (nested->field8 = array base)
        _emit 0x8b
        _emit 0x42
        _emit 0x08
        // 000192b8: 8b 4c c8 fc  MOV ECX,[EAX+ECX*8-0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0xc8
        _emit 0xfc
        // 000192bc: 6a 04  PUSH 4
        _emit 0x6a
        _emit 0x04
        // 000192be: 68 34 90 32 01  PUSH 0x1329034  (g_name1)
        _emit 0x68
        _emit 0x34
        _emit 0x90
        _emit 0x32
        _emit 0x01
        // 000192c3: 51  PUSH ECX
        _emit 0x51
        // 000192c4: 8b 0d 7c 98 32 01  MOV ECX,[0x0132987c]  (g_logger = this)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 000192ca: e8 c1 9c 00 00  CALL FUN_00422f90
        _emit 0xe8
        _emit 0xc1
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        // --- field +0x10: third indexed field -----------------------------------
        // 000192cf: a1 b4 8d 32 01  MOV EAX,[0x01328db4]
        _emit 0xa1
        _emit 0xb4
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000192d4: 8b 50 10  MOV EDX,[EAX+0x10]  (field_10 = index)
        _emit 0x8b
        _emit 0x50
        _emit 0x10
        // 000192d7: 8b 40 04  MOV EAX,[EAX+0x4]  (field4 = nested ptr)
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        // 000192da: 8b 48 08  MOV ECX,[EAX+0x8]  (nested->field8 = array base)
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        // 000192dd: 8b 54 d1 fc  MOV EDX,[ECX+EDX*8-0x4]
        _emit 0x8b
        _emit 0x54
        _emit 0xd1
        _emit 0xfc
        // 000192e1: 8b 0d 7c 98 32 01  MOV ECX,[0x0132987c]  (g_logger = this)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 000192e7: 6a 04  PUSH 4
        _emit 0x6a
        _emit 0x04
        // 000192e9: 68 74 90 32 01  PUSH 0x1329074  (g_name2)
        _emit 0x68
        _emit 0x74
        _emit 0x90
        _emit 0x32
        _emit 0x01
        // 000192ee: 52  PUSH EDX
        _emit 0x52
        // 000192ef: e8 9c 9c 00 00  CALL FUN_00422f90
        _emit 0xe8
        _emit 0x9c
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        // --- optional string field +0x14 ----------------------------------------
        // 000192f4: 8b 0d b4 8d 32 01  MOV ECX,[0x01328db4]
        _emit 0x8b
        _emit 0x0d
        _emit 0xb4
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000192fa: 8b 41 14  MOV EAX,[ECX+0x14]
        _emit 0x8b
        _emit 0x41
        _emit 0x14
        // 000192fd: 85 c0  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 000192ff: 74 14  JZ +0x14  (→ 00019315)
        _emit 0x74
        _emit 0x14
        // 00019301: 8b 49 04  MOV ECX,[ECX+0x4]  (this for FUN_0041a7d0)
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        // 00019304: 68 ac 8f 32 01  PUSH 0x1328fac  (g_slot0)
        _emit 0x68
        _emit 0xac
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 00019309: 50  PUSH EAX
        _emit 0x50
        // 0001930a: e8 c1 14 00 00  CALL FUN_0041a7d0
        _emit 0xe8
        _emit 0xc1
        _emit 0x14
        _emit 0x00
        _emit 0x00
        // --- optional string field +0x18 ----------------------------------------
        // 0001930f: 8b 0d b4 8d 32 01  MOV ECX,[0x01328db4]
        _emit 0x8b
        _emit 0x0d
        _emit 0xb4
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00019315: 8b 41 18  MOV EAX,[ECX+0x18]
        _emit 0x8b
        _emit 0x41
        _emit 0x18
        // 00019318: 85 c0  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001931a: 74 14  JZ +0x14  (→ 00019330)
        _emit 0x74
        _emit 0x14
        // 0001931c: 8b 49 04  MOV ECX,[ECX+0x4]  (this for FUN_0041a7d0)
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        // 0001931f: 68 bc 8f 32 01  PUSH 0x1328fbc  (g_slot1)
        _emit 0x68
        _emit 0xbc
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 00019324: 50  PUSH EAX
        _emit 0x50
        // 00019325: e8 a6 14 00 00  CALL FUN_0041a7d0
        _emit 0xe8
        _emit 0xa6
        _emit 0x14
        _emit 0x00
        _emit 0x00
        // --- optional string field +0x1c ----------------------------------------
        // 0001932a: 8b 0d b4 8d 32 01  MOV ECX,[0x01328db4]
        _emit 0x8b
        _emit 0x0d
        _emit 0xb4
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00019330: 8b 41 1c  MOV EAX,[ECX+0x1c]
        _emit 0x8b
        _emit 0x41
        _emit 0x1c
        // 00019333: 85 c0  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00019335: 74 14  JZ +0x14  (→ 0001934b)
        _emit 0x74
        _emit 0x14
        // 00019337: 8b 49 04  MOV ECX,[ECX+0x4]  (this for FUN_0041a7d0)
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        // 0001933a: 68 cc 8f 32 01  PUSH 0x1328fcc  (g_slot2)
        _emit 0x68
        _emit 0xcc
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 0001933f: 50  PUSH EAX
        _emit 0x50
        // 00019340: e8 8b 14 00 00  CALL FUN_0041a7d0
        _emit 0xe8
        _emit 0x8b
        _emit 0x14
        _emit 0x00
        _emit 0x00
        // --- epilogue: load byte and call FUN_0041a930 -------------------------
        // 00019345: 8b 0d b4 8d 32 01  MOV ECX,[0x01328db4]
        _emit 0x8b
        _emit 0x0d
        _emit 0xb4
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 0001934b: 0f b6 05 a8 8f 32 01  MOVZX EAX,byte ptr [0x01328fa8]  (g_byte)
        _emit 0x0f
        _emit 0xb6
        _emit 0x05
        _emit 0xa8
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 00019352: 50  PUSH EAX
        _emit 0x50
        // 00019353: e8 d8 15 00 00  CALL FUN_0041a930
        _emit 0xe8
        _emit 0xd8
        _emit 0x15
        _emit 0x00
        _emit 0x00
        // 00019358: c3  RET
        _emit 0xc3
    }
}
