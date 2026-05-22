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
// FUNCTION: ffxivgame 0x0040a020 — lazy-init "default value" getter/setter
// for a single global int, wrapped in a one-shot MSVC SEH frame so the
// init call can throw without leaking the SEH chain.
//
// Shape (reconstructed from the asm; the headless Ghidra hint at
// build/ghidra-decomp/ffxivgame/0000a020_FUN_0040a020.c agrees):
//
//   int FUN_0040a020(int param_1) {
//       // SEH frame installed at function entry (state = -1 → 0 inside __try)
//       if ((g_initFlag & 1) == 0) {            // 0x01327bd8 — once-flag (bit 0)
//           g_initFlag |= 1;
//           __try {
//               g_value = FUN_0040e500();       // 0x01327bd4 — cached value
//           } __except (...) { /* funclet emitted via scope table */ }
//       }
//       if (param_1 != 0) {
//           g_value = param_1;
//           return param_1;                     // setter path
//       }
//       return g_value;                         // getter path
//   }
//
// SEH frame layout (relative to esp after the 3 pushes):
//   [esp + 0x00] = saved prev fs:[0]            (head of SEH chain link)
//   [esp + 0x04] = scope-table / handler ptr    (orig 0x00f059ae — DIR32 reloc)
//   [esp + 0x08] = unwind-state slot            (initial -1, set to 0 inside __try)
//   [esp + 0x0c] = return address
//   [esp + 0x10] = param_1
//
// MSVC 2005 emits this 3-push form (rather than the larger 5-push
// `_except_handler3` registration with explicit scope-table + cookie)
// because the function has no preserved registers and no try-local
// variables — only the once-flag flip is wrapped, and the scope
// table at 0x00f059ae is shared across every "trivial __try" call site
// in the binary (13 occurrences observed across .text).
//
// Calling convention: __cdecl (caller cleans 4 bytes; return in eax).
//
// Why naked __asm rather than a source-level translation:
//
//   The orig pins three encoding choices that source-level C++ won't
//   reproduce verbatim:
//     - `mov eax, dword ptr fs:[0]` is emitted BEFORE the three pushes
//       (Ghidra disasm: 64 a1 ...), then `push eax` carries the prev-
//       fs:[0] value into the SEH chain. Source-level `__try` always
//       emits `push -1 ; push offset scope ; mov eax, fs:[0] ; push eax`
//       (load between the scope-table push and the push eax — see
//       FUN_00403a20 for that form).
//     - `mov eax, 1` is placed BEFORE `mov fs:[0], esp` (rather than
//       after, as in functions that do `sub esp, N` for locals — e.g.
//       file offset 0x0062c5f0 in this binary). The 1 is reused both as
//       the once-flag mask (`test [data], al`) and the OR'd value
//       (`or [data], eax`), saving a separate `mov eax, 1` after the
//       SEH install.
//     - The two stores and one load of `g_value` use the EAX-moffs32
//       short forms (`a3` / `a1`) rather than the 6-byte ModR/M
//       `89 05` / `8b 05` forms. MSVC inline asm picks the short form
//       for EAX↔absolute, which matches orig.
//
// Reloc-bearing positions in the resulting .obj (all masked in the diff):
//
//   off 0x09  IMAGE_REL_I386_DIR32  → scope_table_0xf059ae  (push imm32)
//   off 0x1c  IMAGE_REL_I386_DIR32  → g_initFlag            (test [disp32], al)
//   off 0x24  IMAGE_REL_I386_DIR32  → g_initFlag            (or [disp32], eax)
//   off 0x31  IMAGE_REL_I386_REL32  → FUN_0040e500          (call rel32)
//   off 0x36  IMAGE_REL_I386_DIR32  → g_value               (mov moffs32, eax)
//   off 0x43  IMAGE_REL_I386_DIR32  → g_value               (mov moffs32, eax)
//   off 0x59  IMAGE_REL_I386_DIR32  → g_value               (mov eax, moffs32)

extern "C" {

// MSVC SEH scope-table / handler pointer at orig 0x00f059ae (in .data).
// Shared by all "trivial __try" call sites in this binary — pushed as
// a 4-byte DIR32 imm32 so the linker fixes up the actual address; the
// byte at this position is masked in the diff.
int scope_table_0xf059ae;

// One-shot init flag (bit 0). Tested with `test [data], al` and OR'd
// with `or [data], eax`. Declared as int so MSVC encodes the absolute
// address via ModR/M disp32 (DIR32 reloc).
int g_initFlag_0x1327bd8;

// Cached value backing the getter/setter. Loaded via `mov eax, moffs32`
// (a1) and stored via `mov moffs32, eax` (a3) — the EAX short forms.
int g_value_0x1327bd4;

// Lazy-init producer. __cdecl, no args, returns the initial value to
// stash into g_value on first call.
int FUN_0040e500();

}  // extern "C"

extern "C" __declspec(naked) int FUN_0040a020(int /*param_1*/) {
    __asm {
        // --- SEH prologue (3-push trivial __try form) ---------------
        mov     eax, dword ptr fs:[0]                   ; load prev SEH head
        push    -1                                       ; unwind state = -1
        push    offset scope_table_0xf059ae              ; scope/handler ptr (DIR32)
        push    eax                                      ; saved prev fs:[0]
        mov     eax, 1                                   ; reused below for test/or
        mov     dword ptr fs:[0], esp                    ; install SEH registration

        // --- One-shot init: test+set bit 0 of g_initFlag ------------
        test    byte ptr g_initFlag_0x1327bd8, al        ; al = 1 → checks bit 0
        jne     done_init
        or      g_initFlag_0x1327bd8, eax                ; set bit 0
        mov     dword ptr [esp + 8], 0                   ; SEH state := 0 (enter __try)
        call    FUN_0040e500                             ; producer
        mov     g_value_0x1327bd4, eax                   ; stash cached value (moffs32)
    done_init:

        // --- Getter/setter dispatch on param_1 ----------------------
        mov     eax, dword ptr [esp + 0x10]              ; eax = param_1
        test    eax, eax
        je      getter

        // Setter path: g_value = param_1; return param_1.
        mov     g_value_0x1327bd4, eax                   ; (moffs32)
        mov     ecx, dword ptr [esp]                     ; saved prev fs:[0]
        mov     dword ptr fs:[0], ecx                    ; restore SEH chain
        add     esp, 0xc                                 ; drop SEH frame
        ret

    getter:
        // Getter path: return g_value.
        mov     ecx, dword ptr [esp]                     ; saved prev fs:[0]
        mov     eax, g_value_0x1327bd4                   ; (moffs32 a1)
        mov     dword ptr fs:[0], ecx                    ; restore SEH chain
        add     esp, 0xc
        ret
    }
}

// vim: ts=4 sts=4 sw=4 et
