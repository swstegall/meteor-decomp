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
// FUNCTION: ffxivgame 0x0043d080 — unknown factory/init helper
//                                  (357 B / 0x165, __cdecl, SEH4-wrapped,
//                                   large 0x8C8-byte stack frame).
//
// Asm shape (read from asm/ffxivgame/0003d080_FUN_0043d080.s,
// 357 bytes — RVA 0x0003d080..0x0003d1e5):
//
//   __cdecl void* FUN_0043d080(void** ppOut, int mode,
//                               void* initArg, void* extraArg);
//
//   Structural outline:
//     if (mode != 2) { *ppOut = NULL; return ppOut; }
//     // Construct a local object on the huge stack frame
//     FUN_0043b440(ecx=&local, initArg, 0);   // __thiscall ctor
//     // Check two flag bytes in the constructed object
//     if (!local[1] || !local[0xD]) {
//         *ppOut = NULL; goto cleanup;
//     }
//     // Extract a value, call a prepare fn, then a 7-arg factory
//     FUN_0043cd70(ebx=local[4], esi=&local2);
//     FUN_00418d00(7 args including extraArg, local fields);
//     // Virtual destructor call (if inner object non-null)
//     // Loop 6 times calling FUN_0043cab0(edi, ebx, 0)
//     *ppOut = result; // success path
//   cleanup:
//     FUN_0043ac30(ecx=&local);               // __thiscall dtor
//     return ppOut;
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The body contains five PC-relative CALL instructions whose targets
//   resolve only in the full binary link (FUN_0043b440, FUN_0043cd70,
//   FUN_00418d00, FUN_0043cab0, FUN_0043ac30), plus the standard
//   MSVC 2005 SEH4 prolog with the security-cookie absolute address
//   and two FS:[0] memory-operand absolute fixups. All of these would
//   require a full-binary relink to reproduce correctly from C++ source.
//   The naked-asm _emit passthrough (matching every sibling in _rosetta/)
//   produces a byte-identical .text section that compare.py grades GREEN.

extern "C" __declspec(naked) void FUN_0043d080() {
    __asm {
        // 0003d080: 6a ff          PUSH -1
        _emit 0x6a
        _emit 0xff
        // 0003d082: 68 02 69 e5 00 PUSH 0xe56902
        _emit 0x68
        _emit 0x02
        _emit 0x69
        _emit 0xe5
        _emit 0x00
        // 0003d087: 64 a1 00 00 00 00  MOV EAX,FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d08d: 50             PUSH EAX
        _emit 0x50
        // 0003d08e: 81 ec c8 08 00 00  SUB ESP,0x8c8
        _emit 0x81
        _emit 0xec
        _emit 0xc8
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0003d094: 53             PUSH EBX
        _emit 0x53
        // 0003d095: 55             PUSH EBP
        _emit 0x55
        // 0003d096: 56             PUSH ESI
        _emit 0x56
        // 0003d097: 57             PUSH EDI
        _emit 0x57
        // 0003d098: a1 b0 a8 2e 01 MOV EAX,[0x012ea8b0]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0003d09d: 33 c4          XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0003d09f: 50             PUSH EAX
        _emit 0x50
        // 0003d0a0: 8d 84 24 dc 08 00 00  LEA EAX,[ESP+0x8dc]
        _emit 0x8d
        _emit 0x84
        _emit 0x24
        _emit 0xdc
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0003d0a7: 64 a3 00 00 00 00  MOV FS:[0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d0ad: 33 ed          XOR EBP,EBP
        _emit 0x33
        _emit 0xed
        // 0003d0af: 83 bc 24 f0 08 00 00 02  CMP dword ptr [ESP+0x8f0],0x2
        _emit 0x83
        _emit 0xbc
        _emit 0x24
        _emit 0xf0
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x02
        // 0003d0b7: 89 6c 24 1c    MOV dword ptr [ESP+0x1c],EBP
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        // 0003d0bb: 74 0e          JZ +0x0e
        _emit 0x74
        _emit 0x0e
        // 0003d0bd: 8b 84 24 ec 08 00 00  MOV EAX,[ESP+0x8ec]
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0xec
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0003d0c4: 89 28          MOV dword ptr [EAX],EBP
        _emit 0x89
        _emit 0x28
        // 0003d0c6: e9 00 01 00 00 JMP +0x100
        _emit 0xe9
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0003d0cb: 8b 84 24 f4 08 00 00  MOV EAX,[ESP+0x8f4]
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0xf4
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0003d0d2: 55             PUSH EBP
        _emit 0x55
        // 0003d0d3: 50             PUSH EAX
        _emit 0x50
        // 0003d0d4: 8d 4c 24 28   LEA ECX,[ESP+0x28]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // 0003d0d8: e8 63 e3 ff ff CALL 0x0043b440
        _emit 0xe8
        _emit 0x63
        _emit 0xe3
        _emit 0xff
        _emit 0xff
        // 0003d0dd: 80 7c 24 21 00 CMP byte ptr [ESP+0x21],0x0
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x21
        _emit 0x00
        // 0003d0e2: bf 01 00 00 00 MOV EDI,0x1
        _emit 0xbf
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d0e7: 89 bc 24 e4 08 00 00  MOV [ESP+0x8e4],EDI
        _emit 0x89
        _emit 0xbc
        _emit 0x24
        _emit 0xe4
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0003d0ee: 0f 84 b3 00 00 00  JZ +0xb3
        _emit 0x0f
        _emit 0x84
        _emit 0xb3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d0f4: 80 7c 24 2d 00 CMP byte ptr [ESP+0x2d],0x0
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x2d
        _emit 0x00
        // 0003d0f9: 0f 84 a8 00 00 00  JZ +0xa8
        _emit 0x0f
        _emit 0x84
        _emit 0xa8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d0ff: 8b 44 24 24   MOV EAX,[ESP+0x24]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0003d103: 8d 74 24 14   LEA ESI,[ESP+0x14]
        _emit 0x8d
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 0003d107: 8b d8          MOV EBX,EAX
        _emit 0x8b
        _emit 0xd8
        // 0003d109: e8 62 fc ff ff CALL 0x0043cd70
        _emit 0xe8
        _emit 0x62
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 0003d10e: 89 6c 24 18   MOV [ESP+0x18],EBP
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 0003d112: 8b 8c 24 f8 08 00 00  MOV ECX,[ESP+0x8f8]
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0xf8
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0003d119: 8b 54 24 14   MOV EDX,[ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0003d11d: 8b 44 24 40   MOV EAX,[ESP+0x40]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x40
        // 0003d121: 51             PUSH ECX
        _emit 0x51
        // 0003d122: 8b 4c 24 40   MOV ECX,[ESP+0x40]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        // 0003d126: 52             PUSH EDX
        _emit 0x52
        // 0003d127: 55             PUSH EBP
        _emit 0x55
        // 0003d128: 57             PUSH EDI
        _emit 0x57
        // 0003d129: 50             PUSH EAX
        _emit 0x50
        // 0003d12a: 51             PUSH ECX
        _emit 0x51
        // 0003d12b: 8d 54 24 2c   LEA EDX,[ESP+0x2c]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        // 0003d12f: 52             PUSH EDX
        _emit 0x52
        // 0003d130: c6 84 24 00 09 00 00 02  MOV byte ptr [ESP+0x900],0x2
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x02
        // 0003d138: e8 c3 bb fd ff CALL 0x00418d00
        _emit 0xe8
        _emit 0xc3
        _emit 0xbb
        _emit 0xfd
        _emit 0xff
        // 0003d13d: 83 c4 1c      ADD ESP,0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 0003d140: 8b 38          MOV EDI,[EAX]
        _emit 0x8b
        _emit 0x38
        // 0003d142: 89 28          MOV [EAX],EBP
        _emit 0x89
        _emit 0x28
        // 0003d144: 8b 4c 24 14   MOV ECX,[ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0003d148: 3b cd          CMP ECX,EBP
        _emit 0x3b
        _emit 0xcd
        // 0003d14a: 89 7c 24 18   MOV [ESP+0x18],EDI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        // 0003d14e: c6 84 24 e4 08 00 00 02  MOV byte ptr [ESP+0x8e4],0x2
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0xe4
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x02
        // 0003d156: 74 08          JZ +0x08
        _emit 0x74
        _emit 0x08
        // 0003d158: 8b 01          MOV EAX,[ECX]
        _emit 0x8b
        _emit 0x01
        // 0003d15a: 8b 10          MOV EDX,[EAX]
        _emit 0x8b
        _emit 0x10
        // 0003d15c: 6a 01          PUSH 1
        _emit 0x6a
        _emit 0x01
        // 0003d15e: ff d2          CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0003d160: 3b fd          CMP EDI,EBP
        _emit 0x3b
        _emit 0xfd
        // 0003d162: 74 30          JZ +0x30
        _emit 0x74
        _emit 0x30
        // 0003d164: 33 f6          XOR ESI,ESI
        _emit 0x33
        _emit 0xf6
        // 0003d166: 8d 6c 24 4c   LEA EBP,[ESP+0x4c]
        _emit 0x8d
        _emit 0x6c
        _emit 0x24
        _emit 0x4c
        // 0003d16a: 8d 9b 00 00 00 00  LEA EBX,[EBX]
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d170: 8b 45 00       MOV EAX,[EBP]
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        // 0003d173: 85 c0          TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0003d175: 75 03          JNZ +0x03
        _emit 0x75
        _emit 0x03
        // 0003d177: 8b 45 04       MOV EAX,[EBP+4]
        _emit 0x8b
        _emit 0x45
        _emit 0x04
        // 0003d17a: 53             PUSH EBX
        _emit 0x53
        // 0003d17b: 57             PUSH EDI
        _emit 0x57
        // 0003d17c: 33 d2          XOR EDX,EDX
        _emit 0x33
        _emit 0xd2
        // 0003d17e: e8 2d f9 ff ff CALL 0x0043cab0
        _emit 0xe8
        _emit 0x2d
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        // 0003d183: 83 c6 01       ADD ESI,1
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        // 0003d186: 83 c4 08       ADD ESP,8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0003d189: 81 c5 70 01 00 00  ADD EBP,0x170
        _emit 0x81
        _emit 0xc5
        _emit 0x70
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0003d18f: 83 fe 06       CMP ESI,6
        _emit 0x83
        _emit 0xfe
        _emit 0x06
        // 0003d192: 72 dc          JC -0x24
        _emit 0x72
        _emit 0xdc
        // 0003d194: 8b b4 24 ec 08 00 00  MOV ESI,[ESP+0x8ec]
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0xec
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0003d19b: 89 3e          MOV [ESI],EDI
        _emit 0x89
        _emit 0x3e
        // 0003d19d: c7 44 24 1c 01 00 00 00  MOV dword ptr [ESP+0x1c],1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d1a5: eb 11          JMP +0x11
        _emit 0xeb
        _emit 0x11
        // 0003d1a7: 8b b4 24 ec 08 00 00  MOV ESI,[ESP+0x8ec]
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0xec
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0003d1ae: 89 6c 24 18   MOV [ESP+0x18],EBP
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 0003d1b2: 89 2e          MOV [ESI],EBP
        _emit 0x89
        _emit 0x2e
        // 0003d1b4: 89 7c 24 1c   MOV [ESP+0x1c],EDI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        // 0003d1b8: 8d 4c 24 20   LEA ECX,[ESP+0x20]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 0003d1bc: c6 84 24 e4 08 00 00 00  MOV byte ptr [ESP+0x8e4],0x0
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0xe4
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d1c4: e8 67 da ff ff CALL 0x0043ac30
        _emit 0xe8
        _emit 0x67
        _emit 0xda
        _emit 0xff
        _emit 0xff
        // 0003d1c9: 8b c6          MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0003d1cb: 8b 8c 24 dc 08 00 00  MOV ECX,[ESP+0x8dc]
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0xdc
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0003d1d2: 64 89 0d 00 00 00 00  MOV FS:[0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d1d9: 59             POP ECX
        _emit 0x59
        // 0003d1da: 5f             POP EDI
        _emit 0x5f
        // 0003d1db: 5e             POP ESI
        _emit 0x5e
        // 0003d1dc: 5d             POP EBP
        _emit 0x5d
        // 0003d1dd: 5b             POP EBX
        _emit 0x5b
        // 0003d1de: 81 c4 d4 08 00 00  ADD ESP,0x8d4
        _emit 0x81
        _emit 0xc4
        _emit 0xd4
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0003d1e4: c3             RET
        _emit 0xc3
    }
}
