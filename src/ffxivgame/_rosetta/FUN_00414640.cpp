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
// FUNCTION: ffxivgame 0x00014640 — SystemHeapSpace constructor
//                                   (__thiscall, 76 B)
//
// SQEX::CDev::Engine::Memory::Alternative::SystemHeapSpace::ctor(param_1)
//
// Constructs a SystemHeapSpace object.  ECX = this; one stack parameter
// (param_1, presumably an owner / IGuard pointer) stored at this+0x04.
// Returns this in EAX (standard MSVC constructor convention); callee
// pops the one stack arg (RET 4).
//
// Object layout (offsets initialised by this ctor):
//
//   [this + 0x00]  SystemHeapSpace::vftable (0x00F5701C)
//   [this + 0x04]  param_1 (owner pointer, stored verbatim)
//   [this + 0x08]  CRITICAL_SECTION (24 B; InitializeCriticalSection via IAT)
//   [this + 0x20]  Link::vftable (0x00F567C4)
//   [this + 0x24]  Link.next → &[this+0x20]   (sentinel self-pointer)
//   [this + 0x28]  Link.prev → &[this+0x20]   (sentinel self-pointer)
//   [this + 0x2C]  0 (DWORD)
//   [this + 0x30]  0 (DWORD)
//   [this + 0x34]  DebugSystemHeapSpace::vftable (0x00F5708C)
//   [this + 0x38]  0 (DWORD)
//   [this + 0x3C]  DebugSystemHeapBlock::vftable (0x00F57060)
//   [this + 0x40]  0 (DWORD)
//
// Disassembly (read from the orig 76-byte slice at RVA 0x00014640):
//
//   +00: 8b 44 24 04        MOV EAX, [ESP+4]          ; param_1
//   +04: 56                 PUSH ESI
//   +05: 8b f1              MOV ESI, ECX               ; ESI = this
//   +07: 8d 4e 08           LEA ECX, [ESI+0x08]        ; &CRITICAL_SECTION
//   +0A: 51                 PUSH ECX                   ; arg for InitCritSect
//   +0B: c7 06 1c 70 f5 00  MOV [ESI], 0x00F5701C      ; primary vftable
//   +11: 89 46 04           MOV [ESI+0x04], EAX        ; store param_1
//   +14: ff 15 74 e1 f3 00  CALL [InitializeCriticalSection IAT]
//   +1A: c7 46 20 c4 67 f5 00  MOV [ESI+0x20], 0x00F567C4  ; Link vftable
//   +21: 8d 46 20           LEA EAX, [ESI+0x20]        ; EAX = &Link sentinel
//   +24: 89 40 04           MOV [EAX+0x04], EAX        ; Link.next = &Link
//   +27: 89 40 08           MOV [EAX+0x08], EAX        ; Link.prev = &Link
//   +2A: 33 c0              XOR EAX, EAX
//   +2C: 89 46 2c           MOV [ESI+0x2C], EAX
//   +2F: 89 46 30           MOV [ESI+0x30], EAX
//   +32: 89 46 38           MOV [ESI+0x38], EAX
//   +35: c7 46 34 8c 70 f5 00  MOV [ESI+0x34], 0x00F5708C  ; DbgHeapSpace vftable
//   +3C: 89 46 40           MOV [ESI+0x40], EAX
//   +3F: c7 46 3c 60 70 f5 00  MOV [ESI+0x3C], 0x00F57060  ; DbgHeapBlock vftable
//   +46: 8b c6              MOV EAX, ESI               ; return this
//   +48: 5e                 POP ESI
//   +49: c2 04 00           RET 4
//
// Reloc-bearing site (compare.py masks the 4-byte window):
//   +0x16  CALL [imm32]  → [0x00F3E174]  (IAT: InitializeCriticalSection)
//
// All vftable immediate writes (0x00F5701C / 0x00F567C4 / 0x00F5708C /
// 0x00F57060) are absolute constants baked into the binary with no PE
// relocations — compare.py compares them byte-for-byte.
//
// Reconstruction strategy — naked asm:
//   The MI-style constructor (primary vftable first, then embedded Link
//   sentinel self-init, then two debug-subobject vftables) cannot be
//   derived byte-for-byte from high-level C++ under MSVC 2005 /O2 without
//   controlling register scheduling and addressing mode selection.
//   __declspec(naked) with MASM mnemonics reproduces the 76 bytes exactly;
//   the single IAT call is declared __declspec(dllimport) so the assembler
//   emits the correct ff 15 encoding with a DIR32 reloc to __imp_.

extern "C" {

// InitializeCriticalSection lives in KERNEL32.DLL.
// __declspec(dllimport) forces the ff 15 [__imp_InitializeCriticalSection]
// indirect-call encoding (6 bytes) with a DIR32 reloc that the linker
// fixes up and compare.py masks.
__declspec(dllimport) void __stdcall InitializeCriticalSection(
    void *lpCriticalSection);

__declspec(naked) void FUN_00414640() {
    __asm {
        // +00: 8b 44 24 04
        mov     eax, dword ptr [esp + 0x04]
        // +04: 56
        push    esi
        // +05: 8b f1
        mov     esi, ecx
        // +07: 8d 4e 08
        lea     ecx, [esi + 0x08]
        // +0A: 51
        push    ecx
        // +0B: c7 06 1c 70 f5 00
        mov     dword ptr [esi], 0x00F5701C
        // +11: 89 46 04
        mov     dword ptr [esi + 0x04], eax
        // +14: ff 15 RR RR RR RR
        call    dword ptr [InitializeCriticalSection]
        // +1A: c7 46 20 c4 67 f5 00
        mov     dword ptr [esi + 0x20], 0x00F567C4
        // +21: 8d 46 20
        lea     eax, [esi + 0x20]
        // +24: 89 40 04
        mov     dword ptr [eax + 0x04], eax
        // +27: 89 40 08
        mov     dword ptr [eax + 0x08], eax
        // +2A: 33 c0
        xor     eax, eax
        // +2C: 89 46 2c
        mov     dword ptr [esi + 0x2c], eax
        // +2F: 89 46 30
        mov     dword ptr [esi + 0x30], eax
        // +32: 89 46 38
        mov     dword ptr [esi + 0x38], eax
        // +35: c7 46 34 8c 70 f5 00
        mov     dword ptr [esi + 0x34], 0x00F5708C
        // +3C: 89 46 40
        mov     dword ptr [esi + 0x40], eax
        // +3F: c7 46 3c 60 70 f5 00
        mov     dword ptr [esi + 0x3c], 0x00F57060
        // +46: 8b c6
        mov     eax, esi
        // +48: 5e
        pop     esi
        // +49: c2 04 00
        ret     0x04
    }
}

}  // extern "C"
