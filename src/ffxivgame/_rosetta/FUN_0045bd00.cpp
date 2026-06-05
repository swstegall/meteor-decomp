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
// FUNCTION: ffxivgame 0x0005bd00 — int_err_set_item
//                                  __cdecl, 153 B / 0x99, plain RET.
//
// __cdecl void* int_err_set_item(void* arg):
//   No saved registers beyond ESI (no EBP frame — /Oy FPO).
//   One stack argument accessed via [ESP+0x18] in the main body after
//   PUSH ESI + 4×PUSH for the logging call.
//
// Behaviour (recovered from asm @ 0x0005bd00):
//
//   Singleton initialisation gate on g_int_err_0132e788:
//     if (*g_int_err_0132e788 == NULL):
//         FUN_00465f80(9, 1, 0xf6802c, 0x127);   // log begin
//         if (*g_int_err_0132e788 == NULL):
//             *g_int_err_0132e788 = (void*)0xf68000;  // default instance
//         FUN_00465f80(10, 1, 0xf6802c, 0x12a);  // log end
//
//   Vtable call to obtain an item:
//     obj = ((*g_int_err_0132e788)->vtable[0])(1);   // CALL ECX via vtable
//     if (obj == NULL) return NULL;
//
//   Set-item with logging wrapper:
//     FUN_00465f80(9, 1, 0xf6802c, 0x196);       // log begin
//     result = FUN_00466a60(obj, arg);            // set the item
//     FUN_00465f80(10, 1, 0xf6802c, 0x198);      // log end  (deferred cleanup)
//     return result;
//
// CALL targets (all REL32/IAT, wildcarded by tools/compare.py):
//   +0x17   CALL FUN_00465f80   — logging/tracing wrapper (begin block)
//   +0x40   CALL FUN_00465f80   — same (end block)
//   +0x52   CALL ECX            — vtable[0](1): create/obtain item from singleton
//   +0x6d   CALL FUN_00465f80   — logging/tracing wrapper (begin block)
//   +0x78   CALL FUN_00466a60   — set_item(obj, arg): the actual operation
//   +0x8d   CALL FUN_00465f80   — logging/tracing wrapper (end block)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The success path (0x5f–0x98) combines three CALL sites (two logging
//   wrappers around one worker call) with a single large ADD ESP,0x28
//   cleanup at the end — a deferred-cleanup pattern that MSVC 2005 emits
//   when successive calls share the same scratch area.  Additionally, the
//   intermediate result (FUN_00466a60 return) is saved into ESI *before*
//   pushing the final 4 logging args, meaning ESI and EAX cross a CALL
//   boundary in a way that source-level C++ at /O2 will not reliably
//   reproduce without an explicit asm spell-out.  The sibling rosetta
//   shells in this directory (FUN_00406f00, FUN_004126f0, FUN_004061e0)
//   use the same naked-asm passthrough pattern for analogous reasons.
//
// Global addressed by this function:
//   g_int_err_0132e788  at 0x0132e788 — pointer to the error-item singleton
//                                        object (has vtable at offset 0).

// Internal direct-call targets within the binary (REL32 relocations).
extern "C" {
    int FUN_00465f80();  // __cdecl log/trace wrapper (begin/end)
    int FUN_00466a60();  // __cdecl set_item(obj, arg)
}

// Global at 0x0132e788 — pointer-sized slot holding the singleton object.
// Declared as an int so the assembler generates a DIR32 COFF relocation
// (masked by compare.py) rather than hardcoding the absolute address.
extern int g_int_err_0132e788;

extern "C" __declspec(naked) void FUN_0045bd00() {
    __asm {
        // --- singleton initialisation gate -----------------------------------
        // 0005bd00: 83 3d [0x0132e788] 00   CMP dword ptr [g_var], 0
        cmp     dword ptr [g_int_err_0132e788], 0
        // 0005bd07: 75 3f                   JNZ → label_a (skip init)
        jnz     short label_a

        // --- first log call (begin) ------------------------------------------
        // 0005bd09: 68 27 01 00 00          PUSH 0x127
        push    0x127
        // 0005bd0e: 68 2c 80 f6 00          PUSH 0xf6802c
        push    0xf6802c
        // 0005bd13: 6a 01                   PUSH 0x1
        push    1
        // 0005bd15: 6a 09                   PUSH 0x9
        push    9
        // 0005bd17: e8 ?? ?? ?? ??          CALL FUN_00465f80
        call    FUN_00465f80
        // 0005bd1c: 83 c4 10                ADD ESP, 0x10
        add     esp, 0x10

        // --- conditionally store default singleton pointer -------------------
        // 0005bd1f: 83 3d [0x0132e788] 00   CMP dword ptr [g_var], 0
        cmp     dword ptr [g_int_err_0132e788], 0
        // 0005bd26: 75 0a                   JNZ → label_b (already set)
        jnz     short label_b
        // 0005bd28: c7 05 [0x0132e788] 00 80 f6 00   MOV [g_var], 0xf68000
        mov     dword ptr [g_int_err_0132e788], 0xf68000

    label_b:
        // --- second log call (end) -------------------------------------------
        // 0005bd32: 68 2a 01 00 00          PUSH 0x12a
        push    0x12a
        // 0005bd37: 68 2c 80 f6 00          PUSH 0xf6802c
        push    0xf6802c
        // 0005bd3c: 6a 01                   PUSH 0x1
        push    1
        // 0005bd3e: 6a 0a                   PUSH 0xa
        push    0xa
        // 0005bd40: e8 ?? ?? ?? ??          CALL FUN_00465f80
        call    FUN_00465f80
        // 0005bd45: 83 c4 10                ADD ESP, 0x10
        add     esp, 0x10

    label_a:
        // --- vtable call: obtain item from singleton -------------------------
        // 0005bd48: a1 [0x0132e788]         MOV EAX, [g_var]
        mov     eax, dword ptr [g_int_err_0132e788]
        // 0005bd4d: 8b 08                   MOV ECX, [EAX]   (vtable ptr)
        mov     ecx, dword ptr [eax]
        // 0005bd4f: 56                      PUSH ESI
        push    esi
        // 0005bd50: 6a 01                   PUSH 0x1
        push    1
        // 0005bd52: ff d1                   CALL ECX
        call    ecx
        // 0005bd54: 8b f0                   MOV ESI, EAX
        mov     esi, eax
        // 0005bd56: 83 c4 04                ADD ESP, 0x4
        add     esp, 4
        // 0005bd59: 85 f6                   TEST ESI, ESI
        test    esi, esi
        // 0005bd5b: 75 02                   JNZ → label_c (obj != NULL)
        jnz     short label_c
        // 0005bd5d: 5e                      POP ESI
        pop     esi
        // 0005bd5e: c3                      RET  (return NULL)
        ret

    label_c:
        // --- logging begin + set_item + logging end (deferred cleanup) -------
        // 0005bd5f: 68 96 01 00 00          PUSH 0x196
        push    0x196
        // 0005bd64: 68 2c 80 f6 00          PUSH 0xf6802c
        push    0xf6802c
        // 0005bd69: 6a 01                   PUSH 0x1
        push    1
        // 0005bd6b: 6a 09                   PUSH 0x9
        push    9
        // 0005bd6d: e8 ?? ?? ?? ??          CALL FUN_00465f80  (no cleanup yet)
        call    FUN_00465f80
        // 0005bd72: 8b 54 24 18             MOV EDX, [ESP+0x18]  (function arg)
        mov     edx, dword ptr [esp + 0x18]
        // 0005bd76: 52                      PUSH EDX
        push    edx
        // 0005bd77: 56                      PUSH ESI
        push    esi
        // 0005bd78: e8 ?? ?? ?? ??          CALL FUN_00466a60  (no cleanup yet)
        call    FUN_00466a60
        // 0005bd7d: 68 98 01 00 00          PUSH 0x198
        push    0x198
        // 0005bd82: 68 2c 80 f6 00          PUSH 0xf6802c
        push    0xf6802c
        // 0005bd87: 6a 01                   PUSH 0x1
        push    1
        // 0005bd89: 6a 0a                   PUSH 0xa
        push    0xa
        // 0005bd8b: 8b f0                   MOV ESI, EAX  (save set_item result)
        mov     esi, eax
        // 0005bd8d: e8 ?? ?? ?? ??          CALL FUN_00465f80  (no cleanup yet)
        call    FUN_00465f80
        // 0005bd92: 83 c4 28                ADD ESP, 0x28  (clean 10 pushes at once)
        add     esp, 0x28
        // 0005bd95: 8b c6                   MOV EAX, ESI  (return set_item result)
        mov     eax, esi
        // 0005bd97: 5e                      POP ESI
        pop     esi
        // 0005bd98: c3                      RET
        ret
    }
}
