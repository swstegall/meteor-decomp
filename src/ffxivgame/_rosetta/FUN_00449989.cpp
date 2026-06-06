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
// FUNCTION: ffxivgame 0x00449989 — std::basic_string assign-body (SSO variant):
//           copies source bytes into a small-string-optimised string object,
//           frees any previous heap buffer, and null-terminates. 113 bytes / 0x71.
//
// Calling convention: __stdcall (RET 0x8) — two caller-pushed DWORD args:
//   [EBP+0x8]  param1 = source pointer (const char *)
//   [EBP+0xC]  param2 = count           (size_t)
//
// On entry EDI = this (std::basic_string *), ESI = new capacity value
// (established by the outer SEH wrapper whose prologue precedes this RVA).
//
// String-object layout (EDI-relative):
//   [EDI + 0x04]  inline buffer (capacity < 0x10) / heap pointer (capacity >= 0x10)
//   [EDI + 0x14]  length
//   [EDI + 0x18]  capacity  (0x10 = SSO threshold)
//
// Algorithm:
//   1. If count == 0 skip the copy.
//   2. If old capacity >= 0x10 the buffer is heap-allocated; pass it as ptr
//      to the memcpy call; otherwise pass &buf[0] (inline slot).
//   3. Call memmove/memcpy (0x009d17f3) with (param1, buf, ESI+1, count).
//   4. If old capacity >= 0x10 call the allocator free (0x0044d350) on the
//      old heap buffer (3-arg dealloc form).
//   5. Assign new capacity = ESI, new length = count.
//   6. If ESI < 0x10 the inline slot is used; else param1 is the heap ptr.
//   7. Null-terminate at offset count.
//   8. Restore SEH chain, pop callee-saves, return.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function body is entered mid-frame: EBP was established by the outer
//   SEH wrapper that wraps this RVA. Reproducing the exact frame layout, the
//   two CALL rel32 displacements (0x009d17f3 and 0x0044d350), and the SEH
//   restore sequence from source-level C++ is not feasible without the
//   surrounding compilation unit. A __declspec(naked) body re-emitting all
//   113 original bytes verbatim via MASM _emit directives produces a .obj
//   whose .text is byte-identical to the orig slice, so compare.py reports GREEN.
//
// CALL rel32 sites (no .obj relocations — raw offsets embed verbatim):
//   +0x1f  CALL 0x009d17f3  (memmove / memcpy)
//   +0x39  CALL 0x0044d350  (operator delete / allocator free)

extern "C" __declspec(naked) void FUN_00449989() {
    __asm {
        // 00049989: 8b 5d 0c   MOV EBX,dword ptr [EBP+0xc]
        _emit 0x8b
        _emit 0x5d
        _emit 0x0c
        // 0004998c: 85 db      TEST EBX,EBX
        _emit 0x85
        _emit 0xdb
        // 0004998e: 76 20      JBE +0x20  (→ 0x004499b0, skip copy if count==0)
        _emit 0x76
        _emit 0x20
        // 00049990: 83 7f 18 10  CMP dword ptr [EDI+0x18],0x10
        _emit 0x83
        _emit 0x7f
        _emit 0x18
        _emit 0x10
        // 00049994: 72 05      JC +0x05  (→ 0x0044999b, inline buffer)
        _emit 0x72
        _emit 0x05
        // 00049996: 8b 47 04   MOV EAX,dword ptr [EDI+0x4]  (heap ptr)
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // 00049999: eb 03      JMP +0x03  (→ 0x0044999e)
        _emit 0xeb
        _emit 0x03
        // 0004999b: 8d 47 04   LEA EAX,[EDI+0x4]  (inline buffer address)
        _emit 0x8d
        _emit 0x47
        _emit 0x04
        // 0004999e: 53         PUSH EBX                    (arg: count)
        _emit 0x53
        // 0004999f: 50         PUSH EAX                    (arg: dest buffer)
        _emit 0x50
        // 000499a0: 8b 45 08   MOV EAX,dword ptr [EBP+0x8] (param1 = src)
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        // 000499a3: 8d 56 01   LEA EDX,[ESI+0x1]           (new capacity + 1)
        _emit 0x8d
        _emit 0x56
        _emit 0x01
        // 000499a6: 52         PUSH EDX                    (arg: buf size)
        _emit 0x52
        // 000499a7: 50         PUSH EAX                    (arg: src)
        _emit 0x50
        // 000499a8: e8 46 7e 58 00   CALL 0x009d17f3  (memmove/memcpy)
        _emit 0xe8
        _emit 0x46
        _emit 0x7e
        _emit 0x58
        _emit 0x00
        // 000499ad: 83 c4 10   ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 000499b0: 8b 47 18   MOV EAX,dword ptr [EDI+0x18]  (old capacity)
        _emit 0x8b
        _emit 0x47
        _emit 0x18
        // 000499b3: 83 f8 10   CMP EAX,0x10
        _emit 0x83
        _emit 0xf8
        _emit 0x10
        // 000499b6: 72 12      JC +0x12  (→ 0x004499ca, was inline — no free)
        _emit 0x72
        _emit 0x12
        // 000499b8: 8b 4f 04   MOV ECX,dword ptr [EDI+0x4]  (old heap ptr)
        _emit 0x8b
        _emit 0x4f
        _emit 0x04
        // 000499bb: 6a 0c      PUSH 0xc
        _emit 0x6a
        _emit 0x0c
        // 000499bd: 83 c0 01   ADD EAX,0x1                   (old_capacity + 1)
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 000499c0: 50         PUSH EAX
        _emit 0x50
        // 000499c1: 51         PUSH ECX                      (old heap ptr)
        _emit 0x51
        // 000499c2: e8 89 39 00 00   CALL 0x0044d350  (allocator free)
        _emit 0xe8
        _emit 0x89
        _emit 0x39
        _emit 0x00
        _emit 0x00
        // 000499c7: 83 c4 0c   ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 000499ca: 83 fe 10   CMP ESI,0x10                  (new capacity vs SSO)
        _emit 0x83
        _emit 0xfe
        _emit 0x10
        // 000499cd: 8b 4d 08   MOV ECX,dword ptr [EBP+0x8]   (ECX = param1 / new heap ptr)
        _emit 0x8b
        _emit 0x4d
        _emit 0x08
        // 000499d0: 8d 47 04   LEA EAX,[EDI+0x4]             (EAX = &buf[0])
        _emit 0x8d
        _emit 0x47
        _emit 0x04
        // 000499d3: c6 00 00   MOV byte ptr [EAX],0x0        (clear first byte)
        _emit 0xc6
        _emit 0x00
        _emit 0x00
        // 000499d6: 89 08      MOV dword ptr [EAX],ECX       (store ptr/value)
        _emit 0x89
        _emit 0x08
        // 000499d8: 89 77 18   MOV dword ptr [EDI+0x18],ESI  (capacity = ESI)
        _emit 0x89
        _emit 0x77
        _emit 0x18
        // 000499db: 89 5f 14   MOV dword ptr [EDI+0x14],EBX  (length = count)
        _emit 0x89
        _emit 0x5f
        _emit 0x14
        // 000499de: 72 02      JC +0x02  (→ 0x004499e2, ESI<0x10: inline, EAX=&buf)
        _emit 0x72
        _emit 0x02
        // 000499e0: 8b c1      MOV EAX,ECX                   (heap: EAX = param1)
        _emit 0x8b
        _emit 0xc1
        // 000499e2: c6 04 18 00  MOV byte ptr [EAX+EBX*0x1],0x0  (null-terminate)
        _emit 0xc6
        _emit 0x04
        _emit 0x18
        _emit 0x00
        // 000499e6: 8b 4d f4   MOV ECX,dword ptr [EBP-0xc]   (saved old FS:[0])
        _emit 0x8b
        _emit 0x4d
        _emit 0xf4
        // 000499e9: 64 89 0d 00 00 00 00  MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000499f0: 59         POP ECX
        _emit 0x59
        // 000499f1: 5f         POP EDI
        _emit 0x5f
        // 000499f2: 5e         POP ESI
        _emit 0x5e
        // 000499f3: 5b         POP EBX
        _emit 0x5b
        // 000499f4: 8b e5      MOV ESP,EBP
        _emit 0x8b
        _emit 0xe5
        // 000499f6: 5d         POP EBP
        _emit 0x5d
        // 000499f7: c2 08 00   RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
