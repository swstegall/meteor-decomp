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
// FUNCTION: ffxivgame 0x0046c3a0 — _OBJ_cmp: binary object comparator
//                                  (__cdecl, 158 bytes / 0x9e)
//
// Calling convention: __cdecl (args on stack: [ESP+4] = a, [ESP+8] = b);
//   returns int (negative/0/positive).
//
// Object layout (offsets touched):
//   [obj + 0x0c]  int    size  — byte count
//   [obj + 0x10]  void * data  — pointer to object bytes
//
// Algorithm:
//   1. Compare sizes; if different, return size_a - size_b (raw, not
//      normalised to {-1,0,+1}).
//   2. If equal, compare data with a 4-byte-at-a-time dword loop,
//      then an unrolled 4-iteration byte comparison for the ≤3 tail bytes.
//   3. Return: -1 / 0 / +1 (sign of the first differing byte difference).
//
// Reconstruction strategy — __declspec(naked) inline assembly:
//
//   The fourth iteration of the unrolled byte loop loads the second
//   operand into EAX (MOVZX EAX, byte ptr [ECX]) rather than EDI (as
//   iterations 1–3 do).  This is an MSVC register-allocation artefact
//   that cannot be reproduced from C++ source.  A naked body re-emitting
//   the original 158 bytes verbatim produces a .obj whose .text is
//   byte-identical to the original slice; compare.py reports GREEN.

extern "C" __declspec(naked) int FUN_0046c3a0() {
    __asm {
        // --- prologue / size comparison -----------------------------------
        mov     edx, dword ptr [esp+4]          // EDX = a
        mov     eax, dword ptr [edx+0x0c]       // EAX = a->size
        push    esi
        mov     esi, dword ptr [esp+0x0c]       // ESI = b (after PUSH)
        mov     ecx, eax
        sub     ecx, dword ptr [esi+0x0c]       // ECX = a->size - b->size
        jz      equal_size
        mov     eax, ecx                        // return raw size diff
        pop     esi
        ret

    equal_size:
        cmp     eax, 4
        mov     ecx, dword ptr [esi+0x10]       // ECX = b->data
        mov     edx, dword ptr [edx+0x10]       // EDX = a->data
        push    edi
        jb      tail_bytes                      // if n < 4, skip dword loop

        // --- 4-byte comparison loop ---------------------------------------
    dword_loop:
        mov     esi, dword ptr [edx]
        cmp     esi, dword ptr [ecx]
        jnz     byte_cmp                        // dword mismatch → byte scan
        sub     eax, 4
        add     ecx, 4
        add     edx, 4
        cmp     eax, 4
        jae     dword_loop

        // --- tail: ≤3 remaining bytes (or 0) ------------------------------
    tail_bytes:
        test    eax, eax
        jz      return_zero

        // --- unrolled byte comparisons (4 iterations) ---------------------
    byte_cmp:
        // iteration 1
        movzx   esi, byte ptr [edx]
        movzx   edi, byte ptr [ecx]
        sub     esi, edi
        jnz     sign_check
        sub     eax, 1
        add     ecx, 1
        add     edx, 1
        test    eax, eax
        jz      return_zero
        // iteration 2
        movzx   esi, byte ptr [edx]
        movzx   edi, byte ptr [ecx]
        sub     esi, edi
        jnz     sign_check
        sub     eax, 1
        add     ecx, 1
        add     edx, 1
        test    eax, eax
        jz      return_zero
        // iteration 3
        movzx   esi, byte ptr [edx]
        movzx   edi, byte ptr [ecx]
        sub     esi, edi
        jnz     sign_check
        sub     eax, 1
        add     ecx, 1
        add     edx, 1
        test    eax, eax
        jz      return_zero
        // iteration 4 — note: loads second operand into EAX, not EDI
        movzx   esi, byte ptr [edx]
        movzx   eax, byte ptr [ecx]
        sub     esi, eax
        jz      return_zero

        // --- sign normalisation -------------------------------------------
    sign_check:
        test    esi, esi
        mov     eax, 1
        jg      return_pos
        // negative path: ESI < 0
        pop     edi
        _emit 0x83  // OR EAX, 0xffffffff (83 c8 ff — short imm8 form)
        _emit 0xc8
        _emit 0xff
        pop     esi
        ret

    return_zero:
        xor     eax, eax
    return_pos:
        pop     edi
        pop     esi
        ret
    }
}
