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
// FUNCTION: ffxivgame 0x00413550 — intrusive-list count via sentinel #2
//
// __thiscall int Outer::count2()
//   ECX : this  (Outer*)
//   EAX : return value (element count)
//
// Asm shape (27 bytes, RVA 0x00013550):
//
//   8b 51 04       MOV  EDX,[ECX+0x4]       ; EDX = this->m_block
//   8b 4a 4c       MOV  ECX,[EDX+0x4c]      ; ECX = block->sentinel2.prev (cur)
//   83 c2 44       ADD  EDX,0x44            ; EDX = &block->sentinel2 (sentry)
//   33 c0          XOR  EAX,EAX             ; n = 0
//   3b ca          CMP  ECX,EDX             ; cur == sentry?
//   74 0b          JZ   exit                ; if empty list, return 0
//   90             NOP                      ; loop alignment to 0x413560
//   8b 49 08       MOV  ECX,[ECX+0x8]       ; cur = cur->prev  (loop top)
//   83 c0 01       ADD  EAX,0x1             ; n++
//   3b ca          CMP  ECX,EDX             ; cur == sentry?
//   75 f6          JNZ  loop top            ; if not, continue
//   exit:
//   c3             RET
//
// This is a size() walker on an intrusive doubly-linked sentinel list.
// `this->m_block` (at +4) points to a DetachableHeapBlock-family object
// whose second embedded-Link sentinel lives at offset +0x44. The sentinel's
// field at +8 (= block+0x4c) is the traversal start; the loop follows the
// same field (+8) through each node until it loops back to the sentinel,
// counting one step per element.
//
// Register allocation (MSVC 2005 /O2 /Oy; no frame, leaf function):
//   EDX = b (m_block → then modified in-place to sentry = b+0x44)
//   ECX = cur (reuses the `this` register after m_block is loaded)
//   EAX = n  (return-value register)
//
// The NOP at +0x0f aligns the loop entry point (0x413560) to a 16-byte
// boundary (MSVC /O2 loop alignment heuristic).
//
// Naked passthrough — the C++ source form (while (cur != sentry) with
// sentry = &b->sentinel2, cur starting at b->sentinel2.prev) generates
// this exact register allocation only when MSVC assigns EDX to `b`; the
// inline asm guarantees the correct encoding without register-alloc drift.

extern "C" __declspec(naked) void FUN_00413550()
{
    __asm {
        mov  edx, dword ptr [ecx + 0x4]   // EDX = this->m_block
        mov  ecx, dword ptr [edx + 0x4c]  // ECX = block->sentinel2.prev
        add  edx, 0x44                    // EDX = &block->sentinel2
        xor  eax, eax                     // n = 0
        cmp  ecx, edx                     // cur == sentry?
        jz   done                         // if empty, return 0
        nop                               // align loop to 16 B (0x413560)
    loop_:
        mov  ecx, dword ptr [ecx + 0x8]   // cur = cur->field8
        add  eax, 1                       // n++
        cmp  ecx, edx                     // cur == sentry?
        jnz  loop_                        // if not, continue
    done:
        ret
    }
}
