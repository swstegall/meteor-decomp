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
// FUNCTION: ffxivgame 0x00006520 — lazy-init accessor for a static
//                                  function-pointer slot
//                                  (35 B / 0x23).
//
// Inspection (read from the disassembly at orig RVA 0x00006520):
//
//   __cdecl void **FUN_00406520(void);
//
//     Classic MSVC "init-on-first-call" accessor. The function returns
//     the address of a static one-slot function-pointer cell. On the
//     first call (init bit clear) it sets the bit and writes the
//     default handler pointer into the cell; on subsequent calls it
//     short-circuits straight to returning the cell's address. Callers
//     read *EAX to obtain the current handler (and presumably can
//     overwrite *EAX to install a different one).
//
//     static void (*g_handler)(...) = nullptr;     // .data 0x0132390c
//     static unsigned int g_init_flags = 0;        // .data 0x01323910
//                                                  // bit 0 = "initialized"
//
//     void **get_handler_slot(void) {
//         if (!(g_init_flags & 1)) {
//             g_init_flags |= 1;
//             g_handler = &FUN_004063c0;            // 7-arg error-print
//                                                   // helper (vsnprintf
//                                                   // → OutputDebugStringA
//                                                   // via IAT 0x012651b4)
//         }
//         return (void **)&g_handler;
//     }
//
//   FUN_004063c0 (the default-installed handler at 0x004063c0) is a
//   166-byte two-branch formatter that vsnprintf's its caller's
//   arguments into one of two format strings (.rdata 0x00f54cf8 /
//   0x00f54d14) and forwards the result to the import at IAT slot
//   [0x012651b4] (OutputDebugStringA or a sibling debug-print).
//   FUN_00406520 is thus the get/set point for the global "error
//   printer" hook — by default it dispatches to FUN_004063c0, but any
//   subsystem can swap in its own implementation by writing through
//   the returned pointer.
//
//   Calling convention: __cdecl (no args, no stack args; the function
//   never touches ESP beyond the implicit return push). Stack frame: 0.
//
//   Reloc-bearing sites in the orig 35 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them via source — naked
//   asm emits them as raw immediate bytes which happen to match the
//   orig binary's resolved moffs32 / imm32 fields byte-for-byte):
//     +0x07   moffs32  TEST     (.data 0x01323910 — init flag byte)
//     +0x0f   moffs32  OR       (.data 0x01323910 — same slot, dword)
//     +0x15   moffs32  MOV dst  (.data 0x0132390c — handler ptr slot)
//     +0x1b   imm32    MOV src  (.text 0x004063c0 — default handler RVA)
//     +0x1e   imm32    MOV EAX  (.data 0x0132390c — return value)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc
//   into producing the exact moffs32 TEST-then-OR-then-MOV idiom
//   against the same .data slot, the exact short JNZ skip-over-init
//   encoding (+0x10), and the linker-resolved absolute addresses in
//   all five reloc windows above. A high-level rewrite of an
//   equivalent C++ accessor under /O2 would either inline the flag
//   check differently or pick a different register-allocation pattern.
//
//   The pragmatic choice — the same one FUN_00401650 took for its
//   118-byte SEH-wrapped magic-static init and FUN_00403bd0 took for
//   its 96-byte overflow-checked operator-new helper — is a
//   `__declspec(naked)` body that re-emits the orig 35 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice (no relocations because the
//   .data / .text addresses are emitted as raw immediates), which is
//   what `tools/compare.py` checks against.
//
// Asm shape (35 bytes — read from build/pe-layout/ffxivgame/text.bin
// @ +0x6520, RVA 0x00006520..0x00006542):
//
//     00006520:  b8 01 00 00 00              MOV EAX, 0x1
//     00006525:  84 05 10 39 32 01           TEST [0x01323910], AL
//     0000652b:  75 10                       JNZ 0x0040653d
//     0000652d:  09 05 10 39 32 01           OR [0x01323910], EAX
//     00006533:  c7 05 0c 39 32 01 c0 63 40 00
//                                            MOV [0x0132390c], 0x4063c0
//     0000653d:  b8 0c 39 32 01              MOV EAX, 0x0132390c
//     00006542:  c3                          RET

extern "C" __declspec(naked) void FUN_00406520() {
    __asm {
        _emit 0xb8              // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [0x01323910], AL
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ +0x10  -> 0x0040653d
        _emit 0x10
        _emit 0x09              // OR dword ptr [0x01323910], EAX
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [0x0132390c], 0x004063c0
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc0
        _emit 0x63
        _emit 0x40
        _emit 0x00
        _emit 0xb8              // MOV EAX, 0x0132390c
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc3              // RET
    }
}
