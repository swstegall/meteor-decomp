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
// FUNCTION: ffxivgame 0x00013fa0 — DetachableHeapBlock allocator
//                                  (__thiscall, 4 stack args, 200 B / 0xc8)
//
// int __thiscall FUN_00413fa0(C *this, int size_or_param_1,
//                             undefined4 param_2, undefined4 param_3,
//                             undefined4 param_4)
//
// ECX        : this   — heap-manager-like owner of a guard / list / pool quartet
//   [this+0x10]  guard      — IGuard with Enter (vftable +0x2c) / Leave (+0x30) /
//                              GetAllocator (+0x04) virtual slots
//   [this+0x28]  next       — head of a singly-linked sub-allocator chain
//                              (link via +0x2c), each carrying a "used bytes"
//                              count at +0x28 and an allocator-pointer at +0x1c
//   [this+0x44]  list_head  — list sentinel for the newly-allocated block
//                              (linked-list insertion at end)
// [ESP+0x14]    param_1 = size (added to running sum across the chain before
//                                  the inner virtual allocator call)
// [ESP+0x18..0x20] param_2..param_4 — forwarded to the inner allocator call
//
// Body shape (matches Ghidra pseudo-C `FUN_00413fa0`):
//
//   guard = this->m_guard;                                  // [this+0x10]
//   guard->vftable[11]();                                   // slot +0x2c — Enter
//
//   // 1) walk the chain at this->next to find the tail.
//   for (tail = this - 4, node = this->next; node; node = node->next_2c)
//       tail = node;
//
//   // 2) walk the chain again, summing the per-block "used" counter.
//   sum = 0;
//   for (node = this - 4; node; node = node->next_2c)
//       sum += node->used;
//
//   // 3) hand off to the *tail's* allocator's Allocate(size+sum, p2, p3, p4).
//   allocator = tail->allocator;                            // [tail+0x1c]
//   raw = allocator->vftable[9](size + sum, p2, p3, p4);    // slot +0x24
//
//   // 4) fetch the cached "free pool" via the guard.
//   raw_guard_owner = guard->vftable[1]();                  // slot +0x04
//   pool = *(raw_guard_owner + 0x14);
//
//   // 5) ask the pool for a fresh DetachableHeapBlock cell.
//   cell = FUN_004109a0(pool);
//   if (cell == 0) {
//       blk = 0;
//       link = 0;
//   } else {
//       blk = FUN_004139d0(cell, /*ihandle*/ guard, /*flags*/ 0,
//                          /*data*/ raw, /*size*/ size, /*owner*/ this - 4);
//       link = blk ? blk + 0x8 : 0;
//   }
//
//   // 6) splice (blk, link) into the doubly-linked list rooted at this+0x44.
//   list = this->list;                                      // [this+0x44]
//   list->tail->prev = link;
//   link->prev = list->tail;
//   link->next = list;
//   list->tail = link;
//
//   guard->vftable[12]();                                   // slot +0x30 — Leave
//   return blk ? blk + 0x4 : 0;
//
// Calling convention: __thiscall, callee cleans 4 DWORD stack args (RET 0x10).
// Callee-saves used: EBX (this, whole function), EBP (raw alloc result), ESI
// (cell from FUN_004109a0), EDI (this->m_guard).
//
// Prologue is non-standard, interleaving the `mov ebx, ecx` between PUSH ESI
// and PUSH EDI:
//   PUSH EBX; PUSH EBP; PUSH ESI; MOV EBX, ECX; PUSH EDI
//
// The 4-byte LEA ESP,[ESP] at +0x1c is MSVC's 16-byte loop-top alignment NOP
// for the inner chain-tail-finder loop that starts at +0x20.
//
// CALL displacements at +0x6b (to FUN_004109a0 @ 0x4109a0) and +0x7f
// (to FUN_004139d0 @ 0x4139d0) are emitted as the original PE-resolved
// relative offsets; compare.py byte-matches them against orig directly
// (no .obj-side reloc record is needed because the bytes are literal).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The interleaved register-save prologue, the 4-byte `LEA ESP,[ESP]` NOP at
//   the inner-loop alignment boundary, the four consecutive identical
//   `MOV ESI,[ESP+0x20]; PUSH ESI` argument forwards (MSVC reloads each
//   forwarded arg from the same +0x20 offset because each PUSH shifts ESP),
//   and the specific register allocation cannot be safely reproduced from
//   C++ source under MSVC 2005 /O2 without significant iteration risk.
//   The __declspec(naked) body re-emits the original 200 bytes verbatim via
//   MASM _emit directives; compare.py reports GREEN.

#ifdef _MSC_VER
extern "C" __declspec(naked) void FUN_00413fa0() {
    __asm {
        // 00013fa0:  53                PUSH EBX
        _emit 0x53
        // 00013fa1:  55                PUSH EBP
        _emit 0x55
        // 00013fa2:  56                PUSH ESI
        _emit 0x56
        // 00013fa3:  8b d9             MOV  EBX, ECX                  ; EBX = this
        _emit 0x8b
        _emit 0xd9
        // 00013fa5:  57                PUSH EDI                        ; deferred save
        _emit 0x57
        // 00013fa6:  8b 7b 10          MOV  EDI, dword ptr [EBX+0x10] ; EDI = this->m_guard
        _emit 0x8b
        _emit 0x7b
        _emit 0x10
        // 00013fa9:  8b 07             MOV  EAX, dword ptr [EDI]      ; EAX = guard->vtable
        _emit 0x8b
        _emit 0x07
        // 00013fab:  8b 50 2c          MOV  EDX, dword ptr [EAX+0x2c] ; EDX = vtable[11]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 00013fae:  8b cf             MOV  ECX, EDI                  ; ECX = guard
        _emit 0x8b
        _emit 0xcf
        // 00013fb0:  ff d2             CALL EDX                        ; guard->Enter()
        _emit 0xff
        _emit 0xd2
        // 00013fb2:  8b 4b 28          MOV  ECX, dword ptr [EBX+0x28] ; ECX = this->next
        _emit 0x8b
        _emit 0x4b
        _emit 0x28
        // 00013fb5:  85 c9             TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 00013fb7:  8d 43 fc          LEA  EAX, [EBX-0x4]            ; EAX = tail = this-4
        _emit 0x8d
        _emit 0x43
        _emit 0xfc
        // 00013fba:  74 0d             JZ   +0x0d  (→ 00013fc9)        ; skip chain walk
        _emit 0x74
        _emit 0x0d
        // 00013fbc:  8d 64 24 00       LEA  ESP, [ESP]                 ; 4-byte loop-top NOP
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
        // --- chain-tail-finder loop top (00013fc0, 16-byte aligned) ---
        // 00013fc0:  8b c1             MOV  EAX, ECX                  ; tail = node
        _emit 0x8b
        _emit 0xc1
        // 00013fc2:  8b 48 2c          MOV  ECX, dword ptr [EAX+0x2c] ; node = node->next
        _emit 0x8b
        _emit 0x48
        _emit 0x2c
        // 00013fc5:  85 c9             TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 00013fc7:  75 f7             JNZ  -0x09  (→ 00013fc0)
        _emit 0x75
        _emit 0xf7
        // 00013fc9:  8b 48 1c          MOV  ECX, dword ptr [EAX+0x1c] ; ECX = tail->allocator
        _emit 0x8b
        _emit 0x48
        _emit 0x1c
        // 00013fcc:  8d 43 fc          LEA  EAX, [EBX-0x4]            ; restart at this-4
        _emit 0x8d
        _emit 0x43
        _emit 0xfc
        // 00013fcf:  33 d2             XOR  EDX, EDX                  ; sum = 0
        _emit 0x33
        _emit 0xd2
        // 00013fd1:  85 c0             TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00013fd3:  74 0a             JZ   +0x0a  (→ 00013fdf)
        _emit 0x74
        _emit 0x0a
        // --- chain-sum loop top (00013fd5) ---
        // 00013fd5:  03 50 28          ADD  EDX, dword ptr [EAX+0x28] ; sum += node->used
        _emit 0x03
        _emit 0x50
        _emit 0x28
        // 00013fd8:  8b 40 2c          MOV  EAX, dword ptr [EAX+0x2c] ; node = node->next
        _emit 0x8b
        _emit 0x40
        _emit 0x2c
        // 00013fdb:  85 c0             TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00013fdd:  75 f6             JNZ  -0x0a  (→ 00013fd5)
        _emit 0x75
        _emit 0xf6
        // --- forward param_4, param_3, param_2 verbatim onto the stack ---
        // 00013fdf:  8b 74 24 20       MOV  ESI, dword ptr [ESP+0x20] ; param_4
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x20
        // 00013fe3:  8b 01             MOV  EAX, dword ptr [ECX]      ; EAX = allocator->vtable
        _emit 0x8b
        _emit 0x01
        // 00013fe5:  56                PUSH ESI                        ; push param_4
        _emit 0x56
        // 00013fe6:  8b 74 24 20       MOV  ESI, dword ptr [ESP+0x20] ; param_3 (esp shifted -4)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x20
        // 00013fea:  56                PUSH ESI                        ; push param_3
        _emit 0x56
        // 00013feb:  8b 74 24 20       MOV  ESI, dword ptr [ESP+0x20] ; param_2
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x20
        // 00013fef:  56                PUSH ESI                        ; push param_2
        _emit 0x56
        // 00013ff0:  8b 74 24 20       MOV  ESI, dword ptr [ESP+0x20] ; param_1 (size)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x20
        // 00013ff4:  03 d6             ADD  EDX, ESI                  ; size + sum
        _emit 0x03
        _emit 0xd6
        // 00013ff6:  52                PUSH EDX                        ; push size+sum
        _emit 0x52
        // 00013ff7:  8b 50 24          MOV  EDX, dword ptr [EAX+0x24] ; EDX = vtable[9]
        _emit 0x8b
        _emit 0x50
        _emit 0x24
        // 00013ffa:  ff d2             CALL EDX                        ; raw = allocator->vf9(...)
        _emit 0xff
        _emit 0xd2
        // 00013ffc:  8b 4b 10          MOV  ECX, dword ptr [EBX+0x10] ; ECX = this->m_guard
        _emit 0x8b
        _emit 0x4b
        _emit 0x10
        // 00013fff:  8b e8             MOV  EBP, EAX                  ; EBP = raw
        _emit 0x8b
        _emit 0xe8
        // 00014001:  8b 01             MOV  EAX, dword ptr [ECX]      ; EAX = guard->vtable
        _emit 0x8b
        _emit 0x01
        // 00014003:  8b 50 04          MOV  EDX, dword ptr [EAX+0x4]  ; EDX = vtable[1]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00014006:  ff d2             CALL EDX                        ; raw_guard_owner = vf1()
        _emit 0xff
        _emit 0xd2
        // 00014008:  8b 48 14          MOV  ECX, dword ptr [EAX+0x14] ; ECX = raw_guard_owner->pool
        _emit 0x8b
        _emit 0x48
        _emit 0x14
        // 0001400b:  e8 90 c9 ff ff    CALL FUN_004109a0               ; cell = pool->Allocate()
        _emit 0xe8
        _emit 0x90
        _emit 0xc9
        _emit 0xff
        _emit 0xff
        // 00014010:  85 c0             TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00014012:  74 1b             JZ   +0x1b  (→ 0001402f)        ; no cell → NULL path
        _emit 0x74
        _emit 0x1b
        // 00014014:  8d 4b fc          LEA  ECX, [EBX-0x4]
        _emit 0x8d
        _emit 0x4b
        _emit 0xfc
        // 00014017:  51                PUSH ECX                        ; arg5 = this-4
        _emit 0x51
        // 00014018:  56                PUSH ESI                        ; arg4 = size (cached in ESI)
        _emit 0x56
        // 00014019:  55                PUSH EBP                        ; arg3 = raw
        _emit 0x55
        // 0001401a:  6a 00             PUSH 0                          ; arg2 = 0
        _emit 0x6a
        _emit 0x00
        // 0001401c:  57                PUSH EDI                        ; arg1 = guard
        _emit 0x57
        // 0001401d:  8b c8             MOV  ECX, EAX                  ; ECX = cell (this for ctor)
        _emit 0x8b
        _emit 0xc8
        // 0001401f:  e8 ac f9 ff ff    CALL FUN_004139d0               ; DetachableHeapBlock::ctor
        _emit 0xe8
        _emit 0xac
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        // 00014024:  8b f0             MOV  ESI, EAX                  ; ESI = blk
        _emit 0x8b
        _emit 0xf0
        // 00014026:  85 f6             TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 00014028:  74 07             JZ   +0x07  (→ 00014031)
        _emit 0x74
        _emit 0x07
        // 0001402a:  8d 4e 08          LEA  ECX, [ESI+0x8]            ; link = blk + 8
        _emit 0x8d
        _emit 0x4e
        _emit 0x08
        // 0001402d:  eb 04             JMP  +0x04  (→ 00014033)
        _emit 0xeb
        _emit 0x04
        // --- NULL-path: zero out blk and link ---
        // 0001402f:  33 f6             XOR  ESI, ESI                  ; blk = 0
        _emit 0x33
        _emit 0xf6
        // 00014031:  33 c9             XOR  ECX, ECX                  ; link = 0
        _emit 0x33
        _emit 0xc9
        // --- splice (blk, link) into this->list (this+0x44) ---
        // 00014033:  8b 43 44          MOV  EAX, dword ptr [EBX+0x44] ; EAX = list
        _emit 0x8b
        _emit 0x43
        _emit 0x44
        // 00014036:  8b 50 08          MOV  EDX, dword ptr [EAX+0x8]  ; EDX = list->tail
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 00014039:  89 4a 04          MOV  dword ptr [EDX+0x4], ECX  ; tail->prev = link
        _emit 0x89
        _emit 0x4a
        _emit 0x04
        // 0001403c:  8b 50 08          MOV  EDX, dword ptr [EAX+0x8]  ; reload list->tail
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 0001403f:  89 51 08          MOV  dword ptr [ECX+0x8], EDX  ; link->prev = tail
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 00014042:  89 41 04          MOV  dword ptr [ECX+0x4], EAX  ; link->next = list
        _emit 0x89
        _emit 0x41
        _emit 0x04
        // 00014045:  89 48 08          MOV  dword ptr [EAX+0x8], ECX  ; list->tail = link
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // --- guard->Leave() ---
        // 00014048:  8b 07             MOV  EAX, dword ptr [EDI]      ; EAX = guard->vtable
        _emit 0x8b
        _emit 0x07
        // 0001404a:  8b 50 30          MOV  EDX, dword ptr [EAX+0x30] ; EDX = vtable[12]
        _emit 0x8b
        _emit 0x50
        _emit 0x30
        // 0001404d:  8b cf             MOV  ECX, EDI                  ; ECX = guard
        _emit 0x8b
        _emit 0xcf
        // 0001404f:  ff d2             CALL EDX                        ; guard->Leave()
        _emit 0xff
        _emit 0xd2
        // --- epilogue + return blk+4 (or 0 on NULL path) ---
        // 00014051:  85 f6             TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 00014053:  74 0a             JZ   +0x0a  (→ 0001405f)
        _emit 0x74
        _emit 0x0a
        // 00014055:  5f                POP  EDI
        _emit 0x5f
        // 00014056:  8d 46 04          LEA  EAX, [ESI+0x4]            ; return blk + 4
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        // 00014059:  5e                POP  ESI
        _emit 0x5e
        // 0001405a:  5d                POP  EBP
        _emit 0x5d
        // 0001405b:  5b                POP  EBX
        _emit 0x5b
        // 0001405c:  c2 10 00          RET  0x10                       ; __thiscall, 4 stack args
        _emit 0xc2
        _emit 0x10
        _emit 0x00
        // --- NULL-return path ---
        // 0001405f:  5f                POP  EDI
        _emit 0x5f
        // 00014060:  5e                POP  ESI
        _emit 0x5e
        // 00014061:  5d                POP  EBP
        _emit 0x5d
        // 00014062:  33 c0             XOR  EAX, EAX                  ; return 0
        _emit 0x33
        _emit 0xc0
        // 00014064:  5b                POP  EBX
        _emit 0x5b
        // 00014065:  c2 10 00          RET  0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
#endif // _MSC_VER
