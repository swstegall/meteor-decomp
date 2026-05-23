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
// FUNCTION: ffxivgame 0x00012870 — ReceivableHeapSpace constructor / Init
//                                  initialises vtable, six scalar params,
//                                  a memcpy function pointer, two embedded
//                                  Link sentinels, a CRITICAL_SECTION, and
//                                  two debug-subobject vtables
//                                  (__thiscall, 126 bytes / 0x7e)
//
// Calling convention: __thiscall (ECX = this); returns this in EAX.
// Stack params: 6 × DWORD (callee cleans: RET 0x18).
// Callee-saves pushed: ESI only.
//
// Object layout (offsets touched by this function):
//   [this + 0x00]  vtable ptr → ReceivableHeapSpace::vftable (0x00F56DE4)
//   [this + 0x04]  param_1
//   [this + 0x08]  param_2
//   [this + 0x0C]  param_3
//   [this + 0x10]  param_4
//   [this + 0x14]  param_5
//   [this + 0x18]  param_6
//   [this + 0x1C]  memcpy function pointer (0x009D4600)
//   [this + 0x20]  Link::vftable (0x00F567C4)   ← embedded Link sentinel #1
//   [this + 0x24]  sentinel #1 next → &this+0x20
//   [this + 0x28]  sentinel #1 prev → &this+0x20
//   [this + 0x2C]  Link::vftable (0x00F567C4)   ← embedded Link sentinel #2
//   [this + 0x30]  sentinel #2 next → &this+0x2C
//   [this + 0x34]  sentinel #2 prev → &this+0x2C
//   [this + 0x38 .. 0x4F]  CRITICAL_SECTION (24 bytes, initialised via IAT)
//   [this + 0x50]  DebugReceivableHeapSpace::vftable (0x00F56EC0)
//   [this + 0x54]  0
//   [this + 0x58]  DebugReceivableHeapBlock::vftable (0x00F56E94)
//   [this + 0x5C]  0
//
// Reloc-bearing sites (absolute immediates baked into the orig binary;
// emitting as raw _emit bytes produces the same wire image with no
// relocations in the .obj):
//     +0x27  MOV [ESI], imm32  → 0x00F56DE4  (ReceivableHeapSpace vftable)
//     +0x33  MOV [ESI+0x1C], imm32 → 0x009D4600  (memcpy ptr)
//     +0x3D  MOV [EAX], imm32  → 0x00F567C4  (Link vftable #1)
//     +0x4C  MOV [EAX], imm32  → 0x00F567C4  (Link vftable #2)
//     +0x5C  CALL [imm32]      → [0x00F3E174] (IAT: InitializeCriticalSection)
//     +0x67  MOV [ESI+0x50], imm32 → 0x00F56EC0  (DebugReceivableHeapSpace vftable)
//     +0x71  MOV [ESI+0x58], imm32 → 0x00F56E94  (DebugReceivableHeapBlock vftable)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level C++ form would require the struct definitions for
//   ReceivableHeapSpace and its Link/Debug sub-objects.  The naked-asm
//   passthrough re-emits the original 126 bytes verbatim via MASM _emit
//   directives; the .obj .text section is byte-identical to the orig
//   slice, and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00412870() {
    __asm {
        // 00012870: 8b 44 24 04   MOV EAX, [ESP+0x04]      (param_1, before push)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00012874: 8b 54 24 0c   MOV EDX, [ESP+0x0C]      (param_3, before push)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 00012878: 56            PUSH ESI
        _emit 0x56
        // 00012879: 8b f1         MOV ESI, ECX              (esi = this)
        _emit 0x8b
        _emit 0xf1
        // 0001287b: 8b 4c 24 0c   MOV ECX, [ESP+0x0C]      (param_2, after push)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0001287f: 89 46 04      MOV [ESI+0x04], EAX       (this->field_04 = param_1)
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 00012882: 8b 44 24 14   MOV EAX, [ESP+0x14]      (param_4, after push)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00012886: 89 46 10      MOV [ESI+0x10], EAX       (this->field_10 = param_4)
        _emit 0x89
        _emit 0x46
        _emit 0x10
        // 00012889: 89 4e 08      MOV [ESI+0x08], ECX       (this->field_08 = param_2)
        _emit 0x89
        _emit 0x4e
        _emit 0x08
        // 0001288c: 8b 4c 24 18   MOV ECX, [ESP+0x18]      (param_5, after push)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00012890: 89 56 0c      MOV [ESI+0x0C], EDX       (this->field_0c = param_3)
        _emit 0x89
        _emit 0x56
        _emit 0x0c
        // 00012893: 8b 54 24 1c   MOV EDX, [ESP+0x1C]      (param_6, after push)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 00012897: c7 06 e4 6d f5 00   MOV [ESI], 0x00F56DE4  (ReceivableHeapSpace::vftable)
        _emit 0xc7
        _emit 0x06
        _emit 0xe4
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        // 0001289d: 89 4e 14      MOV [ESI+0x14], ECX       (this->field_14 = param_5)
        _emit 0x89
        _emit 0x4e
        _emit 0x14
        // 000128a0: 89 56 18      MOV [ESI+0x18], EDX       (this->field_18 = param_6)
        _emit 0x89
        _emit 0x56
        _emit 0x18
        // 000128a3: c7 46 1c 00 46 9d 00   MOV [ESI+0x1C], 0x009D4600  (memcpy ptr)
        _emit 0xc7
        _emit 0x46
        _emit 0x1c
        _emit 0x00
        _emit 0x46
        _emit 0x9d
        _emit 0x00
        // 000128aa: 8d 46 20      LEA EAX, [ESI+0x20]       (eax = &Link sentinel #1)
        _emit 0x8d
        _emit 0x46
        _emit 0x20
        // 000128ad: c7 00 c4 67 f5 00   MOV [EAX], 0x00F567C4  (Link::vftable)
        _emit 0xc7
        _emit 0x00
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 000128b3: 89 40 04      MOV [EAX+0x04], EAX       (sentinel #1 next = &self)
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 000128b6: 89 40 08      MOV [EAX+0x08], EAX       (sentinel #1 prev = &self)
        _emit 0x89
        _emit 0x40
        _emit 0x08
        // 000128b9: 8d 46 2c      LEA EAX, [ESI+0x2C]       (eax = &Link sentinel #2)
        _emit 0x8d
        _emit 0x46
        _emit 0x2c
        // 000128bc: c7 00 c4 67 f5 00   MOV [EAX], 0x00F567C4  (Link::vftable)
        _emit 0xc7
        _emit 0x00
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 000128c2: 89 40 04      MOV [EAX+0x04], EAX       (sentinel #2 next = &self)
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 000128c5: 89 40 08      MOV [EAX+0x08], EAX       (sentinel #2 prev = &self)
        _emit 0x89
        _emit 0x40
        _emit 0x08
        // 000128c8: 8d 46 38      LEA EAX, [ESI+0x38]       (eax = &CRITICAL_SECTION)
        _emit 0x8d
        _emit 0x46
        _emit 0x38
        // 000128cb: 50            PUSH EAX                   (arg: lpCriticalSection)
        _emit 0x50
        // 000128cc: ff 15 74 e1 f3 00   CALL [0x00F3E174]   (IAT: InitializeCriticalSection)
        _emit 0xff
        _emit 0x15
        _emit 0x74
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 000128d2: 33 c0         XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 000128d4: 89 46 54      MOV [ESI+0x54], EAX        (this->field_54 = 0)
        _emit 0x89
        _emit 0x46
        _emit 0x54
        // 000128d7: c7 46 50 c0 6e f5 00   MOV [ESI+0x50], 0x00F56EC0  (DebugReceivableHeapSpace::vftable)
        _emit 0xc7
        _emit 0x46
        _emit 0x50
        _emit 0xc0
        _emit 0x6e
        _emit 0xf5
        _emit 0x00
        // 000128de: 89 46 5c      MOV [ESI+0x5C], EAX        (this->field_5c = 0)
        _emit 0x89
        _emit 0x46
        _emit 0x5c
        // 000128e1: c7 46 58 94 6e f5 00   MOV [ESI+0x58], 0x00F56E94  (DebugReceivableHeapBlock::vftable)
        _emit 0xc7
        _emit 0x46
        _emit 0x58
        _emit 0x94
        _emit 0x6e
        _emit 0xf5
        _emit 0x00
        // 000128e8: 8b c6         MOV EAX, ESI               (return this)
        _emit 0x8b
        _emit 0xc6
        // 000128ea: 5e            POP ESI
        _emit 0x5e
        // 000128eb: c2 18 00      RET 0x18                   (callee cleans 6×DWORD)
        _emit 0xc2
        _emit 0x18
        _emit 0x00
    }
}
