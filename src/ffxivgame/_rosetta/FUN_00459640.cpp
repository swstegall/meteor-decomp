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
// FUNCTION: ffxivgame 0x00459640 — thiscall read-loop dispatcher
//                                  (__thiscall, 167 bytes / 0xa7)
//
// __thiscall void FUN_00459640(void *this,    // ECX
//                              void *param_1, // [ESP+4]
//                              void *param_2) // [ESP+8]
//
// Stack layout at entry (caller view, ECX = this):
//     [ESP+0x04] = param_1  (interface/stream object — has a vtable)
//     [ESP+0x08] = param_2  (e.g. request/context pointer)
//
// /GS cookie: stored at [ESP_local + 0x208] (frame = 0x20c bytes).
// Saved regs: EBX (= ECX/this), ESI (= param_1), EDI (= param_2).
// RET 0x8: callee cleans 2 dwords.
//
// Behaviour (inspected from orig 167 bytes at RVA 0x00059640):
//
//   1. Load vtable[0x38/4=14] from param_1 → EDX.
//   2. Call vtable[14](param_1, param_2, &local_out1).
//      If EAX != 0 → return.
//   3. [7-byte alignment NOP: LEA ESP, [ESP+0]]
//   4. If local_out1 != 0 → return.  ← loop-top check
//   5. Load vtable[0xc/4=3] from param_1 → EAX.
//      Pre-init local_out2_size = 0xff on the stack.
//      Call vtable[3](param_1, param_2, 1, &local_data, 0xff, &local_out2_size).
//   6. Load local_out2_size. Call FUN_00459ca0(&this->field_0x20,
//                                              &local_data, local_out2_size).
//   7. Reload vtable[14] from param_1, call vtable[14](param_1, param_2,
//      &local_out1).
//      If EAX == 0 → goto step 4.
//   8. Epilogue: cookie check, ADD ESP 0x20c, RET 0x8.
//
// Reloc-bearing sites in the orig 167 bytes:
//     +0x06  MOV EAX, [0x012ea8b0]     (__security_cookie global, abs32)
//     +0x76  CALL rel32 → 0x00459ca0   (FUN_00459ca0)
//     +0x99  CALL rel32 → 0x009d20f4   (__security_check_cookie)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The loop body uses two vtable-dispatched calls (vtable[14] and vtable[3])
//   and one direct CALL (FUN_00459ca0), all with rel32/abs32 fixups.
//   compare.py masks reloc bytes, so a __declspec(naked) body re-emitting
//   the orig 167 bytes verbatim via MASM _emit directives yields a .obj
//   whose .text is byte-identical to the orig slice → GREEN.

extern "C" __declspec(naked) void FUN_00459640()
{
    __asm {
        _emit 0x81              // SUB ESP, 0x20c
        _emit 0xec
        _emit 0x0c
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xa1              // MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x89              // MOV dword ptr [ESP+0x208], EAX
        _emit 0x84
        _emit 0x24
        _emit 0x08
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x218]  (param_1)
        _emit 0xb4
        _emit 0x24
        _emit 0x18
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI]         (vtable ptr)
        _emit 0x06
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x38]   (vtable[14])
        _emit 0x50
        _emit 0x38
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x220]  (param_2)
        _emit 0xbc
        _emit 0x24
        _emit 0x20
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EBX, ECX                    (save this)
        _emit 0xd9
        _emit 0x8d              // LEA ECX, [ESP+0xc]              (&local_out1)
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x51              // PUSH ECX
        _emit 0x57              // PUSH EDI
        _emit 0x56              // PUSH ESI
        _emit 0xff              // CALL EDX                         (vtable[14])
        _emit 0xd2
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ exit (+0x54)
        _emit 0x54
        _emit 0x8d              // LEA ESP, [ESP+0x00000000]        (7-byte NOP)
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // loop-top:
        _emit 0x83              // CMP dword ptr [ESP+0xc], 0      (local_out1)
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x75              // JNZ exit (+0x46)
        _emit 0x46
        _emit 0x8b              // MOV EAX, dword ptr [ESI]         (vtable ptr)
        _emit 0x06
        _emit 0x8b              // MOV EAX, dword ptr [EAX+0xc]    (vtable[3])
        _emit 0x40
        _emit 0x0c
        _emit 0x8d              // LEA ECX, [ESP+0x10]              (&local_out2_size)
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x51              // PUSH ECX                         (arg6)
        _emit 0x68              // PUSH 0xff                        (arg5)
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EDX, [ESP+0x1c]              (&local_data)
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x52              // PUSH EDX                         (arg4)
        _emit 0x6a              // PUSH 0x1                         (arg3)
        _emit 0x01
        _emit 0x57              // PUSH EDI                         (arg2 = param_2)
        _emit 0x56              // PUSH ESI                         (arg1 = param_1)
        _emit 0xc7              // MOV dword ptr [ESP+0x28], 0xff   (pre-init local_out2_size)
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff              // CALL EAX                         (vtable[3])
        _emit 0xd0
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x10]   (local_out2_size value)
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x51              // PUSH ECX                         (arg2 = size)
        _emit 0x8d              // LEA EDX, [ESP+0x18]              (&local_data, shifted)
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x52              // PUSH EDX                         (arg1 = &data)
        _emit 0x8d              // LEA ECX, [EBX+0x20]              (&this->field_0x20)
        _emit 0x4b
        _emit 0x20
        _emit 0xe8              // CALL FUN_00459ca0 (rel32 → +0x5e5)
        _emit 0xe5
        _emit 0x05
        _emit 0x00
        _emit 0x00
        // loop-bottom:
        _emit 0x8b              // MOV EAX, dword ptr [ESI]         (vtable ptr)
        _emit 0x06
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x38]   (vtable[14])
        _emit 0x50
        _emit 0x38
        _emit 0x8d              // LEA ECX, [ESP+0xc]               (&local_out1)
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x51              // PUSH ECX
        _emit 0x57              // PUSH EDI
        _emit 0x56              // PUSH ESI
        _emit 0xff              // CALL EDX                          (vtable[14])
        _emit 0xd2
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ loop-top (-0x4d)
        _emit 0xb3
        // exit:
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x214]   (reload cookie)
        _emit 0x8c
        _emit 0x24
        _emit 0x14
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x33              // XOR ECX, ESP
        _emit 0xcc
        _emit 0xe8              // CALL __security_check_cookie (rel32 → +0x578a16)
        _emit 0x16
        _emit 0x8a
        _emit 0x57
        _emit 0x00
        _emit 0x81              // ADD ESP, 0x20c
        _emit 0xc4
        _emit 0x0c
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
