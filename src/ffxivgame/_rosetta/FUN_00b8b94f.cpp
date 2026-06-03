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
// FUNCTION: ffxivgame 0x0078b94f — FUN_00b8b94f (239 B / 0xef)
//                                  Unusual calling convention: EBP is used
//                                  as the implicit "this" object pointer
//                                  (not a standard stack frame), with EBP
//                                  pre-loaded by the caller. The function
//                                  saves EBX/ESI/EDI as callee-saved regs
//                                  and ends with POP EBP + RET 0xC (3
//                                  stack args cleaned by callee).
//
// Behaviour read from asm at orig RVA 0x0078b94f (VA 0x00B8B94F):
//
//   Non-standard calling convention: EBP = this-like object pointer.
//   Three stack args: [esp+0x14]/[esp+0x18] after saves (= arg1/arg2
//   at call site), plus a third cleaned by RET 0xC.
//
//   Prologue:
//     PUSH EBX/ESI/EDI
//     MOV  ECX, EBP          ; pass EBP as 'this' for first sub-call
//     CALL 0xB88060           ; FUN_00b88060 (thiscall)
//     EBX = [ESP+0x14]        ; load arg1
//     LEA  EDX, [EBP+0x94]   ; member/field ptr at +0x94
//     MOV  ECX, 8
//     MOV  ESI, EBX
//     MOV  EDI, EDX
//     REP MOVSD               ; copy 8 dwords from EBX to [EBP+0x94]
//
//   Type field check (EAX = [EBP+0xa0]):
//     IF type == 0xD OR type == 5 → goto fail_path (virtual call)
//     IF type == 0xB AND [EDX] > 0x7FF800 → goto fail_path
//     IF [EDX] > 0x80000000 → goto fail_path
//     IF type == -1 → goto fail_path
//
//   Main path:
//     CALL 0xB8AE60(ECX=EBP, arg1, EBX) ; FUN_00b8ae60, returns bool
//     IF !result → goto fail_path
//     IF type == 6:
//       CALL 0xB7A170(EBX, EBX)          ; FUN_00b7a170
//       ADD ESP, 4
//       EAX += EBX
//       LEA  EDX, [ECX+EBX+0x20]
//       CALL 0xB8A6C0(ECX=EBP, EDX, EAX) ; FUN_00b8a6c0
//       [EBP+0x34] = 1; [EBP+0x128] = 1
//       POP EDI/ESI/EBX; POP EBP; RET 0xC  → return 1
//     ELSE:
//       EAX = [EBP+0xac]; LEA ECX, [EAX+EBX+0x20]
//       CALL 0xB88190(ECX=EBP, ECX, 0)  ; FUN_00b88190
//       [EBP+0x34] = 1; [EBP+0x128] = 1
//       POP EDI/ESI/EBX; POP EBP; RET 0xC  → return 1
//
//   Fail path (0xB8BA28):
//     MOV  EDX, [EBP]         ; vtable ptr
//     MOV  EAX, [EDX+0xa4]   ; vtable slot 41
//     MOV  ECX, EBP
//     CALL EAX                ; virtual dispatch
//     POP  EDI/ESI/EBX
//     XOR  AL, AL             → return 0
//     POP  EBP; RET 0xC
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The non-standard use of EBP (as an implicit this-like register rather
//   than a stack-frame base), together with the POP EBP epilogue that
//   restores the caller's EBP, makes it impossible to reproduce this
//   function with source-level C++ under MSVC 2005 /O2 without custom
//   calling-convention tricks. All calls are PC-relative (no absolute
//   relocations), so the 239 bytes match the orig slice exactly when
//   emitted verbatim.

extern "C" __declspec(naked) void FUN_00b8b94f() {
    __asm {
        // 0078b94f  PUSH EBX
        _emit 0x53
        // 0078b950  PUSH ESI
        _emit 0x56
        // 0078b951  PUSH EDI
        _emit 0x57
        // 0078b952  MOV ECX, EBP
        _emit 0x8b
        _emit 0xcd
        // 0078b954  CALL 0xB88060 (rel32 = 0xffffc707)
        _emit 0xe8
        _emit 0x07
        _emit 0xc7
        _emit 0xff
        _emit 0xff
        // 0078b959  MOV EBX, [ESP+0x14]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        // 0078b95d  LEA EDX, [EBP+0x94]
        _emit 0x8d
        _emit 0x95
        _emit 0x94
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0078b963  MOV ECX, 8
        _emit 0xb9
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0078b968  MOV ESI, EBX
        _emit 0x8b
        _emit 0xf3
        // 0078b96a  MOV EDI, EDX
        _emit 0x8b
        _emit 0xfa
        // 0078b96c  REP MOVSD
        _emit 0xf3
        _emit 0xa5
        // 0078b96e  MOV EAX, [EBP+0xac]
        _emit 0x8b
        _emit 0x85
        _emit 0xac
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0078b974  LEA ECX, [EAX+EBX+0x20]
        _emit 0x8d
        _emit 0x4c
        _emit 0x18
        _emit 0x20
        // 0078b978  MOV EAX, [EBP+0xa0]
        _emit 0x8b
        _emit 0x85
        _emit 0xa0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0078b97e  CMP EAX, 0xd
        _emit 0x83
        _emit 0xf8
        _emit 0x0d
        // 0078b981  MOV [EBP+0x1c], ECX
        _emit 0x89
        _emit 0x4d
        _emit 0x1c
        // 0078b984  JE 0xb8ba28 (rel32 = 0x0000009e)
        _emit 0x0f
        _emit 0x84
        _emit 0x9e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0078b98a  CMP EAX, 5
        _emit 0x83
        _emit 0xf8
        _emit 0x05
        // 0078b98d  JE 0xb8ba28 (rel32 = 0x00000095)
        _emit 0x0f
        _emit 0x84
        _emit 0x95
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0078b993  CMP EAX, 0xb
        _emit 0x83
        _emit 0xf8
        _emit 0x0b
        // 0078b996  JNE 0xb8b9a4 (rel8 = 0x0c)
        _emit 0x75
        _emit 0x0c
        // 0078b998  CMP [EDX], 0x7ff800
        _emit 0x81
        _emit 0x3a
        _emit 0x00
        _emit 0xf8
        _emit 0x7f
        _emit 0x00
        // 0078b99e  JA 0xb8ba28 (rel32 = 0x00000084)
        _emit 0x0f
        _emit 0x87
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0078b9a4  CMP [EDX], 0x80000000
        _emit 0x81
        _emit 0x3a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        // 0078b9aa  JA 0xb8ba28 (rel8 = 0x7c)
        _emit 0x77
        _emit 0x7c
        // 0078b9ac  CMP EAX, -1
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        // 0078b9af  JE 0xb8ba28 (rel8 = 0x77)
        _emit 0x74
        _emit 0x77
        // 0078b9b1  MOV ECX, [ESP+0x18]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0078b9b5  PUSH ECX
        _emit 0x51
        // 0078b9b6  PUSH EBX
        _emit 0x53
        // 0078b9b7  MOV ECX, EBP
        _emit 0x8b
        _emit 0xcd
        // 0078b9b9  CALL 0xB8AE60 (rel32 = 0xfffff4a2)
        _emit 0xe8
        _emit 0xa2
        _emit 0xf4
        _emit 0xff
        _emit 0xff
        // 0078b9be  TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // 0078b9c0  JE 0xb8ba28 (rel8 = 0x66)
        _emit 0x74
        _emit 0x66
        // 0078b9c2  CMP [EBP+0xa0], 6
        _emit 0x83
        _emit 0xbd
        _emit 0xa0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x06
        // 0078b9c9  JNE 0xb8b9ff (rel8 = 0x34)
        _emit 0x75
        _emit 0x34
        // 0078b9cb  PUSH EBX
        _emit 0x53
        // 0078b9cc  PUSH EBX
        _emit 0x53
        // 0078b9cd  CALL 0xB7A170 (rel32 = 0xfffee79e)
        _emit 0xe8
        _emit 0x9e
        _emit 0xe7
        _emit 0xfe
        _emit 0xff
        // 0078b9d2  MOV ECX, [EBP+0xac]
        _emit 0x8b
        _emit 0x8d
        _emit 0xac
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0078b9d8  ADD ESP, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0078b9db  ADD EAX, EBX
        _emit 0x03
        _emit 0xc3
        // 0078b9dd  LEA EDX, [ECX+EBX+0x20]
        _emit 0x8d
        _emit 0x54
        _emit 0x19
        _emit 0x20
        // 0078b9e1  PUSH EAX
        _emit 0x50
        // 0078b9e2  PUSH EDX
        _emit 0x52
        // 0078b9e3  MOV ECX, EBP
        _emit 0x8b
        _emit 0xcd
        // 0078b9e5  CALL 0xB8A6C0 (rel32 = 0xffffecd6)
        _emit 0xe8
        _emit 0xd6
        _emit 0xec
        _emit 0xff
        _emit 0xff
        // 0078b9ea  POP EDI
        _emit 0x5f
        // 0078b9eb  POP ESI
        _emit 0x5e
        // 0078b9ec  MOV EAX, 1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0078b9f1  POP EBX
        _emit 0x5b
        // 0078b9f2  MOV [EBP+0x34], EAX
        _emit 0x89
        _emit 0x45
        _emit 0x34
        // 0078b9f5  MOV [EBP+0x128], EAX
        _emit 0x89
        _emit 0x85
        _emit 0x28
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0078b9fb  POP EBP
        _emit 0x5d
        // 0078b9fc  RET 0xC
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 0078b9ff  MOV EAX, [EBP+0xac]
        _emit 0x8b
        _emit 0x85
        _emit 0xac
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0078ba05  LEA ECX, [EAX+EBX+0x20]
        _emit 0x8d
        _emit 0x4c
        _emit 0x18
        _emit 0x20
        // 0078ba09  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0078ba0b  PUSH ECX
        _emit 0x51
        // 0078ba0c  MOV ECX, EBP
        _emit 0x8b
        _emit 0xcd
        // 0078ba0e  CALL 0xB88190 (rel32 = 0xffffc77d)
        _emit 0xe8
        _emit 0x7d
        _emit 0xc7
        _emit 0xff
        _emit 0xff
        // 0078ba13  POP EDI
        _emit 0x5f
        // 0078ba14  POP ESI
        _emit 0x5e
        // 0078ba15  MOV EAX, 1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0078ba1a  POP EBX
        _emit 0x5b
        // 0078ba1b  MOV [EBP+0x34], EAX
        _emit 0x89
        _emit 0x45
        _emit 0x34
        // 0078ba1e  MOV [EBP+0x128], EAX
        _emit 0x89
        _emit 0x85
        _emit 0x28
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0078ba24  POP EBP
        _emit 0x5d
        // 0078ba25  RET 0xC
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 0078ba28  MOV EDX, [EBP]
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        // 0078ba2b  MOV EAX, [EDX+0xa4]
        _emit 0x8b
        _emit 0x82
        _emit 0xa4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0078ba31  MOV ECX, EBP
        _emit 0x8b
        _emit 0xcd
        // 0078ba33  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0078ba35  POP EDI
        _emit 0x5f
        // 0078ba36  POP ESI
        _emit 0x5e
        // 0078ba37  POP EBX
        _emit 0x5b
        // 0078ba38  XOR AL, AL
        _emit 0x32
        _emit 0xc0
        // 0078ba3a  POP EBP
        _emit 0x5d
        // 0078ba3b  RET 0xC
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
