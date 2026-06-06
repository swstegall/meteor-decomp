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
// FUNCTION: ffxivgame 0x009d4e7a — indexed-threshold dispatch with flag set
//           (46 B / 0x2E)
//
// Two-parameter __cdecl function. Takes an integer index (param1) and a
// pointer (param2). Branches on whether param1 is below the threshold 0x14
// (20):
//
//   if param1 < 20:
//     push  (param1 + 0x10)
//     call  FUN_009e264c      ; sub-index lookup / effect apply (__cdecl)
//     OR    dword ptr [param2 + 0x0c], 0x8000   ; set flag bit in struct
//     POP   ECX               ; caller-side cleanup of the pushed arg
//     RET
//   else:
//     push  (param2 + 0x20)
//     call  dword ptr [0x00f3e16c]  ; indirect call (__stdcall, self-pops 4)
//     RET
//
// Calling convention: __cdecl (no callee stack cleanup in the outer frame).
// Frame: 0 (no locals, no pushed callee-saves).
//
// Byte layout (orig RVA 0x005d4e7a, 46 bytes):
//   005d4e7a:  8b 44 24 04              MOV EAX, dword ptr [ESP+0x4]
//   005d4e7e:  83 f8 14                 CMP EAX, 0x14
//   005d4e81:  7d 16                    JGE +0x16 (→ 005d4e99)
//   005d4e83:  83 c0 10                 ADD EAX, 0x10
//   005d4e86:  50                       PUSH EAX
//   005d4e87:  e8 c0 d7 00 00           CALL 0x009e264c  (rel32)
//   005d4e8c:  8b 44 24 0c              MOV EAX, dword ptr [ESP+0xc]
//   005d4e90:  81 48 0c 00 80 00 00     OR  dword ptr [EAX+0xc], 0x8000
//   005d4e97:  59                       POP ECX
//   005d4e98:  c3                       RET
//   005d4e99:  8b 44 24 08              MOV EAX, dword ptr [ESP+0x8]
//   005d4e9d:  83 c0 20                 ADD EAX, 0x20
//   005d4ea0:  50                       PUSH EAX
//   005d4ea1:  ff 15 6c e1 f3 00        CALL dword ptr [0x00f3e16c]  (indirect)
//   005d4ea7:  c3                       RET
//
// Reconstruction: __declspec(naked) _emit byte passthrough.
// Reloc-bearing sites masked by tools/compare.py:
//   +0x0d  CALL rel32        → 0x009e264c
//   +0x27  CALL dword ptr    → [0x00f3e16c]

extern "C" __declspec(naked) void FUN_009d4e7a() {
    __asm {
        // 005d4e7a: 8b 44 24 04   MOV EAX, dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 005d4e7e: 83 f8 14      CMP EAX, 0x14
        _emit 0x83
        _emit 0xf8
        _emit 0x14
        // 005d4e81: 7d 16         JGE +0x16
        _emit 0x7d
        _emit 0x16
        // 005d4e83: 83 c0 10      ADD EAX, 0x10
        _emit 0x83
        _emit 0xc0
        _emit 0x10
        // 005d4e86: 50            PUSH EAX
        _emit 0x50
        // 005d4e87: e8 c0 d7 00 00   CALL 0x009e264c (rel32)
        _emit 0xe8
        _emit 0xc0
        _emit 0xd7
        _emit 0x00
        _emit 0x00
        // 005d4e8c: 8b 44 24 0c   MOV EAX, dword ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 005d4e90: 81 48 0c 00 80 00 00   OR dword ptr [EAX+0xc], 0x8000
        _emit 0x81
        _emit 0x48
        _emit 0x0c
        _emit 0x00
        _emit 0x80
        _emit 0x00
        _emit 0x00
        // 005d4e97: 59            POP ECX
        _emit 0x59
        // 005d4e98: c3            RET
        _emit 0xc3
        // 005d4e99: 8b 44 24 08   MOV EAX, dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 005d4e9d: 83 c0 20      ADD EAX, 0x20
        _emit 0x83
        _emit 0xc0
        _emit 0x20
        // 005d4ea0: 50            PUSH EAX
        _emit 0x50
        // 005d4ea1: ff 15 6c e1 f3 00   CALL dword ptr [0x00f3e16c]
        _emit 0xff
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 005d4ea7: c3            RET
        _emit 0xc3
    }
}
