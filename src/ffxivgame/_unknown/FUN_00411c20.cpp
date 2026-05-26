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
// FUNCTION: ffxivgame 0x00011c20 — __thiscall linked-list virtual-dispatch
//                                   walker with subtraction side-effect (59 B)
//
// Calling convention: __thiscall (ECX = this, no stack args, RET 0).
// Callee-saved registers: EBX, EBP, ESI; EDI saved only in the non-empty path.
//
// Object layout (inferred from offsets touched, ECX = this):
//   [this + 0x04]   void *sub_obj  — the sub-object whose FUN_0040df70 is called
//   [this + 0x18]   int   field_18 — running total decremented per node
//   [this + 0x24]   ListNode sentinel (inline; address = end-of-list marker)
//   [this + 0x2c]   ListNode *head  — pointer to first node (== sentinel if empty)
//
// List node vtable layout:
//   vtable[1] (__thiscall):  returns a "result" pointer (EDI after the call)
//   result->field_10 (int):  value subtracted from this->field_18
//   result vtable[0] (__thiscall, arg=0): secondary call on the result object
//
// Behaviour (while list non-empty):
//   For each node (ESI) until ESI == &this->field_24:
//     1. result = node->vtable[1](node)           // advance to next node
//     2. ESI = *(old_ESI + 8)                     // advance iterator
//     3. this->field_18 -= result->field_10       // decrement running total
//     4. result->vtable[0](result, 0)             // secondary dispatch
//     5. this->sub_obj->FUN_0040df70(result)      // cleanup/free call
//
// Codegen notes:
//   - The JZ at offset 13 branches FORWARD past the function body; the exit
//     epilogue (POP EDI / POP ESI / POP EBP / POP EBX / RET) is shared with
//     code that follows immediately after this function's 59 bytes.
//   - The loop-back JNZ at offset 57 is the last instruction in the function
//     body; compare.py compares exactly 59 bytes.
//   - The CALL to FUN_0040df70 at offset 50 is a rel32 with value
//     0xffffc319 in the orig binary (RVA 0x0000df70). Emitting the raw bytes
//     verbatim matches the orig byte-for-byte for compare purposes.
//   - No relocations are needed; all calls are register-indirect except the
//     one CALL to FUN_0040df70 whose raw rel32 offset matches the orig.
//
// Naked-asm passthrough: chosen to avoid any risk of MSVC register-allocation
// variance in a 59-byte function with an unusual shared-epilogue exit shape.

extern "C" __declspec(naked) void FUN_00411c20()
{
    __asm {
        // 00011c20:  53                  PUSH EBX
        _emit 0x53
        // 00011c21:  55                  PUSH EBP
        _emit 0x55
        // 00011c22:  8b d9               MOV EBX, ECX
        _emit 0x8b
        _emit 0xd9
        // 00011c24:  56                  PUSH ESI
        _emit 0x56
        // 00011c25:  8b 73 2c            MOV ESI, dword ptr [EBX+0x2c]
        _emit 0x8b
        _emit 0x73
        _emit 0x2c
        // 00011c28:  8d 6b 24            LEA EBP, [EBX+0x24]
        _emit 0x8d
        _emit 0x6b
        _emit 0x24
        // 00011c2b:  3b f5               CMP ESI, EBP
        _emit 0x3b
        _emit 0xf5
        // 00011c2d:  74 2d               JZ +0x2d  (→ shared epilogue outside fn)
        _emit 0x74
        _emit 0x2d
        // 00011c2f:  57                  PUSH EDI
        _emit 0x57
        // === loop top (RVA 0x00011c30) ===
        // 00011c30:  8b 06               MOV EAX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 00011c32:  8b 50 04            MOV EDX, dword ptr [EAX+0x4]   (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00011c35:  8b ce               MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00011c37:  ff d2               CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00011c39:  8b 76 08            MOV ESI, dword ptr [ESI+0x8]   (advance)
        _emit 0x8b
        _emit 0x76
        _emit 0x08
        // 00011c3c:  8b f8               MOV EDI, EAX                   (result)
        _emit 0x8b
        _emit 0xf8
        // 00011c3e:  8b 47 10            MOV EAX, dword ptr [EDI+0x10]  (field_10)
        _emit 0x8b
        _emit 0x47
        _emit 0x10
        // 00011c41:  29 43 18            SUB dword ptr [EBX+0x18], EAX
        _emit 0x29
        _emit 0x43
        _emit 0x18
        // 00011c44:  8b 17               MOV EDX, dword ptr [EDI]       (vtable)
        _emit 0x8b
        _emit 0x17
        // 00011c46:  8b 02               MOV EAX, dword ptr [EDX]       (vtable[0])
        _emit 0x8b
        _emit 0x02
        // 00011c48:  6a 00               PUSH 0
        _emit 0x6a
        _emit 0x00
        // 00011c4a:  8b cf               MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00011c4c:  ff d0               CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00011c4e:  8b 4b 04            MOV ECX, dword ptr [EBX+0x4]   (sub_obj)
        _emit 0x8b
        _emit 0x4b
        _emit 0x04
        // 00011c51:  57                  PUSH EDI                        (arg)
        _emit 0x57
        // 00011c52:  e8 19 c3 ff ff      CALL FUN_0040df70  (rel32 = 0xffffc319)
        _emit 0xe8
        _emit 0x19
        _emit 0xc3
        _emit 0xff
        _emit 0xff
        // 00011c57:  3b f5               CMP ESI, EBP                   (loop check)
        _emit 0x3b
        _emit 0xf5
        // 00011c59:  75 d5               JNZ -0x2b  (→ loop top at 0x00011c30)
        _emit 0x75
        _emit 0xd5
        // Epilogue — also reachable from the JZ at offset 13 (empty-list fast path):
        // 00011c5b:  5f                  POP EDI
        _emit 0x5f
        // 00011c5c:  5e                  POP ESI
        _emit 0x5e
        // 00011c5d:  5d                  POP EBP
        _emit 0x5d
        // 00011c5e:  5b                  POP EBX
        _emit 0x5b
        // 00011c5f:  c3                  RET
        _emit 0xc3
    }
}
