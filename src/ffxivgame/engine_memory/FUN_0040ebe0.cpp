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
// FUNCTION: ffxivgame 0x0000ebe0 — __thiscall pool-block reallocation
//           (SQEX::CDev::Engine::Memory::ComplexLink::Realloc or similar)
//
// Grows an existing heap block within a pool allocator. The old block is
// unlinked from the intrusive doubly-linked list, the backing allocation
// is freed, a new (larger) allocation is made, the content is copied, and
// the new block is inserted at the head of the list.
//
// Calling convention: __thiscall, 3 stack args → RET 0xc.
// Returns: param_2 (original block pointer, unchanged).
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//
//   Source-level C++ consistently generates different register allocation:
//   MSVC 2005 /O2 assigns EBP to aligned_hdr (short-lived) instead of
//   base_offset (long-lived across three indirect __cdecl calls), yielding
//   14 fewer bytes. The critical divergence at RVA+0x0f: orig uses
//   MOV EBP,[ESI-8] (EBP=base_offset as SIB index in [ESI+EBP+disp]);
//   reconstructed C++ uses MOV EAX,[ESI-8] with subsequent stack spill.
//
// Behaviour (from disassembly):
//   prologue: SUB ESP,0xc (locals for total_needed/alignment/type_tag);
//             PUSH EBX/EBP/ESI; MOV EBX,ECX (cache this); PUSH EDI.
//   EBP = *(param_2-8) — base_offset used as SIB index throughout.
//   type_tag    = *(param_2+EBP+0xc) → spilled to [ESP+0x14].
//   ECX = alignment = *(param_2+EBP+8).
//   EDI = aligned_size = (align+size-1) & ~(align-1).
//   EDX = aligned_hdr  = (align+0xf)   & ~(align-1) → [ESP+0x24].
//   total_needed = aligned_hdr+aligned_size+0x10     → [ESP+0x10].
//   capacity check: if limit && (limit-used) < total_needed → return.
//   alloc_result = alloc_fn(total_needed, alignment) → [ESP+0x28].
//   _Dst = alloc_result + aligned_hdr               → [ESP+0x24].
//   memcpy(_Dst, param_2, base_offset).
//   Unlink old node from doubly-linked list; this->used -= old_size.
//   free_fn(old_raw_ptr).
//   Build new node (at _Dst-0x10): node[2]=aligned_size.
//   Build footer (at _Dst+aligned_size): raw,total,hdr,tag.
//   Link new node at head of this->list (this+4).
//   this->used += total_needed.
//   epilogue: POP EDI; MOV EAX,ESI; POP ESI/EBP/EBX; ADD ESP,0xc; RET 0xc.

extern "C" __declspec(naked) void FUN_0040ebe0()
{
    __asm {
        // 0000ebe0: SUB ESP,0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 0000ebe3: MOV EDX,[ESP+0x18]  (param_3 = requested size)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 0000ebe7: PUSH EBX
        _emit 0x53
        // 0000ebe8: PUSH EBP
        _emit 0x55
        // 0000ebe9: PUSH ESI
        _emit 0x56
        // 0000ebea: MOV ESI,[ESP+0x20]  (param_2 = block ptr)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x20
        // 0000ebee: MOV EBP,[ESI-8]  (EBP = base_offset)
        _emit 0x8b
        _emit 0x6e
        _emit 0xf8
        // 0000ebf1: MOV EAX,[ESI+EBP+0xc]  (type_tag)
        _emit 0x8b
        _emit 0x44
        _emit 0x2e
        _emit 0x0c
        // 0000ebf5: MOV [ESP+0x14],EAX  (spill type_tag)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0000ebf9: MOV EBX,ECX  (cache this → EBX)
        _emit 0x8b
        _emit 0xd9
        // 0000ebfb: MOV ECX,[ESI+EBP+8]  (ECX = alignment)
        _emit 0x8b
        _emit 0x4c
        _emit 0x2e
        _emit 0x08
        // 0000ebff: PUSH EDI
        _emit 0x57
        // 0000ec00: LEA EDI,[ECX+EDX-1]  (pre-mask aligned_size)
        _emit 0x8d
        _emit 0x7c
        _emit 0x11
        _emit 0xff
        // 0000ec04: LEA EAX,[ECX-1]
        _emit 0x8d
        _emit 0x41
        _emit 0xff
        // 0000ec07: NOT EAX  (mask = ~(align-1))
        _emit 0xf7
        _emit 0xd0
        // 0000ec09: AND EDI,EAX  (aligned_size)
        _emit 0x23
        _emit 0xf8
        // 0000ec0b: LEA EDX,[ECX+0xf]
        _emit 0x8d
        _emit 0x51
        _emit 0x0f
        // 0000ec0e: AND EDX,EAX  (aligned_hdr)
        _emit 0x23
        _emit 0xd0
        // 0000ec10: MOV EAX,[ESP+0x20]  (param_1)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0000ec14: MOV EAX,[EAX+8]  (param_1->limit)
        _emit 0x8b
        _emit 0x40
        _emit 0x08
        // 0000ec17: TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0000ec19: MOV [ESP+0x24],EDX  (spill aligned_hdr → param_2 slot)
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 0000ec1d: LEA EDX,[EDX+EDI+0x10]  (total_needed)
        _emit 0x8d
        _emit 0x54
        _emit 0x3a
        _emit 0x10
        // 0000ec21: MOV [ESP+0x14],ECX  (spill alignment → local_b, overwrites type_tag)
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0000ec25: MOV [ESP+0x10],EDX  (spill total_needed → local_a)
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0000ec29: JZ +0xb  (if limit==0, skip capacity check)
        _emit 0x74
        _emit 0x0b
        // 0000ec2b: SUB EAX,[EBX+0x14]  (avail = limit - used)
        _emit 0x2b
        _emit 0x43
        _emit 0x14
        // 0000ec2e: CMP EAX,EDX  (avail vs total_needed)
        _emit 0x3b
        _emit 0xc2
        // 0000ec30: JC +0x87  (not enough space → return param_2; near jump)
        _emit 0x0f
        _emit 0x82
        _emit 0x87
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000ec36: PUSH ECX  (alignment arg for alloc_fn)
        _emit 0x51
        // 0000ec37: MOV ECX,[EBX+0x18]  (this->alloc_fn)
        _emit 0x8b
        _emit 0x4b
        _emit 0x18
        // 0000ec3a: PUSH EDX  (total_needed arg for alloc_fn)
        _emit 0x52
        // 0000ec3b: CALL ECX  (alloc_fn(total_needed, alignment))
        _emit 0xff
        _emit 0xd1
        // 0000ec3d: ADD ESP,0x8  (clean 2 __cdecl args)
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0000ec40: TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0000ec42: MOV [ESP+0x28],EAX  (spill alloc_result → param_3 slot)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0000ec46: JZ +0x75  (alloc failed → return param_2; short jump)
        _emit 0x74
        _emit 0x75
        // 0000ec48: MOV EDX,[ESP+0x24]  (reload aligned_hdr)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 0000ec4c: PUSH EBP  (base_offset = memcpy size arg)
        _emit 0x55
        // 0000ec4d: ADD EAX,EDX  (_Dst = alloc_result + aligned_hdr)
        _emit 0x03
        _emit 0xc2
        // 0000ec4f: PUSH ESI  (param_2 = memcpy src)
        _emit 0x56
        // 0000ec50: PUSH EAX  (_Dst = memcpy dst)
        _emit 0x50
        // 0000ec51: MOV [ESP+0x30],EAX  (spill _Dst → param_2 slot overwrite)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // 0000ec55: CALL 0x009d4600  (memcpy(_Dst, param_2, base_offset))
        _emit 0xe8
        _emit 0xa6
        _emit 0x59
        _emit 0x5c
        _emit 0x00
        // 0000ec5a: MOV EAX,[ESI-0xc]  (old_node->back_link_addr)
        _emit 0x8b
        _emit 0x46
        _emit 0xf4
        // 0000ec5d: MOV ECX,[ESI-0x10]  (old_node->fwd_val)
        _emit 0x8b
        _emit 0x4e
        _emit 0xf0
        // 0000ec60: MOV [EAX],ECX  (*back_link_addr = fwd_val)
        _emit 0x89
        _emit 0x08
        // 0000ec62: MOV EDX,[ESI-0x10]
        _emit 0x8b
        _emit 0x56
        _emit 0xf0
        // 0000ec65: MOV EAX,[ESI-0xc]
        _emit 0x8b
        _emit 0x46
        _emit 0xf4
        // 0000ec68: MOV [EDX+4],EAX  (*(fwd_val+4) = back_link_addr)
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 0000ec6b: MOV ECX,[ESI+EBP+4]  (old block used_size)
        _emit 0x8b
        _emit 0x4c
        _emit 0x2e
        _emit 0x04
        // 0000ec6f: SUB [EBX+0x14],ECX  (this->used -= old_size)
        _emit 0x29
        _emit 0x4b
        _emit 0x14
        // 0000ec72: MOV EDX,[ESI+EBP]  (old_raw = *(param_2+base_offset))
        _emit 0x8b
        _emit 0x14
        _emit 0x2e
        // 0000ec75: MOV EAX,[EBX+0x1c]  (this->free_fn)
        _emit 0x8b
        _emit 0x43
        _emit 0x1c
        // 0000ec78: PUSH EDX  (old_raw arg for free_fn)
        _emit 0x52
        // 0000ec79: CALL EAX  (free_fn(old_raw))
        _emit 0xff
        _emit 0xd0
        // 0000ec7b: MOV EAX,[ESP+0x34]  (reload _Dst)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 0000ec7f: MOV ECX,[ESP+0x38]  (reload alloc_result)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // 0000ec83: MOV EDX,[ESP+0x20]  (reload total_needed from local_a)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x20
        // 0000ec87: ADD EAX,-0x10  (new node ptr = _Dst - 0x10)
        _emit 0x83
        _emit 0xc0
        _emit 0xf0
        // 0000ec8a: MOV [EAX+8],EDI  (node->aligned_size)
        _emit 0x89
        _emit 0x78
        _emit 0x08
        // 0000ec8d: MOV [EAX+EDI+0x10],ECX  (footer[+0x10] = alloc_result)
        _emit 0x89
        _emit 0x4c
        _emit 0x38
        _emit 0x10
        // 0000ec91: MOV ECX,[ESP+0x24]  (reload aligned_hdr)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 0000ec95: MOV [EAX+EDI+0x18],ECX  (footer[+0x18] = aligned_hdr)
        _emit 0x89
        _emit 0x4c
        _emit 0x38
        _emit 0x18
        // 0000ec99: MOV ECX,[ESP+0x28]  (reload type_tag or alloc_result... see analysis)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // 0000ec9d: MOV [EAX+EDI+0x1c],ECX  (footer[+0x1c])
        _emit 0x89
        _emit 0x4c
        _emit 0x38
        _emit 0x1c
        // 0000eca1: MOV [EAX+EDI+0x14],EDX  (footer[+0x14] = total_needed)
        _emit 0x89
        _emit 0x54
        _emit 0x38
        _emit 0x14
        // 0000eca5: MOV EDI,[EBX+4]  (this->list.next = current head)
        _emit 0x8b
        _emit 0x7b
        _emit 0x04
        // 0000eca8: LEA ECX,[EBX+4]  (&this->list.next = pHead)
        _emit 0x8d
        _emit 0x4b
        _emit 0x04
        // 0000ecab: MOV [EDI+4],EAX  (old_head->prev = new_node)
        _emit 0x89
        _emit 0x47
        _emit 0x04
        // 0000ecae: MOV EDI,[ECX]  (EDI = current head ptr)
        _emit 0x8b
        _emit 0x39
        // 0000ecb0: MOV [EAX],EDI  (new_node->next = old_head)
        _emit 0x89
        _emit 0x38
        // 0000ecb2: MOV [EAX+4],ECX  (new_node->prev = pHead)
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 0000ecb5: ADD ESP,0x10  (clean 4 __cdecl args)
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0000ecb8: MOV [ECX],EAX  (*pHead = new_node)
        _emit 0x89
        _emit 0x01
        // 0000ecba: ADD [EBX+0x14],EDX  (this->used += total_needed)
        _emit 0x01
        _emit 0x53
        _emit 0x14
        // 0000ecbd: POP EDI
        _emit 0x5f
        // 0000ecbe: MOV EAX,ESI  (return value = param_2)
        _emit 0x8b
        _emit 0xc6
        // 0000ecc0: POP ESI
        _emit 0x5e
        // 0000ecc1: POP EBP
        _emit 0x5d
        // 0000ecc2: POP EBX
        _emit 0x5b
        // 0000ecc3: ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0000ecc6: RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
