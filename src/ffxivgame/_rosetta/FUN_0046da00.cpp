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
// FUNCTION: ffxivgame 0x0006da00 — asn1_do_adb (OpenSSL ASN.1 ADB dispatch)
//                                  (__cdecl, 153 B / 0x99)
//
// Signature (OpenSSL asn1_do_adb pattern):
//   __cdecl void* asn1_do_adb(void **pval, const void *tt, int nullerr)
//
//   arg0 (pval)    — pointer-to-value; its dereferenced content is used
//                    as the selector value after adding the ADB field offset
//   arg1 (tt)      — ASN1_TEMPLATE-like pointer; flags at [tt+0], dispatch
//                    table info at [tt+0xc]/[tt+0x10]/[tt+0x14]/[tt+0x18]
//   arg2 (nullerr) — if non-zero, call ERR_put_error when no match found
//
// Calling convention: __cdecl (plain RET; caller cleans args).
// Callee-saved registers used: ESI, EDI.
//
// Behaviour (recovered from asm @ 0x0006da00):
//
//   1. Load tt (arg1) into ESI.
//   2. If neither bit 8 nor bit 9 of ESI->flags is set (!(flags & 0x300)),
//      return tt (ESI) directly.
//   3. Otherwise, call through the function-pointer at ESI->field_0x10 to
//      resolve the ADB pointer; store result in EDI.
//   4. Compute selector = EDI->field_4 + *pval.
//      If selector == 0 (sum is zero), return EDI->field_0x18 (may be NULL).
//   5. If selector != 0:
//      a. If flags & 0x100: key = *selector; call FUN_00464d80(key) → EAX
//      b. Else:             key = *selector; call FUN_0046d2c0(key) → EAX
//      Search the table at EDI->field_0xc (stride 0x18, count EDI->field_0x10)
//      for an entry whose first dword equals EAX.
//   6. If found, return pointer to entry[1] (i.e., EDX+4).
//   7. If not found: check EDI->field_0x14 (default entry).
//      If default != NULL, return it.
//      If default == NULL and nullerr != 0, call ERR_put_error(13, 0x6e, 0xa4,
//      <file_str>, 0x115) then return NULL.
//      If default == NULL and nullerr == 0, return NULL.
//
// CALL targets (all REL32, wildcarded by compare.py):
//   +0x39   CALL FUN_00464d80  — keyed-table lookup (bit 8 path)
//   +0x43   CALL FUN_0046d2c0  — keyed-table lookup (non-bit-8 path)
//   +0x86   CALL FUN_0045c940  — ERR_put_error
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//   The function has two early-exit RET paths and two late-exit RET paths
//   joined at the epilogue, plus a backward loop. MSVC's register allocator
//   at /O2 would not reliably reproduce the exact register assignment
//   (ESI = tt throughout, EDI = ADB result, EDX = table pointer, ECX = loop
//   counter, EAX = lookup key/result) without subtle source-level steering.
//   Additionally, the PUSH 0xf79540 immediate (file-string VA baked at link
//   time) would be resolved differently in an isolated TU.
//   The naked-asm passthrough produces byte-identical .text modulo the three
//   CALL relocation windows (compare.py masks those via COFF reloc table).

extern "C" {
    int FUN_00464d80();    // keyed-table lookup (flags & 0x100 path)
    int FUN_0046d2c0();    // keyed-table lookup (non-0x100 path)
    int FUN_0045c940();    // ERR_put_error
}

extern "C" __declspec(naked) void FUN_0046da00() {
    __asm {
        // 0006da00: 56
        push    esi
        // 0006da01: 8b 74 24 0c
        mov     esi, dword ptr [esp + 0xc]
        // 0006da05: f7 06 00 03 00 00
        test    dword ptr [esi], 0x300
        // 0006da0b: 75 04
        jnz     short has_flags
        // 0006da0d: 8b c6
        mov     eax, esi
        // 0006da0f: 5e
        pop     esi
        // 0006da10: c3
        ret

    has_flags:
        // 0006da11: 8b 46 10
        mov     eax, dword ptr [esi + 0x10]
        // 0006da14: 57
        push    edi
        // 0006da15: ff d0
        call    eax
        // 0006da17: 8b 4c 24 0c
        mov     ecx, dword ptr [esp + 0xc]
        // 0006da1b: 8b f8
        mov     edi, eax
        // 0006da1d: 8b 47 04
        mov     eax, dword ptr [edi + 4]
        // 0006da20: 03 01
        add     eax, dword ptr [ecx]
        // 0006da22: 75 0a
        jnz     short nonzero_sel
        // 0006da24: 8b 47 18
        mov     eax, dword ptr [edi + 0x18]
        // 0006da27: 85 c0
        test    eax, eax
        // 0006da29: 74 41
        jz      short check_arg2
        // 0006da2b: 5f
        pop     edi
        // 0006da2c: 5e
        pop     esi
        // 0006da2d: c3
        ret

    nonzero_sel:
        // 0006da2e: f7 06 00 01 00 00
        test    dword ptr [esi], 0x100
        // 0006da34: 74 0a
        jz      short use_d2c0
        // 0006da36: 8b 10
        mov     edx, dword ptr [eax]
        // 0006da38: 52
        push    edx
        // 0006da39: e8 42 73 ff ff
        call    FUN_00464d80
        // 0006da3e: eb 08
        jmp     short after_lookup

    use_d2c0:
        // 0006da40: 8b 00
        mov     eax, dword ptr [eax]
        // 0006da42: 50
        push    eax
        // 0006da43: e8 78 f8 ff ff
        call    FUN_0046d2c0

    after_lookup:
        // 0006da48: 8b 77 10
        mov     esi, dword ptr [edi + 0x10]
        // 0006da4b: 8b 57 0c
        mov     edx, dword ptr [edi + 0xc]
        // 0006da4e: 83 c4 04
        add     esp, 4
        // 0006da51: 33 c9
        xor     ecx, ecx
        // 0006da53: 85 f6
        test    esi, esi
        // 0006da55: 7e 0e
        jle     short no_entries

    search_loop:
        // 0006da57: 39 02
        cmp     dword ptr [edx], eax
        // 0006da59: 74 38
        jz      short found_entry
        // 0006da5b: 83 c1 01
        add     ecx, 1
        // 0006da5e: 83 c2 18
        add     edx, 0x18
        // 0006da61: 3b ce
        cmp     ecx, esi
        // 0006da63: 7c f2
        jl      short search_loop

    no_entries:
        // 0006da65: 8b 47 14
        mov     eax, dword ptr [edi + 0x14]
        // 0006da68: 85 c0
        test    eax, eax
        // 0006da6a: 75 24
        jnz     short adb_epilogue

    check_arg2:
        // 0006da6c: 83 7c 24 14 00
        cmp     dword ptr [esp + 0x14], 0x0
        // 0006da71: 74 1b
        jz      short return_null
        // 0006da73: 68 15 01 00 00
        push    0x115
        // 0006da78: 68 40 95 f7 00
        push    0xf79540
        // 0006da7d: 68 a4 00 00 00
        push    0xa4
        // 0006da82: 6a 6e
        push    0x6e
        // 0006da84: 6a 0d
        push    0xd
        // 0006da86: e8 b5 ee fe ff
        call    FUN_0045c940
        // 0006da8b: 83 c4 14
        add     esp, 0x14

    return_null:
        // 0006da8e: 33 c0
        xor     eax, eax

    adb_epilogue:
        // 0006da90: 5f
        pop     edi
        // 0006da91: 5e
        pop     esi
        // 0006da92: c3
        ret

    found_entry:
        // 0006da93: 5f
        pop     edi
        // 0006da94: 8d 42 04
        lea     eax, [edx + 4]
        // 0006da97: 5e
        pop     esi
        // 0006da98: c3
        ret
    }
}
