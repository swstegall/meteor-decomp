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
// FUNCTION: ffxivgame 0x00017450 — constructor-like thiscall, passes 6 args
//                                   to FUN_00417040 then stamps vtable 0x00f5787c
//                                   (__thiscall, 6 stack args, RET 0x18, 52 B / 0x34)
//
// Pattern: saves ESI (= this), pre-loads arg6 and arg4 into EAX/EDX before
// adjusting the stack, then reconstructs all six caller args and pushes them
// (right-to-left) onto the stack for the FUN_00417040 thiscall.  After
// FUN_00417040 returns it stamps the derived-class vtable pointer at [this+0x00]
// and returns this in EAX.  Identical shape to the sibling FUN_00417420 except
// that all six args are forwarded from the caller rather than being four
// immediate literals plus two params.
//
// Calling convention: __thiscall — ECX = this, six stack args, RET 0x18.
// Frame: none (/Oy).
//
// Asm (52 bytes @ orig RVA 0x00017450):
//   8b 44 24 18           MOV EAX,[ESP+0x18]        ; cache arg6 before PUSH ESI
//   8b 54 24 10           MOV EDX,[ESP+0x10]        ; cache arg4 before PUSH ESI
//   56                    PUSH ESI                   ; callee-save
//   50                    PUSH EAX                   ; push arg6  (callee slot +0x18)
//   8b 44 24 14           MOV EAX,[ESP+0x14]        ; reload arg3 (shifted by 2 pushes)
//   8b f1                 MOV ESI,ECX               ; ESI = this
//   8b 4c 24 1c           MOV ECX,[ESP+0x1c]        ; reload arg5 (shifted)
//   51                    PUSH ECX                   ; push arg5  (callee slot +0x14)
//   8b 4c 24 14           MOV ECX,[ESP+0x14]        ; reload arg2 (shifted)
//   52                    PUSH EDX                   ; push arg4  (callee slot +0x10)
//   8b 54 24 14           MOV EDX,[ESP+0x14]        ; reload arg1 (shifted)
//   50                    PUSH EAX                   ; push arg3  (callee slot +0x0c)
//   51                    PUSH ECX                   ; push arg2  (callee slot +0x08)
//   52                    PUSH EDX                   ; push arg1  (callee slot +0x04)
//   8b ce                 MOV ECX,ESI               ; ECX = this (thiscall)
//   e8 c8 fb ff ff        CALL FUN_00417040          ; base-class ctor (RET 0x18)
//   c7 06 7c 78 f5 00     MOV dword ptr [ESI],0x00f5787c ; stamp vtable
//   8b c6                 MOV EAX,ESI               ; return this
//   5e                    POP ESI
//   c2 18 00              RET 0x18                   ; callee-cleans 6 stack args

extern "C" void FUN_00417040();   // forward declaration for the CALL relocation

extern "C" __declspec(naked) void FUN_00417450() {
    __asm {
        // 00017450: 8b 44 24 18   MOV EAX,[ESP+0x18]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 00017454: 8b 54 24 10   MOV EDX,[ESP+0x10]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 00017458: 56            PUSH ESI
        _emit 0x56
        // 00017459: 50            PUSH EAX
        _emit 0x50
        // 0001745a: 8b 44 24 14   MOV EAX,[ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0001745e: 8b f1         MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 00017460: 8b 4c 24 1c   MOV ECX,[ESP+0x1c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 00017464: 51            PUSH ECX
        _emit 0x51
        // 00017465: 8b 4c 24 14   MOV ECX,[ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00017469: 52            PUSH EDX
        _emit 0x52
        // 0001746a: 8b 54 24 14   MOV EDX,[ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0001746e: 50            PUSH EAX
        _emit 0x50
        // 0001746f: 51            PUSH ECX
        _emit 0x51
        // 00017470: 52            PUSH EDX
        _emit 0x52
        // 00017471: 8b ce         MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00017473: e8 c8 fb ff ff  CALL FUN_00417040  (reloc — REL32)
        call FUN_00417040
        // 00017478: c7 06 7c 78 f5 00  MOV dword ptr [ESI],0x00f5787c
        _emit 0xc7
        _emit 0x06
        _emit 0x7c
        _emit 0x78
        _emit 0xf5
        _emit 0x00
        // 0001747e: 8b c6         MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 00017480: 5e            POP ESI
        _emit 0x5e
        // 00017481: c2 18 00      RET 0x18
        _emit 0xc2
        _emit 0x18
        _emit 0x00
    }
}
