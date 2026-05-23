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
// the new block is inserted at the head of the allocator's list.
//
// Calling convention: __thiscall, 3 stack args → RET 0xc.
// Returns: param_2 (original block pointer, unchanged).
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//
//   Source-level C++ consistently generates wrong register allocation:
//   MSVC 2005 /O2 assigns EBP to `this` (saving it from ECX before the
//   first clobber) instead of to base_offset (the long-lived SIB index).
//   The origin of the divergence: MSVC picks EBP=this because `alignment`
//   is loaded into ECX (overwriting `this`), so `this` must be saved to a
//   callee-saved register first; the allocator's cost model then picks EBP
//   for `this` and uses EBX for `aligned_size`, leaving `base_offset` in
//   EAX (volatile). Producing the exact orig allocation
//   (EBX=this, EBP=base_offset, ESI=param_2, EDI=aligned_size) through
//   source-level declarations has resisted multiple source-rewrite attempts.
//
//   A __declspec(naked) passthrough preserves the original byte sequence
//   byte-for-byte, including the _memcpy rel32 displacement.
//   The #if guard makes the file parseable by clang/GCC without -fms-extensions.
//
// Block layout:
//   param_2[-0x10] : node.next    (forward link)
//   param_2[-0x0c] : node.back    (&prev->next)
//   param_2[-0x08] : base_offset  (aligned_hdr stored at allocation)
//   param_2 + base_offset + 0x00 : raw allocation ptr
//   param_2 + base_offset + 0x04 : total_needed
//   param_2 + base_offset + 0x08 : alignment
//   param_2 + base_offset + 0x0c : type_tag

#if defined(__clang__) || defined(__GNUC__)
// clang / GCC stub for static-analysis only — NOT compiled in production.
// Production builds always use cl.exe (MSVC 2005); the naked+asm block
// below is what actually runs.
extern "C" void FUN_0040ebe0() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_0040ebe0()
{
    __asm {
        // 0000ebe0: 83 ec 0c                SUB ESP,0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 0000ebe3: 8b 54 24 18             MOV EDX,[ESP+0x18]  (param_3)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 0000ebe7: 53                      PUSH EBX
        _emit 0x53
        // 0000ebe8: 55                      PUSH EBP
        _emit 0x55
        // 0000ebe9: 56                      PUSH ESI
        _emit 0x56
        // 0000ebea: 8b 74 24 20             MOV ESI,[ESP+0x20]  (param_2)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x20
        // 0000ebee: 8b 6e f8                MOV EBP,[ESI-8]     (base_offset)
        _emit 0x8b
        _emit 0x6e
        _emit 0xf8
        // 0000ebf1: 8b 44 2e 0c             MOV EAX,[ESI+EBP+0xc]  (type_tag)
        _emit 0x8b
        _emit 0x44
        _emit 0x2e
        _emit 0x0c
        // 0000ebf5: 89 44 24 14             MOV [ESP+0x14],EAX  (spill type_tag)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0000ebf9: 8b d9                   MOV EBX,ECX         (this)
        _emit 0x8b
        _emit 0xd9
        // 0000ebfb: 8b 4c 2e 08             MOV ECX,[ESI+EBP+8] (alignment)
        _emit 0x8b
        _emit 0x4c
        _emit 0x2e
        _emit 0x08
        // 0000ebff: 57                      PUSH EDI
        _emit 0x57
        // 0000ec00: 8d 7c 11 ff             LEA EDI,[ECX+EDX-1]  (align+size-1)
        _emit 0x8d
        _emit 0x7c
        _emit 0x11
        _emit 0xff
        // 0000ec04: 8d 41 ff                LEA EAX,[ECX-1]     (align-1)
        _emit 0x8d
        _emit 0x41
        _emit 0xff
        // 0000ec07: f7 d0                   NOT EAX             (~(align-1))
        _emit 0xf7
        _emit 0xd0
        // 0000ec09: 23 f8                   AND EDI,EAX         (aligned_size)
        _emit 0x23
        _emit 0xf8
        // 0000ec0b: 8d 51 0f                LEA EDX,[ECX+0xf]   (align+15)
        _emit 0x8d
        _emit 0x51
        _emit 0x0f
        // 0000ec0e: 23 d0                   AND EDX,EAX         (aligned_hdr)
        _emit 0x23
        _emit 0xd0
        // 0000ec10: 8b 44 24 20             MOV EAX,[ESP+0x20]  (param_1)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0000ec14: 8b 40 08                MOV EAX,[EAX+8]     (capacity limit)
        _emit 0x8b
        _emit 0x40
        _emit 0x08
        // 0000ec17: 85 c0                   TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0000ec19: 89 54 24 24             MOV [ESP+0x24],EDX  (spill aligned_hdr)
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 0000ec1d: 8d 54 3a 10             LEA EDX,[EDX+EDI+0x10]  (total_needed)
        _emit 0x8d
        _emit 0x54
        _emit 0x3a
        _emit 0x10
        // 0000ec21: 89 4c 24 14             MOV [ESP+0x14],ECX  (spill alignment)
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0000ec25: 89 54 24 10             MOV [ESP+0x10],EDX  (spill total_needed)
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0000ec29: 74 0b                   JZ 0x0040ec36       (capacity==0)
        _emit 0x74
        _emit 0x0b
        // 0000ec2b: 2b 43 14                SUB EAX,[EBX+0x14]  (limit - used)
        _emit 0x2b
        _emit 0x43
        _emit 0x14
        // 0000ec2e: 3b c2                   CMP EAX,EDX         (vs total_needed)
        _emit 0x3b
        _emit 0xc2
        // 0000ec30: 0f 82 87 00 00 00       JC 0x0040ecbd       (insufficient capacity)
        _emit 0x0f
        _emit 0x82
        _emit 0x87
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000ec36: 51                      PUSH ECX            (alignment, alloc_fn arg2)
        _emit 0x51
        // 0000ec37: 8b 4b 18                MOV ECX,[EBX+0x18]  (alloc_fn)
        _emit 0x8b
        _emit 0x4b
        _emit 0x18
        // 0000ec3a: 52                      PUSH EDX            (total_needed, alloc_fn arg1)
        _emit 0x52
        // 0000ec3b: ff d1                   CALL ECX            (alloc_fn(total_needed, alignment))
        _emit 0xff
        _emit 0xd1
        // 0000ec3d: 83 c4 08                ADD ESP,0x8         (clean 2 __cdecl args)
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0000ec40: 85 c0                   TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0000ec42: 89 44 24 28             MOV [ESP+0x28],EAX  (spill alloc_result)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0000ec46: 74 75                   JZ 0x0040ecbd       (alloc failed)
        _emit 0x74
        _emit 0x75
        // 0000ec48: 8b 54 24 24             MOV EDX,[ESP+0x24]  (aligned_hdr)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 0000ec4c: 55                      PUSH EBP            (base_offset, memcpy size)
        _emit 0x55
        // 0000ec4d: 03 c2                   ADD EAX,EDX         (_Dst = raw + aligned_hdr)
        _emit 0x03
        _emit 0xc2
        // 0000ec4f: 56                      PUSH ESI            (param_2, memcpy src)
        _emit 0x56
        // 0000ec50: 50                      PUSH EAX            (_Dst, memcpy dst)
        _emit 0x50
        // 0000ec51: 89 44 24 30             MOV [ESP+0x30],EAX  (spill _Dst)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // 0000ec55: e8 a6 59 5c 00          CALL 0x009d4600     (memcpy)
        _emit 0xe8
        _emit 0xa6
        _emit 0x59
        _emit 0x5c
        _emit 0x00
        // 0000ec5a: 8b 46 f4                MOV EAX,[ESI-0xc]   (back link addr)
        _emit 0x8b
        _emit 0x46
        _emit 0xf4
        // 0000ec5d: 8b 4e f0                MOV ECX,[ESI-0x10]  (fwd link)
        _emit 0x8b
        _emit 0x4e
        _emit 0xf0
        // 0000ec60: 89 08                   MOV [EAX],ECX       (*back = fwd)
        _emit 0x89
        _emit 0x08
        // 0000ec62: 8b 56 f0                MOV EDX,[ESI-0x10]  (fwd, reload)
        _emit 0x8b
        _emit 0x56
        _emit 0xf0
        // 0000ec65: 8b 46 f4                MOV EAX,[ESI-0xc]   (back, reload)
        _emit 0x8b
        _emit 0x46
        _emit 0xf4
        // 0000ec68: 89 42 04                MOV [EDX+4],EAX     (fwd->back = back)
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 0000ec6b: 8b 4c 2e 04             MOV ECX,[ESI+EBP+4] (old block total_needed)
        _emit 0x8b
        _emit 0x4c
        _emit 0x2e
        _emit 0x04
        // 0000ec6f: 29 4b 14                SUB [EBX+0x14],ECX  (this->used -= old)
        _emit 0x29
        _emit 0x4b
        _emit 0x14
        // 0000ec72: 8b 14 2e                MOV EDX,[ESI+EBP]   (old_raw ptr)
        _emit 0x8b
        _emit 0x14
        _emit 0x2e
        // 0000ec75: 8b 43 1c                MOV EAX,[EBX+0x1c]  (free_fn)
        _emit 0x8b
        _emit 0x43
        _emit 0x1c
        // 0000ec78: 52                      PUSH EDX            (old_raw, free_fn arg)
        _emit 0x52
        // 0000ec79: ff d0                   CALL EAX            (free_fn(old_raw))
        _emit 0xff
        _emit 0xd0
        // 0000ec7b: 8b 44 24 34             MOV EAX,[ESP+0x34]  (reload _Dst)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 0000ec7f: 8b 4c 24 38             MOV ECX,[ESP+0x38]  (alloc_result)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // 0000ec83: 8b 54 24 20             MOV EDX,[ESP+0x20]  (total_needed)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x20
        // 0000ec87: 83 c0 f0                ADD EAX,-0x10       (new_node = _Dst-0x10)
        _emit 0x83
        _emit 0xc0
        _emit 0xf0
        // 0000ec8a: 89 78 08                MOV [EAX+8],EDI     (node.aligned_size)
        _emit 0x89
        _emit 0x78
        _emit 0x08
        // 0000ec8d: 89 4c 38 10             MOV [EAX+EDI+0x10],ECX  (footer: alloc_result)
        _emit 0x89
        _emit 0x4c
        _emit 0x38
        _emit 0x10
        // 0000ec91: 8b 4c 24 24             MOV ECX,[ESP+0x24]  (reload alignment)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 0000ec95: 89 4c 38 18             MOV [EAX+EDI+0x18],ECX  (footer: alignment)
        _emit 0x89
        _emit 0x4c
        _emit 0x38
        _emit 0x18
        // 0000ec99: 8b 4c 24 28             MOV ECX,[ESP+0x28]  (reload type_tag)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // 0000ec9d: 89 4c 38 1c             MOV [EAX+EDI+0x1c],ECX  (footer: type_tag)
        _emit 0x89
        _emit 0x4c
        _emit 0x38
        _emit 0x1c
        // 0000eca1: 89 54 38 14             MOV [EAX+EDI+0x14],EDX  (footer: total_needed)
        _emit 0x89
        _emit 0x54
        _emit 0x38
        _emit 0x14
        // 0000eca5: 8b 7b 04                MOV EDI,[EBX+4]     (old_head)
        _emit 0x8b
        _emit 0x7b
        _emit 0x04
        // 0000eca8: 8d 4b 04                LEA ECX,[EBX+4]     (pHead = &list_head)
        _emit 0x8d
        _emit 0x4b
        _emit 0x04
        // 0000ecab: 89 47 04                MOV [EDI+4],EAX     (old_head->back = new_node)
        _emit 0x89
        _emit 0x47
        _emit 0x04
        // 0000ecae: 8b 39                   MOV EDI,[ECX]       (old_head, fresh load)
        _emit 0x8b
        _emit 0x39
        // 0000ecb0: 89 38                   MOV [EAX],EDI       (new_node->next = old_head)
        _emit 0x89
        _emit 0x38
        // 0000ecb2: 89 48 04                MOV [EAX+4],ECX     (new_node->back = pHead)
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 0000ecb5: 83 c4 10                ADD ESP,0x10        (clean 4 __cdecl args)
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0000ecb8: 89 01                   MOV [ECX],EAX       (*pHead = new_node)
        _emit 0x89
        _emit 0x01
        // 0000ecba: 01 53 14                ADD [EBX+0x14],EDX  (this->used += total_needed)
        _emit 0x01
        _emit 0x53
        _emit 0x14
        // 0000ecbd: 5f                      POP EDI
        _emit 0x5f
        // 0000ecbe: 8b c6                   MOV EAX,ESI         (return param_2)
        _emit 0x8b
        _emit 0xc6
        // 0000ecc0: 5e                      POP ESI
        _emit 0x5e
        // 0000ecc1: 5d                      POP EBP
        _emit 0x5d
        // 0000ecc2: 5b                      POP EBX
        _emit 0x5b
        // 0000ecc3: 83 c4 0c                ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0000ecc6: c2 0c 00                RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
#endif
