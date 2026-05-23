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
// FUNCTION: ffxivgame 0x000115c0 — add free range with coalescing
//           __thiscall (ECX = this), 2 stack args, RET 0x8 (callee-cleans)
//           264 bytes / 0x108
//
// Inserts a free range [param_1, param_1+param_2) into the allocator's
// free list, coalescing left-adjacent or right-adjacent existing free
// chunks as needed. This is the Serializer's "add a free chunk" path for
// SQEX::CDev::Engine::Memory::Alternative::RemovableHeapSpace.
//
// Stack layout (after prologue SUB ESP,8 / PUSH EBX/EBP/ESI/EDI):
//   [ESP+0x1c] = param_1  (range start address)
//   [ESP+0x20] = param_2  (range size)
//
// The sentinel node is embedded in the Serializer at offsets 0x24..0x33:
//   +0x24 = sentinel.vtable  (first-node == sentinel → empty list)
//   +0x28 = sentinel.prev    (last node; insertion point for new chunks)
//   +0x2c = sentinel.next    (first node; list traversal start)
//
// Loop 1: walk list looking for a chunk whose END (chunk[0xc]+chunk[0x10])
//         equals param_1 (left-adjacent). Found → remove chunk, update
//         param_1 ← chunk[0xc], param_2 += chunk[0x10], JMP allocation.
// Loop 2: walk list looking for a chunk whose START (chunk[0xc]) equals
//         param_1+param_2 (right-adjacent). Found → remove chunk,
//         param_2 += chunk[0x10], fall through to allocation.
//         Not found → JMP allocation.
// Allocation: call FUN_0040e2d0 (construct Chunk on stack-local),
//             call FUN_0040e110 (allocate 0x14 bytes from allocator),
//             if allocation succeeds: fill vtable/fields, insert at tail.
//
// Called functions:
//   FUN_0040df70 — __thiscall (ECX=allocator, arg=Chunk*): remove chunk
//   FUN_0040e2d0 — __thiscall (ECX=&stack_local, arg=0x10, arg=cat_str): ctor
//   FUN_0040e110 — __thiscall (ECX=allocator, arg=EAX, arg=0x14): alloc
//
// Relocations (masked by tools/compare.py):
//   REL32: CALL FUN_0040df70 (@0x86), CALL FUN_0040df70 (@0xa3)
//   REL32: CALL FUN_0040e2d0 (@0xb6), CALL FUN_0040e110 (@0xc0)
//   DIR32: PUSH 0xf56ca8 (@0xab, allocator category string)
//   DIR32: MOV [EAX], 0xf56cdc (@0xd3, Chunk vtable for
//          SQEX::CDev::Engine::Memory::Alternative::RemovableHeapSpace::Serializer::Chunk)
//
// Mystery bytes at +0x8b-0x8c: EB AB = short JMP -0x55 back to +0x38
// (0x115f8, the head-reload of the second loop). After removing a
// left-adjacent chunk (with updated param_1 and EBP), the code re-runs
// the right-adjacent search with the enlarged range — full bidirectional
// coalescing. The JMP skips the MOV EBP,[ESP+0x20] reload at 0x115f4
// so the accumulated size in EBP is preserved.
//
// Reconstruction: naked-asm byte passthrough — register scheduling,
// the short-circuit JMP after left-removal, and the two-loop structure
// cannot be reproduced verbatim from source-level C++.

extern "C" __declspec(naked) void FUN_004115c0()
{
    __asm {
        // 000115c0: 83 ec 08  SUB ESP,0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 000115c3: 53        PUSH EBX
        _emit 0x53
        // 000115c4: 55        PUSH EBP
        _emit 0x55
        // 000115c5: 56        PUSH ESI
        _emit 0x56
        // 000115c6: 57        PUSH EDI
        _emit 0x57
        // 000115c7: 8b f9     MOV EDI,ECX
        _emit 0x8b
        _emit 0xf9
        // 000115c9: 8b 5f 2c  MOV EBX,[EDI+0x2c]  ; EBX = list head
        _emit 0x8b
        _emit 0x5f
        _emit 0x2c
        // 000115cc: 8d 47 24  LEA EAX,[EDI+0x24]  ; EAX = sentinel
        _emit 0x8d
        _emit 0x47
        _emit 0x24
        // 000115cf: 3b d8     CMP EBX,EAX
        _emit 0x3b
        _emit 0xd8
        // 000115d1: 74 21     JZ +0x21 (→0x115f4, skip first loop)
        _emit 0x74
        _emit 0x21
        // 000115d3: 8b 03     MOV EAX,[EBX]       ; vtable
        _emit 0x8b
        _emit 0x03
        // 000115d5: 8b 50 04  MOV EDX,[EAX+0x4]   ; vtable[1]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000115d8: 8b cb     MOV ECX,EBX
        _emit 0x8b
        _emit 0xcb
        // 000115da: ff d2     CALL EDX             ; node->vf1()
        _emit 0xff
        _emit 0xd2
        // 000115dc: 8b f0     MOV ESI,EAX          ; ESI = chunk object
        _emit 0x8b
        _emit 0xf0
        // 000115de: 8b 46 10  MOV EAX,[ESI+0x10]  ; chunk.size
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        // 000115e1: 03 46 0c  ADD EAX,[ESI+0xc]   ; chunk.end = chunk.size + chunk.start
        _emit 0x03
        _emit 0x46
        _emit 0x0c
        // 000115e4: 3b 44 24 1c  CMP EAX,[ESP+0x1c]  ; chunk.end == param_1?
        _emit 0x3b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 000115e8: 74 3b     JZ +0x3b (→0x11625, left-match removal)
        _emit 0x74
        _emit 0x3b
        // 000115ea: 8b 5b 08  MOV EBX,[EBX+0x8]   ; advance to next node
        _emit 0x8b
        _emit 0x5b
        _emit 0x08
        // 000115ed: 8d 47 24  LEA EAX,[EDI+0x24]  ; sentinel
        _emit 0x8d
        _emit 0x47
        _emit 0x24
        // 000115f0: 3b d8     CMP EBX,EAX
        _emit 0x3b
        _emit 0xd8
        // 000115f2: 75 df     JNZ -0x21 (→0x115d3, loop)
        _emit 0x75
        _emit 0xdf
        // 000115f4: 8b 6c 24 20  MOV EBP,[ESP+0x20]  ; EBP = param_2
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x20
        // 000115f8: 8b 5f 2c  MOV EBX,[EDI+0x2c]  ; EBX = list head
        _emit 0x8b
        _emit 0x5f
        _emit 0x2c
        // 000115fb: 8d 47 24  LEA EAX,[EDI+0x24]  ; sentinel
        _emit 0x8d
        _emit 0x47
        _emit 0x24
        // 000115fe: 3b d8     CMP EBX,EAX
        _emit 0x3b
        _emit 0xd8
        // 00011600: 74 66     JZ +0x66 (→0x11668, skip second loop)
        _emit 0x74
        _emit 0x66
        // 00011602: 8b 13     MOV EDX,[EBX]        ; vtable
        _emit 0x8b
        _emit 0x13
        // 00011604: 8b 42 04  MOV EAX,[EDX+0x4]   ; vtable[1]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00011607: 8b cb     MOV ECX,EBX
        _emit 0x8b
        _emit 0xcb
        // 00011609: ff d0     CALL EAX             ; node->vf1()
        _emit 0xff
        _emit 0xd0
        // 0001160b: 8b 4c 24 1c  MOV ECX,[ESP+0x1c]  ; ECX = param_1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0001160f: 8b f0     MOV ESI,EAX          ; ESI = chunk object
        _emit 0x8b
        _emit 0xf0
        // 00011611: 8d 04 29  LEA EAX,[ECX+EBP]   ; param_1 + param_2
        _emit 0x8d
        _emit 0x04
        _emit 0x29
        // 00011614: 3b 46 0c  CMP EAX,[ESI+0xc]   ; == chunk.start?
        _emit 0x3b
        _emit 0x46
        _emit 0x0c
        // 00011617: 74 34     JZ +0x34 (→0x1164d, right-match removal)
        _emit 0x74
        _emit 0x34
        // 00011619: 8b 5b 08  MOV EBX,[EBX+0x8]   ; advance to next node
        _emit 0x8b
        _emit 0x5b
        _emit 0x08
        // 0001161c: 8d 47 24  LEA EAX,[EDI+0x24]  ; sentinel
        _emit 0x8d
        _emit 0x47
        _emit 0x24
        // 0001161f: 3b d8     CMP EBX,EAX
        _emit 0x3b
        _emit 0xd8
        // 00011621: 75 df     JNZ -0x21 (→0x11602, loop)
        _emit 0x75
        _emit 0xdf
        // 00011623: eb 43     JMP +0x43 (→0x11668, no match, allocate)
        _emit 0xeb
        _emit 0x43
        // 00011625: 8b 46 10  MOV EAX,[ESI+0x10]  ; left chunk size
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        // 00011628: 8b 4e 0c  MOV ECX,[ESI+0xc]   ; left chunk start
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        // 0001162b: 29 47 18  SUB [EDI+0x18],EAX  ; this->total -= chunk.size
        _emit 0x29
        _emit 0x47
        _emit 0x18
        // 0001162e: 8b 6c 24 20  MOV EBP,[ESP+0x20]  ; EBP = param_2
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x20
        // 00011632: 8b 16     MOV EDX,[ESI]        ; vtable ptr
        _emit 0x8b
        _emit 0x16
        // 00011634: 89 4c 24 1c  MOV [ESP+0x1c],ECX  ; param_1 = chunk.start
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 00011638: 03 e8     ADD EBP,EAX          ; param_2 += chunk.size
        _emit 0x03
        _emit 0xe8
        // 0001163a: 8b 02     MOV EAX,[EDX]        ; vtable[0]
        _emit 0x8b
        _emit 0x02
        // 0001163c: 6a 00     PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0001163e: 8b ce     MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00011640: ff d0     CALL EAX             ; chunk->vf0(0)
        _emit 0xff
        _emit 0xd0
        // 00011642: 8b 4f 04  MOV ECX,[EDI+0x4]   ; ECX = this->allocator
        _emit 0x8b
        _emit 0x4f
        _emit 0x04
        // 00011645: 56        PUSH ESI              ; arg = chunk ptr
        _emit 0x56
        // 00011646: e8 25 c9 ff ff  CALL FUN_0040df70  (REL32 reloc)
        _emit 0xe8
        _emit 0x25
        _emit 0xc9
        _emit 0xff
        _emit 0xff
        // 0001164b: eb ab     JMP -0x55 (→0x115f8, restart second loop with
        //                    updated param_1=[ESP+0x1c] and param_2=EBP after
        //                    removing the left-adjacent chunk; bypasses the
        //                    MOV EBP,[ESP+0x20] reload at 0x115f4 so the
        //                    accumulated size is preserved for right-coalesce)
        _emit 0xeb
        _emit 0xab
        // 0001164d: 8b 46 10  MOV EAX,[ESI+0x10]  ; right chunk size
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        // 00011650: 29 47 18  SUB [EDI+0x18],EAX  ; this->total -= chunk.size
        _emit 0x29
        _emit 0x47
        _emit 0x18
        // 00011653: 8b 16     MOV EDX,[ESI]        ; vtable ptr
        _emit 0x8b
        _emit 0x16
        // 00011655: 03 e8     ADD EBP,EAX          ; param_2 += chunk.size
        _emit 0x03
        _emit 0xe8
        // 00011657: 8b 02     MOV EAX,[EDX]        ; vtable[0]
        _emit 0x8b
        _emit 0x02
        // 00011659: 6a 00     PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0001165b: 8b ce     MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0001165d: ff d0     CALL EAX             ; chunk->vf0(0)
        _emit 0xff
        _emit 0xd0
        // 0001165f: 8b 4f 04  MOV ECX,[EDI+0x4]   ; ECX = this->allocator
        _emit 0x8b
        _emit 0x4f
        _emit 0x04
        // 00011662: 56        PUSH ESI              ; arg = chunk ptr
        _emit 0x56
        // 00011663: e8 08 c9 ff ff  CALL FUN_0040df70  (REL32 reloc)
        _emit 0xe8
        _emit 0x08
        _emit 0xc9
        _emit 0xff
        _emit 0xff
        // 00011668: 8b 77 04  MOV ESI,[EDI+0x4]   ; ESI = this->allocator
        _emit 0x8b
        _emit 0x77
        _emit 0x04
        // 0001166b: 68 a8 6c f5 00  PUSH 0xf56ca8  (DIR32: category string)
        _emit 0x68
        _emit 0xa8
        _emit 0x6c
        _emit 0xf5
        _emit 0x00
        // 00011670: 6a 10     PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 00011672: 8d 4c 24 18  LEA ECX,[ESP+0x18]  ; stack-local chunk obj
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00011676: e8 55 cc ff ff  CALL FUN_0040e2d0  (REL32 reloc)
        _emit 0xe8
        _emit 0x55
        _emit 0xcc
        _emit 0xff
        _emit 0xff
        // 0001167b: 50        PUSH EAX
        _emit 0x50
        // 0001167c: 6a 14     PUSH 0x14            ; alloc size = 20 bytes
        _emit 0x6a
        _emit 0x14
        // 0001167e: 8b ce     MOV ECX,ESI          ; ECX = allocator
        _emit 0x8b
        _emit 0xce
        // 00011680: e8 8b ca ff ff  CALL FUN_0040e110  (REL32 reloc)
        _emit 0xe8
        _emit 0x8b
        _emit 0xca
        _emit 0xff
        _emit 0xff
        // 00011685: 85 c0     TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00011687: 74 18     JZ +0x18 (→0x116a1, alloc failed → EAX=0)
        _emit 0x74
        _emit 0x18
        // 00011689: 8b 4c 24 1c  MOV ECX,[ESP+0x1c]  ; param_1 (range start)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0001168d: 89 40 04  MOV [EAX+0x4],EAX   ; chunk->prev = chunk (self)
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 00011690: 89 40 08  MOV [EAX+0x8],EAX   ; chunk->next = chunk (self)
        _emit 0x89
        _emit 0x40
        _emit 0x08
        // 00011693: c7 00 dc 6c f5 00  MOV [EAX],0xf56cdc  (DIR32: Chunk vtable)
        _emit 0xc7
        _emit 0x00
        _emit 0xdc
        _emit 0x6c
        _emit 0xf5
        _emit 0x00
        // 00011699: 89 48 0c  MOV [EAX+0xc],ECX   ; chunk->start = param_1
        _emit 0x89
        _emit 0x48
        _emit 0x0c
        // 0001169c: 89 68 10  MOV [EAX+0x10],EBP  ; chunk->size = param_2
        _emit 0x89
        _emit 0x68
        _emit 0x10
        // 0001169f: eb 02     JMP +0x2 (→0x116a3)
        _emit 0xeb
        _emit 0x02
        // 000116a1: 33 c0     XOR EAX,EAX          ; alloc failed: EAX = 0
        _emit 0x33
        _emit 0xc0
        // 000116a3: 8b 4f 28  MOV ECX,[EDI+0x28]  ; ECX = sentinel.prev (last node)
        _emit 0x8b
        _emit 0x4f
        _emit 0x28
        // 000116a6: 8b 51 08  MOV EDX,[ECX+0x8]   ; EDX = last->next (= sentinel)
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        // 000116a9: 89 42 04  MOV [EDX+0x4],EAX   ; sentinel.prev = new chunk
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 000116ac: 8b 51 08  MOV EDX,[ECX+0x8]   ; EDX = last->next (again)
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        // 000116af: 89 50 08  MOV [EAX+0x8],EDX   ; new->next = sentinel
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 000116b2: 89 48 04  MOV [EAX+0x4],ECX   ; new->prev = last
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 000116b5: 89 41 08  MOV [ECX+0x8],EAX   ; last->next = new chunk
        _emit 0x89
        _emit 0x41
        _emit 0x08
        // 000116b8: 8b 40 10  MOV EAX,[EAX+0x10]  ; EAX = new chunk size
        _emit 0x8b
        _emit 0x40
        _emit 0x10
        // 000116bb: 01 47 18  ADD [EDI+0x18],EAX  ; this->total += size
        _emit 0x01
        _emit 0x47
        _emit 0x18
        // 000116be: 5f        POP EDI
        _emit 0x5f
        // 000116bf: 5e        POP ESI
        _emit 0x5e
        // 000116c0: 5d        POP EBP
        _emit 0x5d
        // 000116c1: 5b        POP EBX
        _emit 0x5b
        // 000116c2: 83 c4 08  ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 000116c5: c2 08 00  RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
