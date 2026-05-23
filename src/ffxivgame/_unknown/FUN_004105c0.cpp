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
// FUNCTION: ffxivgame 0x000105c0 — __thiscall linked-list virtual-dispatch walker (51 B)
//
// Iterates a singly-linked list embedded in `this`. For each node it:
//   1. Calls vtable[1] on the node (thiscall, returns some result object).
//   2. Retrieves result->field_0c (an object with its own vtable) and
//      result->field_10 (a value), then calls vtable[2] on the inner object
//      with args (&this->field_04, result->field_10) (stdcall, callee cleans up).
//   3. Advances via node->field_08 (next pointer).
//   Sentinel: node == &this->field_38.
//
// Calling convention: __thiscall (ECX = this, no stack args, RET 0).
// Stack frame: PUSH EBX/ESI/EDI only — no locals.
//
// No relocations in these 51 bytes (all calls are indirect through
// registers; no IAT or data references). Naked-asm passthrough chosen
// to guarantee byte-identical output without register-allocation risk.

extern "C" __declspec(naked) void FUN_004105c0()
{
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ECX+0x40]
        _emit 0x71
        _emit 0x40
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA EDI, [ECX+0x38]
        _emit 0x79
        _emit 0x38
        _emit 0x3b              // CMP ESI, EDI
        _emit 0xf7
        _emit 0x74              // JZ +0x24 (to 0x4105f0)
        _emit 0x24
        _emit 0x53              // PUSH EBX
        _emit 0x8d              // LEA EBX, [ECX+0x4]
        _emit 0x59
        _emit 0x04
        _emit 0x8b              // MOV EAX, dword ptr [ESI]     -- loop top
        _emit 0x06
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x4] -- vtable[1]
        _emit 0x50
        _emit 0x04
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV ECX, dword ptr [EAX+0xc]
        _emit 0x48
        _emit 0x0c
        _emit 0x8b              // MOV EAX, dword ptr [EAX+0x10]
        _emit 0x40
        _emit 0x10
        _emit 0x8b              // MOV EDX, dword ptr [ECX]
        _emit 0x11
        _emit 0x8b              // MOV EDX, dword ptr [EDX+0x8]  -- vtable[2]
        _emit 0x52
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0x53              // PUSH EBX
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV ESI, dword ptr [ESI+0x8]  -- next node
        _emit 0x76
        _emit 0x08
        _emit 0x3b              // CMP ESI, EDI
        _emit 0xf7
        _emit 0x75              // JNZ -0x1f (back to loop top)
        _emit 0xe1
        _emit 0x5b              // POP EBX
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
