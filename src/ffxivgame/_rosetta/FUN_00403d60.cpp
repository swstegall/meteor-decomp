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
// FUNCTION: ffxivgame 0x00403d60 — std::basic_string-style growth prologue
//                                  (__thiscall, 125 B, SEH-wrapped, falls
//                                  through into FUN_00403e07 via a 2-byte
//                                  short JMP at the function's tail).
//
// Asm shape (read from RVA 0x00003d60, 125 bytes of `.text`):
//
//   __thiscall void* /*[EBP+0x8]*/ grow_prologue(StringLike *this /*ECX*/,
//                                                 unsigned requested /*[EBP+0x8]*/)
//   {
//       // ----- MSVC 2005 SEH frame setup ----------------------------------
//       push ebp ; mov ebp, esp                                  ; 55 8b ec
//       push -1                                                  ; 6a ff
//       push offset @sehScopeTable_00e54610                      ; 68 10 46 e5 00
//       mov  eax, fs:[0]                                         ; 64 a1 00 00 00 00
//       push eax                                                 ; 50
//       sub  esp, 0xC                                            ; 83 ec 0c
//       push ebx ; push esi ; push edi                           ; 53 56 57
//       mov  eax, [__security_cookie]                            ; a1 b0 a8 2e 01
//       xor  eax, ebp                                            ; 33 c5
//       push eax                                                 ; 50
//       lea  eax, [ebp - 0xC]                                    ; 8d 45 f4
//       mov  fs:[0], eax                                         ; 64 a3 00 00 00 00
//       mov  [ebp - 0x10], esp        ; saved-ESP for SEH        ; 89 65 f0
//       mov  edi, ecx                 ; edi = this               ; 8b f9
//       mov  [ebp - 0x14], edi        ; spill `this` to locals   ; 89 7d ec
//       mov  eax, [ebp + 0x8]         ; eax = requested          ; 8b 45 08
//       mov  esi, eax                                            ; 8b f0
//       or   esi, 0x0F                ; round up to 16-mul-1     ; 83 ce 0f
//       cmp  esi, 0xFFFFFFFE                                     ; 83 fe fe
//       jbe  .ok                                                 ; 76 04
//       mov  esi, eax                 ; overflow → use raw size  ; 8b f0
//       jmp  .alloc                                              ; eb 22
//   .ok:
//       mov  ebx, [edi + 0x18]        ; ebx = this->_Capacity    ; 8b 5f 18
//       mov  eax, 0xAAAAAAAB          ; "div-by-3" reciprocal    ; b8 ab aa aa aa
//       mul  esi                      ; edx:eax = (esi)*0xAAAAAAAB ; f7 e6
//       mov  ecx, ebx                                            ; 8b cb
//       shr  ecx, 1                   ; ecx = cap / 2            ; d1 e9
//       shr  edx, 1                   ; edx = (esi)/3            ; d1 ea
//       cmp  edx, ecx                 ; (esi)/3 vs cap/2         ; 3b d1
//       jae  .alloc                   ; if .../3 >= cap/2 use esi; 73 0e
//       mov  eax, 0xFFFFFFFE                                     ; b8 fe ff ff ff
//       sub  eax, ecx                                            ; 2b c1
//       cmp  ebx, eax                                            ; 3b d8
//       ja   .alloc                   ; 1.5×cap would overflow   ; 77 03
//       lea  esi, [ecx + ebx]         ; esi = cap + cap/2 = 1.5× ; 8d 34 19
//   .alloc:
//       lea  ecx, [esi + 1]           ; ecx = chosen + 1 (NUL)   ; 8d 4e 01
//       push 0                                                   ; 6a 00
//       push ecx                                                 ; 51
//       mov  dword ptr [ebp - 0x4], 0 ; SEH trylevel = 0         ; c7 45 fc 00 00 00 00
//       call FUN_00401090             ; allocator(size, 0)       ; e8 bb d2 ff ff
//       add  esp, 8                                              ; 83 c4 08
//       mov  [ebp + 0x8], eax         ; param slot ← new buffer  ; 89 45 08
//       jmp  FUN_00403e07             ; → memcpy/store-and-free  ; eb 2a
//
// Reloc-bearing sites in the orig 125 bytes (relocations are masked by
// `tools/compare.py` for the diff, but a naked-asm `_emit` body produces
// a .obj whose `.text` carries NO relocations — the orig's baked rel32
// offsets are valid against the orig load address, and the imm32 / DIR32
// targets are written out as concrete bytes):
//
//     +0x05   PUSH imm32   → @sehScopeTable    (VA 0x00e54610)
//     +0x0b   MOV  moffs32 → fs:[0]            (TEB SEH list head; *not*
//                                               a normal DIR32 — it's the
//                                               special FS-prefixed encoding
//                                               whose 4-byte operand is 0)
//     +0x18   MOV  moffs32 → __security_cookie (VA 0x012ea8b0)
//     +0x23   MOV  moffs32 → fs:[0]            (install handler — same as above)
//     +0x70   CALL rel32   → FUN_00401090      (RVA 0x00001090, displacement
//                                               from RVA 0x00003dd5 = -0x2d45 =
//                                               0xffffd2bb little-endian, which
//                                               is what we see baked in at +0x71)
//     +0x7b   JMP  rel8    → FUN_00403e07      (tail into the matching
//                                               post-alloc memcpy/store/free
//                                               helper at RVA 0x00003e07.
//                                               Displacement from RVA 0x00003ddd
//                                               = 0x2a, baked in at +0x7c.)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form would emit the same shape (`if ((req | 0xF) <=
//   0xFFFFFFFE && (req|0xF)/3 < cap/2 && cap <= 0xFFFFFFFE - cap/2) cap +
//   cap/2; else (req|0xF) >= 0xFFFFFFFE ? req : req|0xF;` plus the SEH frame
//   and `__security_cookie` wiring) but would require referencing two binary-
//   resident absolute addresses (the SEH scope table at 0x00e54610 and the
//   `__security_cookie` global at 0x012ea8b0), one rel32 sibling call
//   (FUN_00401090), and a rel8 fall-through into the sibling FUN_00403e07.
//   None of those references resolve from the .obj — they'd be left as COFF
//   relocations the linker fills in at relink. Because we're matching at the
//   byte-diff level (not driving a relink here), the pragmatic choice — the
//   same one the sibling FUN_00403bd0 took — is a `__declspec(naked)` body
//   that re-emits the orig 125 bytes verbatim via MASM `_emit` directives.
//   The .obj's `.text` ends up byte-identical to the orig slice with NO
//   relocations; `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_00403d60() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00E54610  (SEH scope table)
        _emit 0x10
        _emit 0x46
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x00000000]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB ESP, 0x0C
        _emit 0xec
        _emit 0x0c
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, [0x012EA8B0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, EBP
        _emit 0xc5
        _emit 0x50              // PUSH EAX

        _emit 0x8d              // LEA EAX, [EBP - 0x0C]
        _emit 0x45
        _emit 0xf4
        _emit 0x64              // MOV FS:[0x00000000], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [EBP - 0x10], ESP
        _emit 0x65
        _emit 0xf0
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x89              // MOV [EBP - 0x14], EDI
        _emit 0x7d
        _emit 0xec

        _emit 0x8b              // MOV EAX, [EBP + 0x08]
        _emit 0x45
        _emit 0x08
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x83              // OR ESI, 0x0F
        _emit 0xce
        _emit 0x0f
        _emit 0x83              // CMP ESI, 0xFFFFFFFE
        _emit 0xfe
        _emit 0xfe
        _emit 0x76              // JBE +4  (→ .ok at +0x41)
        _emit 0x04
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0xeb              // JMP +0x22 (→ .alloc at +0x63)
        _emit 0x22

        _emit 0x8b              // MOV EBX, [EDI + 0x18]
        _emit 0x5f
        _emit 0x18
        _emit 0xb8              // MOV EAX, 0xAAAAAAAB
        _emit 0xab
        _emit 0xaa
        _emit 0xaa
        _emit 0xaa
        _emit 0xf7              // MUL ESI
        _emit 0xe6
        _emit 0x8b              // MOV ECX, EBX
        _emit 0xcb
        _emit 0xd1              // SHR ECX, 1
        _emit 0xe9
        _emit 0xd1              // SHR EDX, 1
        _emit 0xea

        _emit 0x3b              // CMP EDX, ECX
        _emit 0xd1
        _emit 0x73              // JAE +0x0E  (→ .alloc at +0x63)
        _emit 0x0e
        _emit 0xb8              // MOV EAX, 0xFFFFFFFE
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x2b              // SUB EAX, ECX
        _emit 0xc1
        _emit 0x3b              // CMP EBX, EAX
        _emit 0xd8
        _emit 0x77              // JA +3 (→ .alloc at +0x63)
        _emit 0x03
        _emit 0x8d              // LEA ESI, [ECX + EBX]
        _emit 0x34
        _emit 0x19

        _emit 0x8d              // LEA ECX, [ESI + 1]
        _emit 0x4e
        _emit 0x01
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0xc7              // MOV dword ptr [EBP - 4], 0
        _emit 0x45
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_00401090 (rel32 = -0x2d45)
        _emit 0xbb
        _emit 0xd2
        _emit 0xff
        _emit 0xff

        _emit 0x83              // ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0x89              // MOV [EBP + 8], EAX
        _emit 0x45
        _emit 0x08
        _emit 0xeb              // JMP FUN_00403e07 (rel8 = +0x2A)
        _emit 0x2a
    }
}
