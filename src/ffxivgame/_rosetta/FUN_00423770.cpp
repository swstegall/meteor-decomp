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
// FUNCTION: ffxivgame 0x00423770 — table-entry verify-or-sync helper
//           (__thiscall, 87 bytes)
//
// Calling convention: __thiscall
//   ECX   = this (used as base for LEA into the table at offset 0x1140)
//   [ESP+0x4]  = arg1 (int, table-bank index; must be < 16)
//   [ESP+0x8]  = arg2 (int*, source array pointer)
//   [ESP+0xC]  = arg3 (unsigned int, element count)
//   callee cleans 12 bytes via RET 0xC.
//   Returns: AL=1 (true) if arg1 >= 16 or all 'count' table entries match
//            the pre-loaded arr[0]; AL=0 (false) after a mismatch triggers
//            the memmove-and-update path.
//
// Shape overview:
//
//   1. Load arg1 → EAX; if EAX >= 16 return 1 immediately.
//   2. Compute EDX = this + 0x1140 + arg1*4 (pointer into per-bank table).
//   3. Load arg3 (count) → ECX; load arg2 (array ptr) → ESI.
//   4. Pre-load EDI = *ESI (arr[0]); then loop:
//        compare [EDX] vs EDI; on match advance EDX/ESI/counter and repeat
//        (the backward JC re-enters at the CMP, NOT at the MOV EDI, so EDI
//        is only ever loaded once — the loop compares all table[i] against
//        the initial arr[0] value).
//   5. All matched → return 1.
//   6. Mismatch → memmove(table_ptr, arr_ptr, remaining*4) then return 0.
//
// The backward jump at offset 0x36 (`72 ef`, JC -0x11) targets offset 0x27
// (the CMP instruction), skipping the MOV EDI at offset 0x25.  This
// structural quirk must be preserved byte-for-byte; placing the `loop_body`
// label at the CMP instruction achieves the correct jump displacement.
//
// The CALL at offset 0x48 targets VA 0x009d4600 (memmove-like CRT helper).
// It is a single REL32 relocation; tools/compare.py masks it in the diff.

extern "C" void FUN_009d4600();   // memmove-like helper at VA 0x009d4600

extern "C" __declspec(naked) void FUN_00423770() {
    __asm {
        // ── bounds check: if arg1 >= 16, return true immediately ──────────
        mov     eax, dword ptr [esp + 4]
        cmp     eax, 0x10
        jc      main_body
        mov     al, 1
        ret     0xC

        // ── main body ─────────────────────────────────────────────────────
    main_body:
        // EDX = this + 0x1140 + arg1*4  (ECX still holds 'this' here)
        lea     edx, [ecx + eax*4 + 0x1140]
        // Load arg3 (count) into ECX, overwriting 'this' (no longer needed)
        mov     ecx, dword ptr [esp + 0xC]
        push    esi
        // Load arg2 (array ptr) into ESI. After PUSH ESI, [esp+0xC] == arg2.
        mov     esi, dword ptr [esp + 0xC]
        xor     eax, eax                    // EAX = loop counter = 0
        test    ecx, ecx                    // set flags on count
        push    edi
        jbe     done_ok                     // if count == 0, return true

        // Pre-loop load: EDI = arr[0]. The backward branch re-enters at
        // loop_body (CMP), so this load happens only once.
        mov     edi, dword ptr [esi]

    loop_body:
        cmp     dword ptr [edx], edi        // table[i] == EDI?
        jnz     mismatch
        add     eax, 1                      // i++
        add     edx, 4                      // advance table ptr
        add     esi, 4                      // advance array ptr
        cmp     eax, ecx                    // i < count?
        jc      loop_body                   // loop (back to CMP, not MOV EDI)

    done_ok:
        pop     edi
        mov     al, 1                       // return true
        pop     esi
        ret     0xC

    mismatch:
        // ECX = remaining = count - i; multiply by 4 for byte count
        sub     ecx, eax
        add     ecx, ecx
        add     ecx, ecx
        push    ecx                         // arg3: byte count
        push    esi                         // arg2: source (&arr[i])
        push    edx                         // arg1: dest  (&table[i])
        call    FUN_009d4600                // memmove(table+i, arr+i, remain*4)
        add     esp, 0xC
        pop     edi
        xor     al, al                      // return false
        pop     esi
        ret     0xC
    }
}
