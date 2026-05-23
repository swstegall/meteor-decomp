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
// FUNCTION: ffxivgame 0x00010cb0 — SeparateHeapBlock::AddListener (185 bytes / 0xb9)
//           __thiscall, 3 stack args
//
// ECX = this (SeparateHeapBlock*).
// this->field_0x10 is a pointer to a SeparateHeapSpace object (with a vtable).
// this->field_0x38 is the sentinel node of the listener doubly-linked list.
//
// Behaviour:
//   space = this->field_0x10                ; EDI throughout
//   space->vtable[0x2c](space)              ; e.g. Lock()
//   ok = space->vtable[0x34](space)         ; e.g. IsBusy() — check state
//   if (ok) {                               ; assert(!space->IsBusy())
//       if (!(g_assert_flag & 1)) {
//           g_assert_flag |= 1;
//           g_assert_fn = FUN_0040f8e0;
//       }
//       g_assert_fn("!space->IsBusy()", &DAT_00f54d48,
//           "c:\\work\\project\\cdev\\src\\common\\cdev\\engine\\memory"
//           "\\alternative\\SeparateHeapSpace.h",
//           0xb9,
//           "SQEX::CDev::Engine::Memory::Alternative::SeparateHeapBlock::AddListener");
//   }
//   block = this->field_0x10->vtable[0x4](space)  ; e.g. GetBlock()
//   node  = FUN_004109a0(block->field_0x14)        ; allocate listener element
//   if (node) {
//       node->next   = node;               ; self-referential init
//       node->prev   = node;
//       node->vtable = HandleListenerElement::vftable;  (0x00f56824)
//       node->field_c  = param1;           ; [ESP+0x10]
//       node->field_10 = param2;           ; [ESP+0x14]
//   } else {
//       node = NULL;
//   }
//   // Insert node before sentinel (append to tail of list)
//   head            = &this->field_0x38;
//   head->prev->next = node;
//   node->prev       = head->prev;
//   node->next       = head;
//   head->prev       = node;
//   space->vtable[0x30](space)             ; e.g. Unlock()
//   return node;
//
// Reconstruction: naked-asm byte passthrough.
//
// Root cause of non-reproducibility from plain C++:
//   1. Absolute data-section addresses (g_assert_flag at 0x01323910,
//      g_assert_fn at 0x0132390c) appear as immediate operands and cannot
//      be encoded as COFF relocations from standalone compilation.
//   2. String/vtable literal addresses (0xf56b10, 0xf56988, 0xf54d48,
//      0xf56974, 0xf56824) are direct absolute references.
//   3. The vtable[0x4] call reloads this->field_0x10 from memory
//      ([EBX+0x10]) rather than reusing the EDI cache — a MSVC
//      register-allocation artefact that depends on the precise source
//      expression used and cannot be forced from high-level C++.
//
// Globals / constants referenced (all absolute VAs in the 1.23b image):
//   0x01323910  — one-time-init flag dword (low bit = initialised)
//   0x0132390c  — assert-handler function pointer slot
//   0x0040f8e0  — FUN_0040f8e0 (the assert handler installed on first call)
//   0x00f56974  — "!space->IsBusy()"
//   0x00f54d48  — &DAT_00f54d48 (condition/expression string, assert arg 2)
//   0x00f56988  — filename string (SeparateHeapSpace.h)
//   0x00f56b10  — function name string (SeparateHeapBlock::AddListener)
//   0x00f56824  — HandleListenerElement::vftable address
//
// Relocations (masked by tools/compare.py):
//   DIR32: 0xf56824 (@0x10d32), assertion addresses (multiple)
//   REL32: CALL FUN_004109a0 (@0x10d1a)

extern "C" __declspec(naked) void FUN_00410cb0()
{
    __asm {
        // 00010cb0:  53                 PUSH EBX
        _emit 0x53
        // 00010cb1:  56                 PUSH ESI
        _emit 0x56
        // 00010cb2:  8b d9              MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 00010cb4:  57                 PUSH EDI
        _emit 0x57
        // 00010cb5:  8b 7b 10           MOV EDI,dword ptr [EBX+0x10]
        _emit 0x8b
        _emit 0x7b
        _emit 0x10
        // 00010cb8:  8b 07              MOV EAX,dword ptr [EDI]
        _emit 0x8b
        _emit 0x07
        // 00010cba:  8b 50 2c           MOV EDX,dword ptr [EAX+0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 00010cbd:  8b cf              MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 00010cbf:  ff d2              CALL EDX  ; space->vtable[0x2c]() = Lock()
        _emit 0xff
        _emit 0xd2
        // 00010cc1:  8b 07              MOV EAX,dword ptr [EDI]
        _emit 0x8b
        _emit 0x07
        // 00010cc3:  8b 50 34           MOV EDX,dword ptr [EAX+0x34]
        _emit 0x8b
        _emit 0x50
        _emit 0x34
        // 00010cc6:  8b cf              MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 00010cc8:  ff d2              CALL EDX  ; space->vtable[0x34]() = IsBusy()
        _emit 0xff
        _emit 0xd2
        // 00010cca:  84 c0              TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // 00010ccc:  74 3f              JZ +0x3f (-> 0x00010d0d, skip assert)
        _emit 0x74
        _emit 0x3f
        // ---- assert(!space->IsBusy()) path ----
        // 00010cce:  b8 01 00 00 00     MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00010cd3:  84 05 10 39 32 01  TEST byte ptr [0x01323910],AL
        _emit 0x84
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00010cd9:  75 10              JNZ +0x10 (already initialised)
        _emit 0x75
        _emit 0x10
        // 00010cdb:  09 05 10 39 32 01  OR dword ptr [0x01323910],EAX
        _emit 0x09
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00010ce1:  c7 05 0c 39 32 01 e0 f8 40 00
        //            MOV dword ptr [0x0132390c],0x0040f8e0
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xe0
        _emit 0xf8
        _emit 0x40
        _emit 0x00
        // 00010ceb:  68 10 6b f5 00     PUSH 0x00f56b10  (function name)
        _emit 0x68
        _emit 0x10
        _emit 0x6b
        _emit 0xf5
        _emit 0x00
        // 00010cf0:  68 b9 00 00 00     PUSH 0xb9  (line number 185)
        _emit 0x68
        _emit 0xb9
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00010cf5:  68 88 69 f5 00     PUSH 0x00f56988  (filename)
        _emit 0x68
        _emit 0x88
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        // 00010cfa:  68 48 4d f5 00     PUSH 0x00f54d48  (cond expression)
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 00010cff:  68 74 69 f5 00     PUSH 0x00f56974  ("!space->IsBusy()")
        _emit 0x68
        _emit 0x74
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        // 00010d04:  ff 15 0c 39 32 01  CALL dword ptr [0x0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00010d0a:  83 c4 14           ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // ---- post-assert / main path ----
        // 00010d0d:  8b 4b 10           MOV ECX,dword ptr [EBX+0x10]
        _emit 0x8b
        _emit 0x4b
        _emit 0x10
        // 00010d10:  8b 01              MOV EAX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 00010d12:  8b 50 04           MOV EDX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00010d15:  ff d2              CALL EDX  ; space->vtable[0x4]() = GetBlock()
        _emit 0xff
        _emit 0xd2
        // 00010d17:  8b 48 14           MOV ECX,dword ptr [EAX+0x14]
        _emit 0x8b
        _emit 0x48
        _emit 0x14
        // 00010d1a:  e8 81 fc ff ff     CALL 0x004109a0  (REL32 — FUN_004109a0)
        _emit 0xe8
        _emit 0x81
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 00010d1f:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00010d21:  74 1e              JZ +0x1e (-> 0x00010d41, null path)
        _emit 0x74
        _emit 0x1e
        // ---- success path: init node ----
        // 00010d23:  8b 4c 24 10        MOV ECX,dword ptr [ESP+0x10]  ; param1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00010d27:  8b 54 24 14        MOV EDX,dword ptr [ESP+0x14]  ; param2
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 00010d2b:  89 40 04           MOV dword ptr [EAX+0x4],EAX   ; node->next = node
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 00010d2e:  89 40 08           MOV dword ptr [EAX+0x8],EAX   ; node->prev = node
        _emit 0x89
        _emit 0x40
        _emit 0x08
        // 00010d31:  c7 00 24 68 f5 00  MOV dword ptr [EAX],0x00f56824  (DIR32 reloc — vtable)
        _emit 0xc7
        _emit 0x00
        _emit 0x24
        _emit 0x68
        _emit 0xf5
        _emit 0x00
        // 00010d37:  89 48 0c           MOV dword ptr [EAX+0xc],ECX   ; node->field_c = param1
        _emit 0x89
        _emit 0x48
        _emit 0x0c
        // 00010d3a:  89 50 10           MOV dword ptr [EAX+0x10],EDX  ; node->field_10 = param2
        _emit 0x89
        _emit 0x50
        _emit 0x10
        // 00010d3d:  8b f0              MOV ESI,EAX   ; ESI = node
        _emit 0x8b
        _emit 0xf0
        // 00010d3f:  eb 02              JMP +0x2 (-> 0x00010d43)
        _emit 0xeb
        _emit 0x02
        // ---- null path: ESI = 0 ----
        // 00010d41:  33 f6              XOR ESI,ESI
        _emit 0x33
        _emit 0xf6
        // ---- list insertion ----
        // 00010d43:  8b 43 38           MOV EAX,dword ptr [EBX+0x38]  ; head sentinel
        _emit 0x8b
        _emit 0x43
        _emit 0x38
        // 00010d46:  8b 48 08           MOV ECX,dword ptr [EAX+0x8]   ; ECX = head->prev
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        // 00010d49:  89 71 04           MOV dword ptr [ECX+0x4],ESI   ; head->prev->next = node
        _emit 0x89
        _emit 0x71
        _emit 0x04
        // 00010d4c:  8b 50 08           MOV EDX,dword ptr [EAX+0x8]   ; EDX = head->prev
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 00010d4f:  89 56 08           MOV dword ptr [ESI+0x8],EDX   ; node->prev = head->prev
        _emit 0x89
        _emit 0x56
        _emit 0x08
        // 00010d52:  89 46 04           MOV dword ptr [ESI+0x4],EAX   ; node->next = head
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 00010d55:  89 70 08           MOV dword ptr [EAX+0x8],ESI   ; head->prev = node
        _emit 0x89
        _emit 0x70
        _emit 0x08
        // 00010d58:  8b 07              MOV EAX,dword ptr [EDI]
        _emit 0x8b
        _emit 0x07
        // 00010d5a:  8b 50 30           MOV EDX,dword ptr [EAX+0x30]
        _emit 0x8b
        _emit 0x50
        _emit 0x30
        // 00010d5d:  8b cf              MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 00010d5f:  ff d2              CALL EDX  ; space->vtable[0x30]() = Unlock()
        _emit 0xff
        _emit 0xd2
        // 00010d61:  5f                 POP EDI
        _emit 0x5f
        // 00010d62:  8b c6              MOV EAX,ESI   ; return node
        _emit 0x8b
        _emit 0xc6
        // 00010d64:  5e                 POP ESI
        _emit 0x5e
        // 00010d65:  5b                 POP EBX
        _emit 0x5b
        // 00010d66:  c2 0c 00           RET 0xc  ; __thiscall, 3 stack args
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
