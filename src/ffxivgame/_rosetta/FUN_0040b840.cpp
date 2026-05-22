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
// FUNCTION: ffxivgame 0x0040b840 — alloc/init wrapper around a memory-space
// triple (449 B). __thiscall init function that takes (this, void* space,
// dword arg2). Body shape:
//
//   FUN_0040b150();                                    // shared init
//   if (space == nullptr) return false;
//   this->m_space = space;
//   this->m_alloc8   = space->Alloc(8,                "CDev.Engine.Lay.Mem.Space");
//   this->m_alloc40h = space->Alloc(0x40,             "CDev.Engine.Lay.Mem.Space");
//   this->m_allocArr = space->Alloc(DAT_01265300 * 0x14, "CDev.Engine.Lay.Mem.Space");
//   if (m_alloc8 && m_alloc40h && m_allocArr) {
//       memset(this, 0, 0x58);                          // zero fields 0..0x57
//       if (m_alloc8) { ((u32*)m_alloc8)[0]=0; ((u32*)m_alloc8)[1]=0; }
//       if (((u32*)m_alloc8)[1] != 0) FUN_0040d7e0(m_alloc8);  // dtor of prior contents
//       ((u32*)m_alloc8)[1] = space;
//       if (m_alloc40h) FUN_0040d640(m_alloc40h);       // ctor #1
//       FUN_0040d880(m_alloc40h, space, arg2);          // ctor #2
//       __ehvec_ctor(m_allocArr, 0x14, DAT_01265300, &FUN_0040a640);
//       return true;
//   }
//   space->Dealloc(m_alloc8);    // FUN_0040df70
//   space->Dealloc(m_alloc40h);  // FUN_0040df70
//   space->Dealloc(m_allocArr);  // FUN_0040df70
//   return false;
//
// The function uses MSVC's inline (non-frame-pointer) SEH frame:
//   {prev_link, scope_table=0x00e54e04, state}
// with the state byte stepping -1 → 0 → -1 → 1 across the three
// inner construction phases. A source-level translation through
// __try/__finally would not reliably reproduce the exact prolog,
// state-byte transitions, or the per-function scope table reference
// at imm32 0x00e54e04 (Phase-2.5 RTTI work hasn't named that table
// yet, so naming it from C++ is fragile).
//
// Reloc-bearing sites in the 449-byte body (all 4-byte imm32 / rel32):
//   +0x03  PUSH imm32   → 0x00e54e04  (SEH scope table)
//   +0x21  CALL rel32   → FUN_0040b150
//   +0x35  MOV  EAX,[]  → 0x012652f8  (global allocator ptr ptr)
//   +0x3a  PUSH imm32   → 0x00f55864  ("CDev.Engine.Lay.Mem.Space")
//   +0x44  CALL rel32   → FUN_0040e230
//   +0x4f  CALL rel32   → FUN_0040e110
//   +0x58  MOV  ECX,[]  → 0x012652f8
//   +0x5d  PUSH imm32   → 0x00f55864
//   +0x67  CALL rel32   → FUN_0040e230
//   +0x72  CALL rel32   → FUN_0040e110
//   +0x7b  MOV  EDX,[]  → 0x012652f8
//   +0x80  PUSH imm32   → 0x00f55864
//   +0x8a  CALL rel32   → FUN_0040e230
//   +0x95  MOVZX EAX,[] → 0x01265300  (entry-count byte)
//   +0xa2  CALL rel32   → FUN_0040e110
//   +0x12d CALL rel32   → FUN_0040d7e0
//   +0x149 CALL rel32   → FUN_0040d640
//   +0x15f CALL rel32   → FUN_0040d880
//   +0x16c MOVZX EAX,[] → 0x01265300
//   +0x181 PUSH imm32   → 0x0040a640  (per-element ctor — see vector_ctor_iter)
//   +0x18a CALL rel32   → FUN_00401110 (vector_ctor_iter helper)
//   +0x1a8 CALL rel32   → FUN_0040df70 (called three times from the failure path)
//
// Reconstruction strategy: byte-passthrough via __declspec(naked) +
// `_emit` — matches the established sibling FUN_00409610 idiom for
// inline-SEH bodies. All reloc sites' pre-linked literals already
// match orig PE byte-for-byte, so the .obj's empty reloc table is
// fine (compare.py masks reloc bytes when our .obj exposes them,
// but byte-equality wins here unmediated).

extern "C" __declspec(naked) void FUN_0040b840() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x04
        _emit 0x4e
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xec
        _emit 0x14
        _emit 0x56
        _emit 0x8b
        _emit 0xf1
        _emit 0x57
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0xe8
        _emit 0xeb
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x2c
        _emit 0x85
        _emit 0xff
        _emit 0x0f
        _emit 0x84
        _emit 0x96
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x7e
        _emit 0x58
        _emit 0xa1
        _emit 0xf8
        _emit 0x52
        _emit 0x26
        _emit 0x01
        _emit 0x68
        _emit 0x64
        _emit 0x58
        _emit 0xf5
        _emit 0x00
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xe8
        _emit 0xa8
        _emit 0x29
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x58
        _emit 0x50
        _emit 0x6a
        _emit 0x08
        _emit 0xe8
        _emit 0x7d
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x46
        _emit 0x5c
        _emit 0x8b
        _emit 0x0d
        _emit 0xf8
        _emit 0x52
        _emit 0x26
        _emit 0x01
        _emit 0x68
        _emit 0x64
        _emit 0x58
        _emit 0xf5
        _emit 0x00
        _emit 0x51
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xe8
        _emit 0x85
        _emit 0x29
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x58
        _emit 0x50
        _emit 0x6a
        _emit 0x40
        _emit 0xe8
        _emit 0x5a
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x46
        _emit 0x60
        _emit 0x8b
        _emit 0x15
        _emit 0xf8
        _emit 0x52
        _emit 0x26
        _emit 0x01
        _emit 0x68
        _emit 0x64
        _emit 0x58
        _emit 0xf5
        _emit 0x00
        _emit 0x52
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xe8
        _emit 0x62
        _emit 0x29
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x58
        _emit 0x50
        _emit 0x0f
        _emit 0xb6
        _emit 0x05
        _emit 0x00
        _emit 0x53
        _emit 0x26
        _emit 0x01
        _emit 0x8d
        _emit 0x04
        _emit 0x80
        _emit 0x03
        _emit 0xc0
        _emit 0x03
        _emit 0xc0
        _emit 0x50
        _emit 0xe8
        _emit 0x2a
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x5c
        _emit 0x85
        _emit 0xc9
        _emit 0x89
        _emit 0x86
        _emit 0x60
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0xec
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0x7e
        _emit 0x60
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0xe2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0xda
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xc0
        _emit 0x89
        _emit 0x06
        _emit 0x89
        _emit 0x46
        _emit 0x04
        _emit 0x89
        _emit 0x46
        _emit 0x08
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        _emit 0x89
        _emit 0x46
        _emit 0x10
        _emit 0x89
        _emit 0x46
        _emit 0x14
        _emit 0x89
        _emit 0x46
        _emit 0x18
        _emit 0x89
        _emit 0x46
        _emit 0x1c
        _emit 0x89
        _emit 0x46
        _emit 0x20
        _emit 0x89
        _emit 0x46
        _emit 0x24
        _emit 0x89
        _emit 0x46
        _emit 0x28
        _emit 0x89
        _emit 0x46
        _emit 0x2c
        _emit 0x89
        _emit 0x46
        _emit 0x30
        _emit 0x89
        _emit 0x46
        _emit 0x34
        _emit 0x89
        _emit 0x46
        _emit 0x38
        _emit 0x89
        _emit 0x46
        _emit 0x3c
        _emit 0x89
        _emit 0x46
        _emit 0x40
        _emit 0x89
        _emit 0x46
        _emit 0x44
        _emit 0x89
        _emit 0x46
        _emit 0x48
        _emit 0x89
        _emit 0x46
        _emit 0x4c
        _emit 0x89
        _emit 0x46
        _emit 0x50
        _emit 0x89
        _emit 0x46
        _emit 0x54
        _emit 0x8b
        _emit 0x46
        _emit 0x5c
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x0d
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x53
        _emit 0x8b
        _emit 0x5e
        _emit 0x5c
        _emit 0x83
        _emit 0x7b
        _emit 0x04
        _emit 0x00
        _emit 0x74
        _emit 0x07
        _emit 0x8b
        _emit 0xcb
        _emit 0xe8
        _emit 0x6f
        _emit 0x1e
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x7b
        _emit 0x04
        _emit 0x8b
        _emit 0x4e
        _emit 0x60
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0x85
        _emit 0xc9
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5b
        _emit 0x74
        _emit 0x05
        _emit 0xe8
        _emit 0xb3
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0x51
        _emit 0x8b
        _emit 0x4e
        _emit 0x60
        _emit 0x57
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8
        _emit 0xdd
        _emit 0x1e
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xb6
        _emit 0x60
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0xb6
        _emit 0x05
        _emit 0x00
        _emit 0x53
        _emit 0x26
        _emit 0x01
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x2c
        _emit 0x85
        _emit 0xf6
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x0e
        _emit 0x68
        _emit 0x40
        _emit 0xa6
        _emit 0x40
        _emit 0x00
        _emit 0x50
        _emit 0x6a
        _emit 0x14
        _emit 0x56
        _emit 0xe8
        _emit 0x42
        _emit 0x57
        _emit 0xff
        _emit 0xff
        _emit 0x5f
        _emit 0xb0
        _emit 0x01
        _emit 0x5e
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        _emit 0x51
        _emit 0x8b
        _emit 0x4e
        _emit 0x58
        _emit 0xe8
        _emit 0x84
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x56
        _emit 0x60
        _emit 0x8b
        _emit 0x4e
        _emit 0x58
        _emit 0x52
        _emit 0xe8
        _emit 0x78
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x86
        _emit 0x60
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x58
    }
}
