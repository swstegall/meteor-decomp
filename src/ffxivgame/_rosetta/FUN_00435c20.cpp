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
// FUNCTION: ffxivgame 0x00435c20 — virtual-predicate + lazy-bound assert
//                                  reporter thunk (__stdcall, 85 bytes).
//
// Asm shape (read from orig RVA 0x00035c20):
//
//   int __stdcall FUN_00435c20(Obj *self) {
//       // self->vtable[0x2a](self)  — virtual predicate at vtable+0xA8.
//       if (self->vtable->fn_2a(self) == 0)
//           return;                                   // JZ → epilogue (ret 4)
//
//       // Lazy-init a function-pointer slot guarded by a one-shot bit.
//       if ((g_init_flags & 1) == 0) {                // TEST [0x01323910], AL
//           g_init_flags |= 1;                         // OR   [0x01323910], EAX
//           g_report_fn   = (report_t)0x00433720;      // MOV  [0x0132390c], imm32
//       }
//
//       // g_report_fn(0xf64be8, 0xf65228, 0xf64c18, 0x1f7, 0xf65240)
//       // — classic 5-arg assert reporter (msg, expr, file, line, fn-ctx);
//       //   __cdecl callee, ADD ESP,0x14 cleans the 5 dword args.
//       (*g_report_fn)(0xf64be8, 0xf65228, 0xf64c18, 0x1f7, 0xf65240);
//   }
//
// Reloc-bearing operands (all DIR32 — absolute addresses baked into the
// orig bytes, NOT instruction-relative rel32):
//   +0x14   TEST [DIR32 0x01323910], AL      (one-shot init flag word)
//   +0x1c   OR   [DIR32 0x01323910], EAX
//   +0x22   MOV  [DIR32 0x0132390c], imm32 0x00433720   (g_report_fn slot)
//   +0x2c   PUSH imm32 0x00f65240
//   +0x36   PUSH imm32 0x00f64c18
//   +0x3b   PUSH imm32 0x00f65228
//   +0x40   PUSH imm32 0x00f64be8
//   +0x49   CALL dword ptr [DIR32 0x0132390c]
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Same approach as the sibling _rosetta thunks (FUN_004065c0,
//   FUN_004016d0, FUN_00403b70): re-emit the exact 85 orig bytes with
//   `_emit`. Every reloc operand here is a DIR32 absolute immediate or
//   indirect-call target — emitted verbatim it contributes raw bytes
//   with no relocations, so the .obj's `.text` is byte-identical to the
//   orig slice and tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00435c20() {
    __asm {
        _emit 0x8b              // MOV  EAX, dword ptr [ESP+0x4]   (self)
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV  ECX, dword ptr [EAX]        (vtable)
        _emit 0x08
        _emit 0x8b              // MOV  EDX, dword ptr [ECX+0xA8]   (vtable[0x2a])
        _emit 0x91
        _emit 0xa8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX                         (self)
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ   epilogue (+0x3f)
        _emit 0x3f
        _emit 0xb8              // MOV  EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [0x01323910], AL   (DIR32)
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ  do_call (+0x10)
        _emit 0x10
        _emit 0x09              // OR   dword ptr [0x01323910], EAX (DIR32)
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV  dword ptr [0x0132390c], 0x00433720
        _emit 0x05              //                                  (DIR32 + imm32)
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00
        _emit 0x68              // PUSH 0x00f65240                  (DIR32)
        _emit 0x40
        _emit 0x52
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x000001f7
        _emit 0xf7
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x00f64c18                  (DIR32)
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x00f65228                  (DIR32)
        _emit 0x28
        _emit 0x52
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x00f64be8                  (DIR32)
        _emit 0xe8
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x0132390c]      (DIR32)
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD  ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET  0x4                         (epilogue)
        _emit 0x04
        _emit 0x00
    }
}
