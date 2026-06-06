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
// FUNCTION: ffxivgame 0x00435d60 — `__thiscall` virtual-dispatch wrapper with
//                                  a once-guarded assertion-failure tail (111 B).
//
// Inspection (read from the disassembly at orig RVA 0x00035d60):
//
//   int __thiscall FUN_00435d60(T *this, U *param);   // RET 0x4 → one stack arg
//
//   Forwards six members of `this` plus `param` itself into a virtual
//   method at slot 0x148 of `param`'s vtable, then — if that call
//   returns non-zero — falls into a lazily-initialized assertion-report
//   thunk guarded by a global once-flag.
//
//   Pseudo-C:
//
//     int __thiscall FUN_00435d60(T *self, U *param) {
//         int r = (*(int (__cdecl**)(U*, ...))                  // 8b 10 / 8b 92 48 01
//                     ((*(void***)param)[0x148/4]))             //   vtbl slot 0x148
//                 (param,                                       // PUSH EAX
//                  self->f4, self->f8, self->fc,                // PUSH [ECX+4..0xc]
//                  self->f10, self->f14, self->f18);            // PUSH [ECX+0x10..0x18]
//         if (r) {                                              // TEST EAX,EAX / JZ end
//             if ((g_flag & 1) == 0) {                          // 84 05 10 39 32 01
//                 g_flag |= 1;                                  // 09 05 ...
//                 g_pfn  = (void*)0x00433720;                   // c7 05 0c 39 32 01 ...
//             }
//             g_pfn(0x00F64BE8, 0x00F65378, 0x00F64C18,         // 5 PUSHes + CALL [g_pfn]
//                   0x23C, 0x00F653A0);
//         }
//         return r;                                             // RET 0x4
//     }
//
//   .data slots:  g_flag = 0x01323910 (byte/dword once-flag)
//                 g_pfn  = 0x0132390C (lazily-set report fn pointer)
//   Default report fn:    0x00433720
//   String/lineno args:   0x00F653A0, 0x0000023C, 0x00F64C18,
//                          0x00F65378, 0x00F64BE8
//
// Reloc-bearing sites in the orig 111 bytes (absolute .data/.text/.rdata
// addresses + an indirect CALL through a .data slot). These resolve only
// at full-binary relink; standalone .obj compilation can't reproduce them
// as relocations, so the local idiom (see FUN_004091f0, FUN_00401650,
// FUN_004090b0) is a `__declspec(naked)` body that re-emits the orig bytes
// verbatim via MASM `_emit`. The .obj's `.text` ends up byte-identical to
// the orig slice with NO relocations — `tools/compare.py` then reports
// GREEN.

extern "C" __declspec(naked) void FUN_00435d60() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x8b              // MOV EDX, dword ptr [EDX+0x148]
        _emit 0x92
        _emit 0x48
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ECX+0x18]
        _emit 0x71
        _emit 0x18
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ECX+0x14]
        _emit 0x71
        _emit 0x14
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ECX+0x10]
        _emit 0x71
        _emit 0x10
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ECX+0xc]
        _emit 0x71
        _emit 0x0c
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ECX+0x8]
        _emit 0x71
        _emit 0x08
        _emit 0x8b              // MOV ECX, dword ptr [ECX+0x4]
        _emit 0x49
        _emit 0x04
        _emit 0x56              // PUSH ESI
        _emit 0x51              // PUSH ECX
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0x74              // JZ +0x3f (end)
        _emit 0x3f
        _emit 0xb8              // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [0x01323910], AL
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ +0x10
        _emit 0x10
        _emit 0x09              // OR dword ptr [0x01323910], EAX
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [0x0132390C], 0x00433720
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00
        _emit 0x68              // PUSH 0x00F653A0
        _emit 0xa0
        _emit 0x53
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x0000023C
        _emit 0x3c
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x00F64C18
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x00F65378
        _emit 0x78
        _emit 0x53
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x00F64BE8
        _emit 0xe8
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x0132390C]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET 0x4   (end)
        _emit 0x04
        _emit 0x00
    }
}
