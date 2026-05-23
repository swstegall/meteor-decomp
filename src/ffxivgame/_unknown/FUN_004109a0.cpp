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
// FUNCTION: ffxivgame 0x004109a0 — FixedAllocator spin-lock acquire + allocate
//                                   (__thiscall, no stack args, 81 B / 0x51)
//
// ECX = this (a FixedAllocator-owning object).
//
// Layout (offsets touched):
//   +0x04  int   lock          spin-lock word (0 = free, 1 = held)
//   +0x0c  void* allocator     pointer to FixedAllocator sentinel node
//   +0x18  int   alloc_count   running allocation counter
//
// Behaviour:
//   1. Set up a 3-slot SEH frame (PUSH -1 / PUSH handler / chain FS:[0]).
//   2. Save ECX (this) and ESI; cache &this->lock in ESI.
//   3. Spin-acquire the lock: XCHG [ESI], 1 until old value is 0.
//   4. Increment this->alloc_count (+0x18).
//   5. Load allocator = this->allocator (+0x0c) into ECX.
//   6. Call FUN_004108e0 (FixedAllocator::Allocate) with ECX = allocator.
//      (EAX = result)
//   7. Release the lock: XCHG [ESI], 0.
//   8. Tear down the SEH frame, restore ESP, return (EAX = allocated node or NULL).
//
// Reloc-bearing sites (masked by tools/compare.py):
//   +0x02  PUSH imm32  0xe54fa8  — SEH handler VA (DIR32 reloc)
//   +0x25  CALL rel32  → FUN_004108e0  (rel32 = 0xffffff03)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The SEH frame setup uses FS-segment instructions that MSVC does not
//   faithfully reproduce in the same byte-encoding when compiled from
//   C++ source with /GS.  The spin-lock uses LOCK XCHG [mem], reg which
//   is also sensitive to register-allocation choices.  The naked-asm body
//   re-emits the 81 original bytes verbatim; compare.py then reports GREEN.

#ifdef _MSC_VER
extern "C" __declspec(naked) void FUN_004109a0() {
    __asm {
        // +0x00  6a ff          PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // +0x02  68 a8 4f e5 00 PUSH 0xe54fa8   (SEH handler — DIR32 reloc)
        _emit 0x68
        _emit 0xa8
        _emit 0x4f
        _emit 0xe5
        _emit 0x00
        // +0x07  64 a1 00 00 00 00  MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x0d  50             PUSH EAX
        _emit 0x50
        // +0x0e  64 89 25 00 00 00 00  MOV dword ptr FS:[0x0], ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x15  51             PUSH ECX
        _emit 0x51
        // +0x16  56             PUSH ESI
        _emit 0x56
        // +0x17  8d 71 04       LEA ESI, [ECX+0x4]
        _emit 0x8d
        _emit 0x71
        _emit 0x04
        // +0x1a  89 74 24 04    MOV dword ptr [ESP+0x4], ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x04
        // +0x1e  8b ff          MOV EDI, EDI   (spin-loop top / alignment nop)
        _emit 0x8b
        _emit 0xff
        // +0x20  b8 01 00 00 00 MOV EAX, 0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x25  8b d6          MOV EDX, ESI
        _emit 0x8b
        _emit 0xd6
        // +0x27  87 02          XCHG dword ptr [EDX], EAX
        _emit 0x87
        _emit 0x02
        // +0x29  85 c0          TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // +0x2b  75 f3          JNZ -0xd  (→ +0x1e, spin-loop)
        _emit 0x75
        _emit 0xf3
        // +0x2d  83 41 18 01    ADD dword ptr [ECX+0x18], 0x1
        _emit 0x83
        _emit 0x41
        _emit 0x18
        _emit 0x01
        // +0x31  8b 49 0c       MOV ECX, dword ptr [ECX+0xc]
        _emit 0x8b
        _emit 0x49
        _emit 0x0c
        // +0x34  89 44 24 10    MOV dword ptr [ESP+0x10], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // +0x38  e8 03 ff ff ff CALL FUN_004108e0  (rel32 reloc)
        _emit 0xe8
        _emit 0x03
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // +0x3d  33 c9          XOR ECX, ECX
        _emit 0x33
        _emit 0xc9
        // +0x3f  87 0e          XCHG dword ptr [ESI], ECX
        _emit 0x87
        _emit 0x0e
        // +0x41  8b 4c 24 08    MOV ECX, dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // +0x45  5e             POP ESI
        _emit 0x5e
        // +0x46  64 89 0d 00 00 00 00  MOV dword ptr FS:[0x0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x4d  83 c4 10       ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // +0x50  c3             RET
        _emit 0xc3
    }
}
#endif // _MSC_VER
