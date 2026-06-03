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
// FUNCTION: ffxivgame 0x004173e0 — __thiscall scalar-deleting destructor
//                                  with ownership flag and member cleanup
//                                  (53 bytes).
//
// Layout (inferred from the asm):
//   This (ECX):
//     +0x00  void*  vftable   (overwritten to 0xf576b4 on entry)
//     +0x04  void*  member    (freed via FUN_004162c0 unless owned externally)
//     +0x12  byte   flags     (bit 0 set = member is not owned, skip free)
//
// Source shape:
//
//   void* __thiscall SomeClass::`scalar deleting destructor`(unsigned int flags) {
//       this->vftable = (void*)0xf576b4;
//       if (!(*(char*)((char*)this + 0x12) & 1)) {
//           void* p = *(void**)((char*)this + 4);
//           if (p)
//               FUN_004162c0(p);
//       }
//       if (flags & 1)
//           FUN_004162c0(this);
//       return this;
//   }
//
// Calling convention: __thiscall (ECX = this; one DWORD stack arg = flags;
// callee cleans via `ret 4`).
//
// Frame: PUSH ESI only; no SUB ESP adjustment; no /GS security cookie.
//
// Branch shape:
//   - JNZ skip_member:  short +0x10 (TEST EFLAGS from bit check)
//   - JZ  skip_member:  short +0x09 (TEST EAX for null member)
//   - JZ  skip_delete:  short +0x09 (TEST EFLAGS from param bit check)
//
// Reloc sites (masked by tools/compare.py):
//   +0x07  IMM32  vtable VA 0xf576b4 (in MOV [ESI], imm32)
//   +0x19  REL32  CALL FUN_004162c0  (member free)
//   +0x27  REL32  CALL FUN_004162c0  (self free)

extern "C" void FUN_004162c0(void*);

extern "C" __declspec(naked) void FUN_004173e0() {
    __asm {
        push    esi
        mov     esi, ecx
        test    byte ptr [esi + 0x12], 0x1
        mov     dword ptr [esi], 0xf576b4
        jnz     skip_member
        mov     eax, dword ptr [esi + 0x4]
        test    eax, eax
        jz      skip_member
        push    eax
        call    FUN_004162c0
        add     esp, 0x4
    skip_member:
        test    byte ptr [esp + 0x8], 0x1
        jz      skip_delete
        push    esi
        call    FUN_004162c0
        add     esp, 0x4
    skip_delete:
        mov     eax, esi
        pop     esi
        ret     0x4
    }
}
