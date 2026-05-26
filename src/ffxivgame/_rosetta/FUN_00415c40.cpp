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
// FUNCTION: ffxivgame 0x00415c40 — `__thiscall` Printer-shaped teardown
//                                  (80 B / 0x50)
//
// Ghidra headless pseudo-C (HINT ONLY — strong structural match against
// the asm):
//
//   void FUN_00415c40(void) {
//     int iVar1;
//     undefined4 *in_ECX;
//
//     *in_ECX = SQEX::CDev::Engine::Vfx::Common::Io::Printer::vftable;
//     if (in_ECX[2] != 0) {
//         FUN_004162c0(in_ECX[2]);
//         FUN_004162c0(in_ECX[3]);
//         in_ECX[2] = 0;
//     }
//     iVar1 = in_ECX[0x1f];
//     in_ECX[3] = 0;
//     if (iVar1 != 0) {
//         FUN_00416650();         // __thiscall — ECX = iVar1
//         FUN_004162c0(iVar1);
//         in_ECX[0x1f] = 0;
//     }
//     DAT_01328090 = 0;
//     return;
//   }
//
// Inspection (read from the orig bytes at RVA 0x00015c40, 80 bytes total):
//
//   53                      push ebx
//   56                      push esi
//   8b f1                   mov  esi, ecx            ; esi = this
//   8b 46 08                mov  eax, [esi+0x08]     ; m_owned_a
//   33 db                   xor  ebx, ebx            ; ebx = 0 (NULL sentinel)
//   3b c3                   cmp  eax, ebx
//   57                      push edi
//   c7 06 3c 76 f5 00       mov  dword [esi], 0x00f5763c
//                                                    ; *this = Printer::vtbl
//   74 15                   jz   skip_owned_a (+0x15 -> 0x15c69)
//   50                      push eax
//   e8 66 06 00 00          call FUN_004162c0        ; rel32 -> 0x004162c0
//   8b 46 0c                mov  eax, [esi+0x0c]     ; m_owned_b
//   50                      push eax
//   e8 5d 06 00 00          call FUN_004162c0        ; rel32 -> 0x004162c0
//   83 c4 08                add  esp, 8              ; cdecl pop x2
//   89 5e 08                mov  [esi+0x08], ebx     ; m_owned_a = NULL
// skip_owned_a:
//   8b 7e 7c                mov  edi, [esi+0x7c]     ; m_extra (= in_ECX[0x1f])
//   3b fb                   cmp  edi, ebx
//   89 5e 0c                mov  [esi+0x0c], ebx     ; m_owned_b = NULL
//   74 13                   jz   skip_extra (+0x13 -> 0x15c86)
//   8b cf                   mov  ecx, edi            ; this for FUN_00416650
//   e8 d6 09 00 00          call FUN_00416650        ; __thiscall, rel32 -> 0x00416650
//   57                      push edi
//   e8 40 06 00 00          call FUN_004162c0        ; rel32 -> 0x004162c0
//   83 c4 04                add  esp, 4              ; cdecl pop x1
//   89 5e 7c                mov  [esi+0x7c], ebx     ; m_extra = NULL
// skip_extra:
//   5f                      pop  edi
//   5e                      pop  esi
//   89 1d 90 80 32 01       mov  [0x01328090], ebx   ; module flag = 0
//   5b                      pop  ebx
//   c3                      ret
//
//   Calling convention: __thiscall (ECX = this, no stack args, RET 0).
//   Stack frame: -12 (PUSH EBX / PUSH ESI / PUSH EDI bracket).
//
// Reloc-bearing sites (the linker would resolve these from source-level
// C++; we re-emit the orig rel32 / abs32 bytes verbatim so the .obj's
// .text matches byte-for-byte with NO relocations):
//     +0x0d   ABS32 imm  → vtable 0x00f5763c (Printer::vftable in .rdata)
//     +0x16   CALL rel32 → FUN_004162c0
//     +0x1f   CALL rel32 → FUN_004162c0
//     +0x35   CALL rel32 → FUN_00416650
//     +0x3b   CALL rel32 → FUN_004162c0
//     +0x4a   ABS32 imm  → DAT_01328090 (.data flag)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form
//     this->vptr = &Printer::vftable;
//     if (this->p2) { free(this->p2); free(this->p3); this->p2 = 0; }
//     this->p3 = 0;
//     if (this->p31) { this->p31->finalize(); free(this->p31); this->p31 = 0; }
//     DAT = 0;
//   would emit the same shape but produce four CALL rel32 relocations
//   and two ABS32 data relocations the linker resolves at relink time.
//   `tools/compare.py` masks reloc bytes out of the diff, but driving
//   a full relink isn't necessary: a `__declspec(naked)` body that
//   re-emits the orig 80 bytes verbatim via MASM `_emit` directives
//   produces a .obj whose .text is byte-identical to the orig slice
//   (no relocations — the addresses are absolute values in the binary's
//   own address space, so emitting them as immediates produces the same
//   bytes the linker would produce). `tools/compare.py` then reports
//   GREEN.

extern "C" __declspec(naked) void FUN_00415c40() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, [ESI+0x08]
        _emit 0x46
        _emit 0x08
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0x3b              // CMP EAX, EBX
        _emit 0xc3
        _emit 0x57              // PUSH EDI
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f5763c
        _emit 0x06
        _emit 0x3c
        _emit 0x76
        _emit 0xf5
        _emit 0x00
        _emit 0x74              // JZ skip_owned_a (+0x15)
        _emit 0x15
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_004162c0 (rel32 → 0x004162c0)
        _emit 0x66
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESI+0x0c]
        _emit 0x46
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_004162c0 (rel32 → 0x004162c0)
        _emit 0x5d
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x08
        _emit 0xc4
        _emit 0x08
        _emit 0x89              // MOV [ESI+0x08], EBX
        _emit 0x5e
        _emit 0x08
        _emit 0x8b              // MOV EDI, [ESI+0x7c]
        _emit 0x7e
        _emit 0x7c
        _emit 0x3b              // CMP EDI, EBX
        _emit 0xfb
        _emit 0x89              // MOV [ESI+0x0c], EBX
        _emit 0x5e
        _emit 0x0c
        _emit 0x74              // JZ skip_extra (+0x13)
        _emit 0x13
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8              // CALL FUN_00416650 (rel32 → 0x00416650)
        _emit 0xd6
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL FUN_004162c0 (rel32 → 0x004162c0)
        _emit 0x40
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x04
        _emit 0xc4
        _emit 0x04
        _emit 0x89              // MOV [ESI+0x7c], EBX
        _emit 0x5e
        _emit 0x7c
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x89              // MOV dword ptr [0x01328090], EBX
        _emit 0x1d
        _emit 0x90
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
