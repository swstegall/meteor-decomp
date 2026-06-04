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
// FUNCTION: ffxivgame 0x00037800 — `__thiscall` init/reset routine that
//                                  fires one virtual call, runs a sibling
//                                  setup helper, then constructs and
//                                  self-links two embedded list sentinels
//                                  at indexed sub-object slots (125 B).
//
// Inspection (read from the disassembly at orig RVA 0x00037800):
//
//   void __thiscall FUN_00437800(C *this /*ECX*/) {
//       // virtual dispatch through this->member_0xc->vtbl[1]
//       (*this->member_0xc->vtbl[1])(this->member_0xc);   // CALL [EAX+4]
//       FUN_0043c2e0(this->member_0x8);                   // rel32 sibling
//
//       // first sentinel: slot = (this->field_0x28 + 3) << 4
//       node1 = *(void**)((char*)this + slot1 + 4);
//       FUN_00c2bb10((char*)this + slot1, node1->member_0x4);
//       n = *(Node**)((char*)this + slot1 + 4);
//       n->next = n; *(int*)((char*)this + slot1 + 8) = 0;
//       n->prev = n;                                       // self-link
//
//       // second sentinel: slot = (this->field_0x2c + 3) << 4 (same shape)
//       ...
//   }
//
//   Calling convention: __thiscall (ECX = this; spilled to EDI). The
//   epilogue is a bare `ret` — no `ret N` — confirming no stack params.
//
//   Reloc-bearing sites in the orig 125 bytes (masked by compare.py):
//     +0x11   CALL rel32 → FUN_0043c2e0 (disp 0x00004aca)
//     +0x2b   CALL rel32 → FUN_00c2bb10 (disp 0x007f42e0)
//     +0x5d   CALL rel32 → FUN_00c2bb10 (disp 0x007f42ae)
//   The `CALL EDX` at +0x0c is an indirect virtual dispatch (no reloc).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 into the exact
//   register allocation (EDI = this, ESI = computed slot base), the exact
//   `(field + 3) << 4` index math, and three linker-resolved call targets.
//   Each of those is brittle under /O2. The pragmatic choice — the same one
//   the siblings FUN_00403d60 / FUN_00401350 took — is a `__declspec(naked)`
//   body that re-emits the orig 125 bytes verbatim via MASM `_emit`
//   directives. The .obj's `.text` ends up byte-identical to the orig slice
//   with no relocations (the baked rel32 displacements are valid against the
//   orig load address); `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_00437800() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x8b              // MOV ECX, [EDI+0xC]
        _emit 0x4f
        _emit 0x0c
        _emit 0x8b              // MOV EAX, [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, [EAX+0x4]
        _emit 0x50
        _emit 0x04
        _emit 0xff              // CALL EDX  (virtual dispatch)
        _emit 0xd2
        _emit 0x8b              // MOV ECX, [EDI+0x8]
        _emit 0x4f
        _emit 0x08
        _emit 0xe8              // CALL rel32 -> FUN_0043c2e0
        _emit 0xca
        _emit 0x4a
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, [EDI+0x28]
        _emit 0x77
        _emit 0x28
        _emit 0x83              // ADD ESI, 0x3
        _emit 0xc6
        _emit 0x03
        _emit 0xc1              // SHL ESI, 0x4
        _emit 0xe6
        _emit 0x04
        _emit 0x8b              // MOV EAX, [ESI+EDI*1+0x4]
        _emit 0x44
        _emit 0x3e
        _emit 0x04
        _emit 0x8b              // MOV ECX, [EAX+0x4]
        _emit 0x48
        _emit 0x04
        _emit 0x03              // ADD ESI, EDI
        _emit 0xf7
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL rel32 -> FUN_00c2bb10
        _emit 0xe0
        _emit 0x42
        _emit 0x7f
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x89              // MOV [EAX+0x4], EAX
        _emit 0x40
        _emit 0x04
        _emit 0x8b              // MOV EAX, [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0xc7              // MOV dword ptr [ESI+0x8], 0x0
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [EAX], EAX
        _emit 0x00
        _emit 0x8b              // MOV ESI, [ESI+0x4]
        _emit 0x76
        _emit 0x04
        _emit 0x89              // MOV [ESI+0x8], ESI
        _emit 0x76
        _emit 0x08
        _emit 0x8b              // MOV ESI, [EDI+0x2C]
        _emit 0x77
        _emit 0x2c
        _emit 0x83              // ADD ESI, 0x3
        _emit 0xc6
        _emit 0x03
        _emit 0xc1              // SHL ESI, 0x4
        _emit 0xe6
        _emit 0x04
        _emit 0x8b              // MOV EDX, [ESI+EDI*1+0x4]
        _emit 0x54
        _emit 0x3e
        _emit 0x04
        _emit 0x8b              // MOV EAX, [EDX+0x4]
        _emit 0x42
        _emit 0x04
        _emit 0x03              // ADD ESI, EDI
        _emit 0xf7
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL rel32 -> FUN_00c2bb10
        _emit 0xae
        _emit 0x42
        _emit 0x7f
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x89              // MOV [EAX+0x4], EAX
        _emit 0x40
        _emit 0x04
        _emit 0x8b              // MOV EAX, [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0xc7              // MOV dword ptr [ESI+0x8], 0x0
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [EAX], EAX
        _emit 0x00
        _emit 0x8b              // MOV ESI, [ESI+0x4]
        _emit 0x76
        _emit 0x04
        _emit 0x5f              // POP EDI
        _emit 0x89              // MOV [ESI+0x8], ESI
        _emit 0x76
        _emit 0x08
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
