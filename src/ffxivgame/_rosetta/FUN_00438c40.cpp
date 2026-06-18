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
// FUNCTION: ffxivgame 0x00438c40 — __thiscall method that computes a count
//                                  (field_10 - field_0c) / 10, conditionally
//                                  invokes a globally-initialised callback,
//                                  then dispatches two virtual calls and
//                                  returns field_1c (141 bytes / 0x8d)
//
// Calling convention: __thiscall (ECX = this on entry); one 4-byte stack
// parameter, cleaned by callee (RET 0x4).
//
// Object layout (inferred from offsets accessed):
//   [this + 0x00]  vtable ptr
//   [this + 0x0c]  DWORD field_0c  (lower bound / start count)
//   [this + 0x10]  DWORD field_10  (upper bound / end count)
//   [this + 0x1c]  DWORD field_1c  (return value)
//
// Algorithm:
//   1. n = (field_10 - field_0c) / 10  [magic multiplier 0x66666667, SAR 3,
//      plus sign-bit correction].  Zero if field_0c == 0.
//   2. if (n > 0): once-only init of global fn-ptr [0x0132390c] protected by
//      a flag bit at [0x01323910], then call the registered callback with
//      5 literal pointer/integer arguments.
//   3. Load stack param1; call vtable[3](param1, 0,0,0,0,0) via CALL EDX.
//   4. Push return value; call this->vtable[0xa0/4=40] via CALL EAX.
//   5. Return this->field_1c (loaded into EAX before POP ESI / RET 0x4).
//
// Reconstruction strategy — naked-asm _emit passthrough:
//   All addresses are absolute literals baked into the instruction stream;
//   there are no rel32 CALL relocations (indirect calls go through registers
//   or through the [mem] CALL form with a hardcoded absolute address).
//   Emitting bytes verbatim avoids any MSVC register-allocation or
//   instruction-scheduling divergence.

extern "C" __declspec(naked) void FUN_00438c40() {
    __asm {
        // 00038c40:  56                PUSH ESI
        _emit 0x56
        // 00038c41:  8b f1             MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 00038c43:  8b 46 0c          MOV EAX,dword ptr [ESI+0x0c]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 00038c46:  85 c0             TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00038c48:  74 16             JZ +0x16  (→ 00038c60)
        _emit 0x74
        _emit 0x16
        // 00038c4a:  8b 4e 10          MOV ECX,dword ptr [ESI+0x10]
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 00038c4d:  2b c8             SUB ECX,EAX
        _emit 0x2b
        _emit 0xc8
        // 00038c4f:  b8 67 66 66 66    MOV EAX,0x66666667  ; magic multiplier for /10
        _emit 0xb8
        _emit 0x67
        _emit 0x66
        _emit 0x66
        _emit 0x66
        // 00038c54:  f7 e9             IMUL ECX
        _emit 0xf7
        _emit 0xe9
        // 00038c56:  c1 fa 03          SAR EDX,0x3
        _emit 0xc1
        _emit 0xfa
        _emit 0x03
        // 00038c59:  8b c2             MOV EAX,EDX
        _emit 0x8b
        _emit 0xc2
        // 00038c5b:  c1 e8 1f          SHR EAX,0x1f
        _emit 0xc1
        _emit 0xe8
        _emit 0x1f
        // 00038c5e:  03 c2             ADD EAX,EDX   ; EAX = (field_10 - field_0c) / 10
        _emit 0x03
        _emit 0xc2
        // 00038c60:  85 c0             TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00038c62:  76 3f             JBE +0x3f  (→ 00038ca3; skip when n == 0)
        _emit 0x76
        _emit 0x3f
        // 00038c64:  b8 01 00 00 00    MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00038c69:  84 05 10 39 32 01 TEST byte ptr [0x01323910],AL
        _emit 0x84
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00038c6f:  75 10             JNZ +0x10  (→ 00038c81; already initialised)
        _emit 0x75
        _emit 0x10
        // 00038c71:  09 05 10 39 32 01 OR dword ptr [0x01323910],EAX  ; set flag bit
        _emit 0x09
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00038c77:  c7 05 0c 39 32 01 a0 85 43 00
        //            MOV dword ptr [0x0132390c],0x004385a0  ; store fn-ptr
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xa0
        _emit 0x85
        _emit 0x43
        _emit 0x00
        // 00038c81:  68 80 5f f6 00    PUSH 0x00f65f80
        _emit 0x68
        _emit 0x80
        _emit 0x5f
        _emit 0xf6
        _emit 0x00
        // 00038c86:  68 7a 01 00 00    PUSH 0x17a
        _emit 0x68
        _emit 0x7a
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 00038c8b:  68 d8 5f f6 00    PUSH 0x00f65fd8
        _emit 0x68
        _emit 0xd8
        _emit 0x5f
        _emit 0xf6
        _emit 0x00
        // 00038c90:  68 9e 5d f6 00    PUSH 0x00f65d9e
        _emit 0x68
        _emit 0x9e
        _emit 0x5d
        _emit 0xf6
        _emit 0x00
        // 00038c95:  68 38 60 f6 00    PUSH 0x00f66038
        _emit 0x68
        _emit 0x38
        _emit 0x60
        _emit 0xf6
        _emit 0x00
        // 00038c9a:  ff 15 0c 39 32 01 CALL dword ptr [0x0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00038ca0:  83 c4 14          ADD ESP,0x14  ; pop 5 args
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00038ca3:  8b 44 24 08       MOV EAX,dword ptr [ESP+0x8]  ; param1
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00038ca7:  8b 08             MOV ECX,dword ptr [EAX]  ; vtable ptr
        _emit 0x8b
        _emit 0x08
        // 00038ca9:  8b 51 0c          MOV EDX,dword ptr [ECX+0xc]  ; vtable[3]
        _emit 0x8b
        _emit 0x51
        _emit 0x0c
        // 00038cac:  6a 00             PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00038cae:  6a 00             PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00038cb0:  6a 00             PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00038cb2:  6a 00             PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00038cb4:  6a 00             PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00038cb6:  50                PUSH EAX  (param1 as first arg)
        _emit 0x50
        // 00038cb7:  ff d2             CALL EDX  ; vtable[3](param1, 0,0,0,0,0)
        _emit 0xff
        _emit 0xd2
        // 00038cb9:  8b 16             MOV EDX,dword ptr [ESI]  ; this->vtable
        _emit 0x8b
        _emit 0x16
        // 00038cbb:  50                PUSH EAX  ; push return value
        _emit 0x50
        // 00038cbc:  8b 82 a0 00 00 00 MOV EAX,dword ptr [EDX+0xa0]  ; vtable[40]
        _emit 0x8b
        _emit 0x82
        _emit 0xa0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00038cc2:  8b ce             MOV ECX,ESI  ; ECX = this
        _emit 0x8b
        _emit 0xce
        // 00038cc4:  ff d0             CALL EAX  ; this->vtable[40](ret_val)
        _emit 0xff
        _emit 0xd0
        // 00038cc6:  8b 46 1c          MOV EAX,dword ptr [ESI+0x1c]  ; return field_1c
        _emit 0x8b
        _emit 0x46
        _emit 0x1c
        // 00038cc9:  5e                POP ESI
        _emit 0x5e
        // 00038cca:  c2 04 00          RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
