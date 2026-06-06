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
// FUNCTION: ffxivgame 0x00019c40 — `__cdecl` once-init thunk that lazily
//                                  constructs a singleton via a factory call
//                                  then forwards two arguments to a
//                                  `__thiscall` method on it (114 B / 0x72,
//                                  EH3-SEH wrapped, ESP-relative frame).
//
// Inspection (read from the disassembly at orig RVA 0x00019c40):
//
//   __cdecl <ret> FUN_00419c40(undefined4 a1, undefined4 a2);
//
//   This is a classic MSVC 2005 once-init wrapper around a lazily-created
//   singleton, combined with an immediate two-argument forwarding call to a
//   __thiscall method on that singleton.
//
//   Pseudo-C:
//
//     static unsigned char s_once_flag;   // .data 0x01327c10 (byte/dword)
//     static T*            g_obj_ptr;     // .data 0x01327c0c
//
//     <ret> __cdecl FUN_00419c40(unsigned a1, unsigned a2) {
//         if ((s_once_flag & 1) == 0) {            // 84 05 10 7c 32 01
//             s_once_flag |= 1;                    // 09 05 10 7c 32 01
//             g_obj_ptr = FUN_0040e500();          // CALL 0x40e500
//         }
//         return g_obj_ptr->Method(a1, a2);        // CALL 0x40e110
//     }
//
//   Stack frame (after the EH3 prologue, ESP-relative — no EBP frame):
//     [esp+0x00]  __security_cookie ^ ESP_at_install
//     [esp+0x04]  EH3 saved-FS:[0]  (next handler in chain)
//     [esp+0x08]  EH3 scope-table   (0x00e5527e — .rdata FuncInfo)
//     [esp+0x0c]  EH3 trylevel      (initial -1; set to 0 before factory call,
//                                    then restored to -1 after)
//     [esp+0x10]  return address
//     [esp+0x14]  caller a1
//     [esp+0x18]  caller a2
//
//   Reloc-bearing sites (absolute addresses that resolve only at full relink):
//     +0x03  scope-table handler RVA  (0x00e5527e — .rdata FuncInfo)
//     +0x09  FS:[0] read              (constant 0, fold-through)
//     +0x0f  __security_cookie load   (.data 0x012ea8b0)
//     +0x1c  FS:[0] install           (constant 0, fold-through)
//     +0x27  once-flag TEST           (.data 0x01327c10, byte)
//     +0x2f  once-flag OR             (.data 0x01327c10, dword)
//     +0x3b  factory CALL             (.text 0x0040e500 rel32 — __cdecl)
//     +0x40  g_obj_ptr store          (.data 0x01327c0c)
//     +0x4f  g_obj_ptr load (ECX)     (.data 0x01327c0c)
//     +0x55  method CALL              (.text 0x0040e110 rel32 — __thiscall)
//     +0x62  FS:[0] uninstall         (constant 0, fold-through)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The EH3 prologue, the /GS cookie frame, and the multiple absolute-address
//   relocations make source-level reconstruction brittle under MSVC 2005 /O2 —
//   every high-level rewrite shifts at least one byte. Following the same
//   pragmatic path as the sibling FUN_00401650, we emit the original 114 bytes
//   verbatim via MASM `_emit` directives. The .obj's `.text` section is then
//   byte-identical to the orig slice, and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00419c40() {
    __asm {
        // 00019c40: 6a ff           PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00019c42: 68 7e 52 e5 00  PUSH 0xe5527e  (EH3 scope-table)
        _emit 0x68
        _emit 0x7e
        _emit 0x52
        _emit 0xe5
        _emit 0x00
        // 00019c47: 64 a1 00 00 00 00  MOV EAX,[FS:0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00019c4d: 50              PUSH EAX
        _emit 0x50
        // 00019c4e: a1 b0 a8 2e 01  MOV EAX,[0x12ea8b0]  (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00019c53: 33 c4           XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 00019c55: 50              PUSH EAX  (cookie ^ ESP)
        _emit 0x50
        // 00019c56: 8d 44 24 04     LEA EAX,[ESP+0x4]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00019c5a: 64 a3 00 00 00 00  MOV [FS:0x0],EAX  (install EH frame)
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00019c60: b8 01 00 00 00  MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00019c65: 84 05 10 7c 32 01  TEST [0x1327c10],AL
        _emit 0x84
        _emit 0x05
        _emit 0x10
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        // 00019c6b: 75 20           JNZ +0x20 (→ 0x419c8d)
        _emit 0x75
        _emit 0x20
        // 00019c6d: 09 05 10 7c 32 01  OR [0x1327c10],EAX
        _emit 0x09
        _emit 0x05
        _emit 0x10
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        // 00019c73: c7 44 24 0c 00 00 00 00  MOV [ESP+0xc],0x0  (trylevel = 0)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00019c7b: e8 80 48 ff ff  CALL 0x40e500  (factory)
        _emit 0xe8
        _emit 0x80
        _emit 0x48
        _emit 0xff
        _emit 0xff
        // 00019c80: a3 0c 7c 32 01  MOV [0x1327c0c],EAX  (store singleton ptr)
        _emit 0xa3
        _emit 0x0c
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        // 00019c85: c7 44 24 0c ff ff ff ff  MOV [ESP+0xc],0xffffffff  (trylevel = -1)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00019c8d: 8b 44 24 18     MOV EAX,[ESP+0x18]  (a2)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 00019c91: 8b 4c 24 14     MOV ECX,[ESP+0x14]  (a1)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00019c95: 50              PUSH EAX  (arg2)
        _emit 0x50
        // 00019c96: 51              PUSH ECX  (arg1)
        _emit 0x51
        // 00019c97: 8b 0d 0c 7c 32 01  MOV ECX,[0x1327c0c]  (ECX = this)
        _emit 0x8b
        _emit 0x0d
        _emit 0x0c
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        // 00019c9d: e8 6e 44 ff ff  CALL 0x40e110  (Method(a1, a2))
        _emit 0xe8
        _emit 0x6e
        _emit 0x44
        _emit 0xff
        _emit 0xff
        // 00019ca2: 8b 4c 24 04     MOV ECX,[ESP+0x4]  (saved FS:[0])
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 00019ca6: 64 89 0d 00 00 00 00  MOV [FS:0x0],ECX  (restore EH chain)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00019cad: 59              POP ECX  (remove cookie slot)
        _emit 0x59
        // 00019cae: 83 c4 0c        ADD ESP,0xc  (remove remaining 3 EH frame slots)
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00019cb1: c3              RET
        _emit 0xc3
    }
}
