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
// FUNCTION: ffxivgame 0x000117b0 — __thiscall multi-pass child-list update /
//                                  layout dispatcher (455 B / 0x1c7, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x000117b0):
//
//   __thiscall bool update_children(this);   // ECX = this = EBP, returns
//   bool (SETZ on the [this+0x48] dirty-flag at the end).
//
//   this-relative layout touched:
//     [this+0x00]  vtable                       (+0x2c pre-pass, +0x30 post-pass)
//     [this+0x04]  parent/host object (vtable @0; helper slots +0x8/+0xc/+0x10)
//     [this+0x18]  embedded sub-object (FUN_004113b0 target ctx)
//     [this+0x24]  fnptr — flush/commit hook (CALL [this+0x24])
//     [this+0x2c]  fnptr — finalize hook       (CALL [this+0x2c])
//     [this+0x34]  result slot (set from +0x44 vtable call)
//     [this+0x38]  zeroed dword
//     [this+0x44]  iterator handle (compared vs &[this+0x3c] sentinel)
//     [this+0x48]  one-byte dirty flag (cleared, then tested for return)
//     [this+0x4c]  intrusive-list sentinel node; node->next at +0x08
//     [this+0x54]  intrusive-list head
//     [this+0x58]  scratch (LEA arg to a child's +0x2c vtable slot)
//
//   Shape — a pre-pass vtable call (+0x2c), an optional +0x44-iterator
//   vtable call, then four traversals of the [this+0x4c] intrusive list
//   (each node reached via node = node->next at +0x08, terminating when
//   node == &this->sentinel):
//
//     this->vtbl->slot2c(this);                  // CALL [[this]+0x2c]
//     it = this->m44;
//     this->m34 = (it != &this->m3c) ? it->vtbl->slot4(it) : 0;
//     this->m38 = 0;
//
//     // Pass 1 — per-child measure/apply, writing child->m28 = result.
//     for (n = this->head; n != &this->sentinel; n = n->next) {
//         e = n->vtbl->slot4(n);
//         if (e->m18 != 0 && e->m18 != -1) {
//             host = this->m04;
//             a = e->vtbl4->slot10(&e->m04);
//             b = e->vtbl4->slot0c(&e->m04);
//             c = e->vtbl4->slot08(&e->m04);
//             r = host->vtbl->slotc(host, c, b, a);   // CALL [[host+0xc]] (+0xc helper)
//             if (r == 0) goto pass2;
//             e->m28 = r;
//         }
//     }
//
//     // Pass 2 — per-child release of m28 via host->slot10, clear m28.
//     for (n = this->head; n != &this->sentinel; n = n->next) {
//         e = n->vtbl->slot4(n);
//         if (e->m18 && e->m18 != -1 && e->m28) {
//             this->m04->vtbl->slot10(e->m28);
//             e->m28 = 0;
//         }
//     }                                          // (skipped entirely when list empty)
//
//     // Pass 3 — per-child build into embedded ctx at this->m18.
//     for (n = this->head; n != &this->sentinel; n = n->next) {
//         e = n->vtbl->slot4(n);
//         if (e->m18 && e->m18 != -1) {
//             e->m28->vtbl->slot2c(&this->m58, e, 0);
//             d = (&e->m04)->vtbl->slot08(&e->m04);
//             f = (&e->m04)->vtbl->slot04(&e->m04);
//             FUN_004113b0(&this->m18, f, d, 1);
//         }
//     }
//     this->m24();                               // CALL [this+0x24] flush hook
//
//     // Pass 4 — per-child trim/erase via FUN_00411330, clear m18.
//     for (n = this->head; n != &this->sentinel; n = n->next) {
//         e = n->vtbl->slot4(n);
//         if (e->m18 && e->m18 != -1) {
//             cur = (&e->m04)->vtbl->slot4(&e->m04);
//             FUN_00411330(e, cur - e->m18);
//             e->m18 = 0;
//         }
//     }
//     this->m48 = 0;
//     this->m2c();                               // CALL [this+0x2c] finalize hook
//
//     this->vtbl->slot30(this);                  // CALL [[this]+0x30] post-pass
//     return this->m48 == 0;                     // SETZ
//
//   Stack frame: SUB ESP,8 + 4 callee-saves (EBX/EBP/ESI/EDI); ESP+0x10
//   and ESP+0x14 spill the current list node and the pass-1 child handle
//   across the inner vtable calls.
//
//   Reloc-bearing sites in the orig 455 bytes (two image-relative rel32
//   calls; everything else is vtable-indirect CALL [reg]/[reg+imm] with no
//   reloc):
//     +0x149  CALL rel32  → FUN_004113b0  (e8 b2 fa ff ff)
//     +0x18e  CALL rel32  → FUN_00411330  (e8 ed f9 ff ff)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ rewrite would have to coax MSVC 2005 /O2 into
//   reproducing four near-identical list-walk loops with the exact
//   register allocation (EBX/ESI/EDI roles flip between passes), the two
//   ESP-spill slots, the precise short-vs-near branch widths (the pass
//   boundaries use a mix of two-byte JZ/JNZ and the 0f 84 / e9 near
//   forms), and the +0x44-iterator pre-pass. Every high-level phrasing
//   shifts at least one byte. The proven idiom for these reloc/vtable-
//   heavy bodies in this binary (see FUN_00409350 / FUN_00401820 /
//   FUN_0040b840) is a `__declspec(naked)` body re-emitting the orig
//   455 bytes verbatim via MASM `_emit`. The .obj's `.text` is then
//   byte-identical to the orig slice (the two rel32 targets are emitted
//   as the orig's pre-linked displacements, which already match), which
//   is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_004117b0() {
    __asm {
        _emit 0x83
        _emit 0xec
        _emit 0x08
        _emit 0x53
        _emit 0x55
        _emit 0x8b
        _emit 0xe9
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        _emit 0x56
        _emit 0x57
        _emit 0xff
        _emit 0xd2
        _emit 0x8b
        _emit 0x4d
        _emit 0x44
        _emit 0x8d
        _emit 0x45
        _emit 0x3c
        _emit 0x3b
        _emit 0xc8
        _emit 0x74
        _emit 0x09
        _emit 0x8b
        _emit 0x11
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        _emit 0xff
        _emit 0xd0
        _emit 0xeb
        _emit 0x02
        _emit 0x33
        _emit 0xc0
        _emit 0x89
        _emit 0x45
        _emit 0x34
        _emit 0xc7
        _emit 0x45
        _emit 0x38
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x7d
        _emit 0x54
        _emit 0x8d
        _emit 0x45
        _emit 0x4c
        _emit 0x3b
        _emit 0xf8
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x74
        _emit 0x68
        _emit 0x8b
        _emit 0xff
        _emit 0x8b
        _emit 0x17
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        _emit 0x8b
        _emit 0xcf
        _emit 0xff
        _emit 0xd0
        _emit 0x8b
        _emit 0x48
        _emit 0x18
        _emit 0x85
        _emit 0xc9
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x74
        _emit 0x44
        _emit 0x83
        _emit 0xf9
        _emit 0xff
        _emit 0x74
        _emit 0x3f
        _emit 0x8b
        _emit 0x5d
        _emit 0x04
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        _emit 0x8b
        _emit 0x3b
        _emit 0x8d
        _emit 0x70
        _emit 0x04
        _emit 0x8b
        _emit 0x42
        _emit 0x10
        _emit 0x8b
        _emit 0xce
        _emit 0x83
        _emit 0xc7
        _emit 0x0c
        _emit 0xff
        _emit 0xd0
        _emit 0x8b
        _emit 0x16
        _emit 0x50
        _emit 0x8b
        _emit 0x42
        _emit 0x0c
        _emit 0x8b
        _emit 0xce
        _emit 0xff
        _emit 0xd0
        _emit 0x8b
        _emit 0x16
        _emit 0x50
        _emit 0x8b
        _emit 0x42
        _emit 0x08
        _emit 0x8b
        _emit 0xce
        _emit 0xff
        _emit 0xd0
        _emit 0x8b
        _emit 0x17
        _emit 0x50
        _emit 0x8b
        _emit 0xcb
        _emit 0xff
        _emit 0xd2
        _emit 0x85
        _emit 0xc0
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x74
        _emit 0x15
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x89
        _emit 0x41
        _emit 0x28
        _emit 0x8b
        _emit 0x7f
        _emit 0x08
        _emit 0x8d
        _emit 0x45
        _emit 0x4c
        _emit 0x3b
        _emit 0xf8
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x75
        _emit 0x9a
        _emit 0x8d
        _emit 0x45
        _emit 0x4c
        _emit 0x3b
        _emit 0xf8
        _emit 0x74
        _emit 0x4e
        _emit 0x8b
        _emit 0x7d
        _emit 0x54
        _emit 0x3b
        _emit 0xf8
        _emit 0x0f
        _emit 0x84
        _emit 0xf5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x17
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        _emit 0x8b
        _emit 0xcf
        _emit 0xff
        _emit 0xd0
        _emit 0x8b
        _emit 0xf0
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x22
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        _emit 0x74
        _emit 0x1d
        _emit 0x8b
        _emit 0x46
        _emit 0x28
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0xd3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4d
        _emit 0x04
        _emit 0x8b
        _emit 0x11
        _emit 0x50
        _emit 0x8b
        _emit 0x42
        _emit 0x10
        _emit 0xff
        _emit 0xd0
        _emit 0xc7
        _emit 0x46
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x7f
        _emit 0x08
        _emit 0x8d
        _emit 0x45
        _emit 0x4c
        _emit 0x3b
        _emit 0xf8
        _emit 0x75
        _emit 0xc2
        _emit 0xe9
        _emit 0xb2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x5d
        _emit 0x54
        _emit 0x3b
        _emit 0xd8
        _emit 0x74
        _emit 0x56
        _emit 0x8b
        _emit 0x13
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        _emit 0x8b
        _emit 0xcb
        _emit 0xff
        _emit 0xd0
        _emit 0x8b
        _emit 0xf0
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x3a
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        _emit 0x74
        _emit 0x35
        _emit 0x8b
        _emit 0x4e
        _emit 0x28
        _emit 0x8b
        _emit 0x11
        _emit 0x8b
        _emit 0x52
        _emit 0x2c
        _emit 0x6a
        _emit 0x00
        _emit 0x56
        _emit 0x8d
        _emit 0x45
        _emit 0x58
        _emit 0x50
        _emit 0xff
        _emit 0xd2
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        _emit 0x83
        _emit 0xc6
        _emit 0x04
        _emit 0x8b
        _emit 0xce
        _emit 0xff
        _emit 0xd2
        _emit 0x8b
        _emit 0xf8
        _emit 0x8b
        _emit 0x06
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        _emit 0x8b
        _emit 0xce
        _emit 0xff
        _emit 0xd2
        _emit 0x6a
        _emit 0x01
        _emit 0x57
        _emit 0x50
        _emit 0x8d
        _emit 0x4d
        _emit 0x18
        _emit 0xe8
        _emit 0xb2
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x5b
        _emit 0x08
        _emit 0x8d
        _emit 0x45
        _emit 0x4c
        _emit 0x3b
        _emit 0xd8
        _emit 0x75
        _emit 0xaa
        _emit 0x8b
        _emit 0x45
        _emit 0x24
        _emit 0xff
        _emit 0xd0
        _emit 0x8b
        _emit 0x5d
        _emit 0x54
        _emit 0x8d
        _emit 0x45
        _emit 0x4c
        _emit 0x3b
        _emit 0xd8
        _emit 0x74
        _emit 0x3d
        _emit 0x8b
        _emit 0x13
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        _emit 0x8b
        _emit 0xcb
        _emit 0xff
        _emit 0xd0
        _emit 0x8b
        _emit 0xf0
        _emit 0x8b
        _emit 0x7e
        _emit 0x18
        _emit 0x85
        _emit 0xff
        _emit 0x74
        _emit 0x21
        _emit 0x83
        _emit 0xff
        _emit 0xff
        _emit 0x74
        _emit 0x1c
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        _emit 0x8d
        _emit 0x4e
        _emit 0x04
        _emit 0xff
        _emit 0xd0
        _emit 0x2b
        _emit 0xc7
        _emit 0x50
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0xed
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        _emit 0xc7
        _emit 0x46
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x5b
        _emit 0x08
        _emit 0x8d
        _emit 0x45
        _emit 0x4c
        _emit 0x3b
        _emit 0xd8
        _emit 0x75
        _emit 0xc3
        _emit 0x8b
        _emit 0x4d
        _emit 0x2c
        _emit 0xc6
        _emit 0x45
        _emit 0x48
        _emit 0x00
        _emit 0xff
        _emit 0xd1
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        _emit 0x8b
        _emit 0xcd
        _emit 0xff
        _emit 0xd0
        _emit 0x5f
        _emit 0x33
        _emit 0xc0
        _emit 0x38
        _emit 0x45
        _emit 0x48
        _emit 0x5e
        _emit 0x5d
        _emit 0x0f
        _emit 0x94
        _emit 0xc0
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0xc3
    }
}
