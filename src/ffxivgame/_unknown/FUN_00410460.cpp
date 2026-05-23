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
// FUNCTION: ffxivgame 0x00010460 — linked-list iterator dispatching two
//                                   virtual calls per node (60 B / 0x3c)
//
// __thiscall void Container::FUN_00410460(void *param_1)
//   ECX        : this
//   [ESP+0x04] : param_1 — opaque argument forwarded to inner virtual call
//
// Object layout (inferred):
//   [this + 0x04]  void*      field_04    — passed by address as first arg to inner call
//   [this + 0x38]  Node       m_sentinel  — inline sentinel node (12-byte Node struct)
//     [this+0x38+0x00]  void** vftable
//     [this+0x38+0x04]  int    _pad04
//     [this+0x38+0x08]  Node*  next       — = [this+0x40]; points to first real node or self
//
// Node layout:
//   [node + 0x00]  void**     vftable     — vtable pointer
//   [node + 0x04]  int        _pad04      — unused in this function
//   [node + 0x08]  Node*      next        — forward link
//
// Vtable slot 1 of Node (offset +4):
//   __thiscall Result* Node::get_result()   (ECX = node, no stack args)
//   Returns a Result* in EAX.
//
// Result struct fields:
//   [result + 0x0c]  Obj*   obj   — object with its own vtable
//   [result + 0x10]  void*  arg   — forwarded as second stack arg
//
// Vtable slot 1 of Obj (offset +4):
//   __thiscall void Obj::notify(void *arg_a, void *arg_b, void *param_1)
//   Called as: obj->notify(&this->field_04, result->arg, param_1)
//
// Behaviour:
//   Load head = m_sentinel.next (= [this+0x40]).  If head == &m_sentinel,
//   the list is empty; return immediately.  Otherwise iterate:
//     1. Call head->get_result() via vtable[1]  → result*
//     2. Call result->obj->notify() via vtable[1] with (&field_04, result->arg, param_1)
//     3. Advance head = head->next  ([head+8])
//   until head == &m_sentinel.
//
// Calling convention: __thiscall; callee pops 1 stack arg (RET 0x4).
//
// Register allocation (MSVC 2005 /O2):
//   ESI = node (the walking pointer)
//   EDI = &m_sentinel (sentinel / loop bound)
//   EBX = &this->field_04  (first arg to inner call)
//   EBP = param_1  (third arg to inner call)
//   EBX/EBP saves are deferred until after the empty-list check.

extern "C" __declspec(naked) void FUN_00410460()
{
    __asm {
        // 00010460: 56                    PUSH ESI
        _emit 0x56
        // 00010461: 8b 71 40              MOV ESI,[ECX+0x40]
        _emit 0x8b
        _emit 0x71
        _emit 0x40
        // 00010464: 57                    PUSH EDI
        _emit 0x57
        // 00010465: 8d 79 38              LEA EDI,[ECX+0x38]
        _emit 0x8d
        _emit 0x79
        _emit 0x38
        // 00010468: 3b f7                 CMP ESI,EDI
        _emit 0x3b
        _emit 0xf7
        // 0001046a: 74 2b                 JZ +0x2b  (exit)
        _emit 0x74
        _emit 0x2b
        // 0001046c: 53                    PUSH EBX
        _emit 0x53
        // 0001046d: 55                    PUSH EBP
        _emit 0x55
        // 0001046e: 8b 6c 24 14           MOV EBP,[ESP+0x14]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        // 00010472: 8d 59 04              LEA EBX,[ECX+0x04]
        _emit 0x8d
        _emit 0x59
        _emit 0x04
        // loop:
        // 00010475: 8b 06                 MOV EAX,[ESI]
        _emit 0x8b
        _emit 0x06
        // 00010477: 8b 50 04              MOV EDX,[EAX+0x04]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0001047a: 8b ce                 MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0001047c: ff d2                 CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0001047e: 8b 48 0c              MOV ECX,[EAX+0x0c]
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 00010481: 8b 40 10              MOV EAX,[EAX+0x10]
        _emit 0x8b
        _emit 0x40
        _emit 0x10
        // 00010484: 8b 11                 MOV EDX,[ECX]
        _emit 0x8b
        _emit 0x11
        // 00010486: 8b 52 04              MOV EDX,[EDX+0x04]
        _emit 0x8b
        _emit 0x52
        _emit 0x04
        // 00010489: 55                    PUSH EBP
        _emit 0x55
        // 0001048a: 50                    PUSH EAX
        _emit 0x50
        // 0001048b: 53                    PUSH EBX
        _emit 0x53
        // 0001048c: ff d2                 CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0001048e: 8b 76 08              MOV ESI,[ESI+0x08]
        _emit 0x8b
        _emit 0x76
        _emit 0x08
        // 00010491: 3b f7                 CMP ESI,EDI
        _emit 0x3b
        _emit 0xf7
        // 00010493: 75 e0                 JNZ -0x20  (loop)
        _emit 0x75
        _emit 0xe0
        // 00010495: 5d                    POP EBP
        _emit 0x5d
        // 00010496: 5b                    POP EBX
        _emit 0x5b
        // exit:
        // 00010497: 5f                    POP EDI
        _emit 0x5f
        // 00010498: 5e                    POP ESI
        _emit 0x5e
        // 00010499: c2 04 00              RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
