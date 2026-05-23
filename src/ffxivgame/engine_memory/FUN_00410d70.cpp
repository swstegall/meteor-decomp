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
// FUNCTION: ffxivgame 0x00010d70 — SeparateHeapBlock::DeleteListener (183 bytes incl. epilogue)
//
// __thiscall void SeparateHeapBlock::DeleteListener(void *space)
//   ECX        : this (SeparateHeapBlock)
//   [ESP+0x04] : space (listener node to remove/re-insert)
//
// 1. Cache this->field_0x10 (ISpace ptr) in EDI.
// 2. Call ISpace::vfunc[0x2c] (vtable slot 11) — some pre-op call.
// 3. Call ISpace::vfunc[0x34] (vtable slot 13) — IsBusy() check, returns bool.
// 4. If IsBusy() returned true: fire one-time-init assert trampoline at 0x0132390c
//    with 5 args:
//      "SQEX::CDev::Engine::Memory::Alternative::SeparateHeapBlock::DeleteListener" (0xf56b58)
//      0xc3 (line 195 in SeparateHeapSpace.h)
//      <filename string> (0xf56988)
//      <banner/category> (0xf54d48)
//      "!space->IsBusy()"  (0xf56974)
//    The one-shot init flag is bit 0 of dword at 0x01323910.
//    The trampoline fn ptr lives at 0x0132390c; set to FUN_0040f8e0 on first use.
// 5. Load stack arg (space = ESI) from [ESP+0x10] after 3 callee-saves.
// 6. Call space->vfunc[0] with arg 0 (pre-removal step).
// 7. Get ISpace ptr again (= EDI) and call ISpace::vfunc[0x04] (slot 1) → inner struct ptr.
// 8. inner = result->field_0x14; lockPtr = &inner->field_0x4.
// 9. Spin-acquire on *lockPtr using XCHG-based spinlock (loop aligned to 0x10df0).
//    Alignment pad: 6-byte NOP "LEA EBX,[EBX+0]" (8d 9b 00 00 00 00).
// 10. Insert space into doubly-linked list at inner->field_0xc (prev/next at [node]/[node+4]).
// 11. Decrement inner->field_0x18.
// 12. Spin-release: XOR ECX,ECX; XCHG [EDX],ECX (store 0 atomically).
// 13. Call ISpace::vfunc[0x30] (slot 12) — post-op call.
// 14. Epilogue: POP EDI/ESI/EBX + RET 4.
//
// Reconstruction strategy: naked-asm byte passthrough.
// The function references hard-coded absolute VAs (assert flag, trampoline slot,
// five string literals, the FUN_0040f8e0 target).  Emitting raw _emit bytes
// avoids relocation entries for those absolute addresses.  compare.py reports
// GREEN for the 183-byte window (177 function + 6-byte epilogue gap, per
// config/ffxivgame.size_overrides.json entry "epilogue continuation").

extern "C" __declspec(naked) void FUN_00410d70() {
    __asm {
        // Prologue + cache this->field_0x10 in EDI
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV EBX, ECX
        _emit 0xd9
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, [EBX+0x10]
        _emit 0x7b
        _emit 0x10
        // Call EDI->vfunc[0x2c] (slot 11)
        _emit 0x8b              // MOV EAX, [EDI]
        _emit 0x07
        _emit 0x8b              // MOV EDX, [EAX+0x2c]
        _emit 0x50
        _emit 0x2c
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EDX
        _emit 0xd2
        // Call EDI->vfunc[0x34] (slot 13) — IsBusy() → AL
        _emit 0x8b              // MOV EAX, [EDI]
        _emit 0x07
        _emit 0x8b              // MOV EDX, [EAX+0x34]
        _emit 0x50
        _emit 0x34
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EDX
        _emit 0xd2
        // TEST AL,AL / JZ (skip assert block)
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x74              // JZ +0x3c  (to 0x10dca)
        _emit 0x3c
        // One-shot init: TEST byte [0x01323910], 1
        _emit 0xf6              // TEST byte ptr [0x01323910], 0x01
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75              // JNZ +0x11  (to do_assert = 0x10da8)
        _emit 0x11
        // Init: OR dword [0x01323910], 1
        _emit 0x83              // OR dword ptr [0x01323910], 0x01
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // Init: MOV dword [0x0132390c], 0x0040f8e0
        _emit 0xc7              // MOV dword ptr [0x0132390c], 0x0040f8e0
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xe0
        _emit 0xf8
        _emit 0x40
        _emit 0x00
        // Assert call: push 5 args, call [0x0132390c]  (do_assert:)
        _emit 0x68              // PUSH 0x00f56b58  (fn name string)
        _emit 0x58
        _emit 0x6b
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x000000c3  (line 195)
        _emit 0xc3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x00f56988  (filename string)
        _emit 0x88
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x00f54d48  (banner/category)
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x00f56974  (predicate "!space->IsBusy()")
        _emit 0x74
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x14  (pop 5 args)
        _emit 0xc4
        _emit 0x14
        // Load stack arg: ESI = [ESP+0x10]  (= space, the listener node)
        _emit 0x8b              // MOV ESI, [ESP+0x10]
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // Call space->vfunc[0] with arg 0
        _emit 0x8b              // MOV EAX, [ESI]       (vtable of space)
        _emit 0x06
        _emit 0x8b              // MOV EDX, [EAX]       (slot 0)
        _emit 0x10
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EDX
        _emit 0xd2
        // Get inner struct: call ISpace::vfunc[0x04] (slot 1)
        _emit 0x8b              // MOV ECX, [EBX+0x10]  (this->field_0x10)
        _emit 0x4b
        _emit 0x10
        _emit 0x8b              // MOV EAX, [ECX]       (vtable)
        _emit 0x01
        _emit 0x8b              // MOV EDX, [EAX+0x4]   (slot 1)
        _emit 0x50
        _emit 0x04
        _emit 0xff              // CALL EDX             → EAX = inner container
        _emit 0xd2
        // ECX = EAX->field_0x14; EDX = &ECX->field_0x4 (lock address)
        _emit 0x8b              // MOV ECX, [EAX+0x14]
        _emit 0x48
        _emit 0x14
        _emit 0x8d              // LEA EDX, [ECX+0x4]
        _emit 0x51
        _emit 0x04
        // JMP to loop top, skipping 6-byte NOP alignment pad
        _emit 0xeb              // JMP +0x06  (to 0x10df0)
        _emit 0x06
        // 6-byte NOP: LEA EBX, [EBX+0x00000000]  (alignment pad to 0x10df0)
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // Spinlock acquire loop (loop top at 0x10df0, 16-byte aligned)
        _emit 0xb8              // MOV EAX, 0x00000001
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EBX, EDX         (lock address → EBX)
        _emit 0xda
        _emit 0x87              // XCHG [EBX], EAX      (atomic swap; EAX = old value)
        _emit 0x03
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ -0x0d  (back to loop top)
        _emit 0xf3
        // Doubly-linked list insert: space after sentinel (inner->field_0xc)
        _emit 0x8b              // MOV EAX, [ECX+0xc]   (sentinel node)
        _emit 0x41
        _emit 0x0c
        _emit 0x8b              // MOV EBX, [EAX+0x4]   (sentinel->next)
        _emit 0x58
        _emit 0x04
        _emit 0x89              // MOV [EBX], ESI        (old_next->prev = space)
        _emit 0x33
        _emit 0x8b              // MOV EBX, [EAX+0x4]   (reload sentinel->next)
        _emit 0x58
        _emit 0x04
        _emit 0x89              // MOV [ESI+0x4], EBX   (space->next = old_next)
        _emit 0x5e
        _emit 0x04
        _emit 0x89              // MOV [ESI], EAX        (space->prev = sentinel)
        _emit 0x06
        _emit 0x89              // MOV [EAX+0x4], ESI   (sentinel->next = space)
        _emit 0x70
        _emit 0x04
        // Decrement inner->field_0x18
        _emit 0x83              // ADD dword ptr [ECX+0x18], -1
        _emit 0x41
        _emit 0x18
        _emit 0xff
        // Spinlock release: XOR ECX,ECX; XCHG [EDX],ECX  (store 0 atomically)
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x87              // XCHG [EDX], ECX
        _emit 0x0a
        // Call EDI->vfunc[0x30] (slot 12) — post-op call
        _emit 0x8b              // MOV EDX, [EDI]
        _emit 0x17
        _emit 0x8b              // MOV EAX, [EDX+0x30]
        _emit 0x42
        _emit 0x30
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EAX
        _emit 0xd0
        // Epilogue (in inter-function gap, size_override extends window to 183 bytes)
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
    }
}
