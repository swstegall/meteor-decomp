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
// FUNCTION: ffxivgame 0x00014be0 — DefaultMemoryAllocator::Free (vtable slot 3)
//                                  (14 bytes)
//
// Vtable role: slot 3 of
//   SQEX::CDev::Engine::Phieg::Base::Memory::DefaultMemoryAllocator
// The class's four vtable slots cluster at:
//   slot 0 (0x9580)  : destructor
//   slot 1 (0x14ba0) : Alloc(size)                → __aligned_malloc(size, 0x10)
//   slot 2 (0x14bc0) : AllocAligned(align, size)  → __aligned_malloc(size, align)
//   slot 3 (0x14be0) : Free(ptr)                  → FUN_009d56fd(ptr)   ← THIS FUNCTION
//
// FUN_009d56fd is the binary's canonical `free` helper (a thin
// __cdecl wrapper around `_aligned_free` — see decomp-notes/types/
// ffxivgame/0x00014370.md). Slot 3 simply forwards its caller's
// `ptr` argument to it and returns.
//
// Asm (14 bytes @ orig RVA 0x00014be0):
//   8b 44 24 04     MOV EAX, dword ptr [ESP+0x4]    ; load ptr arg
//   50              PUSH EAX                         ; push as cdecl arg
//   e8 13 0b 5c 00  CALL FUN_009d56fd                ; rel32 (reloc masked)
//   59              POP ECX                          ; cleanup (cheaper than ADD ESP,4)
//   c2 04 00        RET 0x4                          ; __thiscall, 1 stack arg
//
// Calling convention: __thiscall (ECX = this, unused here; RET 0x4
// cleans one stack arg). MSVC chose `MOV EAX,[ESP+4] / PUSH EAX` over
// `PUSH [ESP+4]` and `POP ECX` over `ADD ESP,4` — net 14 bytes vs.
// the 15-byte `PUSH [mem] / CALL / ADD ESP,4 / RET 4` form.
//
// Reconstruction strategy — naked __asm with MASM `call FUN_009d56fd`
// so the COFF assembler emits a proper IMAGE_REL_I386_REL32 reloc on
// the call displacement; compare.py masks those 4 bytes so the diff
// is byte-exact regardless of link-time placement.

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
// The real implementation is the MSVC __declspec(naked) + __asm block
// below, which clang cannot parse. Production builds always use
// cl.exe (MSVC 2005).
extern "C" void FUN_00414be0() { __builtin_unreachable(); }
#else

extern "C" void FUN_009d56fd(void *p);

extern "C" __declspec(naked) void FUN_00414be0() {
    __asm {
        mov  eax, dword ptr [esp + 4]   // 8b 44 24 04
        push eax                         // 50
        call FUN_009d56fd                // e8 ?? ?? ?? ??  (REL32 — reloc masked)
        pop  ecx                         // 59
        ret  4                           // c2 04 00
    }
}

#endif
