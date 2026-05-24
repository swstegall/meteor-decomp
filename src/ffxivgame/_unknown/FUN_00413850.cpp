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
// FUNCTION: ffxivgame 0x00013850 — __thiscall constructor for
//                                  SQEX::CDev::Engine::Memory::Alternative::
//                                  DetachableHeapSpace (133 B, ret 0x1c).
//
// Stashes 7 stack arguments into the first 7 dword slots after the
// vtable, captures &_memcpy as a callable copy-function pointer,
// initialises two embedded Link sentinel-lists (each pre-pointing to
// itself for both prev/next), brings up the object's CRITICAL_SECTION
// lock, then stamps the trailing DebugDetachableHeapSpace /
// DebugDetachableHeapBlock vtable slots with their nulled inner
// pointer pairs.
//
// Layout (this aka esi, ret 0x1c → 7 dword stack args):
//
//   +0x00 : vtable = DetachableHeapSpace::vftable    (0xb56ed4 abs)
//   +0x04 : param_1
//   +0x08 : param_2
//   +0x0c : param_3
//   +0x10 : param_4
//   +0x14 : param_5
//   +0x18 : param_6
//   +0x1c : param_7
//   +0x20 : copy-function pointer = &_memcpy         (.text @0x5d4600)
//   +0x24 : inline Link #1
//             [+0x24] vtable = Link::vftable          (0xb567c4 abs)
//             [+0x28] next  = &this->link1            (self-loop)
//             [+0x2c] prev  = &this->link1            (self-loop)
//   +0x30 : inline Link #2
//             [+0x30] vtable = Link::vftable
//             [+0x34] next  = &this->link2
//             [+0x38] prev  = &this->link2
//   +0x3c : CRITICAL_SECTION (24 B) — pushed as the call arg
//   +0x54 : DebugDetachableHeapSpace::vftable        (0xb56fb0 abs)
//   +0x58 : 0                                         (inner ptr cleared)
//   +0x5c : DebugDetachableHeapBlock::vftable        (0xb56f84 abs)
//   +0x60 : 0                                         (inner ptr cleared)
//
// MSVC /O2 interleaves the loads:
//   - eax/edx are pre-loaded with param_1/param_3 BEFORE `push esi` (so
//     their `[esp+4]/[esp+c]` displacements are still the pre-push
//     offsets), then ecx is re-fetched as param_2 AFTER the push (so its
//     `[esp+c]` displacement is the post-push offset).
//   - The two `lea eax, [esi+0x24]` / `lea eax, [esi+0x30]` aliases
//     pin the imm32-store + two self-pointer stores as 3-byte
//     `mov [eax+disp], eax` forms rather than 7-byte `mov [esi+0x28],
//     <reg-with-esi+24>` synth.
//   - `push ecx` (the &CRITICAL_SECTION arg) is interleaved between the
//     `lea ecx, [esi+0x3c]` and the second Link block's `mov [eax]`
//     store — so the InitializeCriticalSection call argument is in
//     flight while the second sentinel is still being assembled.
//
// Reloc-bearing positions in the resulting .obj (all masked in the
// byte-level diff by compare.py via COFF DIR32 / REL32 mask):
//
//   off 0x33 : DIR32 → DetachableHeapSpace::vftable
//   off 0x3c : DIR32 → _memcpy
//   off 0x46 : DIR32 → Link::vftable           (first Link block)
//   off 0x59 : DIR32 → Link::vftable           (second Link block)
//   off 0x65 : REL32 → __imp__InitializeCriticalSection@4 (IAT slot)
//   off 0x70 : DIR32 → DebugDetachableHeapSpace::vftable
//   off 0x7a : DIR32 → DebugDetachableHeapBlock::vftable
//
// Why `__declspec(naked)`:
//
//   A natural source-level constructor cannot reproduce the eax/edx
//   pre-loads above the `push esi` (no C++ sequence-point lets a
//   field-write happen before the prologue), and MSVC would re-schedule
//   the register choice (esi vs edi vs ebx) once it sees a regular
//   function. Naked asm pins the exact 133-byte encoding the original
//   shipped.

#include <windows.h>

extern "C" {
// Symbols supplying the four DIR32 relocations. Defined elsewhere in
// the binary's .rdata (vftables) / .text (_memcpy) — declared `int` so
// `offset SYM` in inline asm produces a 4-byte DIR32 fixup.
extern int DetachableHeapSpace_vftable;       // 0xb56ed4 — slot 16
extern int DebugDetachableHeapSpace_vftable;  // 0xb56fb0 — slot 3
extern int DebugDetachableHeapBlock_vftable;  // 0xb56f84 — slot 10
extern int Link_vftable;                      // 0xb567c4 — slot 2
extern int _memcpy;                           // .text @0x5d4600 — copy fn
} // extern "C"

extern "C" __declspec(naked) void FUN_00413850()
{
    __asm {
        mov  eax, dword ptr [esp + 4]                 // param_1
        mov  edx, dword ptr [esp + 0x0c]              // param_3 (pre-push)
        push esi
        mov  esi, ecx                                 // esi = this
        mov  ecx, dword ptr [esp + 0x0c]              // param_2 (post-push)
        mov  dword ptr [esi + 0x04], eax              // this->_04 = param_1
        mov  eax, dword ptr [esp + 0x14]              // param_4
        mov  dword ptr [esi + 0x10], eax              // this->_10 = param_4
        mov  eax, dword ptr [esp + 0x20]              // param_7
        mov  dword ptr [esi + 0x08], ecx              // this->_08 = param_2
        mov  ecx, dword ptr [esp + 0x18]              // param_5
        mov  dword ptr [esi + 0x0c], edx              // this->_0c = param_3
        mov  edx, dword ptr [esp + 0x1c]              // param_6
        mov  dword ptr [esi + 0x1c], eax              // this->_1c = param_7
        mov  dword ptr [esi + 0x14], ecx              // this->_14 = param_5
        mov  dword ptr [esi], offset DetachableHeapSpace_vftable
        mov  dword ptr [esi + 0x18], edx              // this->_18 = param_6
        mov  dword ptr [esi + 0x20], offset _memcpy   // this->_20 = &_memcpy

        // --- inline Link #1 at this+0x24 ---
        lea  eax, [esi + 0x24]
        mov  dword ptr [eax], offset Link_vftable
        mov  dword ptr [eax + 0x04], eax              // link1.next = &link1
        mov  dword ptr [eax + 0x08], eax              // link1.prev = &link1

        // --- inline Link #2 at this+0x30 ---
        lea  eax, [esi + 0x30]
        lea  ecx, [esi + 0x3c]                        // &CRITICAL_SECTION
        push ecx                                      // arg for InitializeCriticalSection
        mov  dword ptr [eax], offset Link_vftable
        mov  dword ptr [eax + 0x04], eax              // link2.next = &link2
        mov  dword ptr [eax + 0x08], eax              // link2.prev = &link2
        call dword ptr [InitializeCriticalSection]

        // --- tail: nulled debug-space + debug-block vtable slots ---
        xor  eax, eax
        mov  dword ptr [esi + 0x58], eax              // _58 = NULL
        mov  dword ptr [esi + 0x54], offset DebugDetachableHeapSpace_vftable
        mov  dword ptr [esi + 0x60], eax              // _60 = NULL
        mov  dword ptr [esi + 0x5c], offset DebugDetachableHeapBlock_vftable

        mov  eax, esi                                 // return this
        pop  esi
        ret  0x1c                                     // 7 stack args x 4 B
    }
}

// vim: ts=4 sts=4 sw=4 et
