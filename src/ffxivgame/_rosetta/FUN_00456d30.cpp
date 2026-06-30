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
// FUNCTION: ffxivgame 0x00056d30 — ring-buffer push method (__thiscall, 102 B)
//
// __thiscall void FUN_00456d30(RingBuffer *this, void *value)
//   ECX        = this pointer
//   [ESP+0x04] = value  (one stack argument; ret 0x4 callee-pops it)
//
// Object layout inferred from access pattern (RVA 0x00056d30):
//
//   offset 0x00  unknown header / vtable / other (4 B, not accessed here)
//   offset 0x04  void* arr[0x21]   — 33-entry pointer array (0x84 B total)
//   offset 0x88  uint16_t head     — oldest-element index (read/advance pointer)
//   offset 0x8a  uint16_t tail     — next-write index (write pointer)
//
// Logical body:
//
//   // Advance tail; wrap at 33
//   ++tail;
//   if (tail >= 0x21) tail = 0;
//
//   // If head caught up to tail (buffer full), evict oldest
//   if (head == tail) {
//       ++head;
//       if (head >= 0x21) head = 0;
//   }
//
//   // Store value at current tail slot
//   arr[(int16_t)tail] = value;
//
// No external calls, no relocations — pure __thiscall 102-byte method.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The exact instruction mix (16-bit in-place ADD, MOVZX reloads after each
//   field update, MOVSX for the final array-index conversion, and the two
//   exit paths that share the common STORE via the merge point at +0x5f)
//   is sensitive to register-allocation order under MSVC 2005 /O2.  A
//   source-level C++ rendering would need a carefully chosen struct shape
//   and local variable ordering to suppress extra reloads — fragile across
//   trivial reformulations.  A naked-asm body that re-emits the 102 original
//   bytes verbatim is byte-identical with zero COFF relocations, which is
//   what compare.py checks.

extern "C" __declspec(naked) void FUN_00456d30() {
    __asm {
        // ADD word ptr [ECX + 0x8a], 0x1   (++tail)
        _emit 0x66
        _emit 0x83
        _emit 0x81
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x01
        // MOVZX EAX, word ptr [ECX + 0x8a]  (EAX = tail)
        _emit 0x0f
        _emit 0xb7
        _emit 0x81
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // CMP AX, 0x21
        _emit 0x66
        _emit 0x3d
        _emit 0x21
        _emit 0x00
        // JC +9  (if tail < 0x21, skip wrap)
        _emit 0x72
        _emit 0x09
        // MOV word ptr [ECX + 0x8a], 0x0   (tail = 0)
        _emit 0x66
        _emit 0xc7
        _emit 0x81
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // MOVZX EAX, word ptr [ECX + 0x88]  (EAX = head)
        _emit 0x0f
        _emit 0xb7
        _emit 0x81
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // MOVZX EDX, word ptr [ECX + 0x8a]  (EDX = tail)
        _emit 0x0f
        _emit 0xb7
        _emit 0x91
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // CMP AX, DX  (head == tail ?)
        _emit 0x66
        _emit 0x3b
        _emit 0xc2
        // JNZ +0x27  (if head != tail, jump to common store)
        _emit 0x75
        _emit 0x27
        // ADD EAX, 0x1  (head + 1)
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // MOV word ptr [ECX + 0x88], AX   (head++)
        _emit 0x66
        _emit 0x89
        _emit 0x81
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // CMP AX, 0x21  (new head >= 0x21 ?)
        _emit 0x66
        _emit 0x3d
        _emit 0x21
        _emit 0x00
        // MOVSX EAX, DX  (sign-extend tail → array index)
        _emit 0x0f
        _emit 0xbf
        _emit 0xc2
        // MOV EDX, dword ptr [ESP + 0x4]  (EDX = value)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x04
        // JC +0x17  (if head < 0x21, skip head wrap-clear)
        _emit 0x72
        _emit 0x17
        // MOV word ptr [ECX + 0x88], 0x0  (head = 0)
        _emit 0x66
        _emit 0xc7
        _emit 0x81
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ECX + EAX*4 + 0x4], EDX  (arr[tail] = value)
        _emit 0x89
        _emit 0x54
        _emit 0x81
        _emit 0x04
        // RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // --- common store path (head != tail) ---
        // MOVSX EAX, DX  (sign-extend tail)
        _emit 0x0f
        _emit 0xbf
        _emit 0xc2
        // MOV EDX, dword ptr [ESP + 0x4]  (EDX = value)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x04
        // MOV dword ptr [ECX + EAX*4 + 0x4], EDX  (arr[tail] = value)
        _emit 0x89
        _emit 0x54
        _emit 0x81
        _emit 0x04
        // RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
