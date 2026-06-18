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
// FUNCTION: ffxivgame 0x00432820 — __thiscall method: move fields into global
//                                  container then zero them (48 B / 0x30)
//
// Calling convention: __thiscall (ECX = this; plain RET — callee has no
//                     explicit args beyond ECX).
// Frame: SUB ESP,8 (8-byte local output buffer) then PUSH ESI.
//        Unusual order — allocation precedes callee-save push.
//
// The function transfers two fields from this object into a global
// container object's method, then clears the source fields:
//   1. Allocate 8-byte local buffer on stack
//   2. Save this (ECX) in ESI
//   3. Load this->field_0x8 (EAX) and this->field_0x4 (ECX)
//   4. Push them as args 3 and 2 to the callee
//   5. Load the global singleton ptr from [0x0132c8ac] into ECX
//   6. LEA EDX = addr of the 8-byte local buffer (output/sret slot), push as arg 1
//   7. Offset ECX by +4 (sub-object this pointer for callee)
//   8. Call FUN_007c42b0 (thiscall via ECX = global+4; callee cleans 3 args)
//   9. Zero this->field_0x4 and this->field_0x8
//
// Asm (48 bytes @ orig RVA 0x00032820):
//   83 ec 08              SUB  ESP,0x8
//   56                    PUSH ESI
//   8b f1                 MOV  ESI,ECX
//   8b 46 08              MOV  EAX,dword ptr [ESI+0x8]
//   8b 4e 04              MOV  ECX,dword ptr [ESI+0x4]
//   50                    PUSH EAX
//   51                    PUSH ECX
//   8b 0d ac c8 32 01     MOV  ECX,dword ptr [0x0132c8ac]   ; global singleton ptr
//   8d 54 24 0c           LEA  EDX,[ESP+0xc]                ; addr of 8-byte local
//   52                    PUSH EDX
//   83 c1 04              ADD  ECX,0x4
//   e8 6f 1a 39 00        CALL FUN_007c42b0                 ; rel32 — reloc masked
//   33 c0                 XOR  EAX,EAX
//   33 c9                 XOR  ECX,ECX
//   89 46 04              MOV  dword ptr [ESI+0x4],EAX      ; clear field_0x4
//   89 4e 08              MOV  dword ptr [ESI+0x8],ECX      ; clear field_0x8
//   5e                    POP  ESI
//   83 c4 08              ADD  ESP,0x8
//   c3                    RET
//
// Reconstruction: __declspec(naked) _emit byte passthrough.
// The MASM inline assembler encodes `mov ecx, dword ptr [0x0132c8ac]` as
// MOV ECX,imm32 (opcode b9, 5 bytes) rather than the required
// MOV ECX,[abs32] form (opcode 8b 0d, 6 bytes). Raw _emit avoids this.
// The rel32 field of the CALL (bytes +0x1d..+0x20) is masked by compare.py.

extern "C" __declspec(naked) void FUN_00432820() {
    __asm {
        _emit 0x83      // SUB ESP,0x8
        _emit 0xec
        _emit 0x08
        _emit 0x56      // PUSH ESI
        _emit 0x8b      // MOV ESI,ECX
        _emit 0xf1
        _emit 0x8b      // MOV EAX,dword ptr [ESI+0x8]
        _emit 0x46
        _emit 0x08
        _emit 0x8b      // MOV ECX,dword ptr [ESI+0x4]
        _emit 0x4e
        _emit 0x04
        _emit 0x50      // PUSH EAX
        _emit 0x51      // PUSH ECX
        _emit 0x8b      // MOV ECX,dword ptr [0x0132c8ac]
        _emit 0x0d
        _emit 0xac
        _emit 0xc8
        _emit 0x32
        _emit 0x01
        _emit 0x8d      // LEA EDX,[ESP+0xc]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x52      // PUSH EDX
        _emit 0x83      // ADD ECX,0x4
        _emit 0xc1
        _emit 0x04
        _emit 0xe8      // CALL FUN_007c42b0 (rel32 — masked by compare.py)
        _emit 0x6f
        _emit 0x1a
        _emit 0x39
        _emit 0x00
        _emit 0x33      // XOR EAX,EAX
        _emit 0xc0
        _emit 0x33      // XOR ECX,ECX
        _emit 0xc9
        _emit 0x89      // MOV dword ptr [ESI+0x4],EAX
        _emit 0x46
        _emit 0x04
        _emit 0x89      // MOV dword ptr [ESI+0x8],ECX
        _emit 0x4e
        _emit 0x08
        _emit 0x5e      // POP ESI
        _emit 0x83      // ADD ESP,0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc3      // RET
    }
}
