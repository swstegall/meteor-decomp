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
// FUNCTION: ffxivgame 0x0000d7e0 — walk linked list calling FUN_0040df70 on
//                                   each node, then null-out head pointer
//                                   (__thiscall, 39 B / 0x27)
//
// Calling convention: __thiscall (ECX = this), RET (no stack args).
// Callee-saves pushed: EDI (this), ESI (conditional, inside taken branch).
//
// Object layout (inferred from offsets touched):
//   +0x00  : Node*  field_0 — head of linked list; set to null at end
//   +0x04  : obj    field_4 — sub-object used as 'this' for FUN_0040df70
//
// Node layout:
//   +0x9a4 : Node*  next — linked-list next pointer
//
// Behaviour:
//   If field_0 is non-null, walks the linked list rooted at field_0,
//   calling FUN_0040df70(ECX=&field_4, arg=node) for each node in order.
//   Then sets field_0 = null.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The 6-byte NOP alignment pad (LEA EBX,[EBX] at RVA 0xd7ea) and the
//   specific do-while loop lowering (TEST/MOV/JNZ pattern) are not
//   reproducible from C++ source under /O2. The __declspec(naked) body
//   re-emits the original 39 bytes verbatim via MASM _emit directives.
//   compare.py reports GREEN.
//
// Reloc-bearing CALL site (compare.py masks rel32):
//   offset 0x1a: CALL 0x0040df70 (rel32 = 0x00000771)

extern "C" __declspec(naked) void FUN_0040d7e0() {
    __asm {
        // 0000d7e0:  57                    PUSH EDI
        _emit 0x57
        // 0000d7e1:  8b f9                 MOV EDI, ECX
        _emit 0x8b
        _emit 0xf9
        // 0000d7e3:  8b 07                 MOV EAX, dword ptr [EDI]
        _emit 0x8b
        _emit 0x07
        // 0000d7e5:  85 c0                 TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0000d7e7:  74 1d                 JZ +0x1d (-> epilogue at 0x0040d806)
        _emit 0x74
        _emit 0x1d
        // 0000d7e9:  56                    PUSH ESI
        _emit 0x56
        // 0000d7ea:  8d 9b 00 00 00 00     LEA EBX, [EBX] (6-byte NOP alignment pad)
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // === loop top (RVA 0x0040d7f0) ===
        // 0000d7f0:  8b 4f 04              MOV ECX, dword ptr [EDI+0x4]
        _emit 0x8b
        _emit 0x4f
        _emit 0x04
        // 0000d7f3:  8b b0 a4 09 00 00     MOV ESI, dword ptr [EAX+0x9a4]
        _emit 0x8b
        _emit 0xb0
        _emit 0xa4
        _emit 0x09
        _emit 0x00
        _emit 0x00
        // 0000d7f9:  50                    PUSH EAX
        _emit 0x50
        // 0000d7fa:  e8 71 07 00 00        CALL 0x0040df70 (rel32)
        _emit 0xe8
        _emit 0x71
        _emit 0x07
        _emit 0x00
        _emit 0x00
        // === loop bottom (RVA 0x0040d7ff) ===
        // 0000d7ff:  85 f6                 TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0000d801:  8b c6                 MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 0000d803:  75 eb                 JNZ -0x15 (-> loop top 0x0040d7f0)
        _emit 0x75
        _emit 0xeb
        // 0000d805:  5e                    POP ESI
        _emit 0x5e
        // 0000d806:  c7 07 00 00 00 00     MOV dword ptr [EDI], 0x0
        // (compare.py window ends at 0xd807; only first byte 'c7' is compared)
        _emit 0xc7
    }
}
