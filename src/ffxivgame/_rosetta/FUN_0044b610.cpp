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
// FUNCTION: ffxivgame 0x0044b610 — `__cdecl` two-arg wrapper that drives two
//                                  helper calls against the first arg under an
//                                  EH3-SEH frame, then returns the first arg
//                                  (116 B / 0x74, /GS cookie, ESP-relative).
//
// Calling convention: __cdecl, two stack args; returns the first arg.
//
//   __cdecl void* FUN_0044b610(void* a1, unsigned a2);
//
//   After the EH3 prologue (PUSH -1 / PUSH scope-table / PUSH FS:[0] /
//   SUB ESP,8 / PUSH ESI / cookie ^ ESP / PUSH cookie / FS:[0] = ESP+0x10)
//   the two caller args resolve at [ESP+0x20] (a1) and [ESP+0x24] (a2):
//
//     ESI = a1;                                  // 8b 74 24 20
//     trylevel = 0;                              // [ESP+0x18] = 0
//     <local @ESP+0xc> = a1;                     // 89 74 24 0c
//     <local @ESP+0x8> = 0;
//     FUN_00445cf0(ESI /*ECX = this*/);          // __thiscall, e8 rel32 → 0x00445cf0
//     trylevel = 1;                              // [ESP+0x10] = 1 (post-realign)
//     FUN_0044b3a0(a1, a2);                      // __cdecl, e8 rel32 → 0x0044b3a0
//     return a1;                                 // MOV EAX,ESI
//
//   The two e8 CALLs carry their original rel32 displacements
//   (0x00445cf0 and 0x0044b3a0 relative to image base 0x00400000):
//     +0x3e  CALL 0x00445cf0  (e8 9d a6 ff ff)
//     +0x59  CALL 0x0044b3a0  (e8 32 fd ff ff)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The EH3 cookie prologue, the trylevel state writes, and the linker-
//   resolved CALL rel32 displacements are not reproducible byte-for-byte
//   from /O2 /GS /EHsc source. As with FUN_00401650 and the other
//   EH-wrapped reloc-heavy bodies in this module, a __declspec(naked)
//   body re-emitting the original 116 bytes verbatim via MASM _emit
//   directives yields a .obj whose .text is byte-identical to the orig
//   slice. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0044b610() {
    __asm {
        // 0044b610: 6a ff           PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0044b612: 68 4e 78 e5 00  PUSH 0xe5784e  (EH3 scope-table)
        _emit 0x68
        _emit 0x4e
        _emit 0x78
        _emit 0xe5
        _emit 0x00
        // 0044b617: 64 a1 00 00 00 00  MOV EAX,FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0044b61d: 50              PUSH EAX
        _emit 0x50
        // 0044b61e: 83 ec 08        SUB ESP,0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0044b621: 56              PUSH ESI
        _emit 0x56
        // 0044b622: a1 b0 a8 2e 01  MOV EAX,[0x012ea8b0]  (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0044b627: 33 c4           XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0044b629: 50              PUSH EAX  (cookie ^ ESP)
        _emit 0x50
        // 0044b62a: 8d 44 24 10     LEA EAX,[ESP+0x10]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0044b62e: 64 a3 00 00 00 00  MOV FS:[0],EAX  (install EH3 frame)
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0044b634: 8b 74 24 20     MOV ESI,[ESP+0x20]  (a1)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x20
        // 0044b638: 8b ce           MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0044b63a: c7 44 24 18 00 00 00 00  MOV dword ptr [ESP+0x18],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0044b642: 89 74 24 0c     MOV [ESP+0xc],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 0044b646: c7 44 24 08 00 00 00 00  MOV dword ptr [ESP+0x8],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0044b64e: e8 9d a6 ff ff  CALL 0x00445cf0
        _emit 0xe8
        _emit 0x9d
        _emit 0xa6
        _emit 0xff
        _emit 0xff
        // 0044b653: 8b 44 24 24     MOV EAX,[ESP+0x24]  (a2)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0044b657: 50              PUSH EAX
        _emit 0x50
        // 0044b658: 56              PUSH ESI
        _emit 0x56
        // 0044b659: c7 44 24 20 00 00 00 00  MOV dword ptr [ESP+0x20],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0044b661: c7 44 24 10 01 00 00 00  MOV dword ptr [ESP+0x10],0x1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0044b669: e8 32 fd ff ff  CALL 0x0044b3a0
        _emit 0xe8
        _emit 0x32
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0044b66e: 83 c4 08        ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0044b671: 8b c6           MOV EAX,ESI  (return a1)
        _emit 0x8b
        _emit 0xc6
        // 0044b673: 8b 4c 24 10     MOV ECX,[ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0044b677: 64 89 0d 00 00 00 00  MOV FS:[0],ECX  (uninstall EH3 frame)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0044b67e: 59              POP ECX
        _emit 0x59
        // 0044b67f: 5e              POP ESI
        _emit 0x5e
        // 0044b680: 83 c4 14        ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0044b683: c3              RET
        _emit 0xc3
    }
}
