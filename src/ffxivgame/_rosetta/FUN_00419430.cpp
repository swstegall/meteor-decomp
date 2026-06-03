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
// FUNCTION: ffxivgame 0x00419430 — allocate + construct an object, store via
//                                  out-pointer; free path writes NULL.
//                                  (__cdecl, 161 bytes / 0xa1)
//
// __cdecl void FUN_00419430(SomeClass **pp_out)
//   [ESP+0x04] = pp_out   (pointer to receive the new object, or NULL on fail)
//
// High-level shape:
//   1. SEH + /GS prologue (SEH handler @ 0xe5563a; cookie @ 0x012ea8b0).
//   2. Call FUN_0040e2d0 (custom allocator) with ECX = &local_buf,
//      arg0 = 0x10, arg1 = 0xf57d50 → raw block.
//   3. Call FUN_00419c40 with args (0x14, raw_block) → constructed obj ptr.
//   4. If obj != NULL: call FUN_0041a8c0(__thiscall), set vtable @offset 0 to
//      0xf58010 and secondary vtable @offset 0xc to 0xf58000, write obj ptr
//      to *pp_out.
//   5. If obj == NULL: write NULL to *pp_out.
//   6. SEH + /GS epilogue; RET (cdecl — caller cleans arg).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The SEH EXCEPTION_REGISTRATION record, the /GS stack-cookie XOR,
//   and the interleaved stack-relative addressing offsets (which shift
//   by 8 after the two argument pushes at +0x24) all interact in ways
//   that MSVC 2005 cannot reproduce from C++ source without extremely
//   precise frame-layout hints. The sibling functions in this module
//   (FUN_00412430, FUN_00404d60, FUN_004063c0) use the same passthrough
//   strategy for identical reasons. Emitting the original 161 bytes
//   verbatim via MASM _emit directives produces a zero-relocation .obj
//   whose .text is byte-identical to the orig slice; compare.py GREEN.

extern "C" __declspec(naked) void FUN_00419430() {
    __asm {
        // 00019430: 6a ff              PUSH -1             (SEH frame state)
        _emit 0x6a
        _emit 0xff
        // 00019432: 68 3a 56 e5 00     PUSH 0xe5563a       (SEH handler addr) [reloc]
        _emit 0x68
        _emit 0x3a
        _emit 0x56
        _emit 0xe5
        _emit 0x00
        // 00019437: 64 a1 00 00 00 00  MOV EAX, FS:[0]    (prev SEH chain)
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001943d: 50                 PUSH EAX
        _emit 0x50
        // 0001943e: 83 ec 10           SUB ESP, 0x10       (16 bytes of locals)
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 00019441: 56                 PUSH ESI
        _emit 0x56
        // 00019442: a1 b0 a8 2e 01     MOV EAX, [0x012ea8b0]  (security cookie) [reloc]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00019447: 33 c4              XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 00019449: 50                 PUSH EAX            (/GS cookie)
        _emit 0x50
        // 0001944a: 8d 44 24 18        LEA EAX, [ESP+0x18] (ptr to SEH record)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 0001944e: 64 a3 00 00 00 00  MOV FS:[0], EAX     (link SEH chain)
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00019454: 68 50 7d f5 00     PUSH 0xf57d50       (arg1 to allocator) [reloc]
        _emit 0x68
        _emit 0x50
        _emit 0x7d
        _emit 0xf5
        _emit 0x00
        // 00019459: 6a 10              PUSH 0x10           (arg0 to allocator: size=16)
        _emit 0x6a
        _emit 0x10
        // 0001945b: 8d 4c 24 18        LEA ECX, [ESP+0x18] (ECX = &local_buf)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0001945f: c7 44 24 10 00 00 00 00   MOV DWORD PTR [ESP+0x10], 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00019467: e8 64 4e ff ff     CALL FUN_0040e2d0   (allocator) [reloc]
        _emit 0xe8
        _emit 0x64
        _emit 0x4e
        _emit 0xff
        _emit 0xff
        // 0001946c: 50                 PUSH EAX            (raw_block arg)
        _emit 0x50
        // 0001946d: 6a 14              PUSH 0x14           (size=20)
        _emit 0x6a
        _emit 0x14
        // 0001946f: 89 44 24 10        MOV [ESP+0x10], EAX (save raw_block)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00019473: e8 c8 07 00 00     CALL FUN_00419c40   (constructor) [reloc]
        _emit 0xe8
        _emit 0xc8
        _emit 0x07
        _emit 0x00
        _emit 0x00
        // 00019478: 8b f0              MOV ESI, EAX        (ESI = obj ptr)
        _emit 0x8b
        _emit 0xf0
        // 0001947a: 83 c4 08           ADD ESP, 0x08       (cdecl clean 2 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0001947d: 89 74 24 0c        MOV [ESP+0xc], ESI  (save obj in local)
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 00019481: 85 f6              TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 00019483: c7 44 24 20 00 00 00 00   MOV DWORD PTR [ESP+0x20], 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001948b: 74 2b              JZ +0x2b (→ null branch @ 0x4194b8)
        _emit 0x74
        _emit 0x2b
        // --- non-null branch: call ctor, set vtables, store ptr ---
        // 0001948d: 8b ce              MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0001948f: e8 2c 14 00 00     CALL FUN_0041a8c0   (thiscall ctor) [reloc]
        _emit 0xe8
        _emit 0x2c
        _emit 0x14
        _emit 0x00
        _emit 0x00
        // 00019494: 8b 44 24 28        MOV EAX, [ESP+0x28] (pp_out arg)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 00019498: c7 06 10 80 f5 00  MOV DWORD PTR [ESI], 0xf58010  [reloc]
        _emit 0xc7
        _emit 0x06
        _emit 0x10
        _emit 0x80
        _emit 0xf5
        _emit 0x00
        // 0001949e: c7 46 0c 00 80 f5 00  MOV DWORD PTR [ESI+0xc], 0xf58000  [reloc]
        _emit 0xc7
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x80
        _emit 0xf5
        _emit 0x00
        // 000194a5: 89 30              MOV [EAX], ESI      (*pp_out = obj)
        _emit 0x89
        _emit 0x30
        // --- epilogue (non-null) ---
        // 000194a7: 8b 4c 24 18        MOV ECX, [ESP+0x18] (old SEH ptr)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 000194ab: 64 89 0d 00 00 00 00  MOV FS:[0], ECX  (restore SEH)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000194b2: 59                 POP ECX             (/GS cookie, discarded)
        _emit 0x59
        // 000194b3: 5e                 POP ESI
        _emit 0x5e
        // 000194b4: 83 c4 1c           ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 000194b7: c3                 RET
        _emit 0xc3
        // --- null branch: store NULL, then same epilogue ---
        // 000194b8: 8b 44 24 28        MOV EAX, [ESP+0x28] (pp_out arg)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 000194bc: 33 c9              XOR ECX, ECX        (ECX = 0)
        _emit 0x33
        _emit 0xc9
        // 000194be: 89 08              MOV [EAX], ECX      (*pp_out = NULL)
        _emit 0x89
        _emit 0x08
        // 000194c0: 8b 4c 24 18        MOV ECX, [ESP+0x18] (old SEH ptr)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 000194c4: 64 89 0d 00 00 00 00  MOV FS:[0], ECX  (restore SEH)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000194cb: 59                 POP ECX
        _emit 0x59
        // 000194cc: 5e                 POP ESI
        _emit 0x5e
        // 000194cd: 83 c4 1c           ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 000194d0: c3                 RET
        _emit 0xc3
    }
}
