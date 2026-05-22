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
// FUNCTION: ffxivgame 0x00406de0 — __thiscall constructor for a class
//                                  holding two inline "labelled" sub-objects
//                                  (each prefixed with the 7-byte "(NULL)\0"
//                                  default name, plus three zeroed pointer
//                                  slots) and a Win32 CRITICAL_SECTION
//                                  protecting the whole structure (136 B
//                                  / 0x88).
//
// Layout (this aka esi):
//
//   +0x0000 : sub_a            (sub-object A; this->byte0 = 0 here)
//   +0x0004 : sub_a.name[7]    "(NULL)\0"  — bulk-copied from .rdata global
//                              at orig RVA 0xb54f80 via dword+word+byte
//                              loads (MSVC's canonical inline-memcpy split
//                              for a 7-byte literal at a fixed-address
//                              source).
//   +0x0038 : sub_a.p_handlers (3 dwords zeroed: 0x38, 0x3c, 0x40)
//   +0x2004 : sub_b            (sub-object B; this dword = 1 — flagged
//                              "second slot active" or "managed by parent"
//                              variant of the same record shape).
//   +0x2008 : sub_b.name[7]    "(NULL)\0"  (same global, same copy idiom)
//   +0x203c : sub_b.p_handlers (3 dwords zeroed)
//   +0x4008 : tail flag = 1    (final cookie/flag dword)
//   +0x400c : CRITICAL_SECTION lock guarding the whole object — initialised
//                              before any field writes so a competing thread
//                              entering the object via the lock sees a
//                              consistent post-construction state.
//
// Shape (reconstructed from the asm; Ghidra headless agrees, hint at
// build/ghidra-decomp/ffxivgame/00006de0_FUN_00406de0.c):
//
//   T *T::T(T *this) {
//       this->_b0 = 0;                          // zero sub_a's first byte
//                                                // before any work, so a
//                                                // thread reading via the
//                                                // CS-locked path sees the
//                                                // half-initialised state.
//       InitializeCriticalSection(&this->lock); // arg = &this + 0x400c
//
//       memcpy(&this->sub_a.name, "(NULL)", 7); // copies 7 B incl. NUL
//       this->sub_a.p_handlers[0] = NULL;       // [+0x38]
//       this->sub_a.p_handlers[1] = NULL;       // [+0x3c]
//       this->sub_a.p_handlers[2] = NULL;       // [+0x40]
//
//       this->sub_b.flag         = 1;           // [+0x2004]
//       memcpy(&this->sub_b.name, "(NULL)", 7); // [+0x2008..+0x200e]
//       this->sub_b.p_handlers[0] = NULL;       // [+0x203c]
//       this->sub_b.p_handlers[1] = NULL;       // [+0x2040]
//       this->sub_b.p_handlers[2] = NULL;       // [+0x2044]
//
//       this->tail_flag = 1;                    // [+0x4008]
//       return this;
//   }
//
// Why `__declspec(naked)`:
//
//   The orig schedules `mov [esi], bl` (sub_a's first-byte zero) BEFORE
//   the `call InitializeCriticalSection` — purely to overlap the EBX=0
//   materialisation with the arg-setup window while the call's argument
//   is in flight on the stack. A natural source-level constructor would
//   sequence the zero AFTER InitializeCriticalSection returns (since C
//   sequence-points keep `this->_b0 = 0` after the function call), which
//   forces a different EBX/ESI/EAX schedule and a different
//   prologue-vs-call-arg layout. Naked asm pins the encoding.
//
//   It also pins:
//     - the 7-byte "(NULL)\0" copy split into a 32-bit + 16-bit + 8-bit
//       load from three consecutive offsets inside the same .rdata
//       literal (MSVC's inline-memcpy lowering for fixed-source short
//       copies — not what a hand-written memcpy(..., 7) call would
//       compile to here, since the source happens to live at a fixed
//       global address that MSVC can reach via three DIR32 relocs);
//     - reuse of EBX=0 as the source operand for every NULL store
//       (`mov [esi+0x38], ebx` etc.) rather than the `mov dword ptr ..., 0`
//       7-byte form;
//     - reuse of EAX=1 as the source operand for both the `+0x2004` and
//       `+0x4008` flag stores rather than two `mov ..., 1` immediates.
//
// Reloc-bearing positions in the resulting .obj (all masked in the diff):
//
//   off 0x11   IMAGE_REL_I386_DIR32  → __imp__InitializeCriticalSection@4
//                                       (IAT slot — orig 0x00f3e174)
//   off 0x17   IMAGE_REL_I386_DIR32  → k_null_label + 0  (orig 0x00f54f80)
//   off 0x20   IMAGE_REL_I386_DIR32  → k_null_label + 4  (orig 0x00f54f84)
//   off 0x2a   IMAGE_REL_I386_DIR32  → k_null_label + 6  (orig 0x00f54f86)
//   off 0x47   IMAGE_REL_I386_DIR32  → k_null_label + 0
//   off 0x54   IMAGE_REL_I386_DIR32  → k_null_label + 4
//   off 0x61   IMAGE_REL_I386_DIR32  → k_null_label + 6

#include <windows.h>

extern "C" {

// `(NULL)\0` literal at orig RVA 0xb54f80 (.rdata). MSVC inline asm
// references the dword/word/byte slices via DIR32 relocations — the
// linker patches the addresses, `tools/compare.py` masks them out of
// the byte-level diff.
extern const char k_null_label[8];

} // extern "C"

extern "C" __declspec(naked) void FUN_00406de0() {
    __asm {
        push ebx
        push esi
        mov  esi, ecx                            // esi = this
        lea  eax, [esi + 0x400c]                 // &this->lock
        xor  ebx, ebx                            // ebx = 0 (NULL src for many stores)
        push eax                                 // arg = &lock
        mov  byte ptr [esi], bl                  // this->_b0 = 0 (overlap w/ call arg)
        call dword ptr [InitializeCriticalSection]

        // --- sub_a: "(NULL)\0" name + 3 NULL handler slots ---
        mov  ecx, dword ptr [k_null_label]        // bytes 0..3 of "(NULL)"
        mov  dword ptr [esi + 4], ecx
        movzx edx, word ptr [k_null_label + 4]    // bytes 4..5
        mov  word ptr [esi + 8], dx
        mov  al, byte ptr [k_null_label + 6]      // byte 6 = '\0'
        mov  byte ptr [esi + 0x0a], al
        mov  dword ptr [esi + 0x38], ebx          // sub_a.p_handlers[0] = NULL
        mov  dword ptr [esi + 0x3c], ebx          // sub_a.p_handlers[1] = NULL
        mov  dword ptr [esi + 0x40], ebx          // sub_a.p_handlers[2] = NULL

        // --- sub_b: flag=1 + "(NULL)\0" name + 3 NULL handler slots ---
        mov  eax, 1
        mov  dword ptr [esi + 0x2004], eax        // sub_b.flag = 1
        mov  ecx, dword ptr [k_null_label]
        mov  dword ptr [esi + 0x2008], ecx
        movzx edx, word ptr [k_null_label + 4]
        mov  word ptr [esi + 0x200c], dx
        mov  cl, byte ptr [k_null_label + 6]
        mov  byte ptr [esi + 0x200e], cl
        mov  dword ptr [esi + 0x203c], ebx
        mov  dword ptr [esi + 0x2040], ebx
        mov  dword ptr [esi + 0x2044], ebx

        // --- tail flag + return this ---
        mov  dword ptr [esi + 0x4008], eax        // tail_flag = 1
        mov  eax, esi                             // return this
        pop  esi
        pop  ebx
        ret
    }
}

// vim: ts=4 sts=4 sw=4 et
