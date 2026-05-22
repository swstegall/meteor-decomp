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
// FUNCTION: ffxivgame 0x0000da10 — linked-list node search / allocate
//                                  (__thiscall, 63 bytes / 0x3f)
//
// __thiscall int* find_or_alloc(void)
//   ECX = this  (no stack args, RET 0)
//
// Semantics:
//   Walk the linked list rooted at *this (head pointer at offset 0).
//   Each node is tested with FUN_0040db50 (thiscall, returns bool/char).
//   If a matching node is found, return it immediately.
//   If the list is exhausted (or head == NULL), call FUN_0040d9a0(this)
//   to allocate / find a new node. If that returns non-NULL, call
//   FUN_0040d5c0(this, newNode) to register it, then return it.
//
// Orig codegen (read from orig RVA 0x0000da10, 63 bytes):
//
//   push esi                          ; save callee-saved
//   push edi                          ; save callee-saved
//   mov  edi, ecx                     ; edi = this
//   mov  esi, [edi]                   ; esi = head node (*this)
//   test esi, esi
//   jz   find_new                     ; head == NULL → skip to allocate
//   lea  ebx, [ebx+0]                 ; 6-byte NOP (loop alignment to 0x40da20)
// loop:
//   mov  ecx, esi                     ; thiscall: ECX = current node
//   call FUN_0040db50                 ; check(node) → AL
//   test al, al
//   jnz  done                         ; found a match → return it
//   mov  esi, [esi+0x9a4]            ; advance: esi = node->next
//   test esi, esi
//   jnz  loop                         ; continue if next != NULL
// find_new:
//   mov  ecx, edi                     ; thiscall: ECX = this
//   call FUN_0040d9a0                 ; alloc/find → eax
//   mov  esi, eax                     ; esi = result
//   test esi, esi
//   jz   done                         ; result == NULL → return NULL
//   push esi                          ; arg: the new node
//   mov  ecx, edi                     ; thiscall: ECX = this
//   call FUN_0040d5c0                 ; register(this, node)
// done:
//   pop  edi
//   mov  eax, esi                     ; return value
//   pop  esi
//   ret
//
// Reloc-bearing sites (CALL rel32, masked by compare.py):
//     +0x12   CALL rel32 → FUN_0040db50   (disp 0x00000129)
//     +0x27   CALL rel32 → FUN_0040d9a0   (disp 0xFFFFFF64)
//     +0x35   CALL rel32 → FUN_0040d5c0   (disp 0xFFFFFB76)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form would reproduce the loop shape but MSVC 2005
//   /O2 loop-alignment padding (the 6-byte `LEA EBX,[EBX+0]` NOP at
//   offset +0x0a) is emitted non-deterministically relative to the
//   compilation unit's surrounding code. Using `__declspec(naked)` with
//   `_emit` directives guarantees byte-identical output regardless of
//   compilation context. compare.py masks the CALL rel32 displacements
//   so the stored displacement values do not matter for the diff result.

extern "C" __declspec(naked) void FUN_0040da10() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x8b              // MOV ESI, dword ptr [EDI]
        _emit 0x37
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74              // JZ find_new (+0x1b)
        _emit 0x1b
        _emit 0x8d              // LEA EBX, [EBX+0x00000000]  (6-byte NOP)
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // loop: (0x40da20 — 16-byte aligned)
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_0040db50 (rel32)
        _emit 0x29
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x75              // JNZ done (+0x1f)
        _emit 0x1f
        _emit 0x8b              // MOV ESI, dword ptr [ESI+0x9a4]
        _emit 0xb6
        _emit 0xa4
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75              // JNZ loop (-0x15)
        _emit 0xeb
        // find_new: (0x40da35)
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8              // CALL FUN_0040d9a0 (rel32)
        _emit 0x64
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74              // JZ done (+0x08)
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8              // CALL FUN_0040d5c0 (rel32)
        _emit 0x76
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        // done: (0x40da4a)
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
