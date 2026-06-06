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
// FUNCTION: ffxivgame 0x0001caa0 — conditional dispatch through a global
//                                  function-pointer table, thiscall notify,
//                                  and release helper
//                                  (__cdecl, 98 B / 0x62)
//
// Stack layout at entry (ESP-relative, before any PUSH):
//   [ESP+0x04] : arg0   — index into the global fptr table at 0xf595c4
//   [ESP+0x08] : arg1   — (unused until late; loaded as EDX after pushes)
//                         actually [ESP+0x14] after 4 pushes: original arg2?
//   [ESP+0x0C] : arg2   — pointer passed as `this` for CALL 0x0041c540
//   [ESP+0x10] : arg3   — pointer used as context pointer (EBX); if NULL, early return
//
// Shape:
//   if (arg3 == NULL) return;                    // JZ to RET
//   ptr = g_fptr_table[arg0];                    // [EAX*4 + 0xf595c4]
//   if (ptr == NULL) return;                     // JZ to POP EDI / POP EBX / RET
//   result = FUN_0041c540(arg0, arg2);           // __thiscall(this=arg2, arg0)
//   notify(g_obj->field_0x19c);                  // via 0x0132987c object
//   dispatch(ptr, arg2_later, result, ...);       // CALL 0x004232f0 with 6 args
//   release(result);                             // CALL 0x004246f0

extern "C" __declspec(naked) void FUN_0041caa0() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x10]
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x74              // JZ +0x57   (→ POP EBX / RET)
        _emit 0x57
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x08]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [EAX*4 + 0xf595c4]
        _emit 0x3c
        _emit 0x85
        _emit 0xc4
        _emit 0x95
        _emit 0xf5
        _emit 0x00
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x74              // JZ +0x46   (→ POP EDI / POP EBX / RET)
        _emit 0x46
        _emit 0x56              // PUSH ESI
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, EBX
        _emit 0xcb
        _emit 0xe8              // CALL 0x0041c540   (rel32 = 0xfffffa7e)
        _emit 0x7e
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0xa1              // MOV EAX, [0x01329428]
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV ECX, dword ptr [EAX+0x19c]
        _emit 0x88
        _emit 0x9c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, dword ptr [0x0132987c]
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL 0x00423200   (rel32 = 0x00006722)
        _emit 0x22
        _emit 0x67
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x14]
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV ECX, dword ptr [0x0132987c]
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x56              // PUSH ESI
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x53              // PUSH EBX
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x52              // PUSH EDX
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL 0x004232f0   (rel32 = 0x000067fb)
        _emit 0xfb
        _emit 0x67
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL 0x004246f0   (rel32 = 0x00007bf5)
        _emit 0xf5
        _emit 0x7b
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x5e              // POP ESI
        _emit 0x5f              // POP EDI
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
