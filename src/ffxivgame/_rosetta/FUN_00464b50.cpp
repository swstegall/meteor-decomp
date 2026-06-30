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
// FUNCTION: ffxivgame 0x00064b50 — keyed value lookup from a static table with
//                                  OpenSSL-style fallback for out-of-range keys
//                                  (146 B / 0x92, __cdecl, 1 arg).
//
// Signature: __cdecl int FUN_00464b50(unsigned int key)
//
// Allocates a 0x20-byte local frame via __chkstk (MOV EAX,0x20 / CALL 009d29d0).
// After allocation, arg is at [ESP+0x24].
//
// Fast path (0 <= key <= 0x37c):
//   - If key == 0: skip validity check, load table[0].value directly.
//   - If key  > 0: check table[key].valid_flag (at base+4 within the 24-byte
//     entry). If zero → fall into error path with code 0x16a.
//     If non-zero: load table[key].value (at base+0 within entry), return it.
//
// Table layout:
//   base address: 0xf70bac
//   stride:       24 bytes (ECX = key*3, then [ECX*8 + base])
//   field[0] at base+0x00 — return value
//   field[1] at base+0x04 — validity flag
//
// Out-of-range path (key > 0x37c):
//   Load global manager pointer from [0x0132e7b8].
//   If null → return 0.
//   Otherwise build a 5-field lookup record in the local frame:
//     local[0]  = 3         (record type)
//     local[1]  = &local[2] (next-pointer)
//     local[2-3] = uninitialised
//     local[4]  = key       (lookup key stored at ESP+0x10)
//   Call FUN_00466b60(manager, &local[0]).
//   If result == 0 → fall to error path with code 0x17b.
//   Otherwise return result->field4->field4.
//
// Error path:
//   Calls FUN_0045c940 (ERR_put_error) with:
//     lib=8, func=0x66, reason=0x65, file=0xf78830, line=0x16a or 0x17b
//   Then returns 0.
//
// CALL targets (REL32, wildcarded by compare.py):
//   +0x05  CALL FUN_009d29d0  — __chkstk / stack probe
//   +0x5e  CALL FUN_00466b60  — registry lookup helper
//   +0x84  CALL FUN_0045c940  — ERR_put_error (OpenSSL error recorder)

extern "C" {
    void FUN_009d29d0();   // __chkstk / stack frame allocator
    int  FUN_00466b60();   // registry lookup (manager, record) -> node*
    void FUN_0045c940();   // ERR_put_error(lib, func, reason, file, line)
}

extern "C" __declspec(naked) int FUN_00464b50() {
    __asm {
        // --- prologue: allocate 0x20-byte local frame ---
        mov     eax, 0x20
        call    FUN_009d29d0

        // --- load arg (now at ESP+0x24 after frame alloc) ---
        mov     eax, dword ptr [esp + 0x24]
        cmp     eax, 0x37c
        ja      short out_of_range

        // --- fast path: key in [0, 0x37c] ---
        test    eax, eax
        jz      short get_value              // key==0: skip validity check

        lea     ecx, [eax + eax*2]           // ecx = key * 3
        cmp     dword ptr [ecx*8 + 0xf70bb0], 0  // table[key].valid_flag == 0?
        jnz     short get_value              // non-zero: valid entry

        // invalid entry → error with line 0x16a
        push    0x16a
        jmp     short error_common           // join common error tail

    get_value:
        lea     edx, [eax + eax*2]           // edx = key * 3
        mov     eax, dword ptr [edx*8 + 0xf70bac]  // EAX = table[key].value
        add     esp, 0x20
        ret

        // --- out-of-range path: key > 0x37c ---
    out_of_range:
        // mov ecx, dword ptr [0x0132e7b8]  — 8b 0d b8 e7 32 01
        // MASM inline asm emits MOV ECX,imm32 (b9) for [abs_addr] literals;
        // force the correct 6-byte memory-load encoding with _emit.
        _emit 0x8b
        _emit 0x0d
        _emit 0xb8
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        test    ecx, ecx
        jz      short return_zero            // null → return 0

        // build lookup record in local frame and call registry helper
        mov     dword ptr [esp + 0x10], eax  // local[4] = key
        lea     eax, [esp]                   // eax = &local[0]
        push    eax                          // arg2: &local[0]
        lea     edx, [esp + 0xc]             // edx = &local[2] (relative to pre-push ESP)
        push    ecx                          // arg1: manager ptr
        mov     dword ptr [esp + 0x8], 3    // local[0] = 3 (record type)
        mov     dword ptr [esp + 0xc], edx  // local[1] = &local[2]
        call    FUN_00466b60
        add     esp, 0x8

        test    eax, eax
        jz      short error_path2            // null result → error 0x17b

        // success: return result->field4->field4
        mov     ecx, dword ptr [eax + 4]
        mov     eax, dword ptr [ecx + 4]
        add     esp, 0x20
        ret

        // error: lookup returned null → push line 0x17b then fall through
    error_path2:
        push    0x17b

        // common error tail (entered with line code already on stack)
    error_common:
        push    0xf78830                     // file string pointer
        push    0x65                         // reason
        push    0x66                         // func
        push    0x8                          // lib
        call    FUN_0045c940                 // ERR_put_error
        add     esp, 0x14                    // pop 5 args (line + 4 more)

    return_zero:
        xor     eax, eax
        add     esp, 0x20
        ret
    }
}
