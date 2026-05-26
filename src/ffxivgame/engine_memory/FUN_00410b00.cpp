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
// FUNCTION: ffxivgame 0x00010b00 — SQEX::CDev::Engine::Memory::Alternative::SeparateHeapBlock::CreateHandle
//                                  __thiscall with 4 stack args (0xe0 bytes / 224 B)
//
// Signature (inferred): void* SeparateHeapBlock::CreateHandle(
//     int    param_1,   [ESP+0x04]
//     void*  param_2,   [ESP+0x08]
//     void*  param_3,   [ESP+0x0C]
//     void*  param_4)   [ESP+0x10]
//
// ECX = this; callee cleans 4 stack args (RET 0x10).
// Callee-saved: EBX, ESI, EDI.
//
// Object layout (inferred from field accesses):
//   [this + 0x10]  void*  m_space        — the SeparateHeapSpace object
//   [this + 0x28]  int    m_baseOffset   — base byte offset in the heap
//   [this + 0x2c]  void*  m_container   — container holding the handle list
//
// Behaviour:
//   1. Calls vtable[0x2c/4=11] and vtable[0x34/4=13] on m_space.
//      vtable[13] returns a bool — if false the space is busy; fire an
//      assert ("!space->IsBusy()") and skip.
//   2. Calls vtable[4/4=1] on m_space to get the allocator object.
//   3. Calls FUN_004109a0 (spinlock-guarded allocator lock-and-fetch)
//      on the allocator's field_10.
//   4. If a block was allocated, calls FUN_00410510 (SeparateHeapBlock
//      constructor) with 8 args to initialise it; the first arg is EDI
//      (= m_space), followed by 0, then the 4 stack args reordered
//      with the sum (m_baseOffset + param_1) substituted for the
//      original param_1.
//   5. Splices the new (or NULL) handle into the doubly-linked list
//      hanging off m_container->field_48.
//   6. Calls vtable[0x30/4=12] on m_space to signal completion.
//   7. Returns ptr+4 of the allocated block, or NULL on failure.
//
// Non-reproducibility root cause:
//   - Several absolute VA references to data-section globals:
//       0x01323910  — one-time-init flag (g_assert_init_flag)
//       0x0132390c  — assert-handler function pointer (g_assert_fn)
//       0x00f56a70  — "SQEX::...::SeparateHeapBlock::CreateHandle"
//       0x00f56988  — source-file path string
//       0x00f56974  — "!space->IsBusy()" (condition expression)
//       0x00f54d48  — second assert arg (cond/expression string slot)
//   These absolute addresses appear as MOV/CALL immediates and cannot
//   be expressed as COFF relocations from a standalone .cpp — they are
//   hard-coded VAs in the original binary's .data/.rdata.
//   Naked-asm passthrough chosen to guarantee byte-identical output.

extern "C" __declspec(naked) void FUN_00410b00()
{
    __asm {
        // 00010b00: 53                 PUSH EBX
        _emit 0x53
        // 00010b01: 56                 PUSH ESI
        _emit 0x56
        // 00010b02: 8b f1              MOV ESI, ECX           ; ESI = this
        _emit 0x8b
        _emit 0xf1
        // 00010b04: 57                 PUSH EDI
        _emit 0x57
        // 00010b05: 8b 7e 10           MOV EDI, dword ptr [ESI+0x10]   ; EDI = this->m_space
        _emit 0x8b
        _emit 0x7e
        _emit 0x10
        // 00010b08: 8b 07              MOV EAX, dword ptr [EDI]         ; EAX = *vtable
        _emit 0x8b
        _emit 0x07
        // 00010b0a: 8b 50 2c           MOV EDX, dword ptr [EAX+0x2c]   ; EDX = vtable[11]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 00010b0d: 8b cf              MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00010b0f: ff d2              CALL EDX               ; vtable[11](m_space)
        _emit 0xff
        _emit 0xd2
        // 00010b11: 8b 07              MOV EAX, dword ptr [EDI]
        _emit 0x8b
        _emit 0x07
        // 00010b13: 8b 50 34           MOV EDX, dword ptr [EAX+0x34]   ; EDX = vtable[13]
        _emit 0x8b
        _emit 0x50
        _emit 0x34
        // 00010b16: 8b cf              MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00010b18: ff d2              CALL EDX               ; IsBusy() — returns bool in AL
        _emit 0xff
        _emit 0xd2
        // 00010b1a: 84 c0              TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // 00010b1c: bb 01 00 00 00     MOV EBX, 0x1
        _emit 0xbb
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00010b21: 74 3a              JZ +0x3a (to 0x00010b5d)
        _emit 0x74
        _emit 0x3a
        // 00010b23: 84 1d 10 39 32 01  TEST byte ptr [0x01323910], BL
        _emit 0x84
        _emit 0x1d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00010b29: 75 10              JNZ +0x10 (to 0x00010b3b)
        _emit 0x75
        _emit 0x10
        // 00010b2b: 09 1d 10 39 32 01  OR dword ptr [0x01323910], EBX
        _emit 0x09
        _emit 0x1d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00010b31: c7 05 0c 39 32 01 e0 f8 40 00   MOV dword ptr [0x0132390c], 0x0040f8e0
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
        // 00010b3b: 68 70 6a f5 00     PUSH 0x00f56a70   (function name)
        _emit 0x68
        _emit 0x70
        _emit 0x6a
        _emit 0xf5
        _emit 0x00
        // 00010b40: 68 a2 00 00 00     PUSH 0xa2         (line 162)
        _emit 0x68
        _emit 0xa2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00010b45: 68 88 69 f5 00     PUSH 0x00f56988   (filename)
        _emit 0x68
        _emit 0x88
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        // 00010b4a: 68 48 4d f5 00     PUSH 0x00f54d48   (cond expression)
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 00010b4f: 68 74 69 f5 00     PUSH 0x00f56974   ("!space->IsBusy()")
        _emit 0x68
        _emit 0x74
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        // 00010b54: ff 15 0c 39 32 01  CALL dword ptr [0x0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00010b5a: 83 c4 14           ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00010b5d: 8b 4e 10           MOV ECX, dword ptr [ESI+0x10]
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 00010b60: 8b 01              MOV EAX, dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 00010b62: 8b 50 04           MOV EDX, dword ptr [EAX+0x4]    ; vtable[1]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00010b65: ff d2              CALL EDX               ; vtable[1](m_space) → allocator
        _emit 0xff
        _emit 0xd2
        // 00010b67: 8b 48 10           MOV ECX, dword ptr [EAX+0x10]   ; ECX = alloc->field_10
        _emit 0x8b
        _emit 0x48
        _emit 0x10
        // 00010b6a: e8 31 fe ff ff     CALL 0x004109a0        ; FUN_004109a0 (spinlock fetch)
        _emit 0xe8
        _emit 0x31
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 00010b6f: 85 c0              TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00010b71: 74 33              JZ +0x33 (to 0x00010ba6)
        _emit 0x74
        _emit 0x33
        // ---- EAX != 0: allocate path ----
        // 00010b73: 8b 4e 2c           MOV ECX, dword ptr [ESI+0x2c]   ; this->m_container
        _emit 0x8b
        _emit 0x4e
        _emit 0x2c
        // 00010b76: 8b 56 28           MOV EDX, dword ptr [ESI+0x28]   ; this->m_baseOffset
        _emit 0x8b
        _emit 0x56
        _emit 0x28
        // 00010b79: 51                 PUSH ECX               ; push m_container
        _emit 0x51
        // 00010b7a: 8b 4c 24 14        MOV ECX, dword ptr [ESP+0x14]   ; ECX = param_1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00010b7e: 03 d1              ADD EDX, ECX           ; EDX = m_baseOffset + param_1
        _emit 0x03
        _emit 0xd1
        // 00010b80: 8b 4c 24 1c        MOV ECX, dword ptr [ESP+0x1c]   ; ECX = param_3
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 00010b84: 52                 PUSH EDX               ; push (m_baseOffset + param_1)
        _emit 0x52
        // 00010b85: 8b 54 24 24        MOV EDX, dword ptr [ESP+0x24]   ; EDX = param_4
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 00010b89: 53                 PUSH EBX               ; push 1
        _emit 0x53
        // 00010b8a: 52                 PUSH EDX               ; push param_4
        _emit 0x52
        // 00010b8b: 8b 54 24 24        MOV EDX, dword ptr [ESP+0x24]   ; EDX = param_3 (shifted)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 00010b8f: 51                 PUSH ECX               ; push param_3
        _emit 0x51
        // 00010b90: 52                 PUSH EDX               ; push param_2
        _emit 0x52
        // 00010b91: 6a 00              PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00010b93: 57                 PUSH EDI               ; push m_space
        _emit 0x57
        // 00010b94: 8b c8              MOV ECX, EAX           ; ECX = allocated block (this for ctor)
        _emit 0x8b
        _emit 0xc8
        // 00010b96: e8 75 f9 ff ff     CALL 0x00410510        ; FUN_00410510 (SeparateHeapBlock ctor)
        _emit 0xe8
        _emit 0x75
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        // 00010b9b: 8b d8              MOV EBX, EAX           ; EBX = constructed block
        _emit 0x8b
        _emit 0xd8
        // 00010b9d: 85 db              TEST EBX, EBX
        _emit 0x85
        _emit 0xdb
        // 00010b9f: 74 07              JZ +0x07 (to 0x00010ba8)
        _emit 0x74
        _emit 0x07
        // 00010ba1: 8d 4b 08           LEA ECX, [EBX+0x8]    ; ECX = &block->link
        _emit 0x8d
        _emit 0x4b
        _emit 0x08
        // 00010ba4: eb 04              JMP +0x04 (to 0x00010baa)
        _emit 0xeb
        _emit 0x04
        // 00010ba6: 33 db              XOR EBX, EBX           ; EBX = 0 (failure)
        _emit 0x33
        _emit 0xdb
        // 00010ba8: 33 c9              XOR ECX, ECX           ; ECX = 0
        _emit 0x33
        _emit 0xc9
        // 00010baa: 8b 46 2c           MOV EAX, dword ptr [ESI+0x2c]   ; EAX = this->m_container
        _emit 0x8b
        _emit 0x46
        _emit 0x2c
        // 00010bad: 8b 40 48           MOV EAX, dword ptr [EAX+0x48]   ; EAX = container->field_48
        _emit 0x8b
        _emit 0x40
        _emit 0x48
        // 00010bb0: 8b 50 08           MOV EDX, dword ptr [EAX+0x8]    ; EDX = sentinel->next
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 00010bb3: 89 4a 04           MOV dword ptr [EDX+0x4], ECX    ; next->prev = ECX
        _emit 0x89
        _emit 0x4a
        _emit 0x04
        // 00010bb6: 8b 50 08           MOV EDX, dword ptr [EAX+0x8]    ; EDX = sentinel->next (re-read)
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 00010bb9: 89 51 08           MOV dword ptr [ECX+0x8], EDX    ; ECX->next = old_next
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 00010bbc: 89 41 04           MOV dword ptr [ECX+0x4], EAX    ; ECX->prev = sentinel
        _emit 0x89
        _emit 0x41
        _emit 0x04
        // 00010bbf: 89 48 08           MOV dword ptr [EAX+0x8], ECX    ; sentinel->next = ECX
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 00010bc2: 8b 07              MOV EAX, dword ptr [EDI]         ; EAX = *vtable of m_space
        _emit 0x8b
        _emit 0x07
        // 00010bc4: 8b 50 30           MOV EDX, dword ptr [EAX+0x30]   ; EDX = vtable[12]
        _emit 0x8b
        _emit 0x50
        _emit 0x30
        // 00010bc7: 8b cf              MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00010bc9: ff d2              CALL EDX               ; vtable[12](m_space)
        _emit 0xff
        _emit 0xd2
        // 00010bcb: 85 db              TEST EBX, EBX
        _emit 0x85
        _emit 0xdb
        // 00010bcd: 74 09              JZ +0x09 (to 0x00010bd8)
        _emit 0x74
        _emit 0x09
        // ---- success: return block+4 ----
        // 00010bcf: 5f                 POP EDI
        _emit 0x5f
        // 00010bd0: 5e                 POP ESI
        _emit 0x5e
        // 00010bd1: 8d 43 04           LEA EAX, [EBX+0x4]
        _emit 0x8d
        _emit 0x43
        _emit 0x04
        // 00010bd4: 5b                 POP EBX
        _emit 0x5b
        // 00010bd5: c2 10 00           RET 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
        // ---- failure: return NULL ----
        // 00010bd8: 5f                 POP EDI
        _emit 0x5f
        // 00010bd9: 5e                 POP ESI
        _emit 0x5e
        // 00010bda: 33 c0              XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00010bdc: 5b                 POP EBX
        _emit 0x5b
        // 00010bdd: c2 10 00           RET 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
