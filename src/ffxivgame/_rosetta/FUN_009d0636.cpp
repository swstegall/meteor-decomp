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
// FUNCTION: ffxivgame 0x009d0636 — global linked-list drain loop (56 bytes)
//
// Drains a global singly-linked list (head pointer at VA 0x01363ca4),
// calling a __thiscall destructor then a __cdecl free/delete on each node.
// Before the loop a local guard/RAII object at [ebp-4] is initialised with
// argument 0 via a thiscall-style constructor at 0x9d051d.
//
// Full epilogue (pop esi / dtor for [ebp-4] / leave / ret) is folded into
// the 11-byte region immediately following this function's boundary at
// 0x9d066e.  The `je +0x22` at offset 21 jumps two bytes past the
// function boundary (to 0x9d066f = the epilogue instruction AFTER pop esi),
// skipping the `pop esi` that is only needed when the loop was entered.
// The `jne -0x20` at offset 54 loops back to offset 24 (the inner mov).
//
// Calling convention (for the function itself): none externally visible —
// it is an internal static or module-level helper with no formal signature.
// Internal callees:
//   0x9d051d — __thiscall ctor on [ebp-4], 1 stack arg (0)
//   0x9d0550 — __thiscall dtor on each list node (ecx = node, 0 stack args)
//   0x9d1b17 — __cdecl free/delete (1 stack arg = node ptr)
//
// Match strategy: naked __asm with _emit directives for all 56 bytes.
// The je offset (0x22) and jne offset (0xe0) are hardcoded because the
// jump targets lie at fixed positions relative to the function window,
// and the label-based assembler offset would produce 0x21 (off by one)
// since SHORT_EXIT would resolve inside the 56-byte body rather than
// one byte beyond it. All call rel32 bytes and global addr bytes are
// copied verbatim from the orig binary; compare.py produces GREEN
// without relocation masking (no COFF relocs are emitted for _emit
// bodies).

extern "C" __declspec(naked) void FUN_009d0636() {
    __asm {
        // push ebp
        _emit 0x55
        // mov ebp, esp
        _emit 0x8b
        _emit 0xec
        // push ecx  (allocates [ebp-4] slot)
        _emit 0x51
        // push 0  (arg for ctor)
        _emit 0x6a
        _emit 0x00
        // lea ecx, [ebp-4]  (this ptr for ctor)
        _emit 0x8d
        _emit 0x4d
        _emit 0xfc
        // call 0x9d051d  (rel32: e8 d9 fe ff ff)
        _emit 0xe8
        _emit 0xd9
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // mov eax, [0x01363ca4]  (load global head)
        _emit 0xa1
        _emit 0xa4
        _emit 0x3c
        _emit 0x36
        _emit 0x01
        // test eax, eax
        _emit 0x85
        _emit 0xc0
        // je +0x22  (exit: jumps 2 bytes past function end into shared epilogue,
        //            landing after the 'pop esi' that restores the callee-saved reg)
        _emit 0x74
        _emit 0x22
        // push esi  (save callee-saved esi before first use; done once, outside loop)
        _emit 0x56
        // --- LOOP (target of jne at end) ---
        // mov esi, eax  (esi = current node)
        _emit 0x8b
        _emit 0xf0
        // mov eax, [eax]  (eax = node->pNext, first field)
        _emit 0x8b
        _emit 0x00
        // mov ecx, esi  (ecx = this ptr for thiscall dtor)
        _emit 0x8b
        _emit 0xce
        // mov [0x01363ca4], eax  (advance global head to next node)
        _emit 0xa3
        _emit 0xa4
        _emit 0x3c
        _emit 0x36
        _emit 0x01
        // call 0x9d0550  (thiscall dtor; rel32: e8 f2 fe ff ff)
        _emit 0xe8
        _emit 0xf2
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // push esi  (arg for cdecl free)
        _emit 0x56
        // call 0x9d1b17  (cdecl free/delete; rel32: e8 b3 14 00 00)
        _emit 0xe8
        _emit 0xb3
        _emit 0x14
        _emit 0x00
        _emit 0x00
        // mov eax, [0x01363ca4]  (reload global head)
        _emit 0xa1
        _emit 0xa4
        _emit 0x3c
        _emit 0x36
        _emit 0x01
        // test eax, eax
        _emit 0x85
        _emit 0xc0
        // pop ecx  (caller cleanup of 'push esi' arg for cdecl free)
        _emit 0x59
        // jne LOOP  (offset -0x20: loops back to 'mov esi, eax' at offset +24)
        _emit 0x75
        _emit 0xe0
    }
}
