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
// FUNCTION: ffxivgame 0x00014140 — engine_memory pool-pair Reserve+link
//                                  (199 B, __thiscall, ret 0xc)
//
// __thiscall int FUN_00414140(this, param_1, param_2, param_3)
//   ECX        : this (some engine_memory pool-pair owner — see layout below)
//   [ESP+0x04] : param_1   (passed straight through to the two inner reserves)
//   [ESP+0x08] : param_2
//   [ESP+0x0c] : param_3
//
// `this` layout used by this function:
//   +0x00 : vtable ptr — slot 11 (offset 0x2c) is "Lock", slot 12 (offset 0x30) is "Unlock"
//   +0x04 : Allocator *reserve_a  — its vtable[3] (offset 0xc) reserves storage
//                                   from sub-pool A; vtable[4] (offset 0x10) releases
//   +0x08 : Allocator *reserve_b  — symmetric sub-pool B
//   +0x14 : opaque ptr passed to FUN_004109a0 (block descriptor / arena handle)
//   +0x28 : LinkAnchor *active_list   — intrusive list head whose field at +8
//                                       is the linked tail of newly-allocated
//                                       composite blocks (see decomp-notes/types/
//                                       ffxivgame/0x00013850.md for the matching
//                                       DetachableHeapSpace inline Link shape)
//
// Behaviour:
//   1. this->vftable[11](this)            // Lock
//   2. va = reserve_a->vtable[3](reserve_a, p1, p2, p3)
//   3. vb = reserve_b->vtable[3](reserve_b, p1, p2, p3)
//   4. branch on (va, vb):
//      - va == 0 && vb == 0 : nothing reserved → fall to Unlock and return 0
//      - va != 0 && vb == 0 : release va via reserve_a->vtable[4]
//      - va == 0 && vb != 0 : release vb via reserve_b->vtable[4]
//      - va != 0 && vb != 0 :
//          desc = FUN_004109a0(this->field_14)       // returns block-pool desc / 0
//          alloc = desc ? FUN_004139d0(desc, va, vb, 0, 0) : 0
//          // splice "alloc+8" (the composite-block payload offset) into
//          // this->field_28 → field_8 doubly-linked list:
//          tail = this->field_28->field_8
//          tail->field_4    = alloc_payload
//          alloc_payload->field_8 = this->field_28->field_8
//          alloc_payload->field_4 = this->field_28
//          this->field_28->field_8 = alloc_payload
//   5. this->vftable[12](this)            // Unlock
//   6. return alloc ? alloc + 4 : 0       // composite-block header offset
//
// Reconstruction strategy: __declspec(naked) `_emit` byte passthrough.
//   MSVC 2005 /O2 interleaves register loads and pushes across the three
//   pre-call sequences in a way that's painful to coerce out of a C++
//   source-level rewrite (the [esp+0x10] local is materialised mid-body,
//   ESP-relative stack-arg loads are interleaved with vtable indirection,
//   and the four callee-save pushes + one push-to-allocate trick around
//   the local var anchor the entire stack-frame shape). Naked asm pins
//   the exact 199-byte encoding, including the two REL32 displacements
//   (call FUN_004109a0 @ off 0x4f, call FUN_004139d0 @ off 0x61). Both
//   displacements are emitted as the absolute orig bytes; the .obj's
//   .text matches orig byte-for-byte without needing a linker fixup.

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
extern "C" int FUN_00414140() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_00414140() {
    __asm {
        // Prologue: push ecx (stack-alloc 4 for local), then 4 callee-save pushes
        _emit 0x51              // PUSH ECX                       ; stack-alloc local @ [esp+0x10]
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV  ESI, ECX                  ; esi = this
        _emit 0xf1
        _emit 0x8b              // MOV  EAX, [ESI]                ; vtable
        _emit 0x06
        _emit 0x8b              // MOV  EDX, [EAX+0x2c]           ; vtable[11] (Lock)
        _emit 0x50
        _emit 0x2c
        _emit 0x57              // PUSH EDI
        _emit 0xff              // CALL EDX                       ; this->Lock()
        _emit 0xd2

        // Reserve A: load 3 stack args, call reserve_a->vtable[3]
        _emit 0x8b              // MOV  EBX, [ESP+0x20]           ; param_3
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        _emit 0x8b              // MOV  ECX, [ESI+0x04]           ; reserve_a
        _emit 0x4e
        _emit 0x04
        _emit 0x8b              // MOV  EBP, [ESP+0x1c]           ; param_2
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b              // MOV  EAX, [ECX]                ; reserve_a->vtable
        _emit 0x01
        _emit 0x8b              // MOV  EDX, [ESP+0x18]           ; param_1
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV  EAX, [EAX+0x0c]           ; vtable[3] (Reserve)
        _emit 0x40
        _emit 0x0c
        _emit 0x53              // PUSH EBX                       ; arg3
        _emit 0x55              // PUSH EBP                       ; arg2
        _emit 0x52              // PUSH EDX                       ; arg1
        _emit 0xc7              // MOV  [ESP+0x1c], 0             ; local = 0 (the eventual result)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff              // CALL EAX                       ; va = reserve_a->Reserve(p1,p2,p3)
        _emit 0xd0

        // Reserve B: re-push param_3/param_2 (held in callee-saves EBX/EBP) and reload param_1
        _emit 0x8b              // MOV  ECX, [ESI+0x08]           ; reserve_b
        _emit 0x4e
        _emit 0x08
        _emit 0x8b              // MOV  EDX, [ECX]                ; reserve_b->vtable
        _emit 0x11
        _emit 0x8b              // MOV  EDX, [EDX+0x0c]           ; vtable[3] (Reserve)
        _emit 0x52
        _emit 0x0c
        _emit 0x53              // PUSH EBX                       ; arg3
        _emit 0x8b              // MOV  EDI, EAX                  ; edi = va
        _emit 0xf8
        _emit 0x8b              // MOV  EAX, [ESP+0x1c]           ; reload param_1
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x55              // PUSH EBP                       ; arg2
        _emit 0x50              // PUSH EAX                       ; arg1
        _emit 0xff              // CALL EDX                       ; vb = reserve_b->Reserve(p1,p2,p3)
        _emit 0xd2

        // Branch on (va, vb)
        _emit 0x85              // TEST EDI, EDI                  ; va == 0?
        _emit 0xff
        _emit 0x8b              // MOV  EBX, EAX                  ; ebx = vb
        _emit 0xd8
        _emit 0x74              // JZ   +0x4e  (-> va_zero path @ off 0x96)
        _emit 0x4e
        _emit 0x85              // TEST EBX, EBX                  ; vb == 0?
        _emit 0xdb
        _emit 0x74              // JZ   +0x44  (-> release_va @ off 0x90)
        _emit 0x44

        // Both va, vb non-null — try composite allocation
        _emit 0x8b              // MOV  ECX, [ESI+0x14]           ; field_14
        _emit 0x4e
        _emit 0x14
        _emit 0xe8              // CALL FUN_004109a0
        _emit 0x0c
        _emit 0xc8
        _emit 0xff
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX                  ; desc == 0?
        _emit 0xc0
        _emit 0x74              // JZ   +0x10  (-> alloc_zero @ off 0x68)
        _emit 0x10
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x53              // PUSH EBX                       ; vb
        _emit 0x57              // PUSH EDI                       ; va
        _emit 0x56              // PUSH ESI                       ; this
        _emit 0x8b              // MOV  ECX, EAX                  ; ecx = desc
        _emit 0xc8
        _emit 0xe8              // CALL FUN_004139d0
        _emit 0x2a
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        _emit 0xeb              // JMP  +2   (-> post_alloc @ off 0x6a)
        _emit 0x02
        // alloc_zero:                                            (off 0x68)
        _emit 0x33              // XOR  EAX, EAX
        _emit 0xc0
        // post_alloc:                                             (off 0x6a)
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x89              // MOV  [ESP+0x10], EAX           ; local = alloc
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x74              // JZ   +5   (-> link_zero @ off 0x77)
        _emit 0x05
        _emit 0x83              // ADD  EAX, 8                    ; payload offset
        _emit 0xc0
        _emit 0x08
        _emit 0xeb              // JMP  +2   (-> do_link @ off 0x79)
        _emit 0x02
        // link_zero:                                              (off 0x77)
        _emit 0x33              // XOR  EAX, EAX                  ; payload = 0
        _emit 0xc0
        // do_link: splice payload into this->field_28 list        (off 0x79)
        _emit 0x8b              // MOV  ECX, [ESI+0x28]           ; active_list
        _emit 0x4e
        _emit 0x28
        _emit 0x8b              // MOV  EDX, [ECX+0x08]           ; old tail
        _emit 0x51
        _emit 0x08
        _emit 0x89              // MOV  [EDX+0x04], EAX           ; old_tail->next = payload
        _emit 0x42
        _emit 0x04
        _emit 0x8b              // MOV  EDX, [ECX+0x08]           ; reload (alias-confounding)
        _emit 0x51
        _emit 0x08
        _emit 0x89              // MOV  [EAX+0x08], EDX           ; payload->prev = old_tail
        _emit 0x50
        _emit 0x08
        _emit 0x89              // MOV  [EAX+0x04], ECX           ; payload->next = active_list
        _emit 0x48
        _emit 0x04
        _emit 0x89              // MOV  [ECX+0x08], EAX           ; active_list->tail = payload
        _emit 0x41
        _emit 0x08
        _emit 0xeb              // JMP  +0x15  (-> unlock @ off 0xa5)
        _emit 0x15

        // release_va: only va is live; release through reserve_a->vtable[4]   (off 0x90)
        _emit 0x8b              // MOV  ECX, [ESI+0x04]           ; reserve_a
        _emit 0x4e
        _emit 0x04
        _emit 0x57              // PUSH EDI                       ; va
        _emit 0xeb              // JMP  +8   (-> shared_release @ off 0x9e)
        _emit 0x08

        // va_zero:                                                (off 0x96)
        _emit 0x85              // TEST EBX, EBX                  ; vb == 0?
        _emit 0xdb
        _emit 0x74              // JZ   +0x0b  (-> unlock @ off 0xa5)
        _emit 0x0b
        _emit 0x8b              // MOV  ECX, [ESI+0x08]           ; reserve_b
        _emit 0x4e
        _emit 0x08
        _emit 0x53              // PUSH EBX                       ; vb

        // shared_release: ECX = allocator, arg already pushed     (off 0x9e)
        _emit 0x8b              // MOV  EAX, [ECX]                ; allocator->vtable
        _emit 0x01
        _emit 0x8b              // MOV  EDX, [EAX+0x10]           ; vtable[4] (Release)
        _emit 0x50
        _emit 0x10
        _emit 0xff              // CALL EDX
        _emit 0xd2

        // unlock:                                                 (off 0xa5)
        _emit 0x8b              // MOV  EAX, [ESI]                ; this->vtable
        _emit 0x06
        _emit 0x8b              // MOV  EDX, [EAX+0x30]           ; vtable[12] (Unlock)
        _emit 0x50
        _emit 0x30
        _emit 0x8b              // MOV  ECX, ESI                  ; this
        _emit 0xce
        _emit 0xff              // CALL EDX                       ; this->Unlock()
        _emit 0xd2

        // Epilogue: load saved alloc, restore callee-saves, return alloc?+4:0
        _emit 0x8b              // MOV  EAX, [ESP+0x10]           ; saved alloc
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x5f              // POP  EDI
        _emit 0x5e              // POP  ESI
        _emit 0x5d              // POP  EBP
        _emit 0x5b              // POP  EBX
        _emit 0x74              // JZ   +7   (-> ret_zero @ off 0xc1)
        _emit 0x07
        _emit 0x83              // ADD  EAX, 4                    ; composite-block header offset
        _emit 0xc0
        _emit 0x04
        _emit 0x59              // POP  ECX                       ; drop stack-alloc
        _emit 0xc2              // RET  0x0c
        _emit 0x0c
        _emit 0x00
        // ret_zero:                                              (off 0xc1)
        _emit 0x33              // XOR  EAX, EAX
        _emit 0xc0
        _emit 0x59              // POP  ECX
        _emit 0xc2              // RET  0x0c
        _emit 0x0c
        _emit 0x00
    }
}
#endif
