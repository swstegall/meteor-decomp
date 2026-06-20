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
// FUNCTION: ffxivgame 0x00059070 — __thiscall bool, 0xad bytes
//                                   vtable-dispatch bind/unbind with
//                                   index tracking at this+0x18.
//
// __thiscall bool FUN_00459070(this)
//   this = ECX in (saved to EDI)
//
// Stack layout after prologue (SUB ESP,8; PUSH EBX; PUSH ESI; PUSH EDI):
//   [ESP+0x00]  saved EDI
//   [ESP+0x04]  saved ESI
//   [ESP+0x08]  saved EBX
//   [ESP+0x0c]  local_0c — COM-like out-ptr from first vtable[0] call
//   [ESP+0x10]  local_10 — COM-like out-ptr from second vtable[0] call
//   [ESP+0x14]  return address
//
// Behaviour:
//   ESI = &this->m18
//   if (this->m18 != -1) {
//       // Unbind old index: get obj from m10 vtable, call set-index slot,
//       // then release, reset index to -1.
//       vtable0 = this->m10->vtbl[0]
//       rc = vtable0(this->m10, 0x11088c0, &local_0c)
//       if (rc >= 0) {
//           local_0c->vtbl[4](local_0c, this->m18)   // set-index
//           local_0c->vtbl[2](local_0c)               // release
//           this->m18 = -1
//       }
//   }
//   // Bind new:
//   vtable0 = this->m10->vtbl[0]
//   BL = 0   (default return false)
//   rc = vtable0(this->m10, 0x11088c0, &local_10)
//   if (rc >= 0) {
//       rc2 = local_10->vtbl[3](local_10, 0x1108470, this, &this->m18)
//       if (rc2 >= 0) {
//           local_10->vtbl[2](local_10)               // release
//           BL = 1
//           return true
//       }
//       // rc2 < 0:
//       this->m18 = -1
//       local_10->vtbl[2](local_10)                   // release
//   }
//   return BL (false)
//
// Reloc-bearing sites (4-byte imm32 windows wildcarded by compare.py):
//   +0x1c  PUSH 0x11088c0  (first vtable[0] name/hash arg)
//   +0x55  PUSH 0x11088c0  (second vtable[0] name/hash arg)
//   +0x70  PUSH 0x1108470  (vtable[3] bind-key arg)

extern "C" __declspec(naked) void FUN_00459070() {
    __asm {
        // --- prologue ---
        _emit 0x83              // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        // CMP dword ptr [EDI+0x18], -1
        _emit 0x83
        _emit 0x7f
        _emit 0x18
        _emit 0xff
        // LEA ESI, [EDI+0x18]
        _emit 0x8d
        _emit 0x77
        _emit 0x18
        // JZ +0x39 (to label_ba)
        _emit 0x74
        _emit 0x39

        // --- first block (unbind old if m18 != -1) ---
        // MOV EAX, [EDI+0x10]
        _emit 0x8b
        _emit 0x47
        _emit 0x10
        // MOV ECX, [EAX]
        _emit 0x8b
        _emit 0x08
        // LEA EDX, [ESP+0xc]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // PUSH EDX
        _emit 0x52
        // PUSH 0x11088c0
        _emit 0x68
        _emit 0xc0
        _emit 0x88
        _emit 0x10
        _emit 0x01
        // PUSH EAX
        _emit 0x50
        // MOV EAX, [ECX]  (vtbl[0])
        _emit 0x8b
        _emit 0x01
        // CALL EAX
        _emit 0xff
        _emit 0xd0
        // TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // JL +0x21 (to label_ba)
        _emit 0x7c
        _emit 0x21

        // rc >= 0: call set-index + release
        // MOV EAX, [ESP+0xc]  (local_0c)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // MOV EDX, [ESI]  (this->m18)
        _emit 0x8b
        _emit 0x16
        // MOV ECX, [EAX]  (vtable)
        _emit 0x8b
        _emit 0x08
        // PUSH EDX  (this->m18)
        _emit 0x52
        // PUSH EAX  (local_0c)
        _emit 0x50
        // MOV EAX, [ECX+0x10]  (vtbl[4])
        _emit 0x8b
        _emit 0x41
        _emit 0x10
        // CALL EAX
        _emit 0xff
        _emit 0xd0
        // MOV EAX, [ESP+0xc]  (local_0c)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // MOV ECX, [EAX]  (vtable)
        _emit 0x8b
        _emit 0x08
        // MOV EDX, [ECX+0x8]  (vtbl[2] = release)
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        // PUSH EAX
        _emit 0x50
        // CALL EDX
        _emit 0xff
        _emit 0xd2
        // MOV dword ptr [ESI], 0xffffffff  (this->m18 = -1)
        _emit 0xc7
        _emit 0x06
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff

        // --- label_ba: bind new ---
        // MOV EAX, [EDI+0x10]
        _emit 0x8b
        _emit 0x47
        _emit 0x10
        // MOV ECX, [EAX]
        _emit 0x8b
        _emit 0x08
        // LEA EDX, [ESP+0x10]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // PUSH EDX
        _emit 0x52
        // PUSH 0x11088c0
        _emit 0x68
        _emit 0xc0
        _emit 0x88
        _emit 0x10
        _emit 0x01
        // PUSH EAX
        _emit 0x50
        // MOV EAX, [ECX]  (vtbl[0])
        _emit 0x8b
        _emit 0x01
        // XOR BL, BL  (default return = false)
        _emit 0x32
        _emit 0xdb
        // CALL EAX
        _emit 0xff
        _emit 0xd0
        // TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // JL +0x40 (to label_14)
        _emit 0x7c
        _emit 0x40

        // rc >= 0: try bind
        // MOV EAX, [ESP+0x10]  (local_10)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // MOV ECX, [EAX]  (vtable)
        _emit 0x8b
        _emit 0x08
        // MOV EDX, [ECX+0xc]  (vtbl[3])
        _emit 0x8b
        _emit 0x51
        _emit 0x0c
        // PUSH ESI  (&this->m18)
        _emit 0x56
        // PUSH EDI  (this)
        _emit 0x57
        // PUSH 0x1108470
        _emit 0x68
        _emit 0x70
        _emit 0x84
        _emit 0x10
        _emit 0x01
        // PUSH EAX
        _emit 0x50
        // CALL EDX
        _emit 0xff
        _emit 0xd2
        // TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // JL +0x17 (to label_02)
        _emit 0x7c
        _emit 0x17

        // bind succeeded: release + return true
        // MOV EAX, [ESP+0x10]  (local_10)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // MOV ECX, [EAX]  (vtable)
        _emit 0x8b
        _emit 0x08
        // MOV EDX, [ECX+0x8]  (vtbl[2] = release)
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        // PUSH EAX
        _emit 0x50
        // MOV BL, 1
        _emit 0xb3
        _emit 0x01
        // CALL EDX
        _emit 0xff
        _emit 0xd2
        // POP EDI
        _emit 0x5f
        // POP ESI
        _emit 0x5e
        // MOV AL, BL
        _emit 0x8a
        _emit 0xc3
        // POP EBX
        _emit 0x5b
        // ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // RET
        _emit 0xc3

        // --- label_02: bind failed — reset m18, release ---
        // MOV EAX, [ESP+0x10]  (local_10)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // MOV dword ptr [ESI], 0xffffffff  (this->m18 = -1)
        _emit 0xc7
        _emit 0x06
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // MOV ECX, [EAX]
        _emit 0x8b
        _emit 0x08
        // MOV EDX, [ECX+0x8]  (vtbl[2] = release)
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        // PUSH EAX
        _emit 0x50
        // CALL EDX
        _emit 0xff
        _emit 0xd2

        // --- label_14: epilogue ---
        // POP EDI
        _emit 0x5f
        // POP ESI
        _emit 0x5e
        // MOV AL, BL
        _emit 0x8a
        _emit 0xc3
        // POP EBX
        _emit 0x5b
        // ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // RET
        _emit 0xc3
    }
}
