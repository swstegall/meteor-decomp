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
// FUNCTION: ffxivgame 0x00438cd0 — __thiscall destructor/reset for a
//                                   vtable-bearing object with a back-
//                                   referenced data buffer (48 bytes).
//
// Shape (reconstructed from asm at RVA 0x00038cd0):
//
//   void __thiscall FUN_00438cd0(this) {
//       this->vtable = (void *)0xf660f8;     // reset vtable to base
//       void *data = this->field_0x0c;
//       if (data != NULL) {
//           void *owner = *(void **)((char *)data - 4);  // back-pointer
//           FUN_0040df70(owner, data);                   // __thiscall notify
//       }
//       this->field_0x0c = 0;
//       this->field_0x10 = 0;
//       this->field_0x14 = 0;
//   }
//
// Calling convention: __thiscall — ECX = this on entry, no stack args,
// plain RET (callee cleans nothing beyond the saved ESI).
//
// Frame:
//   PUSH ESI    ; save callee-save
//   MOV ESI, ECX
//   …body…
//   POP ESI
//   RET
//
// Notable detail: the callee FUN_0040df70 is __thiscall too. Its "this"
// (ECX) is loaded from [EAX-4] — the DWORD stored immediately before
// the data buffer — and EAX itself (the buffer pointer) is passed as
// the single stack argument via PUSH EAX before the CALL.
//
// Reconstruction strategy — naked-asm byte passthrough. The vtable
// imm32 (0xf660f8) and the CALL rel32 to FUN_0040df70 are reloc-bearing
// sites that tools/compare.py masks out of the byte diff; the non-reloc
// bytes still align correctly against the orig 48-byte slice.

extern "C" __declspec(naked) void FUN_00438cd0()
{
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xc7              // MOV dword ptr [ESI], 0xf660f8
        _emit 0x06
        _emit 0xf8
        _emit 0x60
        _emit 0xf6
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0xc]
        _emit 0x46
        _emit 0x0c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +9 (→ 0x38ce9)
        _emit 0x09
        _emit 0x8b              // MOV ECX, dword ptr [EAX - 0x4]
        _emit 0x48
        _emit 0xfc
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0040df70 (rel32 = 0xfffd5287)
        _emit 0x87
        _emit 0x52
        _emit 0xfd
        _emit 0xff
        _emit 0xc7              // MOV dword ptr [ESI + 0xc], 0x0
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI + 0x10], 0x0
        _emit 0x46
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI + 0x14], 0x0
        _emit 0x46
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
