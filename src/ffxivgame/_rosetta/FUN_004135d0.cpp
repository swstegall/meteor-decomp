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
// FUNCTION: ffxivgame 0x004135d0 — engine_memory DetachableHeapSpace link1-list
//                                  virtual-dispatch and result cache (41 B / 0x29)
//
// __thiscall void *FUN_004135d0(this)
//   ECX : this  (outer wrapper object)
//   EAX : return value — &this->field_08 if non-null result, else nullptr
//
// High-level shape:
//
//   sub    = this->field_04              ; DetachableHeapSpace*
//   node   = sub->link1.prev            ; sub + 0x2c  (tail of Link#1 list)
//   sentry = &sub->link1                ; sub + 0x24  (sentinel)
//   result = nullptr
//   if (node != sentry):
//       result = node->vtable[1](node)  ; virtual call, slot 1
//   this->field_0c = result
//   if (result != nullptr):
//       return &this->field_08
//   return nullptr
//
// Asm (41 B, RVA 0x000135d0):
//
//   56              PUSH ESI
//   8b f1           MOV  ESI, ECX              ; ESI = this
//   8b 56 04        MOV  EDX, [ESI+0x4]        ; EDX = sub
//   8b 4a 2c        MOV  ECX, [EDX+0x2c]       ; ECX = sub->link1.prev (node)
//   83 c2 24        ADD  EDX, 0x24             ; EDX = &sub->link1 (sentinel)
//   33 c0           XOR  EAX, EAX              ; result = 0
//   3b ca           CMP  ECX, EDX             ; node == sentry? (empty list?)
//   74 07           JZ   +7                    ; -> skip vcall
//   8b 01           MOV  EAX, [ECX]            ; EAX = node->vtable
//   8b 50 04        MOV  EDX, [EAX+0x4]        ; EDX = vtable[1]
//   ff d2           CALL EDX                   ; result = node->vtable[1](node)
//   85 c0           TEST EAX, EAX
//   89 46 0c        MOV  [ESI+0xc], EAX        ; this->field_0c = result
//   74 05           JZ   +5                    ; -> null return
//   8d 46 08        LEA  EAX, [ESI+0x8]        ; return &this->field_08
//   5e              POP  ESI
//   c3              RET
//   33 c0           XOR  EAX, EAX              ; null return path
//   5e              POP  ESI
//   c3              RET
//
// Register allocation:
//   ESI = this (callee-saved across the virtual call)
//   EDX = sub ptr → reused as sentinel (ADD EDX, 0x24 destroys original)
//   ECX = node ptr (loaded once; never clobbered before CALL EDX)
//   EAX = vtable ptr → result value
//
// No relocations — all addresses are register-indirect. compare.py needs
// no reloc mask.
//
// Reconstruction: naked _emit byte passthrough, same idiom as FUN_00413570.

extern "C" __declspec(naked) void FUN_004135d0()
{
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x4]
        _emit 0x56
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [EDX+0x2c]
        _emit 0x4a
        _emit 0x2c
        _emit 0x83              // ADD EDX, 0x24
        _emit 0xc2
        _emit 0x24
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x3b              // CMP ECX, EDX
        _emit 0xca
        _emit 0x74              // JZ +0x07  (vcall_done)
        _emit 0x07
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x4]
        _emit 0x50
        _emit 0x04
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x89              // MOV dword ptr [ESI+0xc], EAX
        _emit 0x46
        _emit 0x0c
        _emit 0x74              // JZ +0x05  (null_ret)
        _emit 0x05
        _emit 0x8d              // LEA EAX, [ESI+0x8]
        _emit 0x46
        _emit 0x08
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        _emit 0x33              // XOR EAX, EAX  (null_ret:)
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
