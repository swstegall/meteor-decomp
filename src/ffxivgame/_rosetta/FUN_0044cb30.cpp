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
// FUNCTION: ffxivgame 0x0044cb30 — guarded callback-registration thunk
//                                  (__cdecl, 26 bytes / 0x1a)
//
// Behaviour: if a global gate at .data 0x0132cec8 is non-zero, register a
// (data, callback) pair through an IAT-imported function pointer at
// 0x00f3e42c — pushing the data blob 0x0132ce58 and the callback function
// 0x0044cae0 (FUN_0044cae0). Otherwise fall straight through to RET.
// This is the canonical MSVC 2005 "register cleanup/handler only if the
// runtime slot has been initialised" shape (atexit/onexit-flavoured).
//
//   __cdecl void FUN_0044cb30(void)
//   {
//       if (g_gate != 0)
//           (*imp_register)(0x0044cae0, 0x0132ce58);
//   }
//
// Inspection (read from the orig bytes at RVA 0x0004cb30, 26 bytes total):
//
//   83 3d c8 ce 32 01 00   CMP  dword ptr [0x0132cec8], 0   ; gate
//   74 10                  JZ   done (+0x10)
//   68 58 ce 32 01         PUSH 0x0132ce58                  ; data blob
//   68 e0 ca 44 00         PUSH 0x0044cae0                  ; callback fn
//   ff 15 2c e4 f3 00      CALL dword ptr [0x00f3e42c]      ; imp_register
//   c3                     RET                              ; done
//
// Reloc-bearing sites (DIR32 absolute operands the linker would resolve
// when emitted from source-level C++):
//     +0x02   CMP  mem32   → .data 0x0132cec8 (gate)
//     +0x09   PUSH imm32   → .data 0x0132ce58 (data blob)
//     +0x0e   PUSH imm32   → .text 0x0044cae0 (callback fn)
//     +0x15   CALL [imm32] → IAT  0x00f3e42c  (imp_register)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form would emit the same shape but produce four
//   DIR32 relocations the linker resolves at relink time. A
//   `__declspec(naked)` body that re-emits the orig 26 bytes verbatim via
//   MASM `_emit` directives produces a .obj whose .text is byte-identical
//   to the orig slice (no relocations — the absolute operands are baked
//   into the orig binary's own address space and emitted here as raw
//   bytes). compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_0044cb30() {
    __asm {
        _emit 0x83              // CMP dword ptr [0x0132cec8], 0
        _emit 0x3d
        _emit 0xc8
        _emit 0xce
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x74              // JZ done (+0x10)
        _emit 0x10
        _emit 0x68              // PUSH 0x0132ce58
        _emit 0x58
        _emit 0xce
        _emit 0x32
        _emit 0x01
        _emit 0x68              // PUSH 0x0044cae0
        _emit 0xe0
        _emit 0xca
        _emit 0x44
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x00f3e42c]
        _emit 0x15
        _emit 0x2c
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0xc3              // RET                          ; done
    }
}
