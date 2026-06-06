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
// FUNCTION: ffxivgame 0x00413640 — `__thiscall` destructor for
//           `SQEX::CDev::Engine::Memory::Alternative::DetachableHeapSpace`
//           (88 B / 0x58, no stack frame, single saved reg: ESI = this).
//
// Calling convention: __thiscall — `this` arrives in ECX; no stack args;
// epilogue is `pop esi / ret` (no callee stack cleanup, zero args).
//
// Shape (reconstructed from the disassembly):
//
//   The destructor walks the MSVC "most-derived → base" vftable reseat
//   sequence.  Four vftable pointers are reseated during the body:
//
//     [this + 0x00]  ← DetachableHeapSpace::vftable (entry reseat, 0x00f56ed4)
//     [this + 0x5c]  ← IDebugBlock::vftable          (0x00f56788)
//     [this + 0x54]  ← IDebugSpace::vftable           (0x00f567b4)
//     [this + 0x00]  ← ISpace::vftable      (exit/final reseat, 0x00f566fc)
//
//   Between the entry and exit reseats the destructor:
//
//   1. Calls DeleteCriticalSection(&this->critSec) via the Win32 IAT entry
//      at 0x00f3e170 (indirect CALL through fixed IAT pointer).
//      The critical section lives at this + 0x3c.
//
//   2. Reseats the `Link` vftable at [this + 0x30] = Link::vftable (0xf567c4)
//      and unlinks the embedded `link0` node (next at +0x34, prev at +0x38):
//          link0.next->prev = link0.prev;   // [link0.next + 0x8] = link0.prev
//          link0.prev->next = link0.next;   // [link0.prev + 0x4] = link0.next
//
//   3. Reseats the `Link` vftable at [this + 0x24] = Link::vftable (0xf567c4)
//      and unlinks the embedded `link1` node (next at +0x28, prev at +0x2c):
//          link1.next->prev = link1.prev;   // [link1.next + 0x8] = link1.prev
//          link1.prev->next = link1.next;   // [link1.prev + 0x4] = link1.next
//
//   `Link` member layout (from the stores): vftable at +0x00, next at +0x04,
//   prev at +0x08.  (Confirmed: "[link->next + 0x8] = prev" and
//   "[link->prev + 0x4] = next".)
//
// Reconstruction strategy — naked asm with readable MASM mnemonics:
//
//   Every immediate in this function (vftable addresses, IAT slot address) is
//   a compile-time constant, not a named symbol.  MSVC's inline assembler
//   encodes them as raw bytes with no COFF relocations, so the .obj .text
//   section is byte-identical to the original binary slice.
//   tools/compare.py therefore reports GREEN without relocation masking.

extern "C" __declspec(naked) void FUN_00413640() {
    __asm {
        push    esi
        mov     esi, ecx                            // this = ECX (__thiscall)

        // entry vftable reseat: DetachableHeapSpace::vftable
        mov     dword ptr [esi], 0xf56ed4

        lea     eax, [esi + 0x3c]                   // &this->critSec

        // secondary base vftable reseat: IDebugBlock::vftable
        mov     dword ptr [esi + 0x5c], 0xf56788

        push    eax                                 // arg: &critSec

        // secondary base vftable reseat: IDebugSpace::vftable
        mov     dword ptr [esi + 0x54], 0xf567b4

        // DeleteCriticalSection(&critSec) via IAT indirect: ff 15 70 e1 f3 00
        _emit 0xff
        _emit 0x15
        _emit 0x70
        _emit 0xe1
        _emit 0xf3
        _emit 0x00

        // unlink link0 (next at +0x34, prev at +0x38)
        mov     ecx, dword ptr [esi + 0x34]         // ecx = link0.next
        mov     edx, dword ptr [esi + 0x38]         // edx = link0.prev
        mov     eax, 0xf567c4                       // Link::vftable
        mov     dword ptr [esi + 0x30], eax         // link0.vftable = Link::vftable
        mov     dword ptr [ecx + 0x8], edx          // link0.next->prev = link0.prev
        mov     ecx, dword ptr [esi + 0x38]         // ecx = link0.prev
        mov     edx, dword ptr [esi + 0x34]         // edx = link0.next
        mov     dword ptr [ecx + 0x4], edx          // link0.prev->next = link0.next

        // unlink link1 (next at +0x28, prev at +0x2c)
        mov     ecx, dword ptr [esi + 0x2c]         // ecx = link1.prev
        mov     dword ptr [esi + 0x24], eax         // link1.vftable = Link::vftable (eax still 0xf567c4)
        mov     eax, dword ptr [esi + 0x28]         // eax = link1.next
        mov     dword ptr [eax + 0x8], ecx          // link1.next->prev = link1.prev
        mov     edx, dword ptr [esi + 0x2c]         // edx = link1.prev
        mov     eax, dword ptr [esi + 0x28]         // eax = link1.next
        mov     dword ptr [edx + 0x4], eax          // link1.prev->next = link1.next

        // exit/final vftable reseat: ISpace::vftable
        mov     dword ptr [esi], 0xf566fc

        pop     esi
        ret
    }
}
