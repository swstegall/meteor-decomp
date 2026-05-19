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
// FUNCTION: ffxivgame 0x00001650 — static-init wrapper (__cdecl, 118 B)
//
// One-shot initializer for a global C++ object at .data 0x01323748. The
// function takes two args, latches a once-flag at 0x013237a0, calls the
// object's `__thiscall` init/ctor at 0x00406ab0 with those args, then
// hands the destructor thunk at 0x00f2e150 to the CRT `atexit` (RVA
// 0x005d25c2). Finally caches `&g_obj_storage` into the singleton
// pointer at .data 0x01323914 — done on every call, even when the
// once-flag short-circuits the body.
//
// Pseudo-source (logical structure, NOT byte-equivalent on its own —
// see "Why naked asm" below):
//
//   void __cdecl FUN_00401650(arg_t a, arg_t b) {
//       if ((g_init_flag & 1) == 0) {
//           g_init_flag |= 1;
//           g_obj_storage.Init(a, b);     // __thiscall @ 0x00406ab0
//           atexit(&g_obj_dtor_thunk);    // @ 0x00f2e150
//       }
//       g_obj_ptr = &g_obj_storage;
//   }
//
// Original 118 bytes (per asm/ffxivgame/00001650_FUN_00401650.s):
//
//   00001650: 6a ff 68 4b 43 e5 00 64 a1 00 00 00 00 50 a1 b0
//   00001660: a8 2e 01 33 c4 50 8d 44 24 04 64 a3 00 00 00 00
//   00001670: b8 01 00 00 00 84 05 a0 37 32 01 75 2f 09 05 a0
//   00001680: 37 32 01 8b 44 24 18 8b 4c 24 14 50 51 b9 48 37
//   00001690: 32 01 c7 44 24 14 00 00 00 00 e8 11 54 00 00 68
//   000016a0: 50 e1 f2 00 e8 19 0f 5d 00 83 c4 04 b8 48 37 32
//   000016b0: 01 a3 14 39 32 01 8b 4c 24 04 64 89 0d 00 00 00
//   000016c0: 00 59 83 c4 0c c3
//
// Why naked asm: the inlined EH3-style SEH prolog (PUSH -1 / PUSH
// scope_table_RVA / PUSH FS:[0] / cookie XOR ESP) plus the mid-body
// `MOV [ESP+0x14], 0` state-index write to mark "ctor in flight" is a
// compiler-emitted shape that depends on (a) the precise locals layout,
// (b) the function-info scope_table the linker laid down at .rdata RVA
// 0x00e5434b, and (c) MSVC's choice of `a1 / a3` short EAX-to-moffs32
// encodings for the `__security_cookie` load and the `g_obj_ptr` store.
// Coaxing exactly this byte sequence out of plain C++ under
// `/O2 /GS /EHsc` is impractical — each high-level rewrite shifts at
// least one encoding (modrm vs moffs32, SEH state numbering, branch
// short-vs-near). Naked asm lets the reloc-masking diff (compare.py)
// see a byte-exact match modulo the 9 relocations.
//
// Reloc-bearing sites (offsets within the function — these 4-byte
// windows are wildcarded by compare.py against orig):
//   +0x03   scope_table pointer (0x00e5434b — image-relative scope tbl)
//   +0x0f   __security_cookie load            (.data 0x012ea8b0)
//   +0x25   g_init_flag (TEST byte access)    (.data 0x013237a0)
//   +0x2f   g_init_flag (OR dword access)     (.data 0x013237a0)
//   +0x3e   g_obj_storage MOV-ECX (ctor this) (.data 0x01323748)
//   +0x4b   ctor CALL (rel32 to 0x00406ab0)
//   +0x50   atexit handler PUSH               (.text 0x00f2e150)
//   +0x55   _atexit CALL (rel32 to 0x009d25c2)
//   +0x5d   g_obj_storage MOV-EAX             (.data 0x01323748)
//   +0x62   g_obj_ptr MOV-store               (.data 0x01323914)

extern "C" {
    // .data — single security cookie shared across the whole TU.
    extern unsigned __security_cookie;

    // .rdata — MSVC-emitted EH3 scope table (FuncInfo). The bytes here
    // are image-relative (the PE has IMAGE_FILE_RELOCS_STRIPPED so the
    // bytes are final, but compare.py masks the imm32 window anyway via
    // our COFF reloc).
    extern int  g_scope_table;

    // .data — once-flag latched on first entry. Declared `int` so the
    // OR-dword form (`09 05 ...`) matches; the TEST uses a byte-ptr
    // override (`84 05 ...`).
    extern int  g_init_flag;

    // .data — body of the singleton being initialised. Treated as an
    // opaque storage symbol — its address is the `this` arg to the
    // __thiscall ctor, and also the value stored into g_obj_ptr.
    extern int  g_obj_storage;

    // .data — cached pointer to the singleton. Written every call.
    extern int *g_obj_ptr;

    // .text — __thiscall init/ctor on g_obj_storage. Args: (this, arg1,
    // arg2). Declared as a plain function for naked-asm referencing;
    // the assembler emits an `e8` rel32 CALL with a reloc.
    int g_obj_ctor();

    // .text — destructor thunk passed to atexit(). Receives no args.
    int g_obj_dtor_thunk();

    // .text — CRT atexit (RVA 0x005d25c2). One arg (the thunk).
    int _atexit();
}

extern "C" __declspec(naked) void FUN_00401650() {
    __asm {
        // --- /GS + EH3-style SEH prolog --------------------------------
        push    -1                              // 6a ff           (2 B)
        push    offset g_scope_table            // 68 ?? ?? ?? ??  (5 B, reloc +1)
        mov     eax, fs:[0]                     // 64 a1 00 00 00 00 (6 B)
        push    eax                             // 50              (1 B)
        mov     eax, __security_cookie          // a1 ?? ?? ?? ??  (5 B, reloc +1)
        xor     eax, esp                        // 33 c4           (2 B)
        push    eax                             // 50              (1 B)
        lea     eax, [esp + 4]                  // 8d 44 24 04     (4 B)
        mov     fs:[0], eax                     // 64 a3 00 00 00 00 (6 B)

        // --- once-flag guard -------------------------------------------
        mov     eax, 1                          // b8 01 00 00 00  (5 B)
        test    byte ptr g_init_flag, al        // 84 05 ?? ?? ?? ?? (6 B, reloc +2)
        jnz     short cleanup                   // 75 2f           (2 B)
        or      dword ptr g_init_flag, eax      // 09 05 ?? ?? ?? ?? (6 B, reloc +2)

        // --- ctor call (right-to-left for __thiscall) ------------------
        mov     eax, [esp + 0x18]               // 8b 44 24 18     (4 B) — arg2
        mov     ecx, [esp + 0x14]               // 8b 4c 24 14     (4 B) — arg1
        push    eax                             // 50              (1 B)
        push    ecx                             // 51              (1 B)
        mov     ecx, offset g_obj_storage       // b9 ?? ?? ?? ??  (5 B, reloc +1)
        mov     dword ptr [esp + 0x14], 0       // c7 44 24 14 00 00 00 00 (8 B) — EH state -> 0
        call    g_obj_ctor                      // e8 ?? ?? ?? ??  (5 B, reloc +1)

        // --- register atexit dtor --------------------------------------
        push    offset g_obj_dtor_thunk         // 68 ?? ?? ?? ??  (5 B, reloc +1)
        call    _atexit                         // e8 ?? ?? ?? ??  (5 B, reloc +1)
        add     esp, 4                          // 83 c4 04        (3 B)

    cleanup:
        mov     eax, offset g_obj_storage       // b8 ?? ?? ?? ??  (5 B, reloc +1)
        mov     g_obj_ptr, eax                  // a3 ?? ?? ?? ??  (5 B, reloc +1)

        // --- SEH epilog ------------------------------------------------
        mov     ecx, [esp + 4]                  // 8b 4c 24 04     (4 B)
        mov     fs:[0], ecx                     // 64 89 0d 00 00 00 00 (7 B)
        pop     ecx                             // 59              (1 B) — pop cookie
        add     esp, 0xc                        // 83 c4 0c        (3 B) — drop FS:[0]/scope/-1
        ret                                     // c3              (1 B)
    }
}
