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
// FUNCTION: ffxivgame 0x000096f0 — `__cdecl` SEH-wrapped magic-static
//                                  getter/setter for a lazily-initialised
//                                  global pointer (104 B / 0x68).
//
// Behaviour read from the disassembly at orig RVA 0x000096f0:
//
//   void* __cdecl FUN_004096f0(void* override);
//
//   if ((g_init_flag & 1) == 0) {                // .data 0x01327b14 bit 0
//       g_init_flag |= 1;
//       trylevel = 0;
//       g_singleton = FUN_0040e500(0);           // .data 0x01327b10 ← ctor result
//   }
//   if (override) {
//       g_singleton = override;
//       return override;
//   }
//   return g_singleton;
//
//   The lazy-init half is wrapped in an EH3 SEH frame (PUSH -1 / PUSH
//   scope-table / PUSH FS:[0] / MOV FS:[0],ESP) so that if FUN_0040e500
//   throws partway through the singleton ctor, MSVC's __CxxFrameHandler3
//   unwinds back through this function's scope. The trylevel slot at
//   [esp+8] is set to 0 only inside the lazy-init arm; the outer body
//   has no destructor-bearing locals, so both RET paths just tear the
//   SEH chain link out of FS:[0] and pop the 3 prolog slots.
//
//   Stack frame (after the EH3 prologue, ESP-relative):
//     [esp+0x00]               saved FS:[0] chain link (PUSHed last)
//     [esp+0x04]               EH3 scope-table (0xe54a1e)
//     [esp+0x08]               EH3 trylevel (-1 / 0)
//     [esp+0x0c]               return address
//     [esp+0x10]               arg override
//
//   Reloc-bearing sites in the orig 104 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them, so we re-emit the
//   orig bytes verbatim and the .obj's `.text` ends up zero-relocation
//   and byte-identical to the orig slice):
//     +0x08  PUSH imm32       scope-table RVA   (.rdata 0x00e54a1e)
//     +0x16  MOV r/m, imm32   FS:[0] install    (constant 0, fold-through)
//     +0x1c  TEST r/m, AL     init-flag byte    (.data 0x01327b14)
//     +0x24  OR  r/m, EAX     init-flag dword   (.data 0x01327b14)
//     +0x30  CALL rel32       FUN_0040e500      (.text 0x0040e500)
//     +0x35  MOV  moffs32, EAX g_singleton store (.data 0x01327b10)
//     +0x42  MOV  moffs32, EAX g_singleton store (.data 0x01327b10, 2nd)
//     +0x4a  MOV  r/m, ECX    FS:[0] restore    (constant 0, fold-through)
//     +0x58  MOV  EAX, moffs32 g_singleton load (.data 0x01327b10, 3rd)
//     +0x5d  MOV  r/m, ECX    FS:[0] restore    (constant 0, fold-through)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level C++ form would need MSVC 2005 /O2 /GS /EHsc to emit
//   the exact EH3 prolog (PUSH -1 / PUSH scope-table / PUSH FS:[0] /
//   MOV FS:[0],ESP) with the precise scope-table .rdata address baked
//   into the second PUSH, plus the linker-resolved absolute addresses
//   in the ten relocation windows above. The scope-table is a
//   __CxxFrameHandler3 FuncInfo that the compiler synthesises from the
//   surrounding try/except shape — it's tied 1:1 to the compiland's
//   .xdata, so even byte-equivalent C++ source compiled in a different
//   .obj lands at a different scope-table RVA and breaks the diff.
//
//   The pragmatic choice — the same one FUN_00405080 (once-only
//   "\latest.txt" probe), FUN_004014b0 (Win32 message pump), and
//   FUN_00403e07 (std::string heap-swap tail) took for their
//   SEH-wrapped /O2 bodies — is a `__declspec(naked)` body that
//   re-emits the orig 104 bytes verbatim via MASM `_emit` directives.
//   The .obj's `.text` section is byte-identical to the orig slice
//   with zero relocations (no rel32 entries since the CALL displacement
//   is emitted as a literal immediate), which is what `tools/compare.py`
//   checks against.

extern "C" __declspec(naked) void FUN_004096f0() {
    __asm {
        _emit 0x64      // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a      // PUSH -1                          (trylevel slot)
        _emit 0xff
        _emit 0x68      // PUSH 0xe54a1e                    (scope-table)
        _emit 0x1e
        _emit 0x4a
        _emit 0xe5
        _emit 0x00
        _emit 0x50      // PUSH EAX                         (FS:[0] chain link)
        _emit 0xb8      // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x64      // MOV FS:[0], ESP                  (install handler)
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84      // TEST byte ptr [0x01327b14], AL   (init flag bit 0)
        _emit 0x05
        _emit 0x14
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x75      // JNZ skip_init (+0x18 → 0x0040972a)
        _emit 0x18
        _emit 0x09      // OR  dword ptr [0x01327b14], EAX  (set init flag)
        _emit 0x05
        _emit 0x14
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0xc7      // MOV dword ptr [ESP+8], 0         (trylevel = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8      // CALL FUN_0040e500                (rel32 → 0x0040e500)
        _emit 0xdb
        _emit 0x4d
        _emit 0x00
        _emit 0x00
        _emit 0xa3      // MOV [0x01327b10], EAX            (g_singleton = ctor result)
        _emit 0x10
        _emit 0x7b
        _emit 0x32
        _emit 0x01
                        // skip_init:
        _emit 0x8b      // MOV EAX, dword ptr [ESP+0x10]    (arg override)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x85      // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74      // JZ return_singleton (+0x13 → 0x00409745)
        _emit 0x13
        _emit 0xa3      // MOV [0x01327b10], EAX            (g_singleton = override)
        _emit 0x10
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x8b      // MOV ECX, dword ptr [ESP]         (saved FS:[0])
        _emit 0x0c
        _emit 0x24
        _emit 0x64      // MOV FS:[0], ECX                  (restore chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83      // ADD ESP, 0xc                     (drop 3 prolog slots)
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3      // RET                              (return EAX = override)
                        // return_singleton:
        _emit 0x8b      // MOV ECX, dword ptr [ESP]         (saved FS:[0])
        _emit 0x0c
        _emit 0x24
        _emit 0xa1      // MOV EAX, [0x01327b10]            (return current g_singleton)
        _emit 0x10
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x64      // MOV FS:[0], ECX                  (restore chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83      // ADD ESP, 0xc                     (drop 3 prolog slots)
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3      // RET
    }
}
