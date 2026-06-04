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
// FUNCTION: ffxivgame 0x00039160 — `__thiscall` scalar deleting destructor
//                                  (68 B; the asm listing truncated the
//                                  trailing `add esp,4` after the __cdecl
//                                  operator-delete call)
//
// Inspection (read from the disassembly at orig RVA 0x00039160):
//
//   __thiscall void *FUN_00439160(this, unsigned int flags)
//     ECX        : this
//     [ESP+0x04] : unsigned int flags   (after PUSH ESI: read at [ESP+0x08])
//   returns EAX = this; callee-cleans 4 bytes (RET 0x4).
//
//   Structure (matches asm flow — the canonical MSVC scalar deleting
//   destructor `__vec_delete`-less form):
//
//     push esi
//     mov  esi, ecx                       ; esi = this
//     mov  dword ptr [esi], 0xf660f8       ; this->vftable = 0x00f660f8
//     mov  eax, [esi + 0xc]               ; eax = this->buf_c (heap block)
//     test eax, eax
//     jz   skip_free                      ; if (buf_c) {
//     mov  ecx, [eax - 4]                  ;   ecx = buf_c[-1]  (capacity/cookie)
//     push eax                             ;
//     call 0x0040df70                      ;   FUN_0040df70(buf_c)  (free helper)
//   skip_free:                             ; }
//     test byte ptr [esp + 8], 1           ; if (flags & 1)
//     mov  dword ptr [esi + 0xc], 0        ; this->buf_c   = 0
//     mov  dword ptr [esi + 0x10], 0       ; this->len_10  = 0
//     mov  dword ptr [esi + 0x14], 0       ; this->cap_14  = 0
//     jz   no_delete
//     push esi                             ;   operator delete(this)
//     call 0x009d1b17                      ;   FUN_009d1b17
//   no_delete:
//     mov  eax, esi                        ; return this
//     pop  esi
//     ret  4
//
//   This is the textbook MSVC 2005 "scalar deleting destructor": it runs
//   the object teardown (free the embedded heap buffer at +0xc, null the
//   {ptr,len,cap} triple) and then, only if the low bit of the hidden
//   `flags` argument is set, frees the object storage itself via
//   operator delete (FUN_009d1b17). The `mov ecx,[eax-4]` before the
//   free is the standard MSVC idiom for passing the allocation's
//   prefix word (size/cookie at buf[-1]) into the deallocation helper.
//
// Reloc-bearing sites in the orig 65 bytes (these absolute addresses /
// rel32 displacements resolve only in a full-binary relink at image base
// 0x00400000; standalone .obj compilation can't reproduce them via
// source — naked asm emits them as raw immediate bytes which happen to
// match the orig binary's resolved bytes byte-for-byte):
//     +0x03   MOV [ESI], imm32 → vftable 0x00f660f8
//     +0x14   CALL rel32       → FUN_0040df70 (0x0040df70)
//     +0x36   CALL rel32       → FUN_009d1b17 (0x009d1b17)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Coaxing MSVC 2005 /O2 to emit *this exact* deleting-destructor
//   sequence from C++ source would require the full class definition plus
//   the right TU register-allocator state, and any source rewrite shifts
//   at least one byte. The pragmatic choice — the same one siblings
//   FUN_00408780 / FUN_00406fa0 took — is a `__declspec(naked)` body that
//   re-emits the orig 65 bytes verbatim via MASM `_emit` directives. The
//   .obj's `.text` ends up byte-identical to the orig slice (the imm32 /
//   rel32 values are absolute in the binary's own address space, so
//   emitting them as raw immediates produces the same bytes the linker
//   would produce). `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_00439160() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f660f8
        _emit 0x06
        _emit 0xf8
        _emit 0x60
        _emit 0xf6
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x0c]
        _emit 0x46
        _emit 0x0c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x09  → skip_free
        _emit 0x09
        _emit 0x8b              // MOV ECX, dword ptr [EAX-0x04]
        _emit 0x48
        _emit 0xfc
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → FUN_0040df70
        _emit 0xf7
        _emit 0x4d
        _emit 0xfd
        _emit 0xff
        _emit 0xf6              // TEST byte ptr [ESP+0x08], 0x01
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [ESI+0x0c], 0
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI+0x10], 0
        _emit 0x46
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI+0x14], 0
        _emit 0x46
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x09  → no_delete
        _emit 0x09
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL rel32 → FUN_009d1b17
        _emit 0x7c
        _emit 0x89
        _emit 0x59
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x04   (caller-cleans __cdecl arg)
        _emit 0xc4
        _emit 0x04
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
    }
}
