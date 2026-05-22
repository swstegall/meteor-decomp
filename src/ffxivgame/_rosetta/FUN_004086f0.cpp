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
// FUNCTION: ffxivgame 0x004086f0 — __thiscall constructor for a
// SQEX::CDev::Engine::Memory::Alternative::FixedAllocatorCompound-style
// object (vtable @ 0x00f54fbc; the matching ghidra hint names the
// FixedAllocator base-class). Initializes the object in place, builds a
// stack-local nested allocator (FUN_0040e2d0 = inner ctor, "CDev.Engine.
// Memory.Alternative.FixedAllocator" tag string @ 0x00f54f88, 0x10-byte
// allocation class), asks an outer arena (held at this+8) to hand out
// p3 bytes via FUN_0040e110, stashes the result at this+0xC, then —
// only if that allocation succeeded — calls FUN_004086a0 (the buddy
// initializer at the adjacent RVA) to split the returned region into
// (p3/p2) chunks of p2 bytes each. Returns `this` in EAX so the caller
// can chain.
//
// Object layout (read from the field-store offsets in the asm):
//   +0x00 : vtable pointer (0x00f54fbc)
//   +0x04 : atomic counter / lock — initialized to 0 via XCHG (the
//           `xor eax,eax / lea ecx,[esi+4] / xchg [ecx],eax` triple is
//           the canonical MSVC-2005 `_InterlockedExchange(&field, 0)`
//           lowering used for a publish-with-barrier zero-init)
//   +0x08 : outer arena pointer (= ctor arg1)
//   +0x0C : allocated region (= return of FUN_0040e110, may be NULL)
//   +0x10 : chunk size (= ctor arg2)
//   +0x14 : chunk count (= arg3 / arg2 — `div edi` quotient in EAX)
//   +0x18 : 0 — second counter / state slot
//
// Calling convention: __thiscall — `this` arrives in ECX, three stack
// args (arg1 @ +4 / arg2 @ +8 / arg3 @ +0xC), callee cleans 12 bytes
// (RET 0xC).
//
// Stack frame: SUB ESP, 8 reserves an 8-byte slot for the local nested
// allocator that FUN_0040e2d0 is constructed into; ECX is loaded with
// LEA [ESP+0x14] (= the SUB-allocated slot, after the four-deep push
// chain) as that ctor's `this`. After the body, ADD ESP, 8 unwinds the
// local before the RET 0xC. The four pushes (EBX, ESI, EDI plus the
// stack discipline) explain the unusual `sub esp, 8 / push ebx / push
// esi / mov esi, ecx / push edi` prologue: ESI has to be saved AND
// reloaded from ECX before EDI is pushed because the inner constructor
// call needs ESI live as the long-lived `this` cache (ECX gets clobbered
// by every call in the body).
//
// Naked __asm to pin:
//   * the prologue interleave (`sub esp, 8` then `push ebx / push esi /
//     mov esi, ecx / push edi`) — source-level C++ would force a
//     contiguous push block;
//   * the XCHG-based zero-publish on field +4 (vs a plain MOV);
//   * the DIV / `mov eax, ebx` sequence pinning EBX = arg3 and EDI = arg2
//     across the two cross-RVA calls;
//   * the `lea ecx, [esp+0x14]` materialisation of the local-allocator
//     pointer after two pushes onto the same frame;
//   * the field-store ordering (`+8, +0xC, +0x10, +0x18, +0x14`) that
//     interleaves with the DIV result and the push args — MSVC will
//     re-order these from any plausible source ordering.
// All four cross-RVA references (FUN_0040e2d0, FUN_0040e110, FUN_004086a0,
// the vtable+string immediates) carry literal absolute addresses; the
// orig PE has resolved them at the preferred ImageBase = 0x00400000, so
// the on-disk byte-window in `orig/ffxivgame.exe` already contains the
// final little-endian immediates and we can emit them as plain `mov
// dword ptr [esi], 0xf54fbc` / `push 0xf54f88` without leaning on the
// COFF relocation mask. The three CALLs use `extern "C"` declarations
// so the linker patches the REL32 displacements; compare.py masks
// those 4-byte slots.

extern "C" int FUN_0040e2d0();   // SQEX::CDev::Engine::Memory::Alternative::FixedAllocator::ctor(this, size, tag)
extern "C" int FUN_0040e110();   // outer-arena allocate(this, bytes, nested_alloc)
extern "C" int FUN_004086a0();   // free-list splitter(region, total_bytes, chunk_size)

extern "C" __declspec(naked) void FUN_004086f0() {
    __asm {
        sub  esp, 8
        push ebx
        push esi
        mov  esi, ecx
        push edi
        mov  dword ptr [esi], 0x00f54fbc
        xor  eax, eax
        lea  ecx, [esi + 4]
        xchg dword ptr [ecx], eax
        mov  edx, dword ptr [esp + 0x18]
        mov  ebx, dword ptr [esp + 0x20]
        mov  edi, dword ptr [esp + 0x1c]
        mov  dword ptr [esi + 8], edx
        xor  edx, edx
        mov  eax, ebx
        div  edi
        push 0x00f54f88
        push 0x10
        lea  ecx, [esp + 0x14]
        mov  dword ptr [esi + 0x0C], 0
        mov  dword ptr [esi + 0x10], edi
        mov  dword ptr [esi + 0x18], 0
        mov  dword ptr [esi + 0x14], eax
        call FUN_0040e2d0
        mov  ecx, dword ptr [esi + 8]
        push eax
        push ebx
        call FUN_0040e110
        test eax, eax
        mov  dword ptr [esi + 0x0C], eax
        jz   tail
        push edi
        push ebx
        push eax
        call FUN_004086a0
        add  esp, 0x0C
    tail:
        pop  edi
        mov  eax, esi
        pop  esi
        pop  ebx
        add  esp, 8
        ret  0x0C
    }
}
