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
// FUNCTION: ffxivgame 0x0000ebb0 — __thiscall doubly-linked list node removal
//           with size-tracking and free callback (48 B).
//
// Asm (48 bytes @ orig RVA 0x0040ebb0):
//   param_2 (2nd stack arg) is a handle into a node's list header at
//   negative offsets:
//     handle[-0x10] = prev ptr (pointer to prev node's list_head)
//     handle[-0xc]  = next ptr (pointer to next node's list_head)
//     handle[-0x8]  = offset from handle to payload block
//   Unlinks the node (writes prev into *next and next into prev[+4]),
//   subtracts payload[+4] from this->m_size at +0x14, then calls
//   this->m_free_fn (at +0x1c) with payload[0].
//
// Calling convention: __thiscall, 2 stack args (param_1 unused), void return.
//
// Source-level C++ produces ADD EDX,EAX (03 d0) instead of ADD EAX,EDX
// (03 c2) at the block-ptr computation — MSVC 2005 register allocator
// assigns block to EDX rather than EAX, cascading to different registers
// for block[0] and m_free_fn (CALL EAX vs CALL ECX).  Since no
// instruction carries a relocated absolute address, naked-asm byte
// passthrough is the cleanest fix.

extern "C" __declspec(naked) void FUN_0040ebb0() {
    __asm {
        _emit 0x8b  // MOV EAX, [ESP+8]       ; param_2 (2nd stack arg)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b  // MOV EDX, [EAX-0xc]     ; next ptr field
        _emit 0x50
        _emit 0xf4
        _emit 0x56  // PUSH ESI
        _emit 0x8b  // MOV ESI, [EAX-0x10]    ; prev ptr field
        _emit 0x70
        _emit 0xf0
        _emit 0x89  // MOV [EDX], ESI          ; *next = prev  (unlink step 1)
        _emit 0x32
        _emit 0x8b  // MOV EDX, [EAX-0x10]    ; reload prev ptr
        _emit 0x50
        _emit 0xf0
        _emit 0x8b  // MOV ESI, [EAX-0xc]     ; reload next ptr
        _emit 0x70
        _emit 0xf4
        _emit 0x89  // MOV [EDX+4], ESI        ; prev[4] = next  (unlink step 2)
        _emit 0x72
        _emit 0x04
        _emit 0x8b  // MOV EDX, [EAX-8]        ; offset to payload block
        _emit 0x50
        _emit 0xf8
        _emit 0x03  // ADD EAX, EDX             ; EAX = handle + offset = block ptr
        _emit 0xc2
        _emit 0x8b  // MOV EDX, [EAX+4]         ; block[1] (size)
        _emit 0x50
        _emit 0x04
        _emit 0x29  // SUB [ECX+0x14], EDX       ; this->m_size -= block[1]
        _emit 0x51
        _emit 0x14
        _emit 0x8b  // MOV EAX, [EAX]            ; block[0] (arg to free_fn)
        _emit 0x00
        _emit 0x8b  // MOV ECX, [ECX+0x1c]       ; this->m_free_fn
        _emit 0x49
        _emit 0x1c
        _emit 0x50  // PUSH EAX
        _emit 0xff  // CALL ECX
        _emit 0xd1
        _emit 0x83  // ADD ESP, 4                 ; caller cleanup (__cdecl m_free_fn arg)
        _emit 0xc4
        _emit 0x04
        _emit 0x5e  // POP ESI
        _emit 0xc2  // RET 8                      ; __thiscall, clean 2 stack args
        _emit 0x08
        _emit 0x00
    }
}
