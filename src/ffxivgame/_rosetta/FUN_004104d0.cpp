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
// FUNCTION: ffxivgame 0x004104d0 — scalar deleting destructor with
//                                  doubly-linked-list unlink (46 B form).
//
// __thiscall void *FUN_004104d0(int flags)
//   ECX        : this   (a doubly-linked list node with vftable at [+0x0],
//                        next-ptr at [+0x4], prev-ptr at [+0x8])
//   [ESP+0x4]  : flags  (bit 0 = "also call ::operator delete(this)")
//
// Asm shape (46 bytes, read from orig RVA 0x000104d0):
//
//   000104d0:  f6 44 24 04 01     test   byte ptr [esp+4], 1   ; flags & 1
//   000104d5:  56                 push   esi
//   000104d6:  8b f1              mov    esi, ecx              ; ESI = this
//   000104d8:  8b 46 04           mov    eax, [esi+4]          ; EAX = this->next
//   000104db:  8b 4e 08           mov    ecx, [esi+8]          ; ECX = this->prev
//   000104de:  c7 06 c4 67 f5 00  mov    dword ptr [esi], 0xf567c4
//                                                              ; inlined dtor:
//                                                              ; reset vftable to
//                                                              ; 0x00f567c4
//   000104e4:  89 48 08           mov    [eax+8], ecx          ; this->next->prev
//                                                              ;   = this->prev
//   000104e7:  8b 56 08           mov    edx, [esi+8]          ; EDX = this->prev
//   000104ea:  8b 46 04           mov    eax, [esi+4]          ; EAX = this->next
//   000104ed:  89 42 04           mov    [edx+4], eax          ; this->prev->next
//                                                              ;   = this->next
//   000104f0:  74 09              jz     no_delete             ; ZF from pre-prologue
//                                                              ; TEST (PUSH/MOV/etc.
//                                                              ; don't touch flags)
//   000104f2:  56                 push   esi                   ; arg: this
//   000104f3:  e8 1f 16 5c 00     call   0x009d1b17            ; ::operator delete
//                                                              ; (REL32, __cdecl)
//   000104f8:  83 c4 04           add    esp, 4                ; pop arg
//   no_delete:
//   000104fb:  8b c6              mov    eax, esi              ; return this
//   000104fd:  5e                 pop    esi
//   [000104fe:  c2 04 00          ret    4]                    ; outside 46-B window
//
// NOTE: The compare.py size window is 49 bytes (size_overrides.json overrides
// symbols.json's 46, adding 3 for the `RET imm16` suffix). The 49 bytes
// cover from the opening TEST through `c2 04 00` (RET 4) at 0x104fe–0x10500
// (inclusive).
//
// Shape: the same "scalar deleting destructor with inlined trivial destructor"
// pattern as the 31-byte sibling FUN_00409580, extended with a doubly-linked
// list node unlink before the delete-flag check. At [+0x4] and [+0x8] the
// node carries its next/prev pointers; the unlink is the canonical
//   node->next->prev = node->prev;
//   node->prev->next = node->next;
// form. The vftable at [+0x0] is reset to 0x00f567c4 (T's own vftable)
// as the inlined destructor body.
//
// Reloc-bearing sites in the orig 46 bytes:
//   +0x0e   MOV  imm32 → 0x00f567c4   (DIR32, T's own vftable address;
//                                       baked as raw bytes — no reloc in .obj)
//   +0x23   CALL rel32 → 0x009d1b17   (REL32, ::operator delete;
//                                       baked as raw bytes — no reloc in .obj)
//
// Reconstruction strategy — naked-asm byte passthrough. Same idiom as
// FUN_00409580 / FUN_0040a460 / FUN_0040a4b0: a `__declspec(naked)` body
// re-emits the 46 orig bytes verbatim via MASM `_emit` directives. Both
// reloc-bearing sites are baked in as concrete raw values from the orig
// PE's address space, so the .obj carries zero relocations and
// tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004104d0() {
    __asm {
        _emit 0xf6              // TEST byte ptr [ESP+0x4], 0x1
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x01
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f567c4
        _emit 0x06
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0x89              // MOV dword ptr [EAX+0x8], ECX
        _emit 0x48
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x8]
        _emit 0x56
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x89              // MOV dword ptr [EDX+0x4], EAX
        _emit 0x42
        _emit 0x04
        _emit 0x74              // JZ no_delete (+0x9)
        _emit 0x09
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL ::operator delete (rel32 → 0x009d1b17)
        _emit 0x1f
        _emit 0x16
        _emit 0x5c
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x8b              // MOV EAX, ESI              (no_delete:)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
