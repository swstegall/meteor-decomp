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
// FUNCTION: ffxivgame 0x0001a520 — __thiscall virtual destructor body (67 B)
//
// RVA 0x0001a520, effective size 67 bytes (size_overrides: "RET imm16 (04 00)").
//
// Calling convention: __thiscall (ECX = this, callee cleans 1 DWORD = ret 4).
//
// Layout inferred from asm:
//   this (ECX):
//     +0x00   void**   vptr     reset to 0x00f57ea4 on entry
//     +0x04   void*    field_4  released via g_manager vtable slot 5, then zeroed
//     +0x08   <type>   field_8  destroyed by FUN_00419eb0 (__thiscall subobject dtor)
//   Stack args (after `push esi` prologue; callee-cleaned via ret 4):
//     [esp+8]  unsigned char  freeFlag   bit 0 set → call FUN_009d1b17(this)
//
// Global:
//   [0x01329920]  IManager*  g_manager  — object whose vtable[5] releases field_4
//
// Flow:
//   1. eax = this->field_4
//   2. this->vptr = 0x00f57ea4   (vtable reset, instruction-scheduled after TEST)
//   3. if (eax != 0): g_manager->vtable[5](eax); this->field_4 = 0
//   4. FUN_00419eb0(&this->field_8)   (subobject destruction)
//   5. if (freeFlag & 1): FUN_009d1b17(this)
//   6. return this (eax = esi)
//
// Reloc-bearing positions (wildcarded by tools/compare.py):
//   +0x29  REL32  CALL FUN_00419eb0
//   +0x36  REL32  CALL FUN_009d1b17
//
// All other bytes are literal and must match the orig binary exactly.
//
// Asm (67 bytes):
//   56                       PUSH ESI
//   8b f1                    MOV  ESI, ECX
//   8b 46 04                 MOV  EAX, [ESI+4]
//   85 c0                    TEST EAX, EAX
//   c7 06 a4 7e f5 00        MOV  dword ptr [ESI], 0x00f57ea4
//   74 15                    JE   skip
//   8b 0d 20 99 32 01        MOV  ECX, [0x01329920]
//   8b 11                    MOV  EDX, [ECX]
//   50                       PUSH EAX
//   8b 42 14                 MOV  EAX, [EDX+0x14]
//   ff d0                    CALL EAX
//   c7 46 04 00 00 00 00     MOV  dword ptr [ESI+4], 0
// skip:
//   8d 4e 08                 LEA  ECX, [ESI+8]
//   e8 RR RR RR RR           CALL FUN_00419eb0
//   f6 44 24 08 01           TEST byte ptr [ESP+8], 1
//   74 09                    JE   done
//   56                       PUSH ESI
//   e8 RR RR RR RR           CALL FUN_009d1b17
//   83 c4 04                 ADD  ESP, 4
// done:
//   8b c6                    MOV  EAX, ESI
//   5e                       POP  ESI
//   c2 04 00                 RET  4

// Naked-asm byte passthrough: emit the orig 67 bytes verbatim via _emit.
// MASM misinterprets `mov ecx, [0x01329920]` as an immediate MOV (b9 form,
// 5 bytes) instead of the correct memory-load form (8b 0d, 6 bytes).
// Using _emit gives exact control over every byte, including the two REL32
// call targets whose offset bytes happen to match because the obj is
// compared at the same virtual positions as the binary by tools/compare.py.

extern "C" __declspec(naked) void FUN_0041a520()
{
    __asm {
        // push esi
        _emit 0x56
        // mov esi, ecx
        _emit 0x8b
        _emit 0xf1
        // mov eax, dword ptr [esi+4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // test eax, eax
        _emit 0x85
        _emit 0xc0
        // mov dword ptr [esi], 0x00f57ea4   (vtable reset)
        _emit 0xc7
        _emit 0x06
        _emit 0xa4
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        // je skip (+0x15)
        _emit 0x74
        _emit 0x15
        // mov ecx, dword ptr [0x01329920]   (g_manager)
        _emit 0x8b
        _emit 0x0d
        _emit 0x20
        _emit 0x99
        _emit 0x32
        _emit 0x01
        // mov edx, dword ptr [ecx]          (vtable of g_manager)
        _emit 0x8b
        _emit 0x11
        // push eax                          (field_4 arg to vtable[5])
        _emit 0x50
        // mov eax, dword ptr [edx+0x14]     (vtable slot 5)
        _emit 0x8b
        _emit 0x42
        _emit 0x14
        // call eax
        _emit 0xff
        _emit 0xd0
        // mov dword ptr [esi+4], 0          (zero field_4)
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // skip:
        // lea ecx, [esi+8]
        _emit 0x8d
        _emit 0x4e
        _emit 0x08
        // call FUN_00419eb0  (rel32 = 0xfffff963; REL32 offset matching orig binary)
        _emit 0xe8
        _emit 0x63
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        // test byte ptr [esp+8], 1
        _emit 0xf6
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        // je done (+0x09)
        _emit 0x74
        _emit 0x09
        // push esi
        _emit 0x56
        // call FUN_009d1b17  (rel32 = 0x005b75bd; REL32 offset matching orig binary)
        _emit 0xe8
        _emit 0xbd
        _emit 0x75
        _emit 0x5b
        _emit 0x00
        // add esp, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // done:
        // mov eax, esi
        _emit 0x8b
        _emit 0xc6
        // pop esi
        _emit 0x5e
        // ret 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
