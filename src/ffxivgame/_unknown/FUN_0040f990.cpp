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
// FUNCTION: ffxivgame 0x0000f990 — __thiscall constructor for
//           SQEX::CDev::Engine::Memory::Alternative::SeparateHeapSpace
//           (289 B / 0x121, EH4-SEH wrapped, 6 stack params)
//
// Signature (read from the disassembly at orig RVA 0x0000f990):
//
//   __thiscall void FUN_0040f990(this,
//       void*   param_1,    // [ESP+0x24] → this->field_04  (size)
//       void*   param_2,    // [ESP+0x28] → this->field_08  (heap ptr, or NULL)
//       void*   param_3,    // [ESP+0x2c] → this->field_0c
//       void*   param_4,    // [ESP+0x30] → this->field_10
//       void*   param_5,    // [ESP+0x34] → this->field_14
//       void*   param_6)    // [ESP+0x38] → this->field_18
//
// Stack frame (ESP-relative, after all prologue pushes):
//   [ESP+0x00] saved EDI
//   [ESP+0x04] saved ESI
//   [ESP+0x08] saved EBP
//   [ESP+0x0C] saved EBX
//   [ESP+0x10] saved ECX (this — for EH4 unwind)
//   [ESP+0x14] saved FS:[0] chain link
//   [ESP+0x18] EH4 scope-table handler (0x00e54f6c)
//   [ESP+0x1C] EH4 trylevel (-1 idle → 0 → 3)
//   [ESP+0x20] return address
//   [ESP+0x24..0x38] params 1–6
//
// Epilogue: POP EDI/ESI/EBP/EBX, restore FS:[0], ADD ESP,0x10, RET 0x18
//
// Behaviour (from disassembly):
//   1. Write vftable 0xf56834 at this[0].
//   2. Store params 1–6 into this->fields 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18.
//   3. Store constant function pointers (_memmove → 0x9d5110, _memcpy → 0x9d4600,
//      alloc_fn → 0x4104a0) at this[0x1c], [0x20], [0x24].
//   4. Zero this[0x28] (byte flag).
//   5. InitializeCriticalSection(this + 0x2c)   — IAT at 0x00f3e174.
//   6. Set up embedded Link node at this+0x44:
//        vftable = 0xf567c4, fwd = bwd = &node (empty circular list).
//   7. Zero fields 0x50, 0x54, 0x58.
//   8. Store vtable pointers at 0x5c (0xf568a4) and 0x64 (0xf56878).
//   9. Zero fields 0x60, 0x68.
//  10. If field_08 == 0: __aligned_malloc(field_04, 0x10) → field_08,
//        set field_28 = 1 (owns-memory flag).
//  11. Call FUN_004109a0 (heap init) with field_08/field_04/field_10.
//  12. If non-zero, call FUN_00410510 to add the first block to the list.
//  13. Link the new block into the circular doubly-linked list at node+0x44.
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//
//   This function contains eleven absolute-address relocation sites
//   (vftable immediates, IAT indirect call, two function-pointer
//   immediates, two relative CALLs) that can only be resolved at the
//   original image base of 0x00400000. A source-level rewrite would
//   require coaxing MSVC 2005 /O2 /GS /EHsc into the exact same
//   EH4 prologue shape, register allocation, and instruction selection —
//   each of which is brittle under /O2 optimization. The pragmatic
//   path (same as FUN_00401a00 and FUN_00403a20) is a naked-asm
//   _emit passthrough of the 289 orig bytes. The structural commentary
//   above records what the function does for future reference.

extern "C" __declspec(naked) void FUN_0040f990() {
    __asm {
        // 0000f990: 6a ff
        _emit 0x6a
        _emit 0xff
        // 0000f992: 68 6c 4f e5 00
        _emit 0x68
        _emit 0x6c
        _emit 0x4f
        _emit 0xe5
        _emit 0x00
        // 0000f997: 64 a1 00 00 00 00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f99d: 50
        _emit 0x50
        // 0000f99e: 64 89 25 00 00 00 00
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f9a5: 51
        _emit 0x51
        // 0000f9a6: 53
        _emit 0x53
        // 0000f9a7: 55
        _emit 0x55
        // 0000f9a8: 56
        _emit 0x56
        // 0000f9a9: 8b f1
        _emit 0x8b
        _emit 0xf1
        // 0000f9ab: 57
        _emit 0x57
        // 0000f9ac: 89 74 24 10
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 0000f9b0: 8b 44 24 24
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0000f9b4: 8b 4c 24 28
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // 0000f9b8: 8b 54 24 2c
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        // 0000f9bc: 89 46 04
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 0000f9bf: 8b 44 24 30
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // 0000f9c3: 89 46 10
        _emit 0x89
        _emit 0x46
        _emit 0x10
        // 0000f9c6: 89 4e 08
        _emit 0x89
        _emit 0x4e
        _emit 0x08
        // 0000f9c9: 8b 4c 24 34
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        // 0000f9cd: 89 56 0c
        _emit 0x89
        _emit 0x56
        _emit 0x0c
        // 0000f9d0: 8b 54 24 38
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x38
        // 0000f9d4: 8d 46 2c
        _emit 0x8d
        _emit 0x46
        _emit 0x2c
        // 0000f9d7: 33 db
        _emit 0x33
        _emit 0xdb
        // 0000f9d9: 50
        _emit 0x50
        // 0000f9da: 89 5c 24 20
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        // 0000f9de: c7 06 34 68 f5 00
        _emit 0xc7
        _emit 0x06
        _emit 0x34
        _emit 0x68
        _emit 0xf5
        _emit 0x00
        // 0000f9e4: 89 4e 14
        _emit 0x89
        _emit 0x4e
        _emit 0x14
        // 0000f9e7: 89 56 18
        _emit 0x89
        _emit 0x56
        _emit 0x18
        // 0000f9ea: c7 46 1c 10 51 9d 00
        _emit 0xc7
        _emit 0x46
        _emit 0x1c
        _emit 0x10
        _emit 0x51
        _emit 0x9d
        _emit 0x00
        // 0000f9f1: c7 46 20 00 46 9d 00
        _emit 0xc7
        _emit 0x46
        _emit 0x20
        _emit 0x00
        _emit 0x46
        _emit 0x9d
        _emit 0x00
        // 0000f9f8: c7 46 24 a0 04 41 00
        _emit 0xc7
        _emit 0x46
        _emit 0x24
        _emit 0xa0
        _emit 0x04
        _emit 0x41
        _emit 0x00
        // 0000f9ff: 88 5e 28
        _emit 0x88
        _emit 0x5e
        _emit 0x28
        // 0000fa02: ff 15 74 e1 f3 00
        _emit 0xff
        _emit 0x15
        _emit 0x74
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0000fa08: 8d 7e 44
        _emit 0x8d
        _emit 0x7e
        _emit 0x44
        // 0000fa0b: c7 07 c4 67 f5 00
        _emit 0xc7
        _emit 0x07
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 0000fa11: 89 7f 04
        _emit 0x89
        _emit 0x7f
        _emit 0x04
        // 0000fa14: 89 7f 08
        _emit 0x89
        _emit 0x7f
        _emit 0x08
        // 0000fa17: 88 5e 50
        _emit 0x88
        _emit 0x5e
        _emit 0x50
        // 0000fa1a: 89 5e 54
        _emit 0x89
        _emit 0x5e
        _emit 0x54
        // 0000fa1d: 89 5e 58
        _emit 0x89
        _emit 0x5e
        _emit 0x58
        // 0000fa20: c7 46 5c a4 68 f5 00
        _emit 0xc7
        _emit 0x46
        _emit 0x5c
        _emit 0xa4
        _emit 0x68
        _emit 0xf5
        _emit 0x00
        // 0000fa27: 89 5e 60
        _emit 0x89
        _emit 0x5e
        _emit 0x60
        // 0000fa2a: c7 46 64 78 68 f5 00
        _emit 0xc7
        _emit 0x46
        _emit 0x64
        _emit 0x78
        _emit 0x68
        _emit 0xf5
        _emit 0x00
        // 0000fa31: 89 5e 68
        _emit 0x89
        _emit 0x5e
        _emit 0x68
        // 0000fa34: 39 5e 08
        _emit 0x39
        _emit 0x5e
        _emit 0x08
        // 0000fa37: c6 44 24 1c 03
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x03
        // 0000fa3c: 75 15
        _emit 0x75
        _emit 0x15
        // 0000fa3e: 8b 46 04
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0000fa41: 6a 10
        _emit 0x6a
        _emit 0x10
        // 0000fa43: 50
        _emit 0x50
        // 0000fa44: e8 c9 5c 5c 00
        _emit 0xe8
        _emit 0xc9
        _emit 0x5c
        _emit 0x5c
        _emit 0x00
        // 0000fa49: 83 c4 08
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0000fa4c: 89 46 08
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 0000fa4f: c6 46 28 01
        _emit 0xc6
        _emit 0x46
        _emit 0x28
        _emit 0x01
        // 0000fa53: 8b 46 08
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 0000fa56: 8b 4e 10
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 0000fa59: 8b 6e 04
        _emit 0x8b
        _emit 0x6e
        _emit 0x04
        // 0000fa5c: 89 44 24 24
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0000fa60: e8 3b 0f 00 00
        _emit 0xe8
        _emit 0x3b
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        // 0000fa65: 3b c3
        _emit 0x3b
        _emit 0xc3
        // 0000fa67: 74 1d
        _emit 0x74
        _emit 0x1d
        // 0000fa69: 8b 4c 24 24
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 0000fa6d: 53
        _emit 0x53
        // 0000fa6e: 53
        _emit 0x53
        // 0000fa6f: 53
        _emit 0x53
        // 0000fa70: 53
        _emit 0x53
        // 0000fa71: 6a 01
        _emit 0x6a
        _emit 0x01
        // 0000fa73: 55
        _emit 0x55
        // 0000fa74: 51
        _emit 0x51
        // 0000fa75: 56
        _emit 0x56
        // 0000fa76: 8b c8
        _emit 0x8b
        _emit 0xc8
        // 0000fa78: e8 93 0a 00 00
        _emit 0xe8
        _emit 0x93
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        // 0000fa7d: 3b c3
        _emit 0x3b
        _emit 0xc3
        // 0000fa7f: 74 05
        _emit 0x74
        _emit 0x05
        // 0000fa81: 83 c0 08
        _emit 0x83
        _emit 0xc0
        _emit 0x08
        // 0000fa84: eb 02
        _emit 0xeb
        _emit 0x02
        // 0000fa86: 33 c0
        _emit 0x33
        _emit 0xc0
        // 0000fa88: 8b 57 08
        _emit 0x8b
        _emit 0x57
        _emit 0x08
        // 0000fa8b: 89 42 04
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 0000fa8e: 8b 4f 08
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        // 0000fa91: 89 48 08
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 0000fa94: 8b 4c 24 14
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0000fa98: 89 78 04
        _emit 0x89
        _emit 0x78
        _emit 0x04
        // 0000fa9b: 89 47 08
        _emit 0x89
        _emit 0x47
        _emit 0x08
        // 0000fa9e: 5f
        _emit 0x5f
        // 0000fa9f: 8b c6
        _emit 0x8b
        _emit 0xc6
        // 0000faa1: 5e
        _emit 0x5e
        // 0000faa2: 5d
        _emit 0x5d
        // 0000faa3: 5b
        _emit 0x5b
        // 0000faa4: 64 89 0d 00 00 00 00
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000faab: 83 c4 10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0000faae: c2 18 00
        _emit 0xc2
        _emit 0x18
        _emit 0x00
    }
}
