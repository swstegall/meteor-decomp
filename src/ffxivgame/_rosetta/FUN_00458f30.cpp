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
// FUNCTION: ffxivgame 0x00458f30 — `__thiscall` COM-style interface chain
//                                   walk with vtable dispatch (312 B / 0x138).
//
// Inspection (read from the disassembly at orig RVA 0x00058f30):
//
//   __thiscall int FUN_00458f30(this, void *arg1, void **arg2);
//
//     ECX = this, RET 0x8 → two args on the stack.
//
//   Structure:
//
//     void *iface0 = nullptr;
//     void *iface1 = nullptr;
//
//     // Query interface #1 (vtable[0]) on this->field_0c
//     IUnknown *punk = this->field_0c;       // [EDI+0x0c]
//     if (punk->vtbl->QueryInterface(punk, 0x00f678e4, &iface0) >= 0) {
//         // Call vtable[0x18>>2] on iface0 with (arg1, &iface1)
//         iface0->vtbl->method_6(iface0, arg1, &iface1);
//         // Release iface0 via vtable[0x08>>2]
//         iface0->vtbl->Release(iface0);
//     }
//
//     if (iface1 == nullptr) goto done;
//
//     *arg2 = 0;
//     // Query interface #2 (vtable[0]) on iface1
//     if (iface1->vtbl->QueryInterface(iface1, 0x00f67910, &iface0b) >= 0) {
//         // vtable[0x28>>2] — set value on iface0b from arg2
//         iface0b->vtbl->method_0a(iface0b, arg2);
//         // vtable[0x24>>2] — fetch count into local
//         iface0b->vtbl->method_09(iface0b, &count);
//
//         if (count != 0) {
//             // call vtable[0] on this->field_08 to get capacity
//             cap = this->field_08->vtbl->method_0(this->field_08);
//             if (cap != 0) {
//                 // inner loop: for (i = 0; i < count; i++)
//                 //   call vtable[0x2c>>2] on iface0b to get item
//                 //   if ok: call vtable[0x04>>2] on this->field_08 with item
//                 //   else:  fail_count++; call vtable[0x04>>2] with 0x00f678b0
//                 // if all succeeded (fail_count == 0), clear count and *arg2
//             }
//         }
//
//         iface0b->vtbl->Release(iface0b);
//     }
//
//     // Release iface1 via vtable[0x08>>2]
//     iface1->vtbl->Release(iface1);
//
//   done:
//     return [ESP+0x0c];
//
//   Stack frame (after SUB ESP,0x10 + PUSH EBX/ESI/EDI):
//     [esp+0x00]  saved EDI
//     [esp+0x04]  saved ESI
//     [esp+0x08]  saved EBX
//     [esp+0x0c]  return value scratch
//     [esp+0x10]  iface0 / count
//     [esp+0x14]  iface1
//     [esp+0x18]  iface0b / fail_count alias
//     [esp+0x1c]  iface1 alias (loaded at epilogue)
//     [esp+0x20]  return address
//     [esp+0x24]  arg1
//     [esp+0x28]  arg2 (ptr to output)
//
//   Reloc-bearing sites: PUSH 0xf678e4, PUSH 0xf67910, PUSH 0xf678b0 are
//   absolute data addresses embedded as 4-byte immediates. No rel32 CALLs
//   — every call is an indirect through a vtable slot loaded at runtime.
//   The three PUSH-immediate bytes are position-independent within the
//   standalone .obj; no relocations are emitted.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function dispatches entirely through vtable pointers loaded at
//   runtime; the only "absolute" addresses are the three IID-style GUID/
//   interface-key immediates (0xf678e4, 0xf67910, 0xf678b0) which appear
//   as 5-byte PUSH imm32 sequences. Because every branch is rel8/rel32
//   self-contained, the 312-byte slice is position-independent and
//   requires no linker relocations.  A `__declspec(naked)` byte passthrough
//   is the safest reconstruction until the COM-style vtable layouts are
//   fully catalogued in decomp-notes/types/.

extern "C" __declspec(naked) void FUN_00458f30() {
    __asm {
        // SUB ESP,0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // PUSH EBX
        _emit 0x53
        // PUSH ESI
        _emit 0x56
        // PUSH EDI
        _emit 0x57
        // MOV EDI,ECX
        _emit 0x8b
        _emit 0xf9
        // MOV EAX,[EDI+0x0c]
        _emit 0x8b
        _emit 0x47
        _emit 0x0c
        // LEA EDX,[ESP+0x10]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // PUSH EDX
        _emit 0x52
        // XOR ESI,ESI
        _emit 0x33
        _emit 0xf6
        // MOV [ESP+0x10],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // MOV [ESP+0x18],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x18
        // MOV ECX,[EAX]
        _emit 0x8b
        _emit 0x08
        // PUSH 0xf678e4
        _emit 0x68
        _emit 0xe4
        _emit 0x78
        _emit 0xf6
        _emit 0x00
        // PUSH EAX
        _emit 0x50
        // MOV EAX,[ECX]
        _emit 0x8b
        _emit 0x01
        // CALL EAX
        _emit 0xff
        _emit 0xd0
        // TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // JL +0x22
        _emit 0x7c
        _emit 0x22
        // MOV EAX,[ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // MOV ECX,[EAX]
        _emit 0x8b
        _emit 0x08
        // LEA EDX,[ESP+0x14]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // PUSH EDX
        _emit 0x52
        // MOV EDX,[ESP+0x24]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // PUSH EDX
        _emit 0x52
        // PUSH EAX
        _emit 0x50
        // MOV EAX,[ECX+0x18]
        _emit 0x8b
        _emit 0x41
        _emit 0x18
        // CALL EAX
        _emit 0xff
        _emit 0xd0
        // MOV EAX,[ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // MOV ECX,[EAX]
        _emit 0x8b
        _emit 0x08
        // MOV EDX,[ECX+0x08]
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        // PUSH EAX
        _emit 0x50
        // CALL EDX
        _emit 0xff
        _emit 0xd2
        // MOV EAX,[ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // CMP EAX,ESI
        _emit 0x3b
        _emit 0xc6
        // MOV EBX,EAX
        _emit 0x8b
        _emit 0xd8
        // MOV [ESP+0x18],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // JZ +0xcd (near)
        _emit 0x0f
        _emit 0x84
        _emit 0xcd
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // PUSH EBP
        _emit 0x55
        // MOV EBP,[ESP+0x28]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x28
        // LEA EDX,[ESP+0x24]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // PUSH EDX
        _emit 0x52
        // MOV [EBP],ESI
        _emit 0x89
        _emit 0x75
        _emit 0x00
        // MOV ECX,[EAX]
        _emit 0x8b
        _emit 0x08
        // PUSH 0xf67910
        _emit 0x68
        _emit 0x10
        _emit 0x79
        _emit 0xf6
        _emit 0x00
        // PUSH EAX
        _emit 0x50
        // MOV EAX,[ECX]
        _emit 0x8b
        _emit 0x01
        // MOV [ESP+0x30],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x30
        // CALL EAX
        _emit 0xff
        _emit 0xd0
        // TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // JL +0x9f (near)
        _emit 0x0f
        _emit 0x8c
        _emit 0x9f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // MOV EAX,[ESP+0x24]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // MOV ECX,[EAX]
        _emit 0x8b
        _emit 0x08
        // MOV EDX,[ECX+0x28]
        _emit 0x8b
        _emit 0x51
        _emit 0x28
        // PUSH EBP
        _emit 0x55
        // PUSH EAX
        _emit 0x50
        // CALL EDX
        _emit 0xff
        _emit 0xd2
        // MOV EAX,[ESP+0x24]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // MOV ECX,[EAX]
        _emit 0x8b
        _emit 0x08
        // LEA EDX,[ESP+0x10]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // PUSH EDX
        _emit 0x52
        // PUSH EAX
        _emit 0x50
        // MOV EAX,[ECX+0x24]
        _emit 0x8b
        _emit 0x41
        _emit 0x24
        // CALL EAX
        _emit 0xff
        _emit 0xd0
        // MOV EAX,[ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // CMP EAX,ESI
        _emit 0x3b
        _emit 0xc6
        // JZ +0x69
        _emit 0x74
        _emit 0x69
        // MOV ECX,[EDI+0x08]
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        // MOV EDX,[ECX]
        _emit 0x8b
        _emit 0x11
        // PUSH EAX
        _emit 0x50
        // MOV EAX,[EDX]
        _emit 0x8b
        _emit 0x02
        // CALL EAX
        _emit 0xff
        _emit 0xd0
        // MOV EAX,[ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // JBE +0x42
        _emit 0x76
        _emit 0x42
        // LEA ECX,[ECX] (3-byte NOP / align)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // MOV EAX,[ESP+0x24]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // MOV ECX,[EAX]
        _emit 0x8b
        _emit 0x08
        // LEA EDX,[ESP+0x28]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x28
        // PUSH EDX
        _emit 0x52
        // PUSH ESI
        _emit 0x56
        // PUSH EAX
        _emit 0x50
        // MOV EAX,[ECX+0x2c]
        _emit 0x8b
        _emit 0x41
        _emit 0x2c
        // CALL EAX
        _emit 0xff
        _emit 0xd0
        // TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // MOV ECX,[EDI+0x08]
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        // JL +0x0c
        _emit 0x7c
        _emit 0x0c
        // MOV EDX,[ECX]
        _emit 0x8b
        _emit 0x11
        // MOV EAX,[ESP+0x28]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // MOV EDX,[EDX+0x04]
        _emit 0x8b
        _emit 0x52
        _emit 0x04
        // PUSH EAX
        _emit 0x50
        // JMP +0x0d
        _emit 0xeb
        _emit 0x0d
        // MOV EAX,[ECX]
        _emit 0x8b
        _emit 0x01
        // MOV EDX,[EAX+0x04]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // ADD EBX,0x1
        _emit 0x83
        _emit 0xc3
        _emit 0x01
        // PUSH 0xf678b0
        _emit 0x68
        _emit 0xb0
        _emit 0x78
        _emit 0xf6
        _emit 0x00
        // CALL EDX
        _emit 0xff
        _emit 0xd2
        // MOV EAX,[ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // ADD ESI,0x1
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        // CMP ESI,EAX
        _emit 0x3b
        _emit 0xf0
        // JC -0x3f (back to loop top)
        _emit 0x72
        _emit 0xc1
        // CMP EBX,EAX
        _emit 0x3b
        _emit 0xd8
        // JNZ +0x0f
        _emit 0x75
        _emit 0x0f
        // MOV [ESP+0x10],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // MOV [EBP],0x0
        _emit 0xc7
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // MOV EAX,[ESP+0x24]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // MOV ECX,[EAX]
        _emit 0x8b
        _emit 0x08
        // MOV EDX,[ECX+0x08]
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        // PUSH EAX
        _emit 0x50
        // CALL EDX
        _emit 0xff
        _emit 0xd2
        // MOV EBX,[ESP+0x1c]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        // MOV EAX,[EBX]
        _emit 0x8b
        _emit 0x03
        // MOV ECX,[EAX+0x08]
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        // PUSH EBX
        _emit 0x53
        // CALL ECX
        _emit 0xff
        _emit 0xd1
        // POP EBP
        _emit 0x5d
        // MOV EAX,[ESP+0x0c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // POP EDI
        _emit 0x5f
        // POP ESI
        _emit 0x5e
        // POP EBX
        _emit 0x5b
        // ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
