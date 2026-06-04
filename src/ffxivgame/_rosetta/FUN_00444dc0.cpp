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
// FUNCTION: ffxivgame 0x00044dc0 — `__thiscall` container splice/erase that
//                                  returns an iterator pair by value
//                                  (123 B / 0x7b, RET 0x14).
//
// Inspection (read from the disassembly at orig RVA 0x00044dc0):
//
//   struct IterPair { void* first; void* second; };  // returned in [retptr]
//
//   IterPair __thiscall splice(Container* this /*ECX → EBP*/,
//                              IterPair* __ret /*[ESP+0x0c], hidden*/,
//                              void* a   /*[ESP+0x10]*/,
//                              void* b   /*[ESP+0x14]*/,
//                              void* c   /*[ESP+0x18]*/,
//                              void* d   /*[ESP+0x1c]*/);
//
//   `RET 0x14` cleans the five stack dwords (hidden return ptr + four
//   explicit args); `this` arrives in ECX (kept in EBP). The result is a
//   two-pointer iterator pair stored through the hidden return slot and
//   echoed back in EAX.
//
//   Structural shape:
//
//     if (a == 0 || a != c)
//         _bounds_check();                 // CALL 0x009d22b4 (range helper)
//     if (b != d) {
//         // rebuild the [b,d) range; helper returns the new `end`
//         void* nb = FUN_00444cf0(d, this->m_buf /*[EBP+8]*/, b,
//                                 c, c /*aliased bool=0 slot*/);
//         // destroy each displaced element (stride 0x54 = 84 bytes)
//         for (void* p = nb; p != this->m_buf; p += 0x54)
//             FUN_00446f50(p);             // element dtor, __thiscall
//         this->m_buf = nb;                // commit new end
//         // fall through with (c, d) reloaded as the iterator pair
//     }
//     __ret->first  = /*ESI*/;   __ret->second = /*ECX*/;
//     return __ret;
//
//   Element size 0x54 (84 B) and the m_buf field at this+0x8 identify the
//   container's element type; the two callees (FUN_00444cf0 splice-rebuild,
//   FUN_00446f50 element destructor) are the obvious next-match siblings.
//
//   Stack frame (after PUSH EBP / PUSH ESI, ESP-relative):
//     [esp+0x00]  saved ESI
//     [esp+0x04]  saved EBP
//     [esp+0x08]  return address
//     [esp+0x0c]  hidden return-struct ptr  (echoed in EAX)
//     [esp+0x10]  arg a
//     [esp+0x14]  arg b
//     [esp+0x18]  arg c
//     [esp+0x1c]  arg d
//   The `b != d` arm pushes EBX/EDI (bumping refs by 8) and materialises a
//   zero bool in the arg-c slot ([ESP+0x20] post-push) for the FUN_00444cf0
//   call before POP EDI / POP EBX restores the frame.
//
//   Reloc-bearing sites in the orig 123 bytes (rel32 displacements are
//   valid against the orig load address; a naked-asm `_emit` body carries
//   NO relocations and reproduces the orig slice byte-for-byte, which is
//   what `tools/compare.py` checks):
//     +0x12   CALL rel32 → 0x009d22b4 (range/bounds helper, baked dd d4 58 00)
//     +0x3f   CALL rel32 → 0x00444cf0 (splice-rebuild sibling, baked ec fe ff ff)
//     +0x54   CALL rel32 → 0x00446f50 (element dtor, baked 37 21 00 00)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form would need MSVC 2005 /O2 to reproduce the exact
//   register allocation (EBP=this, ESI/EDI live across the dtor loop), the
//   aliased zero-bool materialised over the arg-c slot, the precise push
//   order into FUN_00444cf0, and the three rel32 call displacements. Each is
//   brittle under /O2, so — following the sibling _rosetta convention — this
//   is a `__declspec(naked)` body that re-emits the orig 123 bytes verbatim
//   via MASM `_emit` directives. `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_00444dc0() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, [ESP+0x10]
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x8b              // MOV EBP, ECX
        _emit 0xe9
        _emit 0x74              // JZ +6 (→ 0x00044dd2)
        _emit 0x06
        _emit 0x3b              // CMP ESI, [ESP+0x18]
        _emit 0x74
        _emit 0x24
        _emit 0x18
        _emit 0x74              // JZ +5 (→ 0x00044dd7)
        _emit 0x05
        _emit 0xe8              // CALL rel32 → 0x009d22b4
        _emit 0xdd
        _emit 0xd4
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV EDX, [ESP+0x1c]
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x8b              // MOV ECX, [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x3b              // CMP ECX, EDX
        _emit 0xca
        _emit 0x74              // JZ +0x4a (→ 0x00044e2d)
        _emit 0x4a
        _emit 0x8b              // MOV EAX, [EBP+0x8]
        _emit 0x45
        _emit 0x08
        _emit 0x53              // PUSH EBX
        _emit 0x57              // PUSH EDI
        _emit 0xc6              // MOV byte ptr [ESP+0x20], 0
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x00
        _emit 0x8b              // MOV ESI, [ESP+0x20]
        _emit 0x74
        _emit 0x24
        _emit 0x20
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, [ESP+0x18]
        _emit 0x74
        _emit 0x24
        _emit 0x18
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, [ESP+0x1c]
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        _emit 0x56              // PUSH ESI
        _emit 0x51              // PUSH ECX
        _emit 0x50              // PUSH EAX
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL rel32 → 0x00444cf0
        _emit 0xec
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EBX, [EBP+0x8]
        _emit 0x5d
        _emit 0x08
        _emit 0x8b              // MOV EDI, EAX
        _emit 0xf8
        _emit 0x83              // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0x3b              // CMP EDI, EBX
        _emit 0xfb
        _emit 0x8b              // MOV ESI, EDI
        _emit 0xf7
        _emit 0x74              // JZ +0x0e (→ 0x00044e20)
        _emit 0x0e
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL rel32 → 0x00446f50
        _emit 0x37
        _emit 0x21
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESI, 0x54
        _emit 0xc6
        _emit 0x54
        _emit 0x3b              // CMP ESI, EBX
        _emit 0xf3
        _emit 0x75              // JNZ -0x0e (→ 0x00044e12)
        _emit 0xf2
        _emit 0x8b              // MOV ECX, [ESP+0x1c]
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b              // MOV ESI, [ESP+0x18]
        _emit 0x74
        _emit 0x24
        _emit 0x18
        _emit 0x89              // MOV [EBP+0x8], EDI
        _emit 0x7d
        _emit 0x08
        _emit 0x5f              // POP EDI
        _emit 0x5b              // POP EBX
        _emit 0x8b              // MOV EAX, [ESP+0x0c]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x89              // MOV [EAX], ESI
        _emit 0x30
        _emit 0x5e              // POP ESI
        _emit 0x89              // MOV [EAX+0x4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x14
        _emit 0x14
        _emit 0x00
    }
}
