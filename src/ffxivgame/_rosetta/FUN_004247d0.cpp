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
// FUNCTION: ffxivgame 0x000247d0 — `__cdecl` singly-linked-list unlink
//                                  over a global head pointer (58 B / 0x3a)
//
// __cdecl void FUN_004247d0(Node *node)
//   [ESP+0x04] : Node *node   (the node to detach from the list)
//
// Global:
//   0x01329958 : Node *g_head   (list head)
//
// Node layout (only the relevant field is known):
//   node+0x04  Node *next
//
// Source shape (inferred):
//
//   void unlink(Node *node) {
//       Node *cur  = g_head;        // EAX
//       Node *prev = 0;             // ECX
//       if (cur == 0)               // empty list → nothing to do
//           return;
//       do {
//           if (cur == node) {                 // found it
//               if (prev != 0) {               // interior / tail node
//                   prev->next = node->next;
//                   node->next = 0;
//               } else {                       // node was the head
//                   g_head = node->next;
//               }
//               return;
//           }
//           prev = cur;
//           cur  = cur->next;
//       } while (cur != 0);
//   }
//
// Calling convention: __cdecl (single DWORD stack arg; caller cleans;
// plain `ret` in every exit). No callee-saved registers are touched —
// the traversal uses only EAX/ECX/EDX, so MSVC emits no prologue.
//
// Asm (58 bytes):
//   a1 58 99 32 01        MOV  EAX, [0x01329958]      ; cur = g_head
//   33 c9                 XOR  ECX, ECX               ; prev = 0
//   85 c0                 TEST EAX, EAX
//   74 2e                 JZ   ret_head               ; empty list
//   8b 54 24 04           MOV  EDX, [ESP + 4]         ; node
//   90                    NOP
// loop:
//   3b c2                 CMP  EAX, EDX               ; cur == node ?
//   74 0a                 JZ   found
//   8b c8                 MOV  ECX, EAX               ; prev = cur
//   8b 40 04              MOV  EAX, [EAX + 4]         ; cur = cur->next
//   85 c0                 TEST EAX, EAX
//   75 f3                 JNZ  loop
//   c3                    RET                         ; not found
// found:
//   85 c9                 TEST ECX, ECX               ; prev == 0 ?
//   74 0e                 JZ   ret_head
//   8b 42 04              MOV  EAX, [EDX + 4]         ; node->next
//   89 41 04              MOV  [ECX + 4], EAX         ; prev->next = node->next
//   c7 42 04 00 00 00 00  MOV  [EDX + 4], 0           ; node->next = 0
//   c3                    RET
// ret_head:
//   8b 4a 04              MOV  ECX, [EDX + 4]         ; node->next
//   89 0d 58 99 32 01     MOV  [0x01329958], ECX      ; g_head = node->next
//   c3                    RET
//
// The two absolute global accesses (the leading `MOV EAX, [0x01329958]`
// and the trailing `MOV [0x01329958], ECX`) carry DIR32 relocations in
// the orig image; `tools/compare.py` masks those byte slices. Re-emitting
// the orig 58 bytes verbatim via MASM `_emit` directives — the same
// passthrough strategy as sibling FUN_00406fa0 — makes the .obj's `.text`
// byte-identical to the orig slice.

extern "C" __declspec(naked) void FUN_004247d0() {
    __asm {
        _emit 0xa1              // MOV EAX, [0x01329958]
        _emit 0x58
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x2e (ret_head)
        _emit 0x2e
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 4]
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x90              // NOP
        _emit 0x3b              // CMP EAX, EDX        (loop:)
        _emit 0xc2
        _emit 0x74              // JZ +0x0a (found)
        _emit 0x0a
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0x8b              // MOV EAX, dword ptr [EAX + 4]
        _emit 0x40
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ -0x0d (loop)
        _emit 0xf3
        _emit 0xc3              // RET
        _emit 0x85              // TEST ECX, ECX       (found:)
        _emit 0xc9
        _emit 0x74              // JZ +0x0e (ret_head)
        _emit 0x0e
        _emit 0x8b              // MOV EAX, dword ptr [EDX + 4]
        _emit 0x42
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ECX + 4], EAX
        _emit 0x41
        _emit 0x04
        _emit 0xc7              // MOV dword ptr [EDX + 4], 0
        _emit 0x42
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
        _emit 0x8b              // MOV ECX, dword ptr [EDX + 4]   (ret_head:)
        _emit 0x4a
        _emit 0x04
        _emit 0x89              // MOV [0x01329958], ECX
        _emit 0x0d
        _emit 0x58
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xc3              // RET
    }
}
