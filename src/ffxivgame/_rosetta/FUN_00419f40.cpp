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
// FUNCTION: ffxivgame 0x00019f40 — __stdcall allocation wrapper for a named
//                                   memory space (56 B / 0x38)
//
// __stdcall void* FUN_00419f40(unsigned int size)
//   [ESP+0x04] : size  — number of bytes to allocate
//
// Source shape:
//   Builds a local 8-byte SpaceDesc { int value; const char* name; }
//   on the stack using FUN_0040e2d0 as a thiscall setter, then calls
//   the global allocator (lazy-initialised via FUN_0040e500) to perform
//   the actual allocation via FUN_0040e110.
//
//   void* FUN_00419f40(unsigned int size) {
//       SpaceDesc local;               // 8-byte local: [ESP+0]..[ESP+7]
//       FUN_0040e2d0(&local, 0x10, (void*)0xf57b04);
//       void *allocator = *(void**)0x01327fc0;
//       if (!allocator)
//           allocator = FUN_0040e500();
//       return FUN_0040e110(allocator, size, &local);
//   }
//
// Frame layout (entry → epilogue):
//   SUB ESP, 0x8          : allocate 8-byte local SpaceDesc
//   PUSH ESI              : callee-save
//   [ESP+0x10]            : size (arg1 — shifted by SUB+PUSH = 12 bytes)
//   ESI                   : holds &local throughout (returned by e2d0 in EAX)
//
// Calling convention: __stdcall; callee cleans 1 DWORD arg via RET 0x4.
// Callee-saves used: ESI only.
//
// Call targets (REL32 — masked by tools/compare.py):
//   FUN_0040e2d0  __thiscall 2-field setter  (RET 0x8)
//   FUN_0040e500  __cdecl    global-space init singleton
//   FUN_0040e110  __thiscall allocator alloc() (RET 0x8)
//
// Absolute immediates baked into the binary's address space:
//   0x01327fc0  DA_global_allocator_ptr  (global void* at that VA)
//   0xf57b04    DA_space_name_str        (string literal VA)
//
// Reconstruction strategy — naked asm:
//   The function body is trivially expressible but the absolute VA immediates
//   (0x01327fc0, 0xf57b04) are baked as raw immediates in the orig binary and
//   are reproduced verbatim here. REL32 call targets are masked out by
//   tools/compare.py so they need not be byte-exact.

extern "C" void FUN_0040e2d0(void);
extern "C" void FUN_0040e500(void);
extern "C" void FUN_0040e110(void);

extern "C" __declspec(naked) void FUN_00419f40() {
    __asm {
        sub     esp, 8
        push    esi
        push    0xf57b04
        push    0x10
        lea     ecx, [esp + 0xc]
        call    FUN_0040e2d0
        mov     esi, eax
        mov     eax, dword ptr [0x01327fc0]
        test    eax, eax
        jnz     skip
        call    FUN_0040e500
    skip:
        mov     ecx, dword ptr [esp + 0x10]
        push    esi
        push    ecx
        mov     ecx, eax
        call    FUN_0040e110
        pop     esi
        add     esp, 8
        ret     4
    }
}
