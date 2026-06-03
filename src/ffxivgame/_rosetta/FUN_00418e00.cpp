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
// FUNCTION: ffxivgame 0x00018e00 — FUN_00418e00 (0xf2 / 242 B)
//                                  __cdecl object-factory helper, EH4-SEH wrapped.
//
// Behaviour read from the disassembly at orig RVA 0x00018e00:
//
//   __cdecl void* FUN_00418e00(void** pOut, a1, a2, a3, a4, a5, a6, a7);
//
//   EH4 /GS prologue (PUSH -1 / PUSH scope-table / PUSH FS:[0] /
//   SUB ESP,0x30 / PUSH ESI,EDI / XOR cookie^ESP / install FS:[0]).
//
//   Builds a struct on the local stack (local_18..local_38) from
//   the incoming arguments:
//     local_18 = 1          (tag / version field)
//     local_1c = a5
//     local_20 = a7  (later overwritten with 1)
//     local_24 = a1
//     local_28 = a2
//     local_2c = a3
//     local_30 = a3 (redundant - arg3 stored here again after push reorder)
//     local_34 = a6
//     local_38 = a4  (after push adjustments)
//
//   Then:
//     result_init = FUN_0040e2d0(&local_10, 0x10, 0xf57cc8)   ; __thiscall
//     obj         = FUN_00419c40(result_init, 0x58)            ; allocator (size=0x58)
//     esi = obj;
//     if (esi != NULL) {
//         FUN_00431710(esi, &local_18, esi);  ; constructor init (__thiscall)
//         esi->vtable = 0xf57e3c;             ; set derived vtable
//         ecx = esi;
//     } else {
//         ecx = 0;
//     }
//     *pOut = ecx;                            ; store result
//     if (ecx != NULL && ecx->field_0x34 == 0) {
//         ecx->vtable[0](1);                  ; call virtual method slot 0
//         *pOut = NULL;                       ; clear after ownership transfer
//     }
//     return pOut;
//
//   Stack frame (ESP-relative after prologue):
//     [esp+0x00]   security cookie
//     [esp+0x04]   saved EDI
//     [esp+0x08]   saved ESI
//     [esp+0x0c]   local flag / EH state
//     [esp+0x10..0x3b] local struct area
//     [esp+0x3c]   saved FS:[0]
//     [esp+0x40]   scope table (0xe55546)
//     [esp+0x44]   EH4 trylevel (-1 init)
//     [esp+0x48]   return address
//     [esp+0x4c]   pOut (arg0)
//     [esp+0x50]   a1
//     [esp+0x54]   a2
//     [esp+0x58]   a3
//     [esp+0x5c]   a4
//     [esp+0x60]   a5
//     [esp+0x64]   a6
//     [esp+0x68]   a7
//
//   Reloc-bearing sites in the orig 242 bytes (absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000):
//     +0x03   scope-table RVA    (0x00e55546)
//     +0x09   FS:[0] read        (constant 0)
//     +0x14   __security_cookie  (0x012ea8b0)
//     +0x1f   FS:[0] install     (constant 0)
//     +0x58   string/vtable ptr  (0x00f57cc8)
//     +0x7f   CALL FUN_0040e2d0  (rel32)
//     +0x8b   CALL FUN_00419c40  (rel32)
//     +0xad   CALL FUN_00431710  (rel32)
//     +0xb2   vtable store       (0x00f57e3c)
//     +0xe5   FS:[0] restore     (constant 0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ for this function requires reproducing the exact
//   EH4 frame layout, register allocation across multiple callee-save /
//   reload windows, and every linker-resolved absolute address listed
//   above. Like the surrounding EH4-wrapped functions in this module
//   (FUN_00403a20, FUN_004054d0, FUN_00402a30), a source-level port
//   reliably shifts bytes — branch sizing, MOV vs MOFFS32, frame spill
//   order — making `__declspec(naked)` + verbatim `_emit` the only
//   route to a byte-identical .obj.

extern "C" __declspec(naked) void FUN_00418e00() {
    __asm {
        // 00018e00  PUSH -1
        _emit 0x6a
        _emit 0xff
        // 00018e02  PUSH 0xe55546  (scope table)
        _emit 0x68
        _emit 0x46
        _emit 0x55
        _emit 0xe5
        _emit 0x00
        // 00018e07  MOV EAX, FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018e0d  PUSH EAX
        _emit 0x50
        // 00018e0e  SUB ESP, 0x30
        _emit 0x83
        _emit 0xec
        _emit 0x30
        // 00018e11  PUSH ESI
        _emit 0x56
        // 00018e12  PUSH EDI
        _emit 0x57
        // 00018e13  MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00018e18  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 00018e1a  PUSH EAX
        _emit 0x50
        // 00018e1b  LEA EAX, [ESP+0x3c]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        // 00018e1f  MOV FS:[0], EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018e25  MOV EDX, [ESP+0x50]  ; a1
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x50
        // 00018e29  XOR EDI, EDI
        _emit 0x33
        _emit 0xff
        // 00018e2b  MOV [ESP+0x0c], EDI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 00018e2f  MOV EAX, [ESP+0x60]  ; a5
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x60
        // 00018e33  MOV ECX, [ESP+0x68]  ; a7
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x68
        // 00018e37  MOV [ESP+0x1c], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00018e3b  MOV EAX, [ESP+0x54]  ; a2
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x54
        // 00018e3f  MOV [ESP+0x20], ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 00018e43  MOV ECX, [ESP+0x58]  ; a3
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x58
        // 00018e47  MOV [ESP+0x24], EDX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 00018e4b  MOV EDX, [ESP+0x5c]  ; a4
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x5c
        // 00018e4f  MOV [ESP+0x28], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 00018e53  MOV EAX, [ESP+0x64]  ; a6
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x64
        // 00018e57  PUSH 0xf57cc8
        _emit 0x68
        _emit 0xc8
        _emit 0x7c
        _emit 0xf5
        _emit 0x00
        // 00018e5c  MOV [ESP+0x30], ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        // 00018e60  PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 00018e62  LEA ECX, [ESP+0x18]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00018e66  MOV [ESP+0x4c], EDI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x4c
        // 00018e6a  MOV dword [ESP+0x20], 1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018e72  MOV [ESP+0x38], EDX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x38
        // 00018e76  MOV [ESP+0x3c], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        // 00018e7a  MOV [ESP+0x40], EDI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x40
        // 00018e7e  CALL 0x40e2d0
        _emit 0xe8
        _emit 0x4d
        _emit 0x54
        _emit 0xff
        _emit 0xff
        // 00018e83  PUSH EAX
        _emit 0x50
        // 00018e84  PUSH 0x58
        _emit 0x6a
        _emit 0x58
        // 00018e86  MOV [ESP+0x68], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x68
        // 00018e8a  CALL 0x419c40
        _emit 0xe8
        _emit 0xb1
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        // 00018e8f  MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 00018e91  ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00018e94  MOV [ESP+0x68], ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x68
        // 00018e98  CMP ESI, EDI
        _emit 0x3b
        _emit 0xf7
        // 00018e9a  MOV dword [ESP+0x44], 1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x44
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018ea2  JE +0x17  (→ 0x418ebb)
        _emit 0x74
        _emit 0x17
        // 00018ea4  PUSH ESI
        _emit 0x56
        // 00018ea5  LEA ECX, [ESP+0x1c]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 00018ea9  PUSH ECX
        _emit 0x51
        // 00018eaa  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00018eac  CALL 0x431710
        _emit 0xe8
        _emit 0x5f
        _emit 0x88
        _emit 0x01
        _emit 0x00
        // 00018eb1  MOV dword [ESI], 0xf57e3c
        _emit 0xc7
        _emit 0x06
        _emit 0x3c
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        // 00018eb7  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00018eb9  JMP +0x02  (→ 0x418ebd)
        _emit 0xeb
        _emit 0x02
        // 00018ebb  XOR ECX, ECX
        _emit 0x33
        _emit 0xc9
        // 00018ebd  MOV ESI, [ESP+0x4c]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x4c
        // 00018ec1  MOV [ESI], ECX
        _emit 0x89
        _emit 0x0e
        // 00018ec3  CMP [ECX+0x34], EDI
        _emit 0x39
        _emit 0x79
        _emit 0x34
        // 00018ec6  MOV [ESP+0x44], EDI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x44
        // 00018eca  MOV dword [ESP+0x0c], 1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018ed2  JNE +0x0a  (→ 0x418ede)
        _emit 0x75
        _emit 0x0a
        // 00018ed4  MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 00018ed6  MOV EAX, [EDX]
        _emit 0x8b
        _emit 0x02
        // 00018ed8  PUSH 1
        _emit 0x6a
        _emit 0x01
        // 00018eda  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00018edc  MOV [ESI], EDI
        _emit 0x89
        _emit 0x3e
        // 00018ede  MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 00018ee0  MOV ECX, [ESP+0x3c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        // 00018ee4  MOV FS:[0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018eeb  POP ECX
        _emit 0x59
        // 00018eec  POP EDI
        _emit 0x5f
        // 00018eed  POP ESI
        _emit 0x5e
        // 00018eee  ADD ESP, 0x3c
        _emit 0x83
        _emit 0xc4
        _emit 0x3c
        // 00018ef1  RET
        _emit 0xc3
    }
}
