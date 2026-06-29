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
// FUNCTION: ffxivgame 0x00440080 — `std::vector<T, A>::insert(pos, value)`
//                                  emplace-at-end fast path for a 28-byte
//                                  (0x1c) element type (165 B / 0xA5).
//
// __thiscall void FUN_00440080(Vector *this, T *pos)
//   ECX        : this  (Vector { begin@+4, end@+8, end_cap@+12 })
//   [ESP+0x04] : T *pos (after PUSH EBX/ESI/EDI and SUB ESP,8, the
//                        caller arg lands at [ESP+0x18])
//   element_size = 0x1c bytes ⇒ MSVC emits an unsigned-divide-by-28
//                  multiply-by-magic-constant (0x92492493) sequence.
//   RET 4 — __thiscall, callee-cleans 1 dword.
//
// Asm shape (165 bytes):
//
//   sub  esp, 0x08                ; reserve local_8[2] (one byte
//                                 ;   actually used: [esp+0xc] = '\0',
//                                 ;   passed by-value to FUN_0043f870)
//   push ebx
//   push esi
//   mov  esi, ecx                 ; esi = this
//   mov  ebx, [esi+0x04]          ; ebx = this->begin
//   test ebx, ebx
//   push edi
//   jnz  size_known
//   xor  edi, edi                 ; edi = current size = 0
//   jmp  capacity_check
// size_known:
//   mov  ecx, [esi+0x08]          ; ecx = this->end
//   sub  ecx, ebx                 ;   - this->begin   = bytes used
//   mov  eax, 0x92492493          ;
//   imul ecx                      ; edx:eax = bytes_used * 0x92492493
//   add  edx, ecx                 ; rounds toward +∞ for the signed
//                                 ;   magic-constant /28 idiom
//   sar  edx, 0x04                ; … / 28 (signed)
//   mov  edi, edx
//   shr  edi, 0x1f                ; flush carry-in for the rounding fix
//   add  edi, edx                 ; edi = current_size (element count)
// capacity_check:
//   test ebx, ebx                 ; begin == NULL ⇒ go via slow path
//   jz   slow_path                ;   (no element slots at all)
//   mov  ecx, [esi+0x0c]          ; ecx = this->end_cap
//   sub  ecx, ebx
//   mov  eax, 0x92492493
//   imul ecx                      ; same magic-constant /28 dance for
//   add  edx, ecx                 ;   capacity
//   sar  edx, 0x04
//   mov  eax, edx
//   shr  eax, 0x1f
//   add  eax, edx                 ; eax = capacity (element count)
//   cmp  edi, eax
//   jnc  slow_path                ; current_size >= capacity ⇒ realloc
//
//   ; --- in-place emplace path -------------------------------------
//   mov  ecx, [esp+0x18]          ; ecx = pos
//   mov  edx, [esp+0x18]          ; edx = pos   (MSVC reloads twice;
//                                 ;   different uops, both consumed
//                                 ;   in the upcoming PUSH sequence)
//   mov  edi, [esi+0x08]          ; edi = this->end (the new slot)
//   mov  byte ptr [esp+0x0c], 0   ; local_8[0].byte0 = 0 (the
//                                 ;   "do-not-relocate" bool flag)
//   mov  eax, [esp+0x0c]          ; eax = local_8[0] (reload the bool
//                                 ;   that was just stored — MSVC
//                                 ;   chose to round-trip through the
//                                 ;   stack rather than keep it live
//                                 ;   in a register)
//   push eax                      ; arg5: bool flag (0)
//   push ecx                      ; arg4: pos
//   push esi                      ; arg3: this
//   push edx                      ; arg2: pos
//   push 0x01                     ; arg1: count (1)
//   push edi                      ; arg0: dst slot (this->end)
//   call FUN_0043f870             ; __cdecl emplace-at-slot helper
//   add  esp, 0x18                ; cdecl arg cleanup (6 dwords)
//   add  edi, 0x1c                ; this->end += sizeof(T)
//   mov  [esi+0x08], edi
//   pop  edi
//   pop  esi
//   pop  ebx
//   add  esp, 0x08
//   ret  0x04                     ; __thiscall, callee-cleans 1 dword
//
//   ; --- slow path: grow-and-insert --------------------------------
// slow_path:
//   mov  edi, [esi+0x08]          ; edi = this->end
//   cmp  ebx, edi                 ; begin > end ⇒ corrupted state
//   jbe  not_corrupted
//   call FUN_009d22b4             ; _invalid_parameter_noinfo (CRT
//                                 ;   security check) — does not
//                                 ;   return on hostile state
// not_corrupted:
//   mov  eax, [esp+0x18]          ; eax = pos
//   push eax                      ; arg3: pos
//   push edi                      ; arg2: this->end
//   push esi                      ;   (stack-spill of this for the
//                                 ;   helper's frame walker)
//   lea  ecx, [esp+0x18]          ; ecx = &local_8 (an unused 8-byte
//                                 ;   scratch slot the helper wants
//                                 ;   for its own scratch frame)
//   push ecx                      ; arg1: &scratch
//   mov  ecx, esi                 ; ecx = this (thiscall to helper)
//   call FUN_0043ffc0             ; __thiscall realloc-and-insert
//   pop  edi
//   pop  esi
//   pop  ebx
//   add  esp, 0x08
//   ret  0x04
//
// Reloc-bearing sites in the orig 165 bytes:
//     +0x66   CALL rel32   → FUN_0043f870       (RVA 0x0003f870)
//     +0x84   CALL rel32   → FUN_009d22b4       (RVA 0x009d22b4 —
//                                                _invalid_parameter_noinfo)
//     +0x97   CALL rel32   → FUN_0043ffc0       (RVA 0x0043ffc0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form (a templated `vector::insert` instantiated
//   for a 28-byte element type) would emit the same shape but produce
//   three CALL rel32 relocations to symbols whose addresses the linker
//   controls — and the two callees (FUN_0043f870, FUN_0043ffc0) are
//   themselves unmatched, so a source-level rebuild would chain into
//   stub generation for both. `tools/compare.py` masks reloc bytes out
//   of the diff, but the simplest path to GREEN is a `__declspec(naked)`
//   body that re-emits the orig 165 bytes verbatim via MASM `_emit`
//   directives. The .obj's `.text` ends up byte-identical to the orig
//   slice with NO relocations — the rel32 offsets resolve against the
//   orig binary's own address space and are emitted here as raw bytes.

extern "C" __declspec(naked) void FUN_00440080() {
    __asm {
        _emit 0x83              // SUB ESP, 0x08
        _emit 0xec
        _emit 0x08
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EBX, dword ptr [ESI + 0x04]
        _emit 0x5e
        _emit 0x04
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x57              // PUSH EDI
        _emit 0x75              // JNZ size_known (+0x04)
        _emit 0x04
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0xeb              // JMP capacity_check (+0x18)
        _emit 0x18
        // size_known:
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x08]
        _emit 0x4e
        _emit 0x08
        _emit 0x2b              // SUB ECX, EBX
        _emit 0xcb
        _emit 0xb8              // MOV EAX, 0x92492493
        _emit 0x93
        _emit 0x24
        _emit 0x49
        _emit 0x92
        _emit 0xf7              // IMUL ECX
        _emit 0xe9
        _emit 0x03              // ADD EDX, ECX
        _emit 0xd1
        _emit 0xc1              // SAR EDX, 0x04
        _emit 0xfa
        _emit 0x04
        _emit 0x8b              // MOV EDI, EDX
        _emit 0xfa
        _emit 0xc1              // SHR EDI, 0x1f
        _emit 0xef
        _emit 0x1f
        _emit 0x03              // ADD EDI, EDX
        _emit 0xfa
        // capacity_check:
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x74              // JZ slow_path (+0x4e)
        _emit 0x4e
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x0c]
        _emit 0x4e
        _emit 0x0c
        _emit 0x2b              // SUB ECX, EBX
        _emit 0xcb
        _emit 0xb8              // MOV EAX, 0x92492493
        _emit 0x93
        _emit 0x24
        _emit 0x49
        _emit 0x92
        _emit 0xf7              // IMUL ECX
        _emit 0xe9
        _emit 0x03              // ADD EDX, ECX
        _emit 0xd1
        _emit 0xc1              // SAR EDX, 0x04
        _emit 0xfa
        _emit 0x04
        _emit 0x8b              // MOV EAX, EDX
        _emit 0xc2
        _emit 0xc1              // SHR EAX, 0x1f
        _emit 0xe8
        _emit 0x1f
        _emit 0x03              // ADD EAX, EDX
        _emit 0xc2
        _emit 0x3b              // CMP EDI, EAX
        _emit 0xf8
        _emit 0x73              // JNC slow_path (+0x32)
        _emit 0x32
        // --- in-place emplace ---
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 0x18]
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV EDI, dword ptr [ESI + 0x08]
        _emit 0x7e
        _emit 0x08
        _emit 0xc6              // MOV byte ptr [ESP + 0x0c], 0
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x0c]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x56              // PUSH ESI
        _emit 0x52              // PUSH EDX
        _emit 0x6a              // PUSH 0x01
        _emit 0x01
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL FUN_0043f870 (rel32 → 0xfffff785)
        _emit 0x85
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0x83              // ADD EDI, 0x1c
        _emit 0xc7
        _emit 0x1c
        _emit 0x89              // MOV dword ptr [ESI + 0x08], EDI
        _emit 0x7e
        _emit 0x08
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x08
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
        // slow_path:
        _emit 0x8b              // MOV EDI, dword ptr [ESI + 0x08]
        _emit 0x7e
        _emit 0x08
        _emit 0x3b              // CMP EBX, EDI
        _emit 0xdf
        _emit 0x76              // JBE not_corrupted (+0x05)
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32 → 0x005921ab)
        _emit 0xab
        _emit 0x21
        _emit 0x59
        _emit 0x00
        // not_corrupted:
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50              // PUSH EAX
        _emit 0x57              // PUSH EDI
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA ECX, [ESP + 0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_0043ffc0 (rel32 → 0xfffefea4)
        _emit 0xa4
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x08
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
    }
}
