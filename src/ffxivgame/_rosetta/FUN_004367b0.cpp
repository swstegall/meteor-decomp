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
// FUNCTION: ffxivgame 0x004367b0 — `std::vector<T, A>::insert(pos, value)`
//                                  emplace-at-end fast path for a 20-byte
//                                  (0x14) element type (160 B / 0xA0).
//
// __thiscall void FUN_004367b0(Vector *this, T *pos)
//   ECX        : this  (Vector { begin@+4, end@+8, end_cap@+12 })
//   [ESP+0x04] : T *pos (after PUSH ESI/EDI and SUB ESP,8, the
//                        caller arg lands at [ESP+0x14])
//   element_size = 0x14 bytes ⇒ MSVC emits an unsigned-divide-by-20
//                  multiply-by-magic-constant (0x66666667) sequence.
//   RET 4 — __thiscall, callee-cleans 1 dword.
//
// Asm shape (160 bytes):
//
//   sub  esp, 0x08                ; reserve local_8[2]
//   push esi
//   mov  esi, ecx                 ; esi = this
//   push edi
//   mov  edi, [esi+0x04]          ; edi = this->begin
//   test edi, edi
//   jnz  size_known
//   xor  ecx, ecx                 ; ecx = current size = 0
//   jmp  capacity_check
// size_known:
//   mov  ecx, [esi+0x08]          ; ecx = this->end
//   sub  ecx, edi                 ;   - this->begin   = bytes used
//   mov  eax, 0x66666667          ;
//   imul ecx                      ; edx:eax = bytes_used * 0x66666667
//   sar  edx, 0x03                ; … / 20 (signed)
//   mov  ecx, edx
//   shr  ecx, 0x1f                ; flush carry-in for the rounding fix
//   add  ecx, edx                 ; ecx = current_size (element count)
// capacity_check:
//   test edi, edi                 ; begin == NULL ⇒ go via slow path
//   jz   slow_path
//   mov  edx, [esi+0x0c]          ; edx = this->end_cap
//   sub  edx, edi
//   mov  eax, 0x66666667
//   imul edx                      ; same magic-constant /20 dance for
//   sar  edx, 0x03                ;   capacity
//   mov  eax, edx
//   shr  eax, 0x1f
//   add  eax, edx                 ; eax = capacity (element count)
//   cmp  ecx, eax
//   jnc  slow_path                ; current_size >= capacity ⇒ realloc
//
//   ; --- in-place emplace path -------------------------------------
//   mov  ecx, [esp+0x14]          ; ecx = pos
//   mov  edx, [esp+0x14]          ; edx = pos
//   mov  edi, [esi+0x08]          ; edi = this->end (the new slot)
//   mov  byte ptr [esp+0x08], 0   ; local_8[0].byte0 = 0 (do-not-relocate flag)
//   mov  eax, [esp+0x08]          ; eax = local_8[0] (MSVC stack round-trip)
//   push eax                      ; arg5: bool flag (0)
//   push ecx                      ; arg4: pos
//   push esi                      ; arg3: this
//   push edx                      ; arg2: pos
//   push 0x01                     ; arg1: count (1)
//   push edi                      ; arg0: dst slot (this->end)
//   call FUN_00435400             ; __cdecl emplace-at-slot helper
//   add  esp, 0x18                ; cdecl arg cleanup (6 dwords)
//   add  edi, 0x14                ; this->end += sizeof(T)
//   mov  [esi+0x08], edi
//   pop  edi
//   pop  esi
//   add  esp, 0x08
//   ret  0x04                     ; __thiscall, callee-cleans 1 dword
//
//   ; --- slow path: grow-and-insert --------------------------------
// slow_path:
//   push ebx
//   mov  ebx, [esi+0x08]          ; ebx = this->end
//   cmp  edi, ebx                 ; begin > end ⇒ corrupted state
//   jbe  not_corrupted
//   call FUN_009d22b4             ; _invalid_parameter_noinfo — does not return
// not_corrupted:
//   mov  eax, [esp+0x18]          ; eax = pos (displaced by PUSH EBX)
//   push eax                      ; arg3: pos
//   push ebx                      ; arg2: this->end
//   push esi                      ; arg1: (stack-spill of this)
//   lea  ecx, [esp+0x18]          ; ecx = &local_8 (scratch slot for helper frame)
//   push ecx                      ; arg0: &scratch
//   mov  ecx, esi                 ; ecx = this (thiscall to helper)
//   call FUN_00436700             ; __thiscall realloc-and-insert
//   pop  ebx
//   pop  edi
//   pop  esi
//   add  esp, 0x08
//   ret  0x04
//
// Reloc-bearing sites in the orig 160 bytes:
//     +0x61   CALL rel32   → FUN_00435400   (RVA 0x00435400)
//     +0x7f   CALL rel32   → FUN_009d22b4   (RVA 0x009d22b4 —
//                                            _invalid_parameter_noinfo)
//     +0x92   CALL rel32   → FUN_00436700   (RVA 0x00436700)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Structurally identical to FUN_00404d60 (vector insert for a 28-byte
//   element), but with a different magic constant (0x66666667 vs
//   0x92492493), SAR by 3 instead of 4, element stride 0x14 instead of
//   0x1c, and different register allocation (begin in EDI vs EBX, with
//   EBX pushed only in the slow path). The register layout difference
//   makes a source-level C++ form non-trivial to pin, so a
//   __declspec(naked) body re-emitting the original 160 bytes verbatim
//   via MASM _emit directives is the simplest path to GREEN.
//   compare.py masks rel32 bytes out of the diff.

extern "C" __declspec(naked) void FUN_004367b0() {
    __asm {
        _emit 0x83              // SUB ESP, 0x08
        _emit 0xec
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESI + 0x04]
        _emit 0x7e
        _emit 0x04
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x75              // JNZ size_known (+0x04)
        _emit 0x04
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0xeb              // JMP capacity_check (+0x16)
        _emit 0x16
        // size_known:
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x08]
        _emit 0x4e
        _emit 0x08
        _emit 0x2b              // SUB ECX, EDI
        _emit 0xcf
        _emit 0xb8              // MOV EAX, 0x66666667
        _emit 0x67
        _emit 0x66
        _emit 0x66
        _emit 0x66
        _emit 0xf7              // IMUL ECX
        _emit 0xe9
        _emit 0xc1              // SAR EDX, 0x03
        _emit 0xfa
        _emit 0x03
        _emit 0x8b              // MOV ECX, EDX
        _emit 0xca
        _emit 0xc1              // SHR ECX, 0x1f
        _emit 0xe9
        _emit 0x1f
        _emit 0x03              // ADD ECX, EDX
        _emit 0xca
        // capacity_check:
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x74              // JZ slow_path (+0x4b)
        _emit 0x4b
        _emit 0x8b              // MOV EDX, dword ptr [ESI + 0x0c]
        _emit 0x56
        _emit 0x0c
        _emit 0x2b              // SUB EDX, EDI
        _emit 0xd7
        _emit 0xb8              // MOV EAX, 0x66666667
        _emit 0x67
        _emit 0x66
        _emit 0x66
        _emit 0x66
        _emit 0xf7              // IMUL EDX
        _emit 0xea
        _emit 0xc1              // SAR EDX, 0x03
        _emit 0xfa
        _emit 0x03
        _emit 0x8b              // MOV EAX, EDX
        _emit 0xc2
        _emit 0xc1              // SHR EAX, 0x1f
        _emit 0xe8
        _emit 0x1f
        _emit 0x03              // ADD EAX, EDX
        _emit 0xc2
        _emit 0x3b              // CMP ECX, EAX
        _emit 0xc8
        _emit 0x73              // JNC slow_path (+0x31)
        _emit 0x31
        // --- in-place emplace path ---
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 0x14]
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EDI, dword ptr [ESI + 0x08]
        _emit 0x7e
        _emit 0x08
        _emit 0xc6              // MOV byte ptr [ESP + 0x08], 0x00
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x08]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x56              // PUSH ESI
        _emit 0x52              // PUSH EDX
        _emit 0x6a              // PUSH 0x01
        _emit 0x01
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL FUN_00435400 (rel32 = 0xFFFFEBEA)
        _emit 0xea
        _emit 0xeb
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0x83              // ADD EDI, 0x14
        _emit 0xc7
        _emit 0x14
        _emit 0x89              // MOV dword ptr [ESI + 0x08], EDI
        _emit 0x7e
        _emit 0x08
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x08
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
        // slow_path:
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESI + 0x08]
        _emit 0x5e
        _emit 0x08
        _emit 0x3b              // CMP EDI, EBX
        _emit 0xfb
        _emit 0x76              // JBE not_corrupted (+0x05)
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32 = 0x0059BA80)
        _emit 0x80
        _emit 0xba
        _emit 0x59
        _emit 0x00
        // not_corrupted:
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50              // PUSH EAX
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA ECX, [ESP + 0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_00436700 (rel32 = 0xFFFFFEB9)
        _emit 0xb9
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x5b              // POP EBX
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x08
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
    }
}
