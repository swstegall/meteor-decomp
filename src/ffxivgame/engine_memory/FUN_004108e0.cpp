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
// FUNCTION: ffxivgame 0x004108e0 — FixedAllocator::Allocate
//                                  __thiscall, no stack args (177 bytes / 0xb1)
//
// ECX = this (= sentinel node of the circular free-list).
// Returns: int* (the allocated node) or NULL on failure (EAX).
//
// The object at ECX is a sentinel node of a circular doubly-linked list of
// free blocks.  Each node has layout:
//   [node +  0]  Node*  prev
//   [node +  4]  Node*  next
// The sentinel's next field is at [ECX+4]; when next == this the list is
// empty.
//
// Behaviour:
//   node = *(ECX + 4)          ; first free block (or sentinel if empty)
//   if (node == this) {        ; empty — panic path
//       char buf[0x400];
//       buf[0x3fe] = 0;        ; pre-null before snprintf_s
//       _snprintf_s(buf, 0x400, 0x3fe,
//                  "Memory allocation fail, for Fixed Allocator.");
//       _strcat_s(buf, 0x400, "\n");
//       (*g_log_fn)(buf, 3);   ; call through IAT function pointer
//       if (!(g_init_flag & 1)) {
//           g_init_flag |= 1;
//           g_assert_fn = FUN_0040f8e0;
//       }
//       (*g_assert_fn)("false", &g_cond_str,
//                      "c:\\work\\...\\FixedAllocator.h", 0x3c,
//                      "SQEX::CDev::Engine::Memory::Alternative::FixedAllocator::Allocate");
//       return NULL;
//   }
//   node->prev->next = node->next;  ; unlink from free list
//   node->next->prev = node->prev;
//   return node;
//
// Reconstruction: naked-asm byte passthrough.
//
// The root cause of non-reproducibility from C++:
//   1.  MSVC hoists the `MOV EAX,[ECX+4]` load before `SUB ESP,0x400`.
//   2.  MSVC batches the three __cdecl arg stacks (snprintf_s, strcat_s,
//       g_log_fn) and emits a single `ADD ESP,0x24` cleanup.
//   3.  Absolute data-section addresses (g_init_flag at 0x01323910,
//       g_assert_fn at 0x0132390c, g_log_fn at 0x012651b4) appear as
//       immediate operands and are not reproducible as COFF relocations
//       from standalone C++ compilation.
//
// Globals / constants referenced (all absolute VAs in the 1.23b image):
//   0x012651b4  — IAT slot holding the log function pointer
//   0x01323910  — one-time-init flag byte (low bit = initialised)
//   0x0132390c  — assert-handler function pointer slot
//   0x0040f8e0  — FUN_0040f8e0 (the assert handler installed on first call)
//   0x00f56944  — "Memory allocation fail, for Fixed Allocator."
//   0x00f54d98  — "\n"
//   0x00f56900  — "false"
//   0x00f568b0  — filename string
//   0x00f54d48  — condition/expression string (assert arg 2)
//   0x00f56510  — function name string

extern "C" __declspec(naked) void FUN_004108e0()
{
    __asm {
        // 000108e0:  8b 41 04               MOV EAX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 000108e3:  81 ec 00 04 00 00      SUB ESP,0x400
        _emit 0x81
        _emit 0xec
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 000108e9:  3b c1                  CMP EAX,ECX
        _emit 0x3b
        _emit 0xc1
        // 000108eb:  0f 85 8a 00 00 00      JNZ +0x8a (→ success path)
        _emit 0x0f
        _emit 0x85
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // ---- error path (node == this, free list empty) ----------------
        // 000108f1:  68 44 69 f5 00         PUSH 0xf56944  (format string)
        _emit 0x68
        _emit 0x44
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        // 000108f6:  68 fe 03 00 00         PUSH 0x3fe  (count)
        _emit 0x68
        _emit 0xfe
        _emit 0x03
        _emit 0x00
        _emit 0x00
        // 000108fb:  8d 44 24 08            LEA EAX,[ESP+0x8]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 000108ff:  68 00 04 00 00         PUSH 0x400  (size)
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00010904:  50                     PUSH EAX  (buffer)
        _emit 0x50
        // 00010905:  c6 84 24 0e 04 00 00 00   MOV byte ptr [ESP+0x40e],0x0
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x0e
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001090d:  e8 8d 46 5c 00         CALL __snprintf_s (REL32)
        _emit 0xe8
        _emit 0x8d
        _emit 0x46
        _emit 0x5c
        _emit 0x00
        // 00010912:  68 98 4d f5 00         PUSH 0xf54d98  ("\n")
        _emit 0x68
        _emit 0x98
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 00010917:  8d 4c 24 14            LEA ECX,[ESP+0x14]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0001091b:  68 00 04 00 00         PUSH 0x400
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00010920:  51                     PUSH ECX
        _emit 0x51
        // 00010921:  e8 8e 42 5c 00         CALL _strcat_s (REL32)
        _emit 0xe8
        _emit 0x8e
        _emit 0x42
        _emit 0x5c
        _emit 0x00
        // 00010926:  8d 54 24 1c            LEA EDX,[ESP+0x1c]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 0001092a:  6a 03                  PUSH 0x3
        _emit 0x6a
        _emit 0x03
        // 0001092c:  52                     PUSH EDX
        _emit 0x52
        // 0001092d:  ff 15 b4 51 26 01      CALL dword ptr [0x012651b4]
        _emit 0xff
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        // 00010933:  b8 01 00 00 00         MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00010938:  83 c4 24               ADD ESP,0x24  (clean up 9 pushed DWORDs)
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        // 0001093b:  84 05 10 39 32 01      TEST byte ptr [0x01323910],AL
        _emit 0x84
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00010941:  75 10                  JNZ +0x10  (already initialised)
        _emit 0x75
        _emit 0x10
        // 00010943:  09 05 10 39 32 01      OR dword ptr [0x01323910],EAX
        _emit 0x09
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00010949:  c7 05 0c 39 32 01 e0 f8 40 00  MOV dword ptr [0x0132390c],0x0040f8e0
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xe0
        _emit 0xf8
        _emit 0x40
        _emit 0x00
        // 00010953:  68 00 69 f5 00         PUSH 0xf56900  ("false")
        _emit 0x68
        _emit 0x00
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        // 00010958:  6a 3c                  PUSH 0x3c  (line number 60)
        _emit 0x6a
        _emit 0x3c
        // 0001095a:  68 b0 68 f5 00         PUSH 0xf568b0  (filename)
        _emit 0x68
        _emit 0xb0
        _emit 0x68
        _emit 0xf5
        _emit 0x00
        // 0001095f:  68 48 4d f5 00         PUSH 0xf54d48  (cond expression)
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 00010964:  68 10 65 f5 00         PUSH 0xf56510  (function name)
        _emit 0x68
        _emit 0x10
        _emit 0x65
        _emit 0xf5
        _emit 0x00
        // 00010969:  ff 15 0c 39 32 01      CALL dword ptr [0x0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0001096f:  83 c4 14               ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00010972:  33 c0                  XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 00010974:  81 c4 00 04 00 00      ADD ESP,0x400
        _emit 0x81
        _emit 0xc4
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0001097a:  c3                     RET
        _emit 0xc3
        // ---- success path (node != this, list has free blocks) ---------
        // 0001097b:  8b 08                  MOV ECX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x08
        // 0001097d:  8b 50 04               MOV EDX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00010980:  89 51 04               MOV dword ptr [ECX+0x4],EDX  (prev->next = node->next)
        _emit 0x89
        _emit 0x51
        _emit 0x04
        // 00010983:  8b 48 04               MOV ECX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        // 00010986:  8b 10                  MOV EDX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x10
        // 00010988:  89 11                  MOV dword ptr [ECX],EDX  (next->prev = node->prev)
        _emit 0x89
        _emit 0x11
        // 0001098a:  81 c4 00 04 00 00      ADD ESP,0x400
        _emit 0x81
        _emit 0xc4
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00010990:  c3                     RET  (return node in EAX)
        _emit 0xc3
    }
}
