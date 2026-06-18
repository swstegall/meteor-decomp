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
// FUNCTION: ffxivgame 0x00432710 — __thiscall linked-list foreach virtual-caller
//                                  (67 B / 0x43)
//
// Calling convention: __thiscall (ECX = this; no stack args; void return)
// Callee-saved: EBX, ESI, EDI (pushed in prologue; only EDI popped inside
// the 67-byte function body — POP ESI, POP EBX, RET follow at 0x32753 in
// the adjacent fragment recorded by symbols.json)
//
// Semantics (read from the asm at RVA 0x00032710):
//
//   EAX = *(this + 8)           — pointer to the list sentinel node
//   ESI = *EAX                  — first list node (list.begin())
//   EDI = this + 4              — address of the list's head/container slot
//   EBX = *(this + 8)           — end sentinel (same pointer as EAX)
//
//   Loop:
//     for (ESI = *[this+8]; ESI != [EDI+4]; ESI = *ESI)
//         (*[ESI+8])->vtable[1]();   // virtual call: 2nd slot
//
// MSVC debug-STL iterator-validity checks (_ITERATOR_DEBUG_LEVEL):
//
//   CMP EDI, EDI          — always equal; JZ always taken; CALL at
//                           0x00432727 is dead (never reached)
//   CMP ESI, [EDI+4]      — pre-call: assert current != end
//     JNZ skip; CALL debug_fn (VA 0x009d22b4)
//   CMP ESI, [EDI+4]      — post-call: assert current still != end
//     JNZ skip; CALL debug_fn
//
// Three dead bytes at RVA 0x3271d–0x3271f sit between the initial forward
// JMP (eb 03) and the loop top (CMP EDI,EDI at 0x32720). They are never
// executed; the first iteration enters via the forward JMP, and every
// subsequent back-edge also jumps to 0x32720, bypassing them.
//
// Reloc-bearing sites: none — all CALL rel32 displacements and the
// short JMPs are emitted as raw immediate bytes matching the binary.
//
// Reconstruction strategy — naked-asm byte passthrough (same approach as
// FUN_00401460, FUN_00406fa0, FUN_00408780). Source-level C++ cannot
// reproduce the exact debug-STL CALL sequences and the CMP EDI,EDI /
// dead-bytes pattern byte-for-byte without the complete MSVC 2005 STL
// debug headers + matching register-allocation context. The _emit
// passthrough produces an identical .text section.

extern "C" __declspec(naked) void FUN_00432710() {
    __asm {
        // Prologue: load list root, save callee-saved, set up ESI/EDI
        _emit 0x8b  // MOV EAX, dword ptr [ECX + 0x8]
        _emit 0x41
        _emit 0x08
        _emit 0x53  // PUSH EBX
        _emit 0x56  // PUSH ESI
        _emit 0x8b  // MOV ESI, dword ptr [EAX]   (first node = *sentinel)
        _emit 0x30
        _emit 0x57  // PUSH EDI
        _emit 0x8d  // LEA EDI, [ECX + 0x4]        (EDI = &this->list_head)
        _emit 0x79
        _emit 0x04
        _emit 0xeb  // JMP +3   →  skip dead bytes to loop top (0x32720)
        _emit 0x03
        // Dead bytes at RVA 0x3271d–0x3271f (never executed)
        // LEA ECX, [ECX + 0x00] — MSVC 3-byte NOP-equivalent padding
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // Loop top at RVA 0x32720 — always-true container-validity check
        _emit 0x3b  // CMP EDI, EDI   (ZF always 1)
        _emit 0xff
        _emit 0x8b  // MOV EBX, dword ptr [EDI + 0x4]   (end sentinel)
        _emit 0x5f
        _emit 0x04
        _emit 0x74  // JZ +5   →  skip dead CALL (always taken)
        _emit 0x05
        _emit 0xe8  // CALL VA 0x009d22b4   (debug fn, dead)
        _emit 0x88
        _emit 0xfb
        _emit 0x59
        _emit 0x00
        // Loop exit check: if current node == end sentinel, exit
        _emit 0x3b  // CMP ESI, EBX
        _emit 0xf3
        _emit 0x74  // JZ +0x22   →  loop exit (POP EDI at offset 66)
        _emit 0x22
        // Pre-call debug check: assert(current != end)
        _emit 0x3b  // CMP ESI, dword ptr [EDI + 0x4]
        _emit 0x77
        _emit 0x04
        _emit 0x75  // JNZ +5   →  skip debug call
        _emit 0x05
        _emit 0xe8  // CALL VA 0x009d22b4   (iterator dereference of end)
        _emit 0x7a
        _emit 0xfb
        _emit 0x59
        _emit 0x00
        // Virtual call: (*[ESI+8])->vtable[1]()
        _emit 0x8b  // MOV ECX, dword ptr [ESI + 0x8]   (object ptr)
        _emit 0x4e
        _emit 0x08
        _emit 0x8b  // MOV EDX, dword ptr [ECX]          (vtable)
        _emit 0x11
        _emit 0x8b  // MOV EAX, dword ptr [EDX + 0x4]   (slot 1)
        _emit 0x42
        _emit 0x04
        _emit 0xff  // CALL EAX
        _emit 0xd0
        // Post-call debug check: assert(current != end)
        _emit 0x3b  // CMP ESI, dword ptr [EDI + 0x4]
        _emit 0x77
        _emit 0x04
        _emit 0x75  // JNZ +5   →  skip debug call
        _emit 0x05
        _emit 0xe8  // CALL VA 0x009d22b4   (iterator still at end after call)
        _emit 0x66
        _emit 0xfb
        _emit 0x59
        _emit 0x00
        // Advance and loop back
        _emit 0x8b  // MOV ESI, dword ptr [ESI]   (ESI = next node)
        _emit 0x36
        _emit 0xeb  // JMP -0x32   →  back to loop top (0x32720)
        _emit 0xce
        // Loop exit (RVA 0x32752) — only PDI popped here; POP ESI/EBX/RET at 0x32753
        _emit 0x5f  // POP EDI
    }
}
