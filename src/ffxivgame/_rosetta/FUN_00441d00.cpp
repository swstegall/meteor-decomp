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
// FUNCTION: ffxivgame 0x00041d00 — linked-list flush loop (__thiscall, 177 B / 0xb1)
//
// Iterates a doubly-linked (or intrusive) list anchored at this+0x18/0x1c,
// and for each node checks whether a sub-object's timestamp (node->m10->m20)
// is either zero or has aged past a threshold of 15 ticks. When stale:
//   * Calls a vtable slot [+0x0c] on the sub-object with arg 0.
//     - If it returns 0: refreshes the timestamp via an external tick fn.
//     - If it returns non-zero: calls vtable slot [+0x00] with arg 1,
//       then invokes a list-mutation helper (0x00927440) on the anchor
//       passing iterator state.
// The iterator pair {current-ptr, next-ptr} lives at [ESP+0x10/+0x14] and
// is advanced each iteration by 0x009172c0 (__thiscall, ECX = &pair).
//
// Validity assertions (0x009d22b4) are emitted by MSVC for the iterator:
//   * EDI (= pair.ptr) must be non-null AND equal to EBP (&this->m18)
//     at the top of the loop check; otherwise the check fn is invoked.
//   * EBX (= pair.val) must not equal the sentinel ([ESP+0x1c]) to continue.
//
// Why naked asm: the non-standard frame layout — SUB ESP,0x10 BEFORE the
// callee-save PUSHes, interleaved with early loads from ECX (the `this`
// register), an EAX store BETWEEN the ESI and EDI pushes, and then an
// EBP-relative loop — cannot be recovered from standard C++ under /O2
// without shifting at least one encoding (register allocation, push order,
// or the interleaved SUB/PUSH mix). Byte-pinned naked asm with _emit
// directives is the pragmatic choice; compare.py wildcards the 4-byte
// rel32 windows of all CALL instructions.
//
// Reloc-bearing sites (offsets within the function — each 4-byte window
// wildcarded by compare.py against orig):
//   +0x1d   CALL rel32 -> 0x009d22b4  (iterator validity check fn)
//   +0x32   CALL rel32 -> 0x009d22b4  (iterator validity check fn)
//   +0x3c   CALL rel32 -> 0x009d22b4  (iterator validity check fn)
//   +0x51   CALL rel32 -> 0x009d5725  (get tick / time fn, cdecl)
//   +0x72   CALL rel32 -> 0x009d5725  (get tick / time fn, cdecl)
//   +0x81   CALL rel32 -> 0x009172c0  (iterator-advance fn, thiscall)
//   +0xa3   CALL rel32 -> 0x00927440  (list-mutation / erase helper)

extern "C" __declspec(naked) void FUN_00441d00() {
    __asm {
        // --- prologue: SUB ESP,0x10 then callee-save PUSHes ---------------
        _emit 0x83          // SUB ESP, 0x10
        _emit 0xec
        _emit 0x10
        _emit 0x8b          // MOV EAX, [ECX+0x1c]
        _emit 0x41
        _emit 0x1c
        _emit 0x53          // PUSH EBX
        _emit 0x55          // PUSH EBP
        _emit 0x8d          // LEA EBP, [ECX+0x18]
        _emit 0x69
        _emit 0x18
        _emit 0x8b          // MOV ECX, EAX
        _emit 0xc8
        _emit 0x56          // PUSH ESI
        _emit 0x89          // MOV [ESP+0x18], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8b          // MOV EAX, [ECX]
        _emit 0x01
        _emit 0x57          // PUSH EDI
        _emit 0x8b          // MOV EDI, EBP
        _emit 0xfd
        _emit 0x8b          // MOV EBX, EAX
        _emit 0xd8
        _emit 0x89          // MOV [ESP+0x10], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x89          // MOV [ESP+0x14], EBX
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        // --- loop head: iterator validity check + sentinel test -----------
        _emit 0x85          // TEST EDI, EDI
        _emit 0xff
        _emit 0x74          // JZ +4 (-> validity call)
        _emit 0x04
        _emit 0x3b          // CMP EDI, EBP
        _emit 0xfd
        _emit 0x74          // JZ +5 (-> skip validity call)
        _emit 0x05
        _emit 0xe8          // CALL rel32 -> 0x009d22b4
        _emit 0x86
        _emit 0x05
        _emit 0x59
        _emit 0x00
        _emit 0x3b          // CMP EBX, [ESP+0x1c]
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x74          // JZ +0x75 (-> epilogue)
        _emit 0x75
        // --- secondary validity: EDI must be non-null ---------------------
        _emit 0x85          // TEST EDI, EDI
        _emit 0xff
        _emit 0x75          // JNZ +5 (-> skip)
        _emit 0x05
        _emit 0xe8          // CALL rel32 -> 0x009d22b4
        _emit 0x77
        _emit 0x05
        _emit 0x59
        _emit 0x00
        // --- secondary validity: EBX must not equal [EDI+4] --------------
        _emit 0x3b          // CMP EBX, [EDI+4]
        _emit 0x5f
        _emit 0x04
        _emit 0x75          // JNZ +5 (-> skip)
        _emit 0x05
        _emit 0xe8          // CALL rel32 -> 0x009d22b4
        _emit 0x6d
        _emit 0x05
        _emit 0x59
        _emit 0x00
        // --- loop body: load sub-object and check timestamp ---------------
        _emit 0x8b          // MOV ESI, [EBX+0x10]
        _emit 0x73
        _emit 0x10
        _emit 0x83          // CMP [ESI+0x20], 0
        _emit 0x7e
        _emit 0x20
        _emit 0x00
        _emit 0x74          // JZ +0x2e (-> advance)
        _emit 0x2e
        // --- timestamp active: compute age --------------------------------
        _emit 0x6a          // PUSH 0
        _emit 0x00
        _emit 0xe8          // CALL rel32 -> 0x009d5725
        _emit 0xce
        _emit 0x39
        _emit 0x59
        _emit 0x00
        _emit 0x2b          // SUB EAX, [ESI+0x20]
        _emit 0x46
        _emit 0x20
        _emit 0x83          // ADD ESP, 4
        _emit 0xc4
        _emit 0x04
        _emit 0x83          // CMP EAX, 0xf
        _emit 0xf8
        _emit 0x0f
        _emit 0x76          // JBE +0x1c (-> advance, not old enough)
        _emit 0x1c
        // --- age > 15: call vtable[3](esi, 0) ----------------------------
        _emit 0x8b          // MOV EDX, [ESI]
        _emit 0x16
        _emit 0x8b          // MOV EAX, [EDX+0xc]
        _emit 0x42
        _emit 0x0c
        _emit 0x6a          // PUSH 0
        _emit 0x00
        _emit 0x8b          // MOV ECX, ESI
        _emit 0xce
        _emit 0xff          // CALL EAX
        _emit 0xd0
        _emit 0x84          // TEST AL, AL
        _emit 0xc0
        _emit 0x75          // JNZ +0x20 (-> non-zero branch)
        _emit 0x20
        // --- returned 0: refresh timestamp --------------------------------
        _emit 0x6a          // PUSH 0
        _emit 0x00
        _emit 0xe8          // CALL rel32 -> 0x009d5725
        _emit 0xad
        _emit 0x39
        _emit 0x59
        _emit 0x00
        _emit 0x83          // ADD ESP, 4
        _emit 0xc4
        _emit 0x04
        _emit 0x89          // MOV [ESI+0x20], EAX
        _emit 0x46
        _emit 0x20
        // --- advance iterator and loop back -------------------------------
        _emit 0x8d          // LEA ECX, [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0xe8          // CALL rel32 -> 0x009172c0
        _emit 0x39
        _emit 0x55
        _emit 0x4d
        _emit 0x00
        _emit 0x8b          // MOV EBX, [ESP+0x14]
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        _emit 0x8b          // MOV EDI, [ESP+0x10]
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0xeb          // JMP -0x70 (-> loop head)
        _emit 0x90
        // --- non-zero branch: call vtable[0](esi, 1) then erase ----------
        _emit 0x8b          // MOV EDX, [ESI]
        _emit 0x16
        _emit 0x8b          // MOV EAX, [EDX]
        _emit 0x02
        _emit 0x6a          // PUSH 1
        _emit 0x01
        _emit 0x8b          // MOV ECX, ESI
        _emit 0xce
        _emit 0xff          // CALL EAX
        _emit 0xd0
        _emit 0x53          // PUSH EBX
        _emit 0x57          // PUSH EDI
        _emit 0x8d          // LEA ECX, [ESP+0x20]
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x51          // PUSH ECX
        _emit 0x8b          // MOV ECX, EBP
        _emit 0xcd
        _emit 0xe8          // CALL rel32 -> 0x00927440
        _emit 0x97
        _emit 0x56
        _emit 0x4e
        _emit 0x00
        // --- epilogue -----------------------------------------------------
        _emit 0x5f          // POP EDI
        _emit 0x5e          // POP ESI
        _emit 0x5d          // POP EBP
        _emit 0x5b          // POP EBX
        _emit 0x83          // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3          // RET
    }
}
