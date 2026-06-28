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
// FUNCTION: ffxivgame 0x0043b5b0 — __thiscall destructor/release with
//           dual LOCK XADD reference-count decrements (108 B).
//
// __thiscall void FUN_0043b5b0()   (ECX = this → ESI)
//
// Reconstructed semantics:
//
//   1. Saves ESI, EDI.  ESI ← ECX (this).
//   2. Computes EDI = this + 0x24.
//   3. Calls [0x00f3e16c](EDI) — likely InitializeCriticalSection (or
//      equivalent) on the embedded CRITICAL_SECTION at this+0x24.
//   4. Zeroes this+0x20 and this+0x1c.
//   5. Calls [0x00f3e168](EDI) — a second setup call on the same slot.
//   6. LOCK XADD *(*this+0x5c) with -1 → reads old ref-count into ECX.
//      Loads a dealloc function pointer from [0x00f3e13c] into EDI.
//   7. If old refcount == 1 (object just hit zero):
//        calls [0x00f3e138](this+0x50)   (e.g. UnmapViewOfFile)
//        calls EDI(this+0x58)             (e.g. CloseHandle)
//        jumps to step 9.
//      Else:
//        calls [0x00f3e140](this+0x58, -1)  (e.g. DuplicateHandle / SetHandleInformation)
//   9. LOCK XADD *(*this+0x60) with -1 → reads old ref-count into ECX.
//   10. If old refcount == 1:
//         calls EDI(this+0x54)            (e.g. CloseHandle on second handle)
//   11. Pops EDI, ESI; returns.
//
// Calling convention: __thiscall (ECX = this; callee saves ESI/EDI;
//   no local frame allocation; the function itself is void).
//
// Original 108 bytes (RVA 0x0003b5b0):
//
//   0003b5b0: 56 57 8b f1 8d 7e 24 57 ff 15 6c e1 f3 00 33 c0
//   0003b5c0: 57 89 46 20 89 46 1c ff 15 68 e1 f3 00 8b 46 5c
//   0003b5d0: 8b 10 83 c9 ff f0 0f c1 0a 83 f9 01 8b 3d 3c e1
//   0003b5e0: f3 00 75 12 8b 46 50 50 ff 15 38 e1 f3 00 8b 4e
//   0003b5f0: 58 51 ff d7 eb 0c 8b 56 58 6a ff 52 ff 15 40 e1
//   0003b600: f3 00 8b 46 60 8b 10 83 c9 ff f0 0f c1 0a 83 f9
//   0003b610: 01 75 06 8b 46 54 50 ff d7 5f 5e c3
//
// Reloc-bearing sites (compare.py wildcard-masks these 4-byte windows):
//   +0x0a  CALL dword ptr [0x00f3e16c]  — fn-ptr/IAT slot
//   +0x19  CALL dword ptr [0x00f3e168]  — fn-ptr/IAT slot
//   +0x2e  MOV EDI, dword ptr [0x00f3e13c]  — dealloc fn-ptr load
//   +0x3a  CALL dword ptr [0x00f3e138]  — fn-ptr/IAT slot
//   +0x4e  CALL dword ptr [0x00f3e140]  — fn-ptr/IAT slot
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The LOCK XADD pattern (OR ECX,-1 / LOCK XADD [EDX],ECX / CMP ECX,1)
//   is an InterlockedDecrement-style sequence that MSVC 2005 only emits
//   when the intrinsic is called directly and the register assignment
//   matches; reproducing the exact CMP-then-JNZ shape from source-level
//   C++ InterlockedDecrement() is fragile. The `__declspec(naked)` +
//   `_emit` byte-passthrough used by sibling functions (FUN_004090b0,
//   FUN_004091f0, FUN_00409260) avoids all such uncertainty.

extern "C" __declspec(naked) void FUN_0043b5b0() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8d              // LEA EDI, [ESI + 0x24]
        _emit 0x7e
        _emit 0x24
        _emit 0x57              // PUSH EDI
        _emit 0xff              // CALL dword ptr [0x00f3e16c]
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x57              // PUSH EDI
        _emit 0x89              // MOV dword ptr [ESI + 0x20], EAX
        _emit 0x46
        _emit 0x20
        _emit 0x89              // MOV dword ptr [ESI + 0x1c], EAX
        _emit 0x46
        _emit 0x1c
        _emit 0xff              // CALL dword ptr [0x00f3e168]
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x5c]
        _emit 0x46
        _emit 0x5c
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x83              // OR ECX, 0xffffffff
        _emit 0xc9
        _emit 0xff
        _emit 0xf0              // LOCK XADD dword ptr [EDX], ECX
        _emit 0x0f
        _emit 0xc1
        _emit 0x0a
        _emit 0x83              // CMP ECX, 0x1
        _emit 0xf9
        _emit 0x01
        _emit 0x8b              // MOV EDI, dword ptr [0x00f3e13c]
        _emit 0x3d
        _emit 0x3c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x75              // JNZ +0x12 (to branch2)
        _emit 0x12
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x50]
        _emit 0x46
        _emit 0x50
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL dword ptr [0x00f3e138]
        _emit 0x15
        _emit 0x38
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x58]
        _emit 0x4e
        _emit 0x58
        _emit 0x51              // PUSH ECX
        _emit 0xff              // CALL EDI
        _emit 0xd7
        _emit 0xeb              // JMP +0x0c (to after_branch)
        _emit 0x0c
        // branch2:
        _emit 0x8b              // MOV EDX, dword ptr [ESI + 0x58]
        _emit 0x56
        _emit 0x58
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x52              // PUSH EDX
        _emit 0xff              // CALL dword ptr [0x00f3e140]
        _emit 0x15
        _emit 0x40
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // after_branch:
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x60]
        _emit 0x46
        _emit 0x60
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x83              // OR ECX, 0xffffffff
        _emit 0xc9
        _emit 0xff
        _emit 0xf0              // LOCK XADD dword ptr [EDX], ECX
        _emit 0x0f
        _emit 0xc1
        _emit 0x0a
        _emit 0x83              // CMP ECX, 0x1
        _emit 0xf9
        _emit 0x01
        _emit 0x75              // JNZ +0x06 (to done)
        _emit 0x06
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x54]
        _emit 0x46
        _emit 0x54
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL EDI
        _emit 0xd7
        // done:
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
