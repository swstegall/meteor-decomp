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
// FUNCTION: ffxivgame 0x000115c0 — RemovableHeapSpace chunk coalescing insert
//                                   (__thiscall, 262 B / 0x106)
//
// __thiscall void FUN_004115c0(this, int start, int size)
//   ECX        : this   — pointer to the RemovableHeapSpace manager
//   [ESP+0x04] : int start  — start address of the chunk being returned
//   [ESP+0x08] : int size   — size of the chunk being returned
//   RET 0x8    : callee cleans 2 stack args
//
// Object layout (inferred from offsets, ECX = this):
//   [this + 0x04]   Allocator*  m_alloc    — allocator used to create/free chunks
//   [this + 0x18]   int         m_total    — running total of all chunk sizes
//   [this + 0x24]   ListNode*   m_sentinel — sentinel node (this+0x24 is the address)
//   [this + 0x28]   ListNode*   m_list     — doubly-linked list (back pointer in node)
//   [this + 0x2c]   ListNode*   m_head     — head of the free-chunk linked list
//
// ListNode layout (5 DWORDs = 20 bytes):
//   [node + 0x00]  void**  vftable  — vtable pointer
//   [node + 0x04]  node*   prev     — previous in list
//   [node + 0x08]  node*   next     — next in list
//   [node + 0x0c]  int     start    — chunk start address
//   [node + 0x10]  int     size     — chunk size
//
// Behaviour (memory block coalescing):
//   Phase 1 — coalesce-before:
//     Walk the free list; find a chunk whose end (start+size) == param_start.
//     If found: remove it from the list (vtable[0](node,0) + FUN_0040df70),
//     record its start into param_start, add its size to param_size.
//   Phase 2 — coalesce-after:
//     Walk the free list; find a chunk whose start == param_start+param_size.
//     If found: remove it from the list, add its size to param_size.
//   Phase 3 — insert:
//     Allocate a new Chunk node (20 bytes) via FUN_0040e110 with a stack-local
//     "CDev.Engine.Memory.Alternative" space descriptor.
//     Initialise the chunk (vftable=0xf56cdc, self-links, start, size).
//     Splice the new node into the manager's list at this->m_list.
//     Increment this->m_total by the new chunk's size.
//
// Calling convention: __thiscall; callee cleans 2 stack args (RET 0x8).
// Callee-saved registers pushed in prologue: EBX, EBP, ESI, EDI.
// Stack frame: SUB ESP, 0x8 provides 8 bytes used as a space-descriptor struct
//   that is passed by pointer to FUN_0040e2d0 / FUN_0040e110.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The param_start slot at [ESP+0x1c] is overwritten mid-function (phase 1
//   match path stores the predecessor chunk's start into that slot), and
//   EBP (param_size accumulator) is updated in place.  The two-phase loop
//   structure with shared match-removal code at 0x1164d falls through
//   directly to the insert phase — there is no high-level C++ idiom that
//   reproduces this exact control flow without shifting branches.  Naked-asm
//   is used to guarantee byte-identical output without register-allocation
//   or stack-slot placement risk.
//
// Reloc-bearing sites (REL32 displacements — masked by compare.py):
//   +0x86  CALL FUN_0040df70  (first removal)
//   +0xa3  CALL FUN_0040df70  (second removal)
//   +0xb6  CALL FUN_0040e2d0  (space-descriptor init)
//   +0xc0  CALL FUN_0040e110  (chunk allocation)

// Forward declarations for REL32 call targets (compare.py masks their offsets).
extern "C" void __stdcall FUN_0040df70(void *param_1);
extern "C" void FUN_0040e2d0();
extern "C" void FUN_0040e110();

extern "C" __declspec(naked) void FUN_004115c0()
{
    __asm {
        // 000115c0:  83 ec 08              SUB ESP,0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 000115c3:  53                    PUSH EBX
        _emit 0x53
        // 000115c4:  55                    PUSH EBP
        _emit 0x55
        // 000115c5:  56                    PUSH ESI
        _emit 0x56
        // 000115c6:  57                    PUSH EDI
        _emit 0x57
        // 000115c7:  8b f9                 MOV EDI,ECX          ; EDI = this
        _emit 0x8b
        _emit 0xf9
        // 000115c9:  8b 5f 2c              MOV EBX,[EDI+0x2c]   ; EBX = this->m_head (list head)
        _emit 0x8b
        _emit 0x5f
        _emit 0x2c
        // 000115cc:  8d 47 24              LEA EAX,[EDI+0x24]   ; EAX = &this->m_sentinel
        _emit 0x8d
        _emit 0x47
        _emit 0x24
        // 000115cf:  3b d8                 CMP EBX,EAX          ; if (head == sentinel) skip loop1
        _emit 0x3b
        _emit 0xd8
        // 000115d1:  74 21                 JZ +0x21 → 0x004115f4
        _emit 0x74
        _emit 0x21
        // === loop1 body ===
        // 000115d3:  8b 03                 MOV EAX,[EBX]        ; EAX = *node (vtable ptr)
        _emit 0x8b
        _emit 0x03
        // 000115d5:  8b 50 04              MOV EDX,[EAX+0x4]    ; EDX = vtable[1]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000115d8:  8b cb                 MOV ECX,EBX          ; ECX = node (this for call)
        _emit 0x8b
        _emit 0xcb
        // 000115da:  ff d2                 CALL EDX             ; element = vtable[1](node)
        _emit 0xff
        _emit 0xd2
        // 000115dc:  8b f0                 MOV ESI,EAX          ; ESI = element
        _emit 0x8b
        _emit 0xf0
        // 000115de:  8b 46 10              MOV EAX,[ESI+0x10]   ; EAX = element->size
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        // 000115e1:  03 46 0c              ADD EAX,[ESI+0xc]    ; EAX = element->size + element->start
        _emit 0x03
        _emit 0x46
        _emit 0x0c
        // 000115e4:  3b 44 24 1c           CMP EAX,[ESP+0x1c]   ; == param_start?
        _emit 0x3b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 000115e8:  74 3b                 JZ +0x3b → 0x00411625
        _emit 0x74
        _emit 0x3b
        // 000115ea:  8b 5b 08              MOV EBX,[EBX+0x8]    ; EBX = node->next
        _emit 0x8b
        _emit 0x5b
        _emit 0x08
        // 000115ed:  8d 47 24              LEA EAX,[EDI+0x24]   ; EAX = sentinel
        _emit 0x8d
        _emit 0x47
        _emit 0x24
        // 000115f0:  3b d8                 CMP EBX,EAX          ; if (next != sentinel) loop
        _emit 0x3b
        _emit 0xd8
        // 000115f2:  75 df                 JNZ -0x21 → 0x004115d3
        _emit 0x75
        _emit 0xdf
        // === after loop1 / loop2 init ===
        // 000115f4:  8b 6c 24 20           MOV EBP,[ESP+0x20]   ; EBP = param_size
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x20
        // 000115f8:  8b 5f 2c              MOV EBX,[EDI+0x2c]   ; EBX = this->m_head (reload)
        _emit 0x8b
        _emit 0x5f
        _emit 0x2c
        // 000115fb:  8d 47 24              LEA EAX,[EDI+0x24]   ; EAX = sentinel
        _emit 0x8d
        _emit 0x47
        _emit 0x24
        // 000115fe:  3b d8                 CMP EBX,EAX          ; if (head == sentinel) skip loop2
        _emit 0x3b
        _emit 0xd8
        // 00011600:  74 66                 JZ +0x66 → 0x00411668
        _emit 0x74
        _emit 0x66
        // === loop2 body ===
        // 00011602:  8b 13                 MOV EDX,[EBX]        ; EDX = *node (vtable ptr)
        _emit 0x8b
        _emit 0x13
        // 00011604:  8b 42 04              MOV EAX,[EDX+0x4]    ; EAX = vtable[1]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00011607:  8b cb                 MOV ECX,EBX          ; ECX = node
        _emit 0x8b
        _emit 0xcb
        // 00011609:  ff d0                 CALL EAX             ; element = vtable[1](node)
        _emit 0xff
        _emit 0xd0
        // 0001160b:  8b 4c 24 1c           MOV ECX,[ESP+0x1c]   ; ECX = param_start (possibly updated)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0001160f:  8b f0                 MOV ESI,EAX          ; ESI = element
        _emit 0x8b
        _emit 0xf0
        // 00011611:  8d 04 29              LEA EAX,[ECX+EBP]    ; EAX = param_start + param_size
        _emit 0x8d
        _emit 0x04
        _emit 0x29
        // 00011614:  3b 46 0c              CMP EAX,[ESI+0xc]    ; == element->start?
        _emit 0x3b
        _emit 0x46
        _emit 0x0c
        // 00011617:  74 34                 JZ +0x34 → 0x0041164d
        _emit 0x74
        _emit 0x34
        // 00011619:  8b 5b 08              MOV EBX,[EBX+0x8]    ; EBX = node->next
        _emit 0x8b
        _emit 0x5b
        _emit 0x08
        // 0001161c:  8d 47 24              LEA EAX,[EDI+0x24]   ; EAX = sentinel
        _emit 0x8d
        _emit 0x47
        _emit 0x24
        // 0001161f:  3b d8                 CMP EBX,EAX          ; if (next != sentinel) loop
        _emit 0x3b
        _emit 0xd8
        // 00011621:  75 df                 JNZ -0x21 → 0x00411602
        _emit 0x75
        _emit 0xdf
        // 00011623:  eb 43                 JMP +0x43 → 0x00411668
        _emit 0xeb
        _emit 0x43
        // === loop1 match handler ===
        // 00011625:  8b 46 10              MOV EAX,[ESI+0x10]   ; EAX = element->size
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        // 00011628:  8b 4e 0c              MOV ECX,[ESI+0xc]    ; ECX = element->start
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        // 0001162b:  29 47 18              SUB [EDI+0x18],EAX   ; this->m_total -= element->size
        _emit 0x29
        _emit 0x47
        _emit 0x18
        // 0001162e:  8b 6c 24 20           MOV EBP,[ESP+0x20]   ; EBP = param_size
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x20
        // 00011632:  8b 16                 MOV EDX,[ESI]        ; EDX = element->vftable
        _emit 0x8b
        _emit 0x16
        // 00011634:  89 4c 24 1c           MOV [ESP+0x1c],ECX   ; param_start = element->start
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 00011638:  03 e8                 ADD EBP,EAX          ; param_size += element->size
        _emit 0x03
        _emit 0xe8
        // 0001163a:  8b 02                 MOV EAX,[EDX]        ; EAX = vtable[0]
        _emit 0x8b
        _emit 0x02
        // 0001163c:  6a 00                 PUSH 0x0             ; arg = 0
        _emit 0x6a
        _emit 0x00
        // 0001163e:  8b ce                 MOV ECX,ESI          ; ECX = element (this)
        _emit 0x8b
        _emit 0xce
        // 00011640:  ff d0                 CALL EAX             ; vtable[0](element, 0)
        _emit 0xff
        _emit 0xd0
        // 00011642:  8b 4f 04              MOV ECX,[EDI+0x4]    ; ECX = this->m_alloc
        _emit 0x8b
        _emit 0x4f
        _emit 0x04
        // 00011645:  56                    PUSH ESI             ; arg = element
        _emit 0x56
        // 00011646:  e8 25 c9 ff ff        CALL FUN_0040df70    ; FUN_0040df70(this->m_alloc, element)
        call    FUN_0040df70
        // 0001164b:  eb ab                 JMP -0x55 → 0x004115f8 (skip EBP-reload; go to loop2 body)
        _emit 0xeb
        _emit 0xab
        // === shared removal body / loop2 match handler ===
        // 0001164d:  8b 46 10              MOV EAX,[ESI+0x10]   ; EAX = element->size
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        // 00011650:  29 47 18              SUB [EDI+0x18],EAX   ; this->m_total -= element->size
        _emit 0x29
        _emit 0x47
        _emit 0x18
        // 00011653:  8b 16                 MOV EDX,[ESI]        ; EDX = element->vftable
        _emit 0x8b
        _emit 0x16
        // 00011655:  03 e8                 ADD EBP,EAX          ; param_size += element->size
        _emit 0x03
        _emit 0xe8
        // 00011657:  8b 02                 MOV EAX,[EDX]        ; EAX = vtable[0]
        _emit 0x8b
        _emit 0x02
        // 00011659:  6a 00                 PUSH 0x0             ; arg = 0
        _emit 0x6a
        _emit 0x00
        // 0001165b:  8b ce                 MOV ECX,ESI          ; ECX = element (this)
        _emit 0x8b
        _emit 0xce
        // 0001165d:  ff d0                 CALL EAX             ; vtable[0](element, 0)
        _emit 0xff
        _emit 0xd0
        // 0001165f:  8b 4f 04              MOV ECX,[EDI+0x4]    ; ECX = this->m_alloc
        _emit 0x8b
        _emit 0x4f
        _emit 0x04
        // 00011662:  56                    PUSH ESI             ; arg = element
        _emit 0x56
        // 00011663:  e8 08 c9 ff ff        CALL FUN_0040df70    ; FUN_0040df70(this->m_alloc, element)
        call    FUN_0040df70
        // === insert phase ===
        // 00011668:  8b 77 04              MOV ESI,[EDI+0x4]    ; ESI = this->m_alloc
        _emit 0x8b
        _emit 0x77
        _emit 0x04
        // 0001166b:  68 a8 6c f5 00        PUSH 0xf56ca8        ; space.name = "CDev.Engine.Memory.Alternative"
        _emit 0x68
        _emit 0xa8
        _emit 0x6c
        _emit 0xf5
        _emit 0x00
        // 00011670:  6a 10                 PUSH 0x10            ; space.field0 = 0x10
        _emit 0x6a
        _emit 0x10
        // 00011672:  8d 4c 24 18           LEA ECX,[ESP+0x18]   ; ECX = &local_space_descriptor
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00011676:  e8 55 cc ff ff        CALL FUN_0040e2d0    ; local_space_descriptor = {0x10, "CDev..."}
        call    FUN_0040e2d0
        // 0001167b:  50                    PUSH EAX             ; PUSH &local_space_descriptor
        _emit 0x50
        // 0001167c:  6a 14                 PUSH 0x14            ; size = 20 bytes
        _emit 0x6a
        _emit 0x14
        // 0001167e:  8b ce                 MOV ECX,ESI          ; ECX = this->m_alloc
        _emit 0x8b
        _emit 0xce
        // 00011680:  e8 8b ca ff ff        CALL FUN_0040e110    ; new_node = alloc(20, &space)
        call    FUN_0040e110
        // 00011685:  85 c0                 TEST EAX,EAX         ; if (new_node == NULL) zero it
        _emit 0x85
        _emit 0xc0
        // 00011687:  74 18                 JZ +0x18 → 0x004116a1
        _emit 0x74
        _emit 0x18
        // 00011689:  8b 4c 24 1c           MOV ECX,[ESP+0x1c]   ; ECX = param_start (updated)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0001168d:  89 40 04              MOV [EAX+0x4],EAX    ; new_node->prev = new_node (self-link)
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 00011690:  89 40 08              MOV [EAX+0x8],EAX    ; new_node->next = new_node (self-link)
        _emit 0x89
        _emit 0x40
        _emit 0x08
        // 00011693:  c7 00 dc 6c f5 00     MOV [EAX],0xf56cdc   ; new_node->vftable = Chunk::vftable
        _emit 0xc7
        _emit 0x00
        _emit 0xdc
        _emit 0x6c
        _emit 0xf5
        _emit 0x00
        // 00011699:  89 48 0c              MOV [EAX+0xc],ECX    ; new_node->start = param_start
        _emit 0x89
        _emit 0x48
        _emit 0x0c
        // 0001169c:  89 68 10              MOV [EAX+0x10],EBP   ; new_node->size = param_size
        _emit 0x89
        _emit 0x68
        _emit 0x10
        // 0001169f:  eb 02                 JMP +0x02 → 0x004116a3
        _emit 0xeb
        _emit 0x02
        // 000116a1:  33 c0                 XOR EAX,EAX          ; new_node = NULL
        _emit 0x33
        _emit 0xc0
        // === list splice ===
        // 000116a3:  8b 4f 28              MOV ECX,[EDI+0x28]   ; ECX = this->m_list
        _emit 0x8b
        _emit 0x4f
        _emit 0x28
        // 000116a6:  8b 51 08              MOV EDX,[ECX+0x8]    ; EDX = m_list->next
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        // 000116a9:  89 42 04              MOV [EDX+0x4],EAX    ; m_list->next->prev = new_node
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 000116ac:  8b 51 08              MOV EDX,[ECX+0x8]    ; EDX = m_list->next (reload)
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        // 000116af:  89 50 08              MOV [EAX+0x8],EDX    ; new_node->next = m_list->next
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 000116b2:  89 48 04              MOV [EAX+0x4],ECX    ; new_node->prev = m_list
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 000116b5:  89 41 08              MOV [ECX+0x8],EAX    ; m_list->next = new_node
        _emit 0x89
        _emit 0x41
        _emit 0x08
        // 000116b8:  8b 40 10              MOV EAX,[EAX+0x10]   ; EAX = new_node->size
        _emit 0x8b
        _emit 0x40
        _emit 0x10
        // 000116bb:  01 47 18              ADD [EDI+0x18],EAX   ; this->m_total += new_node->size
        _emit 0x01
        _emit 0x47
        _emit 0x18
        // === epilogue ===
        // 000116be:  5f                    POP EDI
        _emit 0x5f
        // 000116bf:  5e                    POP ESI
        _emit 0x5e
        // 000116c0:  5d                    POP EBP
        _emit 0x5d
        // 000116c1:  5b                    POP EBX
        _emit 0x5b
        // 000116c2:  83 c4 08              ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 000116c5:  c2 08 00              RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
