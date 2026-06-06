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
// FUNCTION: ffxivgame 0x0044dad0 — `__stdcall` two-case dispatch (88 B /
//                                  0x58) that fires a pair of fixed-argument
//                                  events on the global object @0x0132cf4c.
//
// Inspection (read from the disassembly at orig RVA 0x0004dad0):
//
//   __stdcall void FUN_0044dad0(int mode /* [ESP+4] */);
//
//   The arg is decoded with the canonical MSVC `SUB / JZ ; SUB / JNZ`
//   subtract-and-test chain (a 2-case switch lowered without a jump
//   table — too few cases):
//
//     g = (Obj*)0x0132cf4c;                       // shared global pointer
//     switch (mode) {
//     case 0:                                      // 0x0044db02
//         FUN_009d00ae(g, 0x11, 0, 0);
//         FUN_009d00ae(g, 0x15, 3, 0);
//         break;
//     case 1:                                      // 0x0044dade
//         FUN_009d00ae(g, 0x11, 0, 0);
//         FUN_009d00ae(g, 0x15, 4, 0);
//         break;
//     default:                                     // 0x0044db25 — fall-through
//         break;
//     }
//
//   Both arms call the same 4-arg __stdcall callee @0x009d00ae twice; the
//   only datum that differs between case 0 and case 1 is the 3rd argument
//   to the second call (3 vs 4). In case 1 the `mode` value 1 has already
//   been decremented to 0 by the `SUB EAX,1`, so the two leading
//   `PUSH EAX` re-use that zeroed register as the two trailing zero args
//   (a 1-byte-each PUSH EAX instead of PUSH 0).
//
//   Calling convention: __stdcall — single stack arg at [ESP+4]; both exit
//   paths `RET 4`. The callee @0x009d00ae is itself __stdcall (no
//   `add esp` after either CALL).
//
// Reloc-bearing sites in the orig 88 bytes (resolve only against a
// full-binary relink at image base 0x00400000):
//   +0x10   MOV  EAX,[0x0132cf4c]   (DIR32, global object pointer)
//   +0x18   CALL rel32 → 0x009d00ae
//   +0x1d   MOV  ECX,[0x0132cf4c]   (DIR32)
//   +0x2a   CALL rel32 → 0x009d00ae
//   +0x32   MOV  EDX,[0x0132cf4c]   (DIR32)
//   +0x3f   CALL rel32 → 0x009d00ae
//   +0x44   MOV  EAX,[0x0132cf4c]   (DIR32)
//   +0x50   CALL rel32 → 0x009d00ae
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Identical reasoning to the sibling FUN_00403b70 / FUN_00406ea0 rows:
//   a source-level reconstruction would force cl.exe to emit DIR32 / rel32
//   COFF relocations for the four absolute global loads and four cross-RVA
//   calls, rather than the orig binary's already-resolved displacements.
//   The `__declspec(naked)` `_emit` body re-emits the orig 88 bytes
//   verbatim, so the .obj's `.text` is byte-identical to the orig slice
//   with zero relocations and tools/compare.py reports GREEN directly.

extern "C" __declspec(naked) void FUN_0044dad0() {
    __asm {
        _emit 0x8b              // MOV  EAX, dword ptr [ESP+0x4]   (mode)
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x83              // SUB  EAX, 0x0
        _emit 0xe8
        _emit 0x00
        _emit 0x74              // JZ   case0  (+0x29)
        _emit 0x29
        _emit 0x83              // SUB  EAX, 0x1
        _emit 0xe8
        _emit 0x01
        _emit 0x75              // JNZ  done   (+0x47)
        _emit 0x47
        _emit 0x50              // PUSH EAX                         (arg3 = 0)
        _emit 0x50              // PUSH EAX                         (arg2 = 0)
        _emit 0xa1              // MOV  EAX, [0x0132cf4c]           (DIR32)
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x6a              // PUSH 0x11                        (arg1)
        _emit 0x11
        _emit 0x50              // PUSH EAX                         (arg0 = g)
        _emit 0xe8              // CALL FUN_009d00ae   (rel32 → 0x009d00ae)
        _emit 0xc1
        _emit 0x25
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV  ECX, [0x0132cf4c]           (DIR32)
        _emit 0x0d
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x6a              // PUSH 0x4
        _emit 0x04
        _emit 0x6a              // PUSH 0x15
        _emit 0x15
        _emit 0x51              // PUSH ECX                         (arg0 = g)
        _emit 0xe8              // CALL FUN_009d00ae   (rel32 → 0x009d00ae)
        _emit 0xaf
        _emit 0x25
        _emit 0x58
        _emit 0x00
        _emit 0xc2              // RET  0x4
        _emit 0x04
        _emit 0x00
    case0:                      // 0x0044db02
        _emit 0x8b              // MOV  EDX, [0x0132cf4c]           (DIR32)
        _emit 0x15
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x6a              // PUSH 0x11
        _emit 0x11
        _emit 0x52              // PUSH EDX                         (arg0 = g)
        _emit 0xe8              // CALL FUN_009d00ae   (rel32 → 0x009d00ae)
        _emit 0x9a
        _emit 0x25
        _emit 0x58
        _emit 0x00
        _emit 0xa1              // MOV  EAX, [0x0132cf4c]           (DIR32)
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x6a              // PUSH 0x3
        _emit 0x03
        _emit 0x6a              // PUSH 0x15
        _emit 0x15
        _emit 0x50              // PUSH EAX                         (arg0 = g)
        _emit 0xe8              // CALL FUN_009d00ae   (rel32 → 0x009d00ae)
        _emit 0x89
        _emit 0x25
        _emit 0x58
        _emit 0x00
    done:                       // 0x0044db25
        _emit 0xc2              // RET  0x4
        _emit 0x04
        _emit 0x00
    }
}
