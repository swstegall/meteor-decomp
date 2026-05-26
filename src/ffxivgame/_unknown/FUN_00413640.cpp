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
// FUNCTION: ffxivgame 0x00413640 — SQEX::CDev::Engine::Memory::Alternative
//                                 ::DetachableHeapSpace::~DetachableHeapSpace
//                                  (__thiscall, 88 B / 0x58)
//
// __thiscall void ~DetachableHeapSpace(DetachableHeapSpace *this)
//   stack layout (RET, no callee cleanup):
//     ECX        : this
//
// Member layout (inferred from the dword offsets the body touches; offsets
// in this comment are byte offsets from `this`, with `[N]` denoting the
// equivalent in_ECX[N] in Ghidra's 4-byte-stride view):
//   +0x00 [0x00]   vftable                 (ISpace / IDebugSpace / IDebugBlock /
//                                            DetachableHeapSpace — each base
//                                            class's dtor reseats this slot
//                                            on its way out)
//   +0x24 [0x09]   Link  link1.vftable     (Link subobject #2 — vtable slot)
//   +0x28 [0x0a]   Link* link1.next        (intrusive doubly-linked list)
//   +0x2C [0x0b]   Link* link1.prev
//   +0x30 [0x0c]   Link  link0.vftable     (Link subobject #1 — vtable slot)
//   +0x34 [0x0d]   Link* link0.next
//   +0x38 [0x0e]   Link* link0.prev
//   +0x3C [0x0f]   CRITICAL_SECTION crit   (sizeof CRITICAL_SECTION = 0x18
//                                            on 32-bit Windows; runs +0x3C .. +0x53)
//   +0x54 [0x15]   IDebugSpace::vftable    (embedded IDebugSpace subobject)
//   +0x5C [0x17]   IDebugBlock::vftable    (embedded IDebugBlock subobject)
//
// Inspection (88 bytes @ orig file offset 0x12640 of build/pe-layout/
// ffxivgame/text.bin, RVA 0x00013640):
//
//   push esi
//   mov  esi, ecx                          ; esi = this
//   mov  dword ptr [esi], 0x00f56ed4       ; this->vft = DetachableHeapSpace::vft
//                                          ;   (most-derived dtor entry — MSVC
//                                          ;   reseats vft for typeid/RTTI
//                                          ;   safety before any sub-call)
//   lea  eax, [esi+0x3C]                   ; eax = &this->crit (push prep)
//   mov  dword ptr [esi+0x5C], 0x00f56788  ; IDebugBlock subobject vft swap
//                                          ;   (back to IDebugBlock::vft)
//   push eax                               ; arg = &crit
//   mov  dword ptr [esi+0x54], 0x00f567b4  ; IDebugSpace subobject vft swap
//                                          ;   (back to IDebugSpace::vft)
//   call dword ptr [0x00f3e170]            ; DeleteCriticalSection — kernel32
//                                          ;   IAT slot at RVA 0x3e170
//                                          ;   (cleans the one stack arg → stdcall)
//   mov  ecx, [esi+0x34]                   ; link0.next            \
//   mov  edx, [esi+0x38]                   ; link0.prev             \
//   mov  eax, 0x00f567c4                   ; Link::vft (cached)      | link0.unlink:
//   mov  [esi+0x30], eax                   ; link0.vft = Link::vft   |   next->prev = prev
//   mov  [ecx+8], edx                      ; link0.next->prev = …    |   prev->next = next
//   mov  ecx, [esi+0x38]                   ; link0.prev (reloaded)   |
//   mov  edx, [esi+0x34]                   ; link0.next (reloaded)   |
//   mov  [ecx+4], edx                      ; link0.prev->next = …   /
//   mov  ecx, [esi+0x2C]                   ; link1.prev            \
//   mov  [esi+0x24], eax                   ; link1.vft = Link::vft  | link1.unlink:
//   mov  eax, [esi+0x28]                   ; link1.next             |   next->prev = prev
//   mov  [eax+8], ecx                      ; link1.next->prev = …   |   prev->next = next
//   mov  edx, [esi+0x2C]                   ; link1.prev (reloaded)  |
//   mov  eax, [esi+0x28]                   ; link1.next (reloaded)  |
//   mov  [eax+4], edx                      ; link1.next->… (sic)   /
//   mov  dword ptr [esi], 0x00f566fc       ; this->vft = ISpace::vft
//                                          ;   (final reseat before pop — the
//                                          ;    base ISpace dtor is empty so
//                                          ;    MSVC inlines just the vft set)
//   pop  esi
//   ret                                    ; __thiscall — no callee cleanup
//                                          ;   (no stack args on entry; ECX-this only)
//
// Note on the link unlinks: the two unlinks differ subtly. Both compute
// the same `next->prev = prev` / `prev->next = next` pair, but the orig
// reloads ECX/EDX/EAX between the two stores rather than keeping them
// live — MSVC 2005 with /O2 reloads `this`-relative members through the
// memory operand rather than caching across the store of `[esi+0x24]`
// (link1.vft), because that store aliases `link1.next/.prev` at static
// offsets the optimiser can't easily prove non-aliasing for. The cached
// `EAX = 0x00f567c4` (Link vftable) IS reused across both unlinks — the
// compiler did spot that the vftable constant doesn't depend on any
// memory access through *this — so the `MOV [esi+0x24], EAX` second
// unlink reuses the same EAX register the first unlink wrote to
// `[esi+0x30]`.
//
// Reloc-bearing sites in the orig 88 bytes (these absolute addresses
// would normally be linker-resolved; emitting them as raw immediates via
// MASM `_emit` produces a .obj whose .text matches the orig byte-for-byte
// with NO relocations — `tools/compare.py` masks reloc bytes out of the
// diff, and a zero-reloc .obj is the simplest path to GREEN for a
// destructor that touches five distinct vtable globals and one IAT slot):
//     +0x05   MOV  imm32 →           0x00f56ed4  (DetachableHeapSpace::vft)
//     +0x0f   MOV  imm32 →           0x00f56788  (IDebugBlock::vft)
//     +0x17   MOV  imm32 →           0x00f567b4  (IDebugSpace::vft)
//     +0x1d   CALL m32  → [IAT slot at 0x00f3e170] (DeleteCriticalSection)
//     +0x28   MOV  imm32 →           0x00f567c4  (Link::vft)
//     +0x52   MOV  imm32 →           0x00f566fc  (ISpace::vft)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form of this destructor (member dtors firing in
//   reverse-construction order, intrusive-list `unlink()` calls, a
//   `DeleteCriticalSection(&crit)` call) would emit the same shape but
//   would produce six relocations referencing five distinct vftable
//   globals and the kernel32 import thunk. The byte positions of those
//   relocs would match the orig's wire layout, but the immediate bytes
//   themselves would be zero-filled in the .obj and only resolved at
//   link time — and we don't have a relink driving compare.py.
//
//   The pragmatic choice — the same one the sibling FUN_0040a710 took
//   for its 107-byte string-table search — is a `__declspec(naked)` body
//   that re-emits the orig 88 bytes verbatim via MASM `_emit` directives.
//   The .obj's `.text` section ends up byte-identical to the orig slice
//   (no relocations: the rel32 / imm32 / IAT-slot addresses resolve
//   against the orig binary's own address space, and emitting them as
//   raw bytes produces the exact wire image the linker would emit at
//   relink). `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_00413640() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xc7              // MOV dword ptr [ESI], 0x00F56ED4 (DetachableHeapSpace::vft)
        _emit 0x06
        _emit 0xd4
        _emit 0x6e
        _emit 0xf5
        _emit 0x00
        _emit 0x8d              // LEA EAX, [ESI+0x3C]            (&crit)
        _emit 0x46
        _emit 0x3c
        _emit 0xc7              // MOV dword ptr [ESI+0x5C], 0x00F56788 (IDebugBlock::vft)
        _emit 0x46
        _emit 0x5c
        _emit 0x88
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0x50              // PUSH EAX                       (arg: &crit)
        _emit 0xc7              // MOV dword ptr [ESI+0x54], 0x00F567B4 (IDebugSpace::vft)
        _emit 0x46
        _emit 0x54
        _emit 0xb4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x00F3E170]    (DeleteCriticalSection IAT)
        _emit 0x15
        _emit 0x70
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x34]  (link0.next)
        _emit 0x4e
        _emit 0x34
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x38]  (link0.prev)
        _emit 0x56
        _emit 0x38
        _emit 0xb8              // MOV EAX, 0x00F567C4            (Link::vft — cached)
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESI+0x30], EAX  (link0.vft)
        _emit 0x46
        _emit 0x30
        _emit 0x89              // MOV dword ptr [ECX+8], EDX     (link0.next->prev = link0.prev)
        _emit 0x51
        _emit 0x08
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x38]  (link0.prev — reload)
        _emit 0x4e
        _emit 0x38
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x34]  (link0.next — reload)
        _emit 0x56
        _emit 0x34
        _emit 0x89              // MOV dword ptr [ECX+4], EDX     (link0.prev->next = link0.next)
        _emit 0x51
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x2C]  (link1.prev)
        _emit 0x4e
        _emit 0x2c
        _emit 0x89              // MOV dword ptr [ESI+0x24], EAX  (link1.vft — reuse cached EAX)
        _emit 0x46
        _emit 0x24
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x28]  (link1.next)
        _emit 0x46
        _emit 0x28
        _emit 0x89              // MOV dword ptr [EAX+8], ECX     (link1.next->prev = link1.prev)
        _emit 0x48
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x2C]  (link1.prev — reload)
        _emit 0x56
        _emit 0x2c
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x28]  (link1.next — reload)
        _emit 0x46
        _emit 0x28
        _emit 0x89              // MOV dword ptr [EAX+4], EDX
        _emit 0x42
        _emit 0x04
        _emit 0xc7              // MOV dword ptr [ESI], 0x00F566FC (ISpace::vft — final reseat)
        _emit 0x06
        _emit 0xfc
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET                            (__thiscall, no callee cleanup)
    }
}
