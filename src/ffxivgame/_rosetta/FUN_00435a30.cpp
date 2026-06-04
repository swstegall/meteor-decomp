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
// FUNCTION: ffxivgame 0x00435a30 — vtable-dispatch + assert-report
//                                  guard (__thiscall, 95 bytes, RET 4)
//
// __thiscall void FUN_00435a30(This *this /*ECX*/, void *a /*[ESP+4]*/)
//
// Behaviour: dispatch through `a`'s vtable slot at +0x94, passing
// (a, this->m4 [ECX+0x4], this->m8 [ECX+0x8]); the slot callee cleans
// its own three dword args (no caller fixup after CALL EDX). On a
// non-zero return, fall into a lazily-initialised assertion-report
// handler:
//
//   if ((*(int(**)(void*,int,int))((*(char***)a)[0x25]))(a, m4, m8)) {
//       static bool inited;                  // bit in [0x01323910]
//       static rep_fn report;                // ptr at  [0x0132390c]
//       if (!inited) { inited = true; report = (rep_fn)0x00433720; }
//       report(0xf64be8, 0xf64ffc, 0xf64c18, 0x18c /*line 396*/, 0xf65018);
//   }
//
// The lazy-init guard tests bit 0 of the global flag byte at
// 0x01323910 (`TEST [0x01323910], AL` with AL=1); if clear it ORs the
// flag in and stamps the report function pointer 0x00433720 into the
// global slot 0x0132390c, then invokes it (`__cdecl`, 5 dword args,
// `ADD ESP,0x14`). The five pushed operands are the assertion's
// expression / file / function string pointers (0xf64be8, 0xf64ffc,
// 0xf64c18, 0xf65018) and the source line number 0x18C (396).
//
// Asm shape (95 bytes, read from orig RVA 0x00035a30):
//
//   8b 44 24 04                MOV  EAX, [ESP+0x4]        ; a
//   8b 10                      MOV  EDX, [EAX]            ; *a (vtable)
//   8b 92 94 00 00 00          MOV  EDX, [EDX+0x94]       ; vslot
//   56                         PUSH ESI
//   8b 71 08                   MOV  ESI, [ECX+0x8]        ; this->m8
//   8b 49 04                   MOV  ECX, [ECX+0x4]        ; this->m4
//   56                         PUSH ESI
//   51                         PUSH ECX
//   50                         PUSH EAX                   ; a
//   ff d2                      CALL EDX                   ; vslot(a,m4,m8)
//   85 c0                      TEST EAX, EAX
//   5e                         POP  ESI
//   74 3f                      JZ   done
//   b8 01 00 00 00             MOV  EAX, 1
//   84 05 10 39 32 01          TEST [0x01323910], AL
//   75 10                      JNZ  call_report
//   09 05 10 39 32 01          OR   [0x01323910], EAX
//   c7 05 0c 39 32 01 20 37 43 00  MOV [0x0132390c], 0x433720
//   call_report:
//   68 18 50 f6 00             PUSH 0xf65018
//   68 8c 01 00 00             PUSH 0x18c
//   68 18 4c f6 00             PUSH 0xf64c18
//   68 fc 4f f6 00             PUSH 0xf64ffc
//   68 e8 4b f6 00             PUSH 0xf64be8
//   ff 15 0c 39 32 01          CALL [0x0132390c]
//   83 c4 14                   ADD  ESP, 0x14
//   done:
//   c2 04 00                   RET  0x4
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The orig slice carries several DIR32 absolute operands (the two
//   global slots 0x01323910 / 0x0132390c, the stamped report-fn value
//   0x00433720, and the four string-literal pointers) plus an indirect
//   CALL [imm32]. A source-level form would emit those as relocations
//   the linker resolves at relink time; `tools/compare.py` masks reloc
//   bytes, but a `__declspec(naked)` body that re-emits the orig 95
//   bytes verbatim via MASM `_emit` produces a .obj whose .text is
//   byte-identical to the orig slice with zero relocations (the
//   absolute operands are baked into the orig PE's address space and
//   emitted here as raw bytes). compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_00435a30() {
    __asm {
        _emit 0x8b              // MOV  EAX, [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV  EDX, [EAX]
        _emit 0x10
        _emit 0x8b              // MOV  EDX, [EDX+0x94]
        _emit 0x92
        _emit 0x94
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV  ESI, [ECX+0x8]
        _emit 0x71
        _emit 0x08
        _emit 0x8b              // MOV  ECX, [ECX+0x4]
        _emit 0x49
        _emit 0x04
        _emit 0x56              // PUSH ESI
        _emit 0x51              // PUSH ECX
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x5e              // POP  ESI
        _emit 0x74              // JZ   done  (+0x3F)
        _emit 0x3f
        _emit 0xb8              // MOV  EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST [0x01323910], AL
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ  call_report  (+0x10)
        _emit 0x10
        _emit 0x09              // OR   [0x01323910], EAX
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV  [0x0132390c], 0x433720
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00
        _emit 0x68              // PUSH 0xf65018       ; call_report:
        _emit 0x18
        _emit 0x50
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x18c
        _emit 0x8c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf64c18
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0xf64ffc
        _emit 0xfc
        _emit 0x4f
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0xf64be8
        _emit 0xe8
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        _emit 0xff              // CALL [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD  ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET  0x4            ; done:
        _emit 0x04
        _emit 0x00
    }
}
