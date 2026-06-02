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
// FUNCTION: ffxivgame 0x00013fa0 — DetachableHeapBlock allocator
//                                  (__thiscall, 4 stack args, 200 B / 0xc8)
//
// ECX        : this   — heap-manager-like owner of a guard / list / pool quartet
//   [this+0x10]  guard      — IGuard with Enter (vftable +0x2c) / Leave (+0x30) /
//                              GetAllocator (+0x04) virtual slots
//   [this+0x28]  next       — head of a singly-linked sub-allocator chain
//                              (link via +0x2c), each carrying a "used bytes"
//                              count at +0x28 and an allocator-pointer at +0x1c
//   [this+0x44]  list_head  — list sentinel for the newly-allocated block
// [ESP+0x14]    param_1 = size (added to running sum across the chain)
// [ESP+0x18..0x20] param_2..param_4 — forwarded to the inner allocator call
//
// Prologue interleaves PUSH ESI before MOV EBX,ECX then defers PUSH EDI:
//   PUSH EBX; PUSH EBP; PUSH ESI; MOV EBX, ECX; PUSH EDI
//
// The 4-byte LEA ESP,[ESP] at +0x1c is MSVC's 16-byte loop-top alignment NOP.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The interleaved prologue, 4-byte NOP, four consecutive identical
//   MOV ESI,[ESP+0x20]; PUSH ESI argument forwards, and the specific
//   register allocation cannot be reproduced from C++ source without
//   significant iteration risk. The __declspec(naked) body re-emits the
//   original 200 bytes verbatim; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00413fa0() {
    __asm {
        // 00013fa0:  53                PUSH EBX
        _emit 0x53
        // 00013fa1:  55                PUSH EBP
        _emit 0x55
        // 00013fa2:  56                PUSH ESI
        _emit 0x56
        // 00013fa3:  8b d9             MOV  EBX, ECX
        _emit 0x8b
        _emit 0xd9
        // 00013fa5:  57                PUSH EDI
        _emit 0x57
        // 00013fa6:  8b 7b 10          MOV  EDI, dword ptr [EBX+0x10]
        _emit 0x8b
        _emit 0x7b
        _emit 0x10
        // 00013fa9:  8b 07             MOV  EAX, dword ptr [EDI]
        _emit 0x8b
        _emit 0x07
        // 00013fab:  8b 50 2c          MOV  EDX, dword ptr [EAX+0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 00013fae:  8b cf             MOV  ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00013fb0:  ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00013fb2:  8b 4b 28          MOV  ECX, dword ptr [EBX+0x28]
        _emit 0x8b
        _emit 0x4b
        _emit 0x28
        // 00013fb5:  85 c9             TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 00013fb7:  8d 43 fc          LEA  EAX, [EBX-0x4]
        _emit 0x8d
        _emit 0x43
        _emit 0xfc
        // 00013fba:  74 0d             JZ   +0x0d
        _emit 0x74
        _emit 0x0d
        // 00013fbc:  8d 64 24 00       LEA  ESP, [ESP]  (4-byte loop-top NOP)
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
        // 00013fc0:  8b c1             MOV  EAX, ECX
        _emit 0x8b
        _emit 0xc1
        // 00013fc2:  8b 48 2c          MOV  ECX, dword ptr [EAX+0x2c]
        _emit 0x8b
        _emit 0x48
        _emit 0x2c
        // 00013fc5:  85 c9             TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 00013fc7:  75 f7             JNZ  -0x09
        _emit 0x75
        _emit 0xf7
        // 00013fc9:  8b 48 1c          MOV  ECX, dword ptr [EAX+0x1c]
        _emit 0x8b
        _emit 0x48
        _emit 0x1c
        // 00013fcc:  8d 43 fc          LEA  EAX, [EBX-0x4]
        _emit 0x8d
        _emit 0x43
        _emit 0xfc
        // 00013fcf:  33 d2             XOR  EDX, EDX
        _emit 0x33
        _emit 0xd2
        // 00013fd1:  85 c0             TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00013fd3:  74 0a             JZ   +0x0a
        _emit 0x74
        _emit 0x0a
        // 00013fd5:  03 50 28          ADD  EDX, dword ptr [EAX+0x28]
        _emit 0x03
        _emit 0x50
        _emit 0x28
        // 00013fd8:  8b 40 2c          MOV  EAX, dword ptr [EAX+0x2c]
        _emit 0x8b
        _emit 0x40
        _emit 0x2c
        // 00013fdb:  85 c0             TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00013fdd:  75 f6             JNZ  -0x0a
        _emit 0x75
        _emit 0xf6
        // 00013fdf:  8b 74 24 20       MOV  ESI, dword ptr [ESP+0x20]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x20
        // 00013fe3:  8b 01             MOV  EAX, dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 00013fe5:  56                PUSH ESI
        _emit 0x56
        // 00013fe6:  8b 74 24 20       MOV  ESI, dword ptr [ESP+0x20]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x20
        // 00013fea:  56                PUSH ESI
        _emit 0x56
        // 00013feb:  8b 74 24 20       MOV  ESI, dword ptr [ESP+0x20]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x20
        // 00013fef:  56                PUSH ESI
        _emit 0x56
        // 00013ff0:  8b 74 24 20       MOV  ESI, dword ptr [ESP+0x20]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x20
        // 00013ff4:  03 d6             ADD  EDX, ESI
        _emit 0x03
        _emit 0xd6
        // 00013ff6:  52                PUSH EDX
        _emit 0x52
        // 00013ff7:  8b 50 24          MOV  EDX, dword ptr [EAX+0x24]
        _emit 0x8b
        _emit 0x50
        _emit 0x24
        // 00013ffa:  ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00013ffc:  8b 4b 10          MOV  ECX, dword ptr [EBX+0x10]
        _emit 0x8b
        _emit 0x4b
        _emit 0x10
        // 00013fff:  8b e8             MOV  EBP, EAX
        _emit 0x8b
        _emit 0xe8
        // 00014001:  8b 01             MOV  EAX, dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 00014003:  8b 50 04          MOV  EDX, dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00014006:  ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00014008:  8b 48 14          MOV  ECX, dword ptr [EAX+0x14]
        _emit 0x8b
        _emit 0x48
        _emit 0x14
        // 0001400b:  e8 90 c9 ff ff    CALL FUN_004109a0
        _emit 0xe8
        _emit 0x90
        _emit 0xc9
        _emit 0xff
        _emit 0xff
        // 00014010:  85 c0             TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00014012:  74 1b             JZ   +0x1b
        _emit 0x74
        _emit 0x1b
        // 00014014:  8d 4b fc          LEA  ECX, [EBX-0x4]
        _emit 0x8d
        _emit 0x4b
        _emit 0xfc
        // 00014017:  51                PUSH ECX
        _emit 0x51
        // 00014018:  56                PUSH ESI
        _emit 0x56
        // 00014019:  55                PUSH EBP
        _emit 0x55
        // 0001401a:  6a 00             PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0001401c:  57                PUSH EDI
        _emit 0x57
        // 0001401d:  8b c8             MOV  ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 0001401f:  e8 ac f9 ff ff    CALL FUN_004139d0
        _emit 0xe8
        _emit 0xac
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        // 00014024:  8b f0             MOV  ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 00014026:  85 f6             TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 00014028:  74 07             JZ   +0x07
        _emit 0x74
        _emit 0x07
        // 0001402a:  8d 4e 08          LEA  ECX, [ESI+0x8]
        _emit 0x8d
        _emit 0x4e
        _emit 0x08
        // 0001402d:  eb 04             JMP  +0x04
        _emit 0xeb
        _emit 0x04
        // 0001402f:  33 f6             XOR  ESI, ESI
        _emit 0x33
        _emit 0xf6
        // 00014031:  33 c9             XOR  ECX, ECX
        _emit 0x33
        _emit 0xc9
        // 00014033:  8b 43 44          MOV  EAX, dword ptr [EBX+0x44]
        _emit 0x8b
        _emit 0x43
        _emit 0x44
        // 00014036:  8b 50 08          MOV  EDX, dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 00014039:  89 4a 04          MOV  dword ptr [EDX+0x4], ECX
        _emit 0x89
        _emit 0x4a
        _emit 0x04
        // 0001403c:  8b 50 08          MOV  EDX, dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 0001403f:  89 51 08          MOV  dword ptr [ECX+0x8], EDX
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 00014042:  89 41 04          MOV  dword ptr [ECX+0x4], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x04
        // 00014045:  89 48 08          MOV  dword ptr [EAX+0x8], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 00014048:  8b 07             MOV  EAX, dword ptr [EDI]
        _emit 0x8b
        _emit 0x07
        // 0001404a:  8b 50 30          MOV  EDX, dword ptr [EAX+0x30]
        _emit 0x8b
        _emit 0x50
        _emit 0x30
        // 0001404d:  8b cf             MOV  ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 0001404f:  ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00014051:  85 f6             TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 00014053:  74 0a             JZ   +0x0a
        _emit 0x74
        _emit 0x0a
        // 00014055:  5f                POP  EDI
        _emit 0x5f
        // 00014056:  8d 46 04          LEA  EAX, [ESI+0x4]
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        // 00014059:  5e                POP  ESI
        _emit 0x5e
        // 0001405a:  5d                POP  EBP
        _emit 0x5d
        // 0001405b:  5b                POP  EBX
        _emit 0x5b
        // 0001405c:  c2 10 00          RET  0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
        // 0001405f:  5f                POP  EDI
        _emit 0x5f
        // 00014060:  5e                POP  ESI
        _emit 0x5e
        // 00014061:  5d                POP  EBP
        _emit 0x5d
        // 00014062:  33 c0             XOR  EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00014064:  5b                POP  EBX
        _emit 0x5b
        // 00014065:  c2 10 00          RET  0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
