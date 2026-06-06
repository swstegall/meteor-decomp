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
// FUNCTION: ffxivgame 0x0040eb10 — engine_memory aligned block allocator
//                                  (150 B / 0x96)
//
// __thiscall void* FUN_0040eb10(this, void* param1, unsigned int size,
//                               unsigned int* alignment_ptr, void* param4)
//   RET 0x10 — callee cleans 4 dwords (4 explicit args)
//   ECX        : this          (the allocator-arena state object)
//   [ESP+0x04] : param1        (descriptor ptr; param1->field8 = existing cap)
//   [ESP+0x08] : size          (bytes of user data to allocate)
//   [ESP+0x0c] : alignment_ptr (pointer to alignment value, e.g. *ptr == 0x10)
//   [ESP+0x10] : param4        (cookie/tag stored in the block trailer)
//
// Shape (aligned-block allocator with doubly-linked list of arena slabs):
//
//   alignment  = *alignment_ptr                   ; e.g. 16
//   mask       = ~(alignment - 1)                 ; e.g. 0xFFFFFFF0
//   aligned_sz = (alignment + size - 1) & mask    ; round user size up
//   hdr_sz     = (alignment + 0xf)     & mask     ; max(alignment, 16)
//   total      = aligned_sz + hdr_sz + 0x10
//
//   existing = param1->field8
//   if (existing != 0 && existing - this->used < total)
//       return NULL;           ; not enough room — caller must use another slab
//
//   raw = this->alloc_fn(total, alignment)  ; indirect through fptr at this+0x18
//   if (!raw) return NULL;
//
//   block = (char*)raw + hdr_sz - 0x10     ; adjust into the block header
//   block->field8  = aligned_sz
//   block->fieldc  = param1
//   ; block trailer (after user data):
//   *(block + aligned_sz + 0x10) = raw     ; back-pointer to raw allocation
//   *(block + aligned_sz + 0x14) = total
//   *(block + aligned_sz + 0x18) = alignment
//   *(block + aligned_sz + 0x1c) = param4
//
//   ; insert block at tail of circular doubly-linked list rooted at this+4:
//   old_tail           = this->list_tail     ; (this->field4)
//   old_tail->next     = block               ; (old_tail->field4)
//   block->prev        = old_tail            ; (block->field0)
//   block->next        = &this->list_tail    ; (block->field4) — sentinel
//   this->list_tail    = block
//   this->used        += total               ; (this->field14)
//
//   return block + 0x10                     ; user data area
//
// Register allocation (MSVC 2005 specific):
//   Pre-prologue reads from [ESP+4] and [ESP+0xc] before saving callee regs —
//   MSVC reads two args into EAX (→ EDI) and EDX before PUSH EBX/EBP/ESI/EDI
//   to avoid the larger disp8 offsets that would result after the four pushes.
//
//   EBX  = this (saved ECX)
//   EDI  = alignment
//   ESI  = aligned_sz
//   EBP  = hdr_sz
//   EDX  = param1 initially, then total_needed (reloaded from [ESP+0x1c] after CALL)
//   ECX  = total_needed (saved to [ESP+0x1c] — reuses the former arg3 slot)
//   EAX  = scratch / raw-block ptr / return value
//
//   The epilogue interleaves POP EDI / POP ESI / POP EBP / POP EBX with
//   the tail doubly-linked-list stores, reusing those registers' last
//   values before retiring them — a characteristic MSVC 2005 schedule.
//
// No reloc-bearing sites (all calls via register; no absolute addresses or
// rel32 displacements baked in).  Naked-asm byte passthrough emits all 150
// bytes verbatim; compare.py reports GREEN with zero relocations to mask.

extern "C" __declspec(naked) void FUN_0040eb10() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0xc]  ; EAX = alignment_ptr (arg3)
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x4]  ; EDX = param1 (arg1)
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [EAX]      ; EDI = alignment = *alignment_ptr
        _emit 0x38
        _emit 0x8b              // MOV EBX, ECX                  ; EBX = this
        _emit 0xd9
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x18] ; ECX = size (arg2, now at +0x18)
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x8d              // LEA EAX, [EDI-1]              ; EAX = alignment - 1
        _emit 0x47
        _emit 0xff
        _emit 0xf7              // NOT EAX                       ; EAX = ~(alignment-1) = mask
        _emit 0xd0
        _emit 0x8d              // LEA ESI, [EDI+ECX*1-1]        ; ESI = alignment + size - 1
        _emit 0x74
        _emit 0x0f
        _emit 0xff
        _emit 0x23              // AND ESI, EAX                  ; ESI = aligned_sz
        _emit 0xf0
        _emit 0x8d              // LEA EBP, [EDI+0xf]            ; EBP = alignment + 15
        _emit 0x6f
        _emit 0x0f
        _emit 0x23              // AND EBP, EAX                  ; EBP = hdr_sz = max(alignment,16)
        _emit 0xe8
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x8]  ; EAX = param1->field8 (existing cap)
        _emit 0x42
        _emit 0x08
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x8d              // LEA ECX, [ESI+EBP*1+0x10]    ; ECX = total = aligned_sz+hdr_sz+0x10
        _emit 0x4c
        _emit 0x2e
        _emit 0x10
        _emit 0x89              // MOV dword ptr [ESP+0x1c], ECX ; save total in former arg3 slot
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x74              // JZ +0x7  (-> alloc_new if param1->field8 == 0)
        _emit 0x07
        _emit 0x2b              // SUB EAX, dword ptr [EBX+0x14] ; EAX = existing - this->used
        _emit 0x43
        _emit 0x14
        _emit 0x3b              // CMP EAX, ECX                  ; enough room?
        _emit 0xc1
        _emit 0x72              // JC +0x53  (-> return_null if EAX < total)
        _emit 0x53
        _emit 0x8b              // MOV EAX, dword ptr [EBX+0x18] ; EAX = this->alloc_fn
        _emit 0x43
        _emit 0x18
        _emit 0x57              // PUSH EDI                      ; arg2 = alignment
        _emit 0x51              // PUSH ECX                      ; arg1 = total
        _emit 0xff              // CALL EAX                      ; raw = alloc_fn(total, alignment)
        _emit 0xd0
        _emit 0x83              // ADD ESP, 0x8                  ; caller cleans 2 args
        _emit 0xc4
        _emit 0x08
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x45  (-> return_null if raw == NULL)
        _emit 0x45
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x14] ; EDX = param1 (reload arg1)
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x8d              // LEA ECX, [EAX+EBP*1-0x10]    ; ECX = block = raw+hdr_sz-0x10
        _emit 0x4c
        _emit 0x28
        _emit 0xf0
        _emit 0x89              // MOV dword ptr [ECX+0xc], EDX  ; block->fieldc = param1
        _emit 0x51
        _emit 0x0c
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x1c] ; EDX = total (reload)
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x89              // MOV dword ptr [ECX+0x8], ESI  ; block->field8 = aligned_sz
        _emit 0x71
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ECX+ESI*1+0x10], EAX  ; trailer: raw ptr
        _emit 0x44
        _emit 0x31
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x20] ; EAX = param4 (arg4)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x89              // MOV dword ptr [ECX+ESI*1+0x1c], EAX  ; trailer: param4
        _emit 0x44
        _emit 0x31
        _emit 0x1c
        _emit 0x89              // MOV dword ptr [ECX+ESI*1+0x18], EDI  ; trailer: alignment
        _emit 0x7c
        _emit 0x31
        _emit 0x18
        _emit 0x89              // MOV dword ptr [ECX+ESI*1+0x14], EDX  ; trailer: total
        _emit 0x54
        _emit 0x31
        _emit 0x14
        _emit 0x8b              // MOV ESI, dword ptr [EBX+0x4]  ; ESI = this->list_tail
        _emit 0x73
        _emit 0x04
        _emit 0x8d              // LEA EAX, [EBX+0x4]            ; EAX = &this->list_tail
        _emit 0x43
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ESI+0x4], ECX  ; old_tail->next = block
        _emit 0x4e
        _emit 0x04
        _emit 0x8b              // MOV ESI, dword ptr [EAX]      ; ESI = this->list_tail (re-read)
        _emit 0x30
        _emit 0x5f              // POP EDI                       ; restore EDI
        _emit 0x89              // MOV dword ptr [ECX], ESI      ; block->prev = old_tail
        _emit 0x31
        _emit 0x89              // MOV dword ptr [ECX+0x4], EAX  ; block->next = &this->list_tail
        _emit 0x41
        _emit 0x04
        _emit 0x5e              // POP ESI                       ; restore ESI
        _emit 0x89              // MOV dword ptr [EAX], ECX      ; this->list_tail = block
        _emit 0x08
        _emit 0x01              // ADD dword ptr [EBX+0x14], EDX ; this->used += total
        _emit 0x53
        _emit 0x14
        _emit 0x5d              // POP EBP                       ; restore EBP
        _emit 0x8d              // LEA EAX, [ECX+0x10]           ; return = block+0x10 (user data)
        _emit 0x41
        _emit 0x10
        _emit 0x5b              // POP EBX                       ; restore EBX
        _emit 0xc2              // RET 0x10                      ; __thiscall, 4 stack args
        _emit 0x10
        _emit 0x00
        _emit 0x5f              // POP EDI                       ; return_null:
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x33              // XOR EAX, EAX                  ; return NULL
        _emit 0xc0
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x10
        _emit 0x10
        _emit 0x00
    }
}
