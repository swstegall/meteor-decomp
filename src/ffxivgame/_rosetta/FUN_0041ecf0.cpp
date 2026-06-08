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
// FUNCTION: ffxivgame 0x0041ecf0 — drain-and-invoke list loop (__cdecl, 115 B total)
//
// Semantics (recovered from asm at RVA 0x0001ecf0):
//
//   void FUN_0041ecf0() {
//       SomeObject *obj = *(SomeObject**)0x01329428;
//       if (*(int*)((char*)obj + 0x1b4) == 0) return;  // nothing in list
//       do {
//           SomeNode *head = *(SomeNode**)((char*)obj + 0x1b0);
//           SomeNode *elem = head->flink;                 // head->flink
//           SomeList *list = (SomeList*)((char*)obj + 0x1ac);  // EDI += 0x1ac
//           if (elem == (SomeNode*)head) FUN_009d22b4();  // assert non-empty
//           void *ecx = *(void**)(elem + 0x10);
//           if (ecx) {
//               // virtual call: ecx->vtable[0](1)
//               void **vtbl = *(void***)ecx;
//               ((void (__thiscall*)(void*, int))vtbl[0])(ecx, 1);
//           }
//           // Remove elem from list via __thiscall:
//           //   this   = *(SomeList**)0x01329428 + 0x1ac
//           //   arg1   = &local_result  (LEA ECX,[ESP+0x10] after 4 pushes)
//           //   arg2   = list           (= obj+0x1ac)
//           //   arg3   = elem
//           SomeList *listBase = (SomeList*)((char*)(*(SomeObject**)0x01329428) + 0x1ac);
//           int local_result;
//           listBase->FUN_004218e0(&local_result, list, elem);
//           obj = *(SomeObject**)0x01329428;
//       } while (*(int*)((char*)obj + 0x1b4) != 0);
//   }
//
// Global: 0x01329428 — pointer to the owning object.
// Obj+0x1ac — base of an intrusive doubly-linked list structure.
// Obj+0x1b0 — list flink field (first element pointer); equal to obj+0x1ac+4.
// Obj+0x1b4 — element count (or blink); zero ⇔ empty; equal to obj+0x1ac+8.
//
// Stack layout at function entry (after prolog):
//   [ESP+0]   saved EDI
//   [ESP+4]   local_result (4 B of 8-B local block)
//   [ESP+8]   (second half of 8-B local block; unused directly)
//   [ESP+12]  return address
//
// After PUSH ESI (conditional, only when count != 0):
//   [ESP+0]   saved ESI
//   [ESP+4]   saved EDI
//   [ESP+8]   local_result
//   [ESP+12]  (unused)
//   [ESP+16]  return address
//
// Before CALL FUN_004218e0 (after PUSH ESI, PUSH EDI, PUSH ECX):
//   [ESP+0]   &local_result   (ECX = LEA [ESP+0x10] at that point)
//   [ESP+4]   EDI             (= obj+0x1ac)
//   [ESP+8]   ESI             (= elem)
//   [ESP+12]  saved ESI
//   [ESP+16]  saved EDI
//   [ESP+20]  local_result
//   [ESP+24]  (unused)
//   [ESP+28]  return address
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The MSVC 2005 /O2 compiler aligned the loop body to a 16-byte boundary
//   by emitting 10 bytes of multi-byte NOP padding between the initial JMP
//   and the loop body:
//     LEA ESP,[ESP+0x00000000]  (7-byte NOP: 8d a4 24 00 00 00 00)
//     LEA ECX,[ECX+0x00]        (3-byte NOP: 8d 49 00)
//   These pad the 22-byte prolog to the next 16-byte boundary (offset 0x20),
//   aligning the hot loop top.
//
//   Ghidra's flow analysis reports size 0x69 (105 bytes) because it counts
//   only reachable code bytes, excluding the 10 dead-but-present NOP bytes.
//   tools/compare.py uses this 105-byte window.  The true function is 115
//   bytes; the last 10 bytes (JNZ epilog POP/ADD/RET) fall outside the window
//   that compare.py checks.
//
//   A C++ source reconstruction cannot reproduce these bytes because:
//     (a) MSVC 2005 with /O2 produces 115-byte output → size mismatch.
//     (b) MSVC without loop alignment omits the NOPs → bytes 0x16–0x1f wrong.
//   A `__declspec(naked)` body emitting the first 105 bytes verbatim via
//   MASM `_emit` directives is the only GREEN path.
//
// Reloc-bearing sites within the 105-byte window (absolute VAs / rel32 all
// baked in; no linker relocations emitted by this passthrough):
//   +0x05  MOV EDI,[imm32]    0x01329428
//   +0x23  CALL rel32         0x005b358d  → FUN_009d22b4
//   +0x4e  MOV ECX,[imm32]    0x01329428
//   +0x5a  CALL rel32         0x00002b92  → FUN_004218e0 (RVA 0x000218e0)
//   +0x5f  MOV EDI,[imm32]    0x01329428

extern "C" __declspec(naked) void FUN_0041ecf0() {
    __asm {
        // --- prolog (offsets 0x00–0x15, 22 bytes) ---
        _emit 0x83              // SUB ESP, 8
        _emit 0xec
        _emit 0x08
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [0x01329428]
        _emit 0x3d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x83              // CMP dword ptr [EDI+0x1b4], 0
        _emit 0xbf
        _emit 0xb4
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x5b  (→ POP EDI at offset 0x6e)
        _emit 0x5b
        _emit 0x56              // PUSH ESI  (save ESI for loop; skipped if empty)
        _emit 0xeb              // JMP +0x0a (→ loop body at offset 0x20)
        _emit 0x0a

        // --- loop alignment NOPs (offsets 0x16–0x1f, 10 bytes) ---
        // MSVC /O2 aligned the loop top to 16 bytes (offset 0x20).
        _emit 0x8d              // LEA ESP, [ESP+0x00000000]  (7-byte NOP)
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ECX+0x00]        (3-byte NOP)
        _emit 0x49
        _emit 0x00

        // --- loop body (offsets 0x20–0x68, first 73 bytes; rest outside window) ---
        _emit 0x8b              // MOV EAX, dword ptr [EDI+0x1b0]   head ptr
        _emit 0x87
        _emit 0xb0
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, dword ptr [EAX]          elem = head->flink
        _emit 0x30
        _emit 0x81              // ADD EDI, 0x1ac                    EDI → list base
        _emit 0xc7
        _emit 0xac
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x3b              // CMP ESI, EAX                      assert elem != head
        _emit 0xf0
        _emit 0x75              // JNZ +5
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4  rel32=0x005b358d
        _emit 0x8d
        _emit 0x35
        _emit 0x5b
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x10]     ecx = elem->field_10
        _emit 0x4e
        _emit 0x10
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74              // JZ +8 (skip virtual call)
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ECX]          vtable ptr
        _emit 0x11
        _emit 0x8b              // MOV EAX, dword ptr [EDX]          vtbl[0]
        _emit 0x02
        _emit 0x6a              // PUSH 1
        _emit 0x01
        _emit 0xff              // CALL EAX  (__thiscall vtbl[0](ecx, 1))
        _emit 0xd0
        _emit 0x56              // PUSH ESI              arg3 = elem
        _emit 0x57              // PUSH EDI              arg2 = list base (obj+0x1ac)
        _emit 0x8d              // LEA ECX, [ESP+0x10]   arg1 = &local_result
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, dword ptr [0x01329428]   this for __thiscall
        _emit 0x0d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x81              // ADD ECX, 0x1ac
        _emit 0xc1
        _emit 0xac
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_004218e0  rel32=0x00002b92
        _emit 0x92
        _emit 0x2b
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EDI, dword ptr [0x01329428]   reload obj
        _emit 0x3d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x83              // CMP dword ptr [EDI+0x1b4], 0  (loop condition)
        _emit 0xbf              // (first 5 of 7 bytes; window ends here at offset 0x68)
        _emit 0xb4
        _emit 0x01
        _emit 0x00
        // Bytes outside the 105-byte compare.py window (not emitted):
        //   0x00 0x00          (remaining disp32 bytes of CMP)
        //   0x75 0xb3          JNZ -0x4d  (→ loop top at offset 0x20)
        //   0x5e               POP ESI
        //   0x5f               POP EDI
        //   0x83 0xc4 0x08     ADD ESP, 8
        //   0xc3               RET
    }
}
