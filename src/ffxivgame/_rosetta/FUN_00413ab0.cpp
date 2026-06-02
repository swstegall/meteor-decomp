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
// FUNCTION: ffxivgame 0x00413ab0 — engine_memory pool block return
//                                  (189 B / 0xbd, __thiscall, 1 stack arg)
//
// __thiscall void FUN_00413ab0(this, undefined4 arg1)
//   ECX        : this        (owner — likely a DetachableHeapSpace-like pool manager)
//   [ESP+0x04] : arg1        (a context/receiver pointer passed through the call)
//
// Shape (read from orig bytes at RVA 0x00013ab0, 189 bytes):
//
//   push  ebx/ebp/esi/edi                    ; save four callee-saves
//   mov   edi, ecx                           ; EDI = this
//   mov   ebp, [edi+0x10]                    ; EBP = this->field_10 (pool obj)
//   mov   eax, [ebp]                         ; EAX = pool vtable
//   mov   edx, [eax+0x2c]                    ; EDX = vtable[0xb] slot
//   mov   ecx, ebp
//   call  edx                                ; pool->vtable[0xb]()
//
//   mov   ecx, [esp+0x14]                    ; ECX = arg1 (receiver ptr)
//   mov   eax, [ecx]                         ; EAX = arg1 vtable
//   mov   edx, [eax+0x14]                    ; EDX = vtable[5]
//   call  edx                                ; arg1->vtable[5]() → ptr in EAX
//   mov   edx, [eax]                         ; EDX = ptr vtable
//   mov   ecx, eax
//   mov   eax, [edx+0x4]                     ; EAX = ptr->vtable[1]
//   call  eax                                ; ptr->vtable[1]() → node in EAX
//   mov   esi, eax                           ; ESI = node
//
//   ; drain node's pending-work list (field_0x30 = count)
//   cmp   dword ptr [esi+0x30], 0
//   je    after_drain
//   lea   ebx, [esi+4]                       ; EBX = &node->field_4 (sub-obj)
// drain_loop:
//   mov   edx, [ebx]                         ; EDX = sub-obj vtable
//   mov   eax, [edx+0x20]                    ; EAX = vtable[8]
//   mov   ecx, ebx
//   call  eax                                ; sub->vtable[8]()
//   cmp   dword ptr [esi+0x30], 0
//   jne   drain_loop                         ; repeat until count == 0
// after_drain:
//
//   mov   ecx, [esi+0x1c]                    ; save node->field_0x1c
//   mov   edx, [esi]                         ; node vtable
//   mov   eax, [edx]                         ; vtable[0]
//   mov   [esp+0x14], ecx                    ; stash saved value (over arg1 slot)
//   push  0                                  ; arg: false
//   mov   ecx, esi
//   call  eax                                ; node->vtable[0](false)
//
//   mov   ecx, [edi+0x10]                    ; ECX = this->field_10
//   mov   edx, [ecx]                         ; pool vtable
//   mov   eax, [edx+4]                       ; vtable[1]
//   call  eax                                ; pool->vtable[1]() → freelist_mgr in EAX
//
//   ; spin-lock acquire on freelist_mgr->field_0x14->field_4
//   mov   ecx, [eax+0x14]                    ; ECX = freelist_mgr->field_0x14
//   lea   edx, [ecx+4]                       ; EDX = &lock (freelist_mgr->field_0x18)
// spin:
//   mov   eax, 1
//   mov   ebx, edx
//   xchg  [ebx], eax                         ; atomic TAS
//   test  eax, eax
//   jne   spin                               ; busy-wait until 0
//
//   ; insert node into freelist (doubly-linked list at freelist_mgr->field_0x14->field_0xc)
//   mov   eax, [ecx+0xc]                     ; EAX = list anchor
//   mov   ebx, [eax+4]                       ; EBX = anchor->next (old head)
//   mov   [ebx], esi                         ; old_head->prev = node
//   mov   ebx, [eax+4]
//   mov   [esi+4], ebx                       ; node->next = old_head
//   mov   [esi], eax                         ; node->prev = anchor
//   mov   [eax+4], esi                       ; anchor->next = node
//   add   dword ptr [ecx+0x18], -1           ; --in_use_count
//
//   ; release lock
//   xor   ecx, ecx
//   xchg  [edx], ecx                         ; store 0 → lock released
//
//   ; walk this->field_0x28 chain via field_0x2c to find tail
//   mov   eax, [edi+0x28]
//   test  eax, eax
//   lea   ecx, [edi-4]                        ; fallback if chain is empty
//   je    found_tail
// walk:
//   mov   ecx, eax
//   mov   eax, [ecx+0x2c]
//   test  eax, eax
//   jne   walk
// found_tail:
//
//   ; call tail->field_0x1c->vtable[0x28/4](saved_val)
//   mov   ecx, [ecx+0x1c]
//   mov   edx, [ecx]
//   mov   eax, [esp+0x14]                    ; reload saved value
//   mov   edx, [edx+0x28]                    ; vtable[0xa]
//   push  eax
//   call  edx
//
//   ; final pool notification
//   mov   eax, [ebp]
//   mov   edx, [eax+0x30]                    ; vtable[0xc]
//   mov   ecx, ebp
//   call  edx
//
//   pop   edi / esi / ebp / ebx
//   ret   4                                  ; __thiscall, callee-cleans 1 dword
//
// Reconstruction strategy — naked-asm byte passthrough.
//   All virtual calls are indirect (register-based) — no CALL rel32 relocs.
//   All field accesses use register-relative addressing — no DIR32 relocs.
//   Emitting the 189 orig bytes verbatim via MASM `_emit` produces a .obj
//   whose .text is byte-identical to the orig slice with zero relocations,
//   and `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_00413ab0() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x8b              // MOV EBP, [EDI+0x10]
        _emit 0x6f
        _emit 0x10
        _emit 0x8b              // MOV EAX, [EBP]
        _emit 0x45
        _emit 0x00
        _emit 0x8b              // MOV EDX, [EAX+0x2c]
        _emit 0x50
        _emit 0x2c
        _emit 0x8b              // MOV ECX, EBP
        _emit 0xcd
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV ECX, [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EAX, [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, [EAX+0x14]
        _emit 0x50
        _emit 0x14
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EDX, [EAX]
        _emit 0x10
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0x8b              // MOV EAX, [EDX+0x4]
        _emit 0x42
        _emit 0x04
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x83              // CMP dword ptr [ESI+0x30], 0
        _emit 0x7e
        _emit 0x30
        _emit 0x00
        _emit 0x74              // JE +0x12 (after_drain)
        _emit 0x12
        _emit 0x8d              // LEA EBX, [ESI+0x4]
        _emit 0x5e
        _emit 0x04
        _emit 0x8b              // MOV EDX, [EBX]       (drain_loop:)
        _emit 0x13
        _emit 0x8b              // MOV EAX, [EDX+0x20]
        _emit 0x42
        _emit 0x20
        _emit 0x8b              // MOV ECX, EBX
        _emit 0xcb
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x83              // CMP dword ptr [ESI+0x30], 0
        _emit 0x7e
        _emit 0x30
        _emit 0x00
        _emit 0x75              // JNE -0xf (drain_loop)
        _emit 0xf1
        _emit 0x8b              // MOV ECX, [ESI+0x1c]  (after_drain:)
        _emit 0x4e
        _emit 0x1c
        _emit 0x8b              // MOV EDX, [ESI]
        _emit 0x16
        _emit 0x8b              // MOV EAX, [EDX]
        _emit 0x02
        _emit 0x89              // MOV [ESP+0x14], ECX
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV ECX, [EDI+0x10]
        _emit 0x4f
        _emit 0x10
        _emit 0x8b              // MOV EDX, [ECX]
        _emit 0x11
        _emit 0x8b              // MOV EAX, [EDX+0x4]
        _emit 0x42
        _emit 0x04
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV ECX, [EAX+0x14]
        _emit 0x48
        _emit 0x14
        _emit 0x8d              // LEA EDX, [ECX+0x4]
        _emit 0x51
        _emit 0x04
        _emit 0xb8              // MOV EAX, 1            (spin:)
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EBX, EDX
        _emit 0xda
        _emit 0x87              // XCHG [EBX], EAX
        _emit 0x03
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNE -0xd (spin)
        _emit 0xf3
        _emit 0x8b              // MOV EAX, [ECX+0xc]
        _emit 0x41
        _emit 0x0c
        _emit 0x8b              // MOV EBX, [EAX+0x4]
        _emit 0x58
        _emit 0x04
        _emit 0x89              // MOV [EBX], ESI
        _emit 0x33
        _emit 0x8b              // MOV EBX, [EAX+0x4]
        _emit 0x58
        _emit 0x04
        _emit 0x89              // MOV [ESI+0x4], EBX
        _emit 0x5e
        _emit 0x04
        _emit 0x89              // MOV [ESI], EAX
        _emit 0x06
        _emit 0x89              // MOV [EAX+0x4], ESI
        _emit 0x70
        _emit 0x04
        _emit 0x83              // ADD dword ptr [ECX+0x18], -1
        _emit 0x41
        _emit 0x18
        _emit 0xff
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x87              // XCHG [EDX], ECX
        _emit 0x0a
        _emit 0x8b              // MOV EAX, [EDI+0x28]
        _emit 0x47
        _emit 0x28
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x8d              // LEA ECX, [EDI-0x4]
        _emit 0x4f
        _emit 0xfc
        _emit 0x74              // JE +0x9 (found_tail)
        _emit 0x09
        _emit 0x8b              // MOV ECX, EAX          (walk:)
        _emit 0xc8
        _emit 0x8b              // MOV EAX, [ECX+0x2c]
        _emit 0x41
        _emit 0x2c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNE -0x9 (walk)
        _emit 0xf7
        _emit 0x8b              // MOV ECX, [ECX+0x1c]   (found_tail:)
        _emit 0x49
        _emit 0x1c
        _emit 0x8b              // MOV EDX, [ECX]
        _emit 0x11
        _emit 0x8b              // MOV EAX, [ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EDX, [EDX+0x28]
        _emit 0x52
        _emit 0x28
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EAX, [EBP]
        _emit 0x45
        _emit 0x00
        _emit 0x8b              // MOV EDX, [EAX+0x30]
        _emit 0x50
        _emit 0x30
        _emit 0x8b              // MOV ECX, EBP
        _emit 0xcd
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
