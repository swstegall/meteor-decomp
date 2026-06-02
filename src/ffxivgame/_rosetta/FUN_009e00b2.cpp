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
// FUNCTION: ffxivgame 0x005e00b2 — resource unregister/close helper
//                                  (__cdecl int FUN_009e00b2(int idx), 151 B / 0x97)
//
// Calling convention: __cdecl (RET with no cleanup); one int argument.
// Callee-saves pushed: ESI (holds arg), EDI (holds handle/result).
//
// Behaviour (recovered from asm @ 0x005e00b2):
//
//   int FUN_009e00b2(int idx) {
//       if (FUN_009e777c(idx) == -1) goto fail;
//
//       void **arr = (void **)data_0137b7e0;   // global pointer array
//       if (idx == 1) {
//           if (((unsigned char *)arr)[0x84] & 1) goto do_check;
//       }
//       if (idx == 2) {
//           if (((unsigned char *)arr)[0x44] & 1) goto do_check;
//       }
//       goto no_check;
//
//   do_check:
//       int h2 = FUN_009e777c(2);
//       int h1 = FUN_009e777c(1);
//       if (h1 == h2) goto fail;
//
//   no_check:
//       int handle = FUN_009e777c(idx);
//       if (ext_f3e1ec(handle) != 0) goto fail;   // some SetHandle/Register fn
//       int result = ext_f3e1c4();                 // some GetHandle fn
//       goto merge;
//
//   fail:
//       result = 0;
//
//   merge:
//       FUN_009e76fb(idx);                         // deref/release slot
//
//       // locate the entry in a two-level indexed bit-field array:
//       // arr_base[(idx >> 5)] is the page pointer;
//       // ((idx & 0x1f) << 6) is the byte offset within the page.
//       void *page = ((void **)data_0137b7e0)[idx >> 5];
//       int  off   = (idx & 0x1f) << 6;
//       ((char *)page)[off + 4] = 0;               // clear flag byte
//
//       if (result != 0) {
//           FUN_009d9d6d(result);
//           return -1;
//       }
//       return 0;
//   }
//
// CALL targets (all REL32/IAT, wildcarded by tools/compare.py):
//   +0x07   CALL FUN_009e777c   — repeated lookup helper
//   +0x32   CALL FUN_009e777c
//   +0x3b   CALL FUN_009e777c
//   +0x47   CALL FUN_009e777c
//   +0x4e   CALL [ext_f3e1ec]   — IAT: some register/set handle
//   +0x58   CALL [ext_f3e1c4]   — IAT: some get handle
//   +0x65   CALL FUN_009e76fb   — slot release helper
//   +0x87   CALL FUN_009d9d6d   — cleanup/close fn
//
// Globals (DIR32, wildcarded):
//   data_0137b7e0 — global pointer-array base (accessed via A1 + SIB forms)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//   The function uses two distinct encodings for the same data symbol
//   (A1 moffs32 for the initial load and 8B/SIB*4 for the indexed lookup),
//   combined with an interleaved CMP/MOV sequence that MSVC's scheduler
//   emits in the full-binary build context but would not spontaneously
//   reproduce in an isolated TU.  Naked asm gives byte-exact output modulo
//   the relocation windows that compare.py wildcards.

extern "C" {
    // .text — internal direct-call targets (REL32 relocations).
    int FUN_009e777c();    // lookup helper; called with various int args
    int FUN_009e76fb();    // slot release helper
    int FUN_009d9d6d();    // cleanup / close

    // .idata — IAT slots. Declared as plain `int` so MASM emits
    // `ff 15 ?? ?? ?? ??` with a DIR32 reloc on the slot address.
    extern int ext_f3e1ec; // some register / set-handle API
    extern int ext_f3e1c4; // some get-handle API

    // .data — global pointer-array base (array of void * at this address).
    extern int data_0137b7e0;
}

extern "C" __declspec(naked) int FUN_009e00b2() {
    __asm {
        // --- prologue ---------------------------------------------------
        push    esi                                 // 56
        mov     esi, dword ptr [esp + 8]            // 8b 74 24 08   arg0 → ESI
        push    edi                                 // 57

        // if (FUN_009e777c(idx) == -1) goto fail
        push    esi                                 // 56
        call    FUN_009e777c                        // e8 ?? ?? ?? ??
        cmp     eax, -1                             // 83 f8 ff
        pop     ecx                                 // 59
        jz      L_fail                              // 74 50

        // CMP + global load are interleaved by the scheduler
        cmp     esi, 1                              // 83 fe 01
        mov     eax, [data_0137b7e0]                // a1 ?? ?? ?? ??   A1 moffs32
        jnz     check_2                             // 75 09

        // idx == 1: check arr[0x84] & 1
        test    byte ptr [eax + 0x84], 1            // f6 80 84 00 00 00 01
        jnz     do_check                            // 75 0b

    check_2:
        // idx == 2: check arr[0x44] & 1
        cmp     esi, 2                              // 83 fe 02
        jnz     no_check                            // 75 1c
        test    byte ptr [eax + 0x44], 1            // f6 40 44 01
        jz      no_check                            // 74 16

    do_check:
        // h2 = FUN_009e777c(2); h1 = FUN_009e777c(1); if h1 == h2 goto fail
        push    2                                   // 6a 02
        call    FUN_009e777c                        // e8 ?? ?? ?? ??
        push    1                                   // 6a 01
        mov     edi, eax                            // 8b f8   EDI = h2
        call    FUN_009e777c                        // e8 ?? ?? ?? ??
        cmp     eax, edi                            // 3b c7
        pop     ecx                                 // 59
        pop     ecx                                 // 59
        jz      L_fail                              // 74 1c

    no_check:
        // handle = FUN_009e777c(idx); then IAT calls
        push    esi                                 // 56
        call    FUN_009e777c                        // e8 ?? ?? ?? ??
        pop     ecx                                 // 59
        push    eax                                 // 50
        call    dword ptr [ext_f3e1ec]              // ff 15 ?? ?? ?? ??
        test    eax, eax                            // 85 c0
        jnz     L_fail                              // 75 0a
        call    dword ptr [ext_f3e1c4]              // ff 15 ?? ?? ?? ??
        mov     edi, eax                            // 8b f8   EDI = result handle
        jmp     L_continue                          // eb 02

    L_fail:
        xor     edi, edi                            // 33 ff

    L_continue:
        // FUN_009e76fb(idx) — release slot
        push    esi                                 // 56
        call    FUN_009e76fb                        // e8 ?? ?? ?? ??

        // Indexed lookup: page = arr[idx >> 5]; off = (idx & 0x1f) << 6
        mov     eax, esi                            // 8b c6
        sar     eax, 5                              // c1 f8 05
        mov     eax, [eax*4 + data_0137b7e0]        // 8b 04 85 ?? ?? ?? ??  SIB*4
        and     esi, 0x1f                           // 83 e6 1f
        shl     esi, 6                              // c1 e6 06
        test    edi, edi                            // 85 ff
        pop     ecx                                 // 59
        mov     byte ptr [eax + esi*1 + 4], 0       // c6 44 30 04 00

        jz      L_zero                              // 74 0c

        // result != 0: close handle, return -1
        push    edi                                 // 57
        call    FUN_009d9d6d                        // e8 ?? ?? ?? ??
        pop     ecx                                 // 59
        or      eax, 0xffffffff                     // 83 c8 ff
        jmp     L_end                               // eb 02

    L_zero:
        xor     eax, eax                            // 33 c0

    L_end:
        // --- epilogue ---------------------------------------------------
        pop     edi                                 // 5f
        pop     esi                                 // 5e
        ret                                         // c3
    }
}
