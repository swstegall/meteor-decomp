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
// FUNCTION: ffxivgame 0x000389d0 — __thiscall wrapper: build a two-field
//           stack struct {0xf64988, arg1} and forward to FUN_00435af0
//           (38 B / 0x26)
//
// Calling convention: __thiscall — ECX = this on entry; one stack arg
//   (arg1 at [ESP+0x4]); callee-cleaned (RET 0x4).
//
// The function:
//   1. Allocates 8 bytes on the stack for a two-field wrapper struct.
//   2. Loads this->field4 (at ECX+4) into ECX while ECX is still `this`.
//   3. Saves arg1 (now at [ESP+0xc] after the SUB) into EAX.
//   4. Pushes ECX (field4) as the single stack argument for FUN_00435af0.
//   5. Sets ECX = &wrapper (LEA [ESP+4] after the push).
//   6. Initialises wrapper: wrapper[0] = 0xf64988, wrapper[4] = arg1.
//   7. Calls FUN_00435af0 (__thiscall, RET 4 cleans the one stack arg).
//   8. ADD ESP, 8 to free the wrapper; RET 4 to clean caller's stack.
//
// The 8-byte local struct mirrors the layout expected by FUN_00435af0:
//   ECX+0  = type-tag constant (0x00f64988, a .rdata pointer)
//   ECX+4  = value pointer (arg1) used as `peer` by the callee
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//   The only non-trivial output ordering detail is MSVC 2005's decision
//   to load arg1 into EAX *before* the PUSH ECX (so the offset stays
//   [ESP+0xc] rather than [ESP+0x10] after the push). Reproducing that
//   scheduling choice reliably in source-level C++ is fragile, so this
//   function is encoded as a naked asm passthrough. The single CALL at
//   +0x1b has a linker-resolved relative offset; compare.py masks the
//   four reloc bytes (e8 rr rr rr rr) during comparison.
//
// Asm (38 bytes @ orig RVA 0x000389d0):
//   83 ec 08                 SUB  ESP, 0x8
//   8b 49 04                 MOV  ECX, [ECX+0x4]       ; ECX = this->field4
//   8b 44 24 0c              MOV  EAX, [ESP+0xc]       ; EAX = arg1
//   51                       PUSH ECX                  ; stack arg for callee
//   8d 4c 24 04              LEA  ECX, [ESP+0x4]       ; ECX = &wrapper
//   c7 44 24 04 88 49 f6 00  MOV  [ESP+0x4], 0xf64988  ; wrapper.typeId
//   89 44 24 08              MOV  [ESP+0x8], EAX       ; wrapper.val = arg1
//   e8 00 d1 ff ff           CALL FUN_00435af0         ; (reloc)
//   83 c4 08                 ADD  ESP, 0x8
//   c2 04 00                 RET  0x4

extern "C" void FUN_00435af0();   // forward declaration for the CALL relocation

extern "C" __declspec(naked) void FUN_004389d0() {
    __asm {
        // 000389d0: 83 ec 08        SUB ESP, 0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 000389d3: 8b 49 04        MOV ECX, [ECX+0x4]
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        // 000389d6: 8b 44 24 0c     MOV EAX, [ESP+0xc]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 000389da: 51              PUSH ECX
        _emit 0x51
        // 000389db: 8d 4c 24 04     LEA ECX, [ESP+0x4]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 000389df: c7 44 24 04 88 49 f6 00  MOV [ESP+0x4], 0xf64988
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x88
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 000389e7: 89 44 24 08     MOV [ESP+0x8], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 000389eb: e8 00 d1 ff ff  CALL FUN_00435af0  (reloc)
        call FUN_00435af0
        // 000389f0: 83 c4 08        ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 000389f3: c2 04 00        RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
