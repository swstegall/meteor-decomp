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
// FUNCTION: ffxivgame 0x00062600 — _DIST_POINT_set_dpname (__cdecl, 180 B / 0xb4)
//
// Sets the dpname field on a DIST_POINT_NAME from an X509_NAME source.
// Checks that dpn != NULL and dpn->type == 1 (relative name), then
// duplicates the input X509_NAME, stores it in dpn->dpname, and
// iterates over each entry in dpn->relativename to add it into the
// duplicated name. Finally calls a sort/canonicalise function. Returns
// 1 on success (or when preconditions aren't met); 0 on allocation or
// add failure (with dpname cleaned up).
//
//   __cdecl int FUN_00462600(
//       void *dpn,    // [ESP+0x8] after PUSH EDI — DIST_POINT_NAME *
//       void *iname   // [ESP+0xc] after PUSH EDI — X509_NAME *
//   );
//
// Inferred layout of dpn (DIST_POINT_NAME):
//   +0x00  int   type            (must equal 1)
//   +0x04  void *relativename   (STACK_OF(X509_NAME_ENTRY) *)
//   +0x08  void *dpname         (X509_NAME * — the field being set)
//
// Register allocation (matches orig):
//   EDI = dpn (param1)
//   EBX = dpn->relativename (+0x04), held across loop
//   ESI = loop index i (starts at 0, increments by 1)
//
// External functions (CALL rel32 — all wildcarded by compare.py):
//   FUN_0045de20  @+0x20  X509_NAME_dup(iname)
//   FUN_00464030  @+0x36  sk_X509_NAME_ENTRY_num(sk)
//   FUN_00464040  @+0x44  sk_X509_NAME_ENTRY_value(sk, i)
//   FUN_0046eac0  @+0x58  X509_NAME_add_entry(name, entry, -1, (i==0)?1:0)
//   FUN_00464030  @+0x68  sk_X509_NAME_ENTRY_num(sk)  [re-query inside loop]
//   FUN_0045ddd0  @+0x7a  X509_NAME_sort(name, 0)
//   FUN_0045de00  @+0x8a  X509_NAME_free(name)
//
// Branch shape:
//   Near JZ  (0F 84) at +0x07 — target at +0xAD (160 bytes, > ±127)
//   Near JNZ (0F 85) at +0x10 — target at +0xAD (151 bytes, > ±127)
//   All other branches encode as short Jcc (2-byte).

extern "C" {
    int FUN_0045de20();  // X509_NAME_dup
    int FUN_00464030();  // sk_X509_NAME_ENTRY_num
    int FUN_00464040();  // sk_X509_NAME_ENTRY_value
    int FUN_0046eac0();  // X509_NAME_add_entry
    int FUN_0045ddd0();  // X509_NAME_sort
    int FUN_0045de00();  // X509_NAME_free
}

extern "C" __declspec(naked) void FUN_00462600() {
    __asm {
        // --- prologue: save EDI, load param1 --------------------------------
        push    edi
        mov     edi, dword ptr [esp+0x8]    // EDI = dpn (param1)
        test    edi, edi
        jz      early_return                // near JZ (0F 84) — >127 bytes away
        cmp     dword ptr [edi], 0x1
        jnz     early_return                // near JNZ (0F 85) — >127 bytes away

        // --- duplicate iname → dpn->dpname ----------------------------------
        mov     eax, dword ptr [esp+0xc]    // EAX = iname (param2)
        push    ebx
        mov     ebx, dword ptr [edi+0x4]    // EBX = dpn->relativename
        push    eax                         // arg: iname
        call    FUN_0045de20                // X509_NAME_dup(iname)
        add     esp, 0x4
        test    eax, eax
        mov     dword ptr [edi+0x8], eax    // dpn->dpname = result
        jnz     continue_after_dup          // short JNZ — non-zero → continue
        pop     ebx
        pop     edi
        ret                                 // return 0 (EAX=0 from dup failure)

        // --- get initial count, loop ----------------------------------------
    continue_after_dup:
        push    esi
        push    ebx                         // arg: relativename
        xor     esi, esi                    // ESI = i = 0
        call    FUN_00464030                // sk_num(relativename)
        add     esp, 0x4
        test    eax, eax
        jle     after_loop                  // count <= 0 → skip loop

    loop_top:
        push    esi                         // arg2: i (index)
        push    ebx                         // arg1: relativename
        call    FUN_00464040                // sk_value(relativename, i) → EAX=entry
        mov     edx, dword ptr [edi+0x8]   // EDX = dpn->dpname
        xor     ecx, ecx
        test    esi, esi
        setz    cl                          // CL = (i == 0) ? 1 : 0
        push    ecx                         // arg4: (i==0) flag
        push    -1                          // arg3: loc = -1
        push    eax                         // arg2: entry
        push    edx                         // arg1: dpn->dpname
        call    FUN_0046eac0                // X509_NAME_add_entry(dpname,entry,-1,flag)
        add     esp, 0x18                   // clean 6 pushes (sk_value args + add_entry args)
        test    eax, eax
        jz      fail2                       // add failed → free dpname, return 0
        push    ebx                         // arg: relativename
        add     esi, 0x1                    // i++
        call    FUN_00464030                // sk_num(relativename) [re-query count]
        add     esp, 0x4
        cmp     esi, eax
        jl      loop_top                    // i < count → continue

        // --- sort/canonicalise the populated name ----------------------------
    after_loop:
        mov     ecx, dword ptr [edi+0x8]   // ECX = dpn->dpname
        push    0x0                         // arg2: 0
        push    ecx                         // arg1: dpname
        call    FUN_0045ddd0                // X509_NAME_sort(dpname, 0)
        add     esp, 0x8
        test    eax, eax
        jge     success                     // result >= 0 → success
        mov     edx, dword ptr [edi+0x8]   // EDX = dpn->dpname (fail path)
        push    edx                         // arg: dpname to free

        // --- shared free+cleanup tail (reached from fail path AND fail2) ----
    do_free:
        call    FUN_0045de00                // X509_NAME_free(dpname)
        add     esp, 0x4
        pop     esi
        pop     ebx
        mov     dword ptr [edi+0x8], 0x0   // dpn->dpname = NULL
        xor     eax, eax
        pop     edi
        ret                                 // return 0

        // --- fail2: add_entry returned 0 inside loop ------------------------
    fail2:
        mov     eax, dword ptr [edi+0x8]   // EAX = dpn->dpname
        push    eax                         // arg: dpname to free
        jmp     do_free                     // short backward JMP (EB E5)

        // --- success: sort succeeded ----------------------------------------
    success:
        pop     esi
        pop     ebx
        mov     eax, 0x1
        pop     edi
        ret

        // --- early return: precondition not met (dpn==NULL or type!=1) ------
    early_return:
        mov     eax, 0x1
        pop     edi
        ret
    }
}
