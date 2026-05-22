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
// FUNCTION: ffxivgame 0x0040a250 — guarded one-shot init/setter for the
// module-global DAT_01327c00 (104 B / 0x68, EH3-SEH wrapped, no /GS).
//
// Behaviour reconstructed from the asm (Ghidra pseudo-C at
// build/ghidra-decomp/ffxivgame/0000a250_FUN_0040a250.c agrees on shape):
//
//   int __cdecl FUN_0040a250(int param_1) {
//       try {
//           if ((DAT_01327c04 & 1) == 0) {            // first-call guard
//               DAT_01327c04 |= 1;                    // arm guard
//               DAT_01327c00 = FUN_0040e500();        // construct singleton
//           }
//       } catch (...) {
//           // Unwind funclet at orig 0x00e54cde clears bit 0 of
//           // DAT_01327c04 so a subsequent retry re-runs FUN_0040e500.
//       }
//       if (param_1 != 0) {
//           DAT_01327c00 = param_1;                   // setter mode
//           return param_1;
//       }
//       return DAT_01327c00;                          // getter mode
//   }
//
// `DAT_01327c04` is a bitset (low bit = "FUN_0040e500 has been called
// once") and `DAT_01327c00` is the cached result / overridable handle.
// Together they form a thread-unsafe lazy singleton with an explicit
// setter override (`param_1 != 0`) — common MSVC-2005 idiom for runtime
// option / locale / allocator-style globals that need a default-on-
// demand value plus an external override hook. This is one of a dense
// cluster of structurally-identical singletons in this binary — see
// the matched siblings at 0x0040a020 / 0x0040a1e0 sharing the same
// 0x68-byte EH3-SEH-wrapped layout, each pointing at a different
// (flag, value) data pair and a different SEH handler trampoline.
//
// Calling convention: __cdecl. The single dword parameter sits at
// `[esp+0x10]` mid-body (return address + the three SEH-prologue dwords
// = 0x10 byte adjust), and the function ends with a bare `ret` —
// caller cleans.
//
// SEH frame after the prologue (esp-relative):
//   [esp + 0x00]  prev FS:[0]   (push'd third)
//   [esp + 0x04]  SEH handler   (push'd second as `push 0x00e54cde`)
//   [esp + 0x08]  initial state (push'd first as `push -1`, then re-
//                                written to 0 just before FUN_0040e500
//                                so the unwind funclet at 0x00e54cde
//                                fires if the singleton ctor throws)
//   [esp + 0x0c]  return addr
//   [esp + 0x10]  param_1
//
// Why naked-asm and not a natural source-level rewrite:
//
//   The 104 bytes are entirely shape-constrained — EH3-SEH prologue
//   (mov-fs0 / push-trylevel / push-handler / push-prev / install-fs0),
//   the `mov eax, 1` + `test byte ptr [DAT], al` + `or [DAT], eax` flag
//   reuse (one register sets the flag bit AND serves as the OR source,
//   saving one instruction), the moffs32 a1/a3 short forms for the
//   DAT_01327c00 load/store (5-byte each vs 6-byte ModR/M), and the
//   two distinct epilogue arms (one bypasses the DAT load when
//   param_1 != 0). A C++ rewrite would shift at least the DAT
//   load/store encoding plus the eax-reuse trick and break the byte
//   diff.
//
// Reloc-bearing positions in the resulting .obj (all masked in the diff):
//
//   off 0x09   IMAGE_REL_I386_DIR32  → SEH handler stub (orig 0x00e54cde)
//   off 0x1c   IMAGE_REL_I386_DIR32  → DAT_01327c04           (test r/m8)
//   off 0x24   IMAGE_REL_I386_DIR32  → DAT_01327c04           (or  r/m32)
//   off 0x31   IMAGE_REL_I386_REL32  → FUN_0040e500           (singleton ctor)
//   off 0x36   IMAGE_REL_I386_DIR32  → DAT_01327c00           (a3 moffs32 store)
//   off 0x43   IMAGE_REL_I386_DIR32  → DAT_01327c00           (a3 moffs32 store)
//   off 0x59   IMAGE_REL_I386_DIR32  → DAT_01327c00           (a1 moffs32 load)

extern "C" {

// SEH handler trampoline at 0x00e54cde in orig (push'd as the EH3
// scope-table / handler entry). Unique symbol for the DIR32 reloc; the
// bytes at this position are masked.
int FUN_00e54cde();

// Lazily-initialised value cell + first-call guard bitset at .data
// 0x01327c00 / 0x01327c04. Declared as `int` so the inline asm can use
// the 5-byte moffs32 a1/a3 short forms for the EAX↔mem moves.
extern int DAT_01327c00;
extern int DAT_01327c04;

// Singleton constructor invoked exactly once under the SEH guard.
int FUN_0040e500();

} // extern "C"

extern "C" __declspec(naked) void FUN_0040a250() {
    __asm {
        // --- EH3-SEH prologue ---------------------------------------
        mov     eax, dword ptr fs:[0]               ; 64 a1 00 00 00 00
        push    -1                                  ; 6a ff
        push    offset FUN_00e54cde                 ; 68 <DIR32>
        push    eax                                 ; 50  (prev fs:[0] → SEH chain)
        mov     eax, 1                              ; b8 01 00 00 00
        mov     dword ptr fs:[0], esp               ; 64 89 25 00 00 00 00  (install SEH frame)

        // --- Body: lazy-init guard ----------------------------------
        test    byte ptr [DAT_01327c04], al         ; 84 05 <DIR32>  (al = 1 → test bit 0)
        jne     L_AFTER_INIT                        ; 75 18
        or      dword ptr [DAT_01327c04], eax       ; 09 05 <DIR32>  (set bit 0)
        mov     dword ptr [esp + 8], 0              ; c7 44 24 08 00 00 00 00
                                                    ; state := 0 → unwind funclet active
                                                    ; (clears bit 0 if ctor throws)
        call    FUN_0040e500                        ; e8 <REL32>
        mov     dword ptr [DAT_01327c00], eax       ; a3 <DIR32>  (5-byte moffs32 store)

    L_AFTER_INIT:
        // Setter-vs-getter dispatch on param_1.
        mov     eax, dword ptr [esp + 0x10]         ; 8b 44 24 10  (param_1)
        test    eax, eax                            ; 85 c0
        je      L_GET                               ; 74 13
        mov     dword ptr [DAT_01327c00], eax       ; a3 <DIR32>  (setter: cache = param_1)
        mov     ecx, dword ptr [esp]                ; 8b 0c 24    (restore prev fs:[0])
        mov     dword ptr fs:[0], ecx               ; 64 89 0d 00 00 00 00
        add     esp, 0xc                            ; 83 c4 0c    (pop SEH frame)
        ret                                         ; c3          (eax already = param_1)

    L_GET:
        mov     ecx, dword ptr [esp]                ; 8b 0c 24    (restore prev fs:[0])
        mov     eax, dword ptr [DAT_01327c00]       ; a1 <DIR32>  (return cached value)
        mov     dword ptr fs:[0], ecx               ; 64 89 0d 00 00 00 00
        add     esp, 0xc                            ; 83 c4 0c
        ret                                         ; c3
    }
}

// vim: ts=4 sts=4 sw=4 et
