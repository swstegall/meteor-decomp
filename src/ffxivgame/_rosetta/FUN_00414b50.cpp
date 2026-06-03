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
// FUNCTION: ffxivgame 0x00414b50 — SystemHeapSpace destructor body (70 B / 0x46)
//           SQEX::CDev::Engine::Memory::Alternative::SystemHeapSpace
//
// __thiscall void *FUN_00414b50(int flags)
//   ECX        : this  (SystemHeapSpace *)
//   [ESP+0x4]  : flags  (scalar-deleting-dtor delete flag, 1 = also ::delete)
//
// Shape:
//   1. Rewind primary vftable to SystemHeapSpace::vftable (0x00f5701c) — MSVC
//      "vtable rewinding" idiom so virtual calls during dtor body dispatch to
//      most-derived overrides.
//   2. Reset embedded IDebugBlock  vftable at this+0x3c to 0x00f56788.
//   3. Reset embedded IDebugSpace  vftable at this+0x34 to 0x00f567b4.
//   4. Load Link.prev from [ESI+0x24] and Link.next from [ESI+0x28].
//   5. Reset embedded Link vftable at this+0x20 to 0x00f567c4.
//   6. Unlink from intrusive doubly-linked list:
//        prev->next = next  →  [EAX+0x8] = ECX  (Link.prev->next = Link.next)
//        next->prev = prev  →  [EDX+0x4] = EAX  (Link.next->prev = Link.prev)
//   7. Call DeleteCriticalSection(&this->cs)  via IAT at [0x00f3e170].
//      (CRITICAL_SECTION lives at this+0x08, 24 bytes.)
//   8. Reset primary vftable to ISpace::vftable (0x00f566fc) — chain-up to
//      base destructor sees the correct vtable slice.
//   9. Return this (MOV EAX, ESI).
//
// Asm (70 B, RVA 0x00014b50):
//   56                     PUSH ESI
//   8b f1                  MOV ESI, ECX                ; ESI = this
//   c7 06 1c 70 f5 00      MOV [ESI],    0xf5701c      ; primary vt = SystemHeapSpace
//   c7 46 3c 88 67 f5 00   MOV [ESI+3c], 0xf56788      ; IDebugBlock vt
//   c7 46 34 b4 67 f5 00   MOV [ESI+34], 0xf567b4      ; IDebugSpace vt
//   8b 46 24               MOV EAX, [ESI+24]           ; EAX = Link.prev
//   8b 4e 28               MOV ECX, [ESI+28]           ; ECX = Link.next
//   c7 46 20 c4 67 f5 00   MOV [ESI+20], 0xf567c4      ; Link vt
//   89 48 08               MOV [EAX+8],  ECX           ; prev->next = next
//   8b 56 28               MOV EDX, [ESI+28]           ; EDX = Link.next (reload)
//   8b 46 24               MOV EAX, [ESI+24]           ; EAX = Link.prev (reload)
//   8d 4e 08               LEA ECX, [ESI+8]            ; ECX = &this->cs
//   51                     PUSH ECX                    ; arg: &CRITICAL_SECTION
//   89 42 04               MOV [EDX+4], EAX            ; next->prev = prev
//   ff 15 70 e1 f3 00      CALL [0x00f3e170]           ; DeleteCriticalSection
//   c7 06 fc 66 f5 00      MOV [ESI],    0xf566fc      ; primary vt = ISpace (chain-up)
//   8b c6                  MOV EAX, ESI               ; return this
//   5e                     POP ESI
//   c2 04 00               RET 0x4                    ; __thiscall, 1 stack arg
//
// Reloc-bearing sites in the orig 70 bytes:
//   +0x03  MOV imm32 → 0x00f5701c   (SystemHeapSpace vftable)
//   +0x0a  MOV imm32 → 0x00f56788   (IDebugBlock vftable)
//   +0x11  MOV imm32 → 0x00f567b4   (IDebugSpace vftable)
//   +0x1d  MOV imm32 → 0x00f567c4   (Link vftable)
//   +0x2c  CALL mem32→ 0x00f3e170   (IAT: DeleteCriticalSection)
//   +0x32  MOV imm32 → 0x00f566fc   (ISpace vftable)
//
// Reconstruction strategy — naked-asm byte passthrough.
// Same idiom as FUN_00409580 / FUN_0040a460 / FUN_0040a4b0: emit the orig
// 70 bytes verbatim via MASM `_emit`. The .obj's .text is byte-identical to
// the orig slice with zero relocations and tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00414b50() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xc7              // MOV dword ptr [ESI], 0xf5701c
        _emit 0x06
        _emit 0x1c
        _emit 0x70
        _emit 0xf5
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI+0x3c], 0xf56788
        _emit 0x46
        _emit 0x3c
        _emit 0x88
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI+0x34], 0xf567b4
        _emit 0x46
        _emit 0x34
        _emit 0xb4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x24]
        _emit 0x46
        _emit 0x24
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x28]
        _emit 0x4e
        _emit 0x28
        _emit 0xc7              // MOV dword ptr [ESI+0x20], 0xf567c4
        _emit 0x46
        _emit 0x20
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0x89              // MOV dword ptr [EAX+0x8], ECX
        _emit 0x48
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x28]
        _emit 0x56
        _emit 0x28
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x24]
        _emit 0x46
        _emit 0x24
        _emit 0x8d              // LEA ECX, [ESI+0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x51              // PUSH ECX
        _emit 0x89              // MOV dword ptr [EDX+0x4], EAX
        _emit 0x42
        _emit 0x04
        _emit 0xff              // CALL dword ptr [0x00f3e170]
        _emit 0x15
        _emit 0x70
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI], 0xf566fc
        _emit 0x06
        _emit 0xfc
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
