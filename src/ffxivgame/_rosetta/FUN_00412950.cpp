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
// FUNCTION: ffxivgame 0x00412950 — __thiscall member: call a pending node's
//                                  first virtual function, acquire a spin-lock,
//                                  append the node to a doubly-linked list,
//                                  release the lock, clear the pending slot,
//                                  then tail-call through a virtual dispatch.
//                                  (101 bytes, no /GS cookie, no EBP frame)
//
// __thiscall void FUN_00412950()    ECX = this
//
// Layout of *this (offsets touched here):
//   +0x14  Obj *field_14  — object whose vtable[1] is called; return value
//                           is a container pointer
//   +0x18  Obj *field_18  — object whose vtable[8] is tail-called
//   +0x30  Node *field_30 — pending node; NULL → early return
//
// Layout of the container returned by field_14->vtable[1]():
//   +0x14  SubObj *sub    — sub-object holding the lock and list head
//
// Layout of *sub:
//   +0x04  LONG spin_lock — XCHG-based test-and-set (0 = free, 1 = held)
//   +0x0c  Node *head     — circular doubly-linked list sentinel
//   +0x18  int  count     — element count; decremented on insert
//
// Layout of list nodes (field_30 and the sentinel):
//   +0x00  Node *field_0  — one link direction
//   +0x04  Node *field_4  — other link direction
//
// Behaviour:
//   1. If field_30 == NULL, return immediately.
//   2. Call field_30->vtable[0](0)  (pass 0 as the only argument).
//   3. Call field_14->vtable[1]()   (returns container* in EAX).
//   4. Spin-acquire sub->spin_lock via XCHG (no explicit LOCK prefix —
//      XCHG r/m32,r32 carries an implicit bus lock on x86).
//   5. Append field_30 to the circular list before the sentinel (head):
//        old_prev = head->field_4;
//        old_prev->field_0 = field_30;
//        field_30->field_4 = old_prev;
//        field_30->field_0 = head;
//        head->field_4     = field_30;
//   6. Decrement sub->count.
//   7. Release sub->spin_lock (XCHG 0 back in via ECX=0, XCHG [EDI],ECX).
//   8. Clear this->field_30 to NULL.
//   9. Tail-call field_18->vtable[8]() via JMP EAX.
//
// No absolute addresses → no COFF relocations; the naked __asm block
// below is byte-identical to the original 101-byte function body.

extern "C" __declspec(naked) void FUN_00412950() {
    __asm {
        // ---- prologue: save ESI = this --------------------------------
        push    esi                                // 56
        mov     esi, ecx                           // 8b f1
        // ---- early-exit if field_30 is NULL ---------------------------
        mov     ecx, dword ptr [esi + 0x30]        // 8b 4e 30
        test    ecx, ecx                           // 85 c9
        jz      early_exit                         // 74 59
        // ---- call field_30->vtable[0](0) ------------------------------
        mov     eax, dword ptr [ecx]               // 8b 01   (load vtable ptr)
        mov     edx, dword ptr [eax]               // 8b 10   (vtable[0])
        push    ebx                                // 53
        push    edi                                // 57
        push    0                                  // 6a 00   (arg 0)
        call    edx                                // ff d2
        // ---- call field_14->vtable[1]() → container in EAX -----------
        mov     ecx, dword ptr [esi + 0x14]        // 8b 4e 14
        mov     eax, dword ptr [ecx]               // 8b 01   (load vtable ptr)
        mov     edx, dword ptr [eax + 0x4]         // 8b 50 04  (vtable[1])
        call    edx                                // ff d2
        // ---- set up sub-object and node pointers ----------------------
        mov     ecx, dword ptr [eax + 0x14]        // 8b 48 14  (sub = container->field_14)
        mov     edx, dword ptr [esi + 0x30]        // 8b 56 30  (reload field_30 → EDX)
        lea     edi, [ecx + 0x4]                   // 8d 79 04  (EDI = &sub->spin_lock)
        // ---- spin-acquire lock: XCHG-based test-and-set ---------------
    spin_loop:
        mov     eax, 1                             // b8 01 00 00 00
        mov     ebx, edi                           // 8b df
        xchg    dword ptr [ebx], eax               // 87 03
        test    eax, eax                           // 85 c0
        jnz     spin_loop                          // 75 f3
        // ---- insert field_30 before list sentinel (head) --------------
        mov     eax, dword ptr [ecx + 0xc]         // 8b 41 0c  (EAX = head)
        mov     ebx, dword ptr [eax + 0x4]         // 8b 58 04  (EBX = head->field_4 = old_prev)
        mov     dword ptr [ebx], edx               // 89 13     (old_prev->field_0 = field_30)
        mov     ebx, dword ptr [eax + 0x4]         // 8b 58 04  (reload old_prev)
        mov     dword ptr [edx + 0x4], ebx         // 89 5a 04  (field_30->field_4 = old_prev)
        mov     dword ptr [edx], eax               // 89 02     (field_30->field_0 = head)
        mov     dword ptr [eax + 0x4], edx         // 89 50 04  (head->field_4 = field_30)
        // ---- decrement list count -------------------------------------
        add     dword ptr [ecx + 0x18], -1         // 83 41 18 ff
        // ---- release spin-lock: XCHG 0 into lock slot ----------------
        xor     ecx, ecx                           // 33 c9
        xchg    dword ptr [edi], ecx               // 87 0f
        // ---- epilogue: clear field_30, tail-call via field_18 --------
        mov     ecx, dword ptr [esi + 0x18]        // 8b 4e 18
        pop     edi                                // 5f
        mov     dword ptr [esi + 0x30], 0          // c7 46 30 00 00 00 00
        mov     edx, dword ptr [ecx]               // 8b 11  (load vtable)
        mov     eax, dword ptr [edx + 0x20]        // 8b 42 20  (vtable[8])
        pop     ebx                                // 5b
        pop     esi                                // 5e
        jmp     eax                                // ff e0  (tail call)
    early_exit:
        pop     esi                                // 5e
        ret                                        // c3
    }
}
