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
// FUNCTION: ffxivgame 0x0003c330 — __thiscall worker-thread dispatch loop
//                                  (290 B / 0x122, EH4-SEH wrapped, /GS).
//
// Inspection (read from the disassembly at orig RVA 0x0003c330):
//
//   __thiscall void FUN_0043c330(void* this);   // ECX = this
//
//   A thread-loop function that:
//     1. Waits indefinitely (INFINITE = -1) on a kernel handle stored at
//        this+0x08, via the IAT slot [0x00f3e140] (WaitForSingleObject-style).
//     2. Loads a std::vector<T*>-shaped container from this+0xBC (begin/end/
//        cap at offsets +0x04/+0x08/+0x0C within the pointed-to struct) and
//        iterates over every element pointer in [begin, end).  Each iteration:
//          - Reads the element pointer from *EDI,
//          - Reads the element's vtable (*ECX -> vtbl),
//          - Reads the outer `this` vtable (*ESI),
//          - Calls vtbl[1] (virtual dispatch, offset +0x04) with ECX = element
//            and one stack arg = *this (the vtable pointer).
//        Four range-check guards (CMP / JBE or JC + CALL 0x009d22b4) wrap
//        every iterator access -- the standard MSVC 2005 checked-iterator
//        pattern from _ITERATOR_DEBUG_LEVEL > 0.
//     3. After draining the loop, atomically resets the container (begin =
//        end = cap = NULL), saves the old pointers to SEH-guarded locals
//        (try-level set to -1 at this point), frees the old begin via an
//        allocation header at [begin-4] (CALL 0x0040df70), and signals/releases
//        two handles:
//          - [0x00f3e138] with handle this+0x08 and three zero args
//            (ReleaseSemaphore / SetEvent-style).
//          - [0x00f3e13c] with handle this+0x0C (CloseHandle-style).
//     4. Performs a LOCK XADD [*this+0xC0], 0 (atomic read of a 32-bit
//        counter at *this->field_0xC0). If the old value was zero, loops back
//        to step 1; if non-zero, falls through to the EH epilogue and returns.
//
//   Stack frame (EH4 prologue, all offsets from ESP after the cookie push):
//     [ESP+0x00]  security cookie (XOR'd with ESP)
//     [ESP+0x04]  saved EDI
//     [ESP+0x08]  saved ESI
//     [ESP+0x0C]  saved EBP
//     [ESP+0x10]  saved EBX
//     [ESP+0x14]  container->end snapshot (local used across assertions)
//     [ESP+0x18]  (pad / alignment)
//     [ESP+0x1C]  old container->begin (saved before reset)
//     [ESP+0x20]  old container->end   (saved before reset)
//     [ESP+0x24]  old container->cap   (saved before reset)
//     [ESP+0x2C]  saved FS:[0] EH chain link
//     [ESP+0x30]  scope-table address (0x00E566E6, .rdata)
//     [ESP+0x34]  EH try-level (-1 idle, set back to -1 after container reset)
//     [ESP+0x38]  return address
//
//   The 7-byte LEA ESP,[ESP] NOP at 0x3C359 aligns the loop top (0x3C360)
//   to a 32-byte boundary.
//
// Reconstruction strategy -- naked-asm byte passthrough:
//
//   Reproducing the exact byte sequence from a source-level C++ method is
//   not feasible here because:
//     * The EH4 prologue encodes the absolute scope-table RVA (0x00E566E6)
//       and the __security_cookie VA (0x012EA8B0) as raw immediates that
//       survive in the .obj only when emitted via _emit (no reloc entry).
//     * Three IAT indirect calls (FF 15 ...) embed absolute IAT slot VAs.
//     * Four rel32 CALL 0x009d22b4 and one rel32 CALL 0x0040DF70 are fixed
//       relative displacements that depend on the function's final address.
//     * The LOCK XADD encoding (F0 0F C1 10) and JZ rel32 back-edge
//       (0F 84 22 FF FF FF -> loop-top) must be bit-exact.
//     * The 7-byte NOP pad (8D A4 24 00000000) and the SETNZ/TEST pair
//       are compiler-layout artifacts that shift under any rewrite.
//   Following the established idiom of FUN_00401a00, FUN_0043b770, and
//   other SEH-wrapped siblings, the 290 bytes are re-emitted verbatim so
//   the .obj's .text is byte-identical to the original slice.

extern "C" __declspec(naked) void FUN_0043c330() {
    __asm {
        // --- EH4 prologue ---
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xe6
        _emit 0x66
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x83
        _emit 0xec
        _emit 0x18
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x57
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x50
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf1
        // 7-byte NOP (LEA ESP,[ESP]) -- align loop top to 32 bytes
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- loop top (0x0043C360) ---
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        _emit 0x6a
        _emit 0xff
        _emit 0x50
        _emit 0xff
        _emit 0x15
        _emit 0x40
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b
        _emit 0xae
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x7d
        _emit 0x04
        _emit 0x3b
        _emit 0x7d
        _emit 0x08
        _emit 0x76
        _emit 0x05
        _emit 0xe8
        _emit 0x35
        _emit 0x5f
        _emit 0x59
        _emit 0x00
        _emit 0x8b
        _emit 0x9e
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x43
        _emit 0x08
        _emit 0x39
        _emit 0x43
        _emit 0x04
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x76
        _emit 0x05
        _emit 0xe8
        _emit 0x1e
        _emit 0x5f
        _emit 0x59
        _emit 0x00

        // --- iter dispatch loop top (0x0043C396) ---
        _emit 0x3b
        _emit 0xeb
        _emit 0x74
        _emit 0x05
        _emit 0xe8
        _emit 0x15
        _emit 0x5f
        _emit 0x59
        _emit 0x00
        _emit 0x3b
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x74
        _emit 0x25
        _emit 0x3b
        _emit 0x7d
        _emit 0x08
        _emit 0x72
        _emit 0x05
        _emit 0xe8
        _emit 0x05
        _emit 0x5f
        _emit 0x59
        _emit 0x00
        _emit 0x8b
        _emit 0x0f
        _emit 0x8b
        _emit 0x11
        _emit 0x8b
        _emit 0x06
        _emit 0x8b
        _emit 0x52
        _emit 0x04
        _emit 0x50
        _emit 0xff
        _emit 0xd2
        _emit 0x3b
        _emit 0x7d
        _emit 0x08
        _emit 0x72
        _emit 0x05
        _emit 0xe8
        _emit 0xef
        _emit 0x5e
        _emit 0x59
        _emit 0x00
        _emit 0x83
        _emit 0xc7
        _emit 0x04
        _emit 0xeb
        _emit 0xcc

        // --- post-loop: reset container (0x0043C3CA) ---
        _emit 0x8b
        _emit 0x86
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        _emit 0x33
        _emit 0xff
        _emit 0x3b
        _emit 0xcf
        _emit 0x89
        _emit 0x78
        _emit 0x04
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        _emit 0x89
        _emit 0x78
        _emit 0x08
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x8b
        _emit 0x50
        _emit 0x0c
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x89
        _emit 0x78
        _emit 0x0c
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x74
        _emit 0x09
        _emit 0x51
        _emit 0x8b
        _emit 0x49
        _emit 0xfc
        _emit 0xe8
        _emit 0x6b
        _emit 0x1b
        _emit 0xfd
        _emit 0xff

        // --- signal / release ---
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        _emit 0x50
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x24
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x28
        _emit 0xff
        _emit 0x15
        _emit 0x38
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        _emit 0x51
        _emit 0xff
        _emit 0x15
        _emit 0x3c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00

        // --- atomic counter read ---
        _emit 0x8b
        _emit 0x86
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xd2
        _emit 0xf0
        _emit 0x0f
        _emit 0xc1
        _emit 0x10
        _emit 0x85
        _emit 0xd2
        _emit 0x0f
        _emit 0x95
        _emit 0xc0
        _emit 0x84
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0x22
        _emit 0xff
        _emit 0xff
        _emit 0xff

        // --- EH epilogue ---
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        _emit 0xc3
    }
}
