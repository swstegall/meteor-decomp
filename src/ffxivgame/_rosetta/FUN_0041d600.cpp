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
// FUNCTION: ffxivgame 0x0001d600 — FUN_0041d600 (__cdecl, 153 B / 0x99)
//
// Two-argument __cdecl function (arg1, arg2 = pointers to objects with fields
// at offsets 0x20 and 0x28).
//
// Shape:
//   1. One-time guard: call FUN_00418280(); if true, test+set g_01323910 bit-0,
//      write FUN_0041d3a0 into g_0132390c, then call through that pointer with
//      5 string/int args (logging/assert initialisation).
//   2. if (!arg1) return;
//   3. if (!arg2) return;
//   4. cmp = FUN_004332a0(arg1[0x20]); if (cmp > 0) return;
//   5. Call FUN_009fc74a(0, s_f59578)  — import thunk (__stdcall, 2 args).
//   6. Load arg2[0x28] into ECX, arg1[0x28] into EDX.
//      Set ECX = g_0132987c (global 'this').
//      Call FUN_00423310 (__thiscall, 5 args: EDX, 0, [old ECX], 0, 2).
//   7. POP EDI; POP ESI; JMP FUN_009fc750  — tail call to import thunk.
//
// Unusual codegen notes:
//   - ESI and EDI are saved AFTER the guard block, not at the standard prologue.
//     MSVC deferred them because the guard block only uses EAX + memory.
//   - MOV dword ptr [g_0132390c], OFFSET FUN_0041d3a0  carries two COFF
//     relocations (one for the address of g_0132390c, one for the value
//     FUN_0041d3a0); compare.py wildcards both 4-byte reloc windows.
//   - The tail call is `E9 XX XX XX XX` (near JMP rel32 to the import thunk
//     FUN_009fc750); not a CALL+RET pair.
//   - All CALL REL32, JMP REL32, PUSH imm32 (string/data pointers), and
//     indirect CALL dword ptr [g_0132390c] and MOV r32,[g_0132987c] sites
//     carry COFF relocations that compare.py wildcards.
//
// Verified byte layout (153 bytes, all jump offsets confirmed):
//   0x07  JZ  +0x3f → skip_guard (0x48)
//   0x14  JNZ +0x10 → skip_init  (0x26)
//   0x4f  JZ  +0x46 → done_pop_esi (0x97)
//   0x58  JZ  +0x3c → done_pop_both (0x96)
//   0x68  JG  +0x2c → done_pop_both (0x96)

extern "C" {
    // Direct-call (REL32) targets
    int  FUN_00418280();      // guard-check: returns bool in AL
    int  FUN_004332a0();      // comparison helper (1 arg, __cdecl)
    int  FUN_009fc74a();      // import thunk: __stdcall 2-arg acquire
    void FUN_0041d3a0();      // log/format handler registered into g_0132390c
    int  FUN_00423310();      // __thiscall method (5 stack args, ECX = this)
    void FUN_009fc750();      // import thunk: tail-call target

    // Global data (DIR32 / memory-direct relocs)
    extern int g_01323910;    // one-time init flag  (0x01323910)
    extern int g_0132390c;    // function pointer    (0x0132390c)
    extern int g_0132987c;    // object pointer      (0x0132987c)

    // String literals in .rdata (each PUSH emits a DIR32 reloc)
    extern char s_f594c0[];   // arg5 of log call
    extern char s_f59510[];   // arg3 of log call
    extern char s_f58bae[];   // arg2 of log call
    extern char s_f59560[];   // arg1 of log call
    extern char s_f59578[];   // arg1 of FUN_009fc74a call
}

extern "C" __declspec(naked) void FUN_0041d600() {
    __asm {
        // 0001d600: e8 7b ac ff ff   CALL FUN_00418280
        call    FUN_00418280
        // 0001d605: 84 c0            TEST AL,AL
        test    al, al
        // 0001d607: 74 3f            JZ skip_guard  (offset +0x3f)
        jz      skip_guard
        // 0001d609: b8 01 00 00 00   MOV EAX,1
        mov     eax, 1
        // 0001d60e: 84 05 <reloc>    TEST byte ptr [g_01323910],AL
        test    byte ptr [g_01323910], al
        // 0001d614: 75 10            JNZ skip_init  (offset +0x10)
        jnz     skip_init
        // 0001d616: 09 05 <reloc>    OR dword ptr [g_01323910],EAX
        or      dword ptr [g_01323910], eax
        // 0001d61c: c7 05 <r> <r>    MOV dword ptr [g_0132390c],OFFSET FUN_0041d3a0
        mov     dword ptr [g_0132390c], offset FUN_0041d3a0
    skip_init:
        // 0001d626: 68 <reloc>       PUSH OFFSET s_f594c0
        push    offset s_f594c0
        // 0001d62b: 68 a6 0d 00 00   PUSH 0xda6
        push    0xda6
        // 0001d630: 68 <reloc>       PUSH OFFSET s_f59510
        push    offset s_f59510
        // 0001d635: 68 <reloc>       PUSH OFFSET s_f58bae
        push    offset s_f58bae
        // 0001d63a: 68 <reloc>       PUSH OFFSET s_f59560
        push    offset s_f59560
        // 0001d63f: ff 15 <reloc>    CALL dword ptr [g_0132390c]
        call    dword ptr [g_0132390c]
        // 0001d645: 83 c4 14         ADD ESP,0x14
        add     esp, 0x14
    skip_guard:
        // 0001d648: 56               PUSH ESI
        push    esi
        // 0001d649: 8b 74 24 08      MOV ESI,[ESP+8]   (arg1)
        mov     esi, dword ptr [esp + 8]
        // 0001d64d: 85 f6            TEST ESI,ESI
        test    esi, esi
        // 0001d64f: 74 46            JZ done_pop_esi   (offset +0x46)
        jz      done_pop_esi
        // 0001d651: 57               PUSH EDI
        push    edi
        // 0001d652: 8b 7c 24 10      MOV EDI,[ESP+0x10]  (arg2)
        mov     edi, dword ptr [esp + 0x10]
        // 0001d656: 85 ff            TEST EDI,EDI
        test    edi, edi
        // 0001d658: 74 3c            JZ done_pop_both  (offset +0x3c)
        jz      done_pop_both
        // 0001d65a: 8b 46 20         MOV EAX,[ESI+0x20]
        mov     eax, dword ptr [esi + 0x20]
        // 0001d65d: 50               PUSH EAX
        push    eax
        // 0001d65e: e8 <reloc>       CALL FUN_004332a0
        call    FUN_004332a0
        // 0001d663: 83 c4 04         ADD ESP,4
        add     esp, 4
        // 0001d666: 85 c0            TEST EAX,EAX
        test    eax, eax
        // 0001d668: 7f 2c            JG done_pop_both  (offset +0x2c)
        jg      done_pop_both
        // 0001d66a: 68 <reloc>       PUSH OFFSET s_f59578
        push    offset s_f59578
        // 0001d66f: 6a 00            PUSH 0
        push    0
        // 0001d671: e8 <reloc>       CALL FUN_009fc74a
        call    FUN_009fc74a
        // 0001d676: 8b 4f 28         MOV ECX,[EDI+0x28]
        mov     ecx, dword ptr [edi + 0x28]
        // 0001d679: 8b 56 28         MOV EDX,[ESI+0x28]
        mov     edx, dword ptr [esi + 0x28]
        // 0001d67c: 6a 02            PUSH 2
        push    2
        // 0001d67e: 6a 00            PUSH 0
        push    0
        // 0001d680: 51               PUSH ECX
        push    ecx
        // 0001d681: 8b 0d <reloc>    MOV ECX,[g_0132987c]
        mov     ecx, dword ptr [g_0132987c]
        // 0001d687: 6a 00            PUSH 0
        push    0
        // 0001d689: 52               PUSH EDX
        push    edx
        // 0001d68a: e8 <reloc>       CALL FUN_00423310
        call    FUN_00423310
        // 0001d68f: 5f               POP EDI
        pop     edi
        // 0001d690: 5e               POP ESI
        pop     esi
        // 0001d691: e9 <reloc>       JMP FUN_009fc750  (tail call)
        jmp     FUN_009fc750
    done_pop_both:
        // 0001d696: 5f               POP EDI
        pop     edi
    done_pop_esi:
        // 0001d697: 5e               POP ESI
        pop     esi
        // 0001d698: c3               RET
        ret
    }
}
