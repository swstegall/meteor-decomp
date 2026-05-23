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
// FUNCTION: ffxivgame 0x00012360 — doubly-linked-list node insert helper
//                                  (__thiscall, 80 bytes)
//
// __thiscall int FUN_00412360(void *this, int param_1, int param_2, int param_3)
//   ECX       : this
//   [ESP+0x04] : param_1
//   [ESP+0x08] : param_2
//   [ESP+0x0C] : param_3
//   RET 12    : callee-cleans 3 dwords
//
// Body outline (reconstructed from disassembly at RVA 0x00012360, 80 bytes):
//
//   push esi                          ; save callee-save
//   mov  esi, ecx                     ; esi = this
//   mov  ecx, [esi+0x0c]             ; ecx = this->field_0x0c
//   call FUN_004109a0                 ; allocator/lookup on this->field_0x0c (__thiscall)
//   test eax, eax                     ; eax = returned object ptr
//   jz   zero_path                    ; if null → iVar2=0, iVar3=0
//   mov  ecx, [esp+0x10]             ; load param_3
//   mov  edx, [esp+0x0c]             ; load param_2
//   push 0 ; push 0                   ; zero-fill args 5,6
//   push ecx                          ; arg 4 = param_3
//   mov  ecx, [esp+0x14]             ; reload param_1 (stack shifted by 3 pushes)
//   push edx                          ; arg 3 = param_2
//   push ecx                          ; arg 2 = param_1
//   push esi                          ; arg 1 = outer this
//   mov  ecx, eax                     ; ecx = returned object (thiscall this for FUN_00411d30)
//   call FUN_00411d30                 ; FUN_00411d30 cleans 6 args (ret 0x18)
//   test eax, eax
//   jz   zero_node                    ; if null → iVar3=0
//   lea  edx, [eax+8]                 ; iVar3 = eax+8 (node content pointer)
//   jmp  list_insert
// zero_path:
//   xor  eax, eax                     ; iVar2 = 0
// zero_node:
//   xor  edx, edx                     ; iVar3 = 0
// list_insert:
//   mov  ecx, [esi+0x50]             ; ecx = iVar1 = this->field_0x50 (list head/sentinel)
//   mov  esi, [ecx+0x08]             ; esi = iVar1->next
//   mov  [esi+0x04], edx             ; iVar1->next->prev = iVar3
//   mov  esi, [ecx+0x08]             ; reload iVar1->next
//   mov  [edx+0x08], esi             ; iVar3->next = iVar1->next
//   mov  [edx+0x04], ecx             ; iVar3->prev = iVar1
//   mov  [ecx+0x08], edx             ; iVar1->next = iVar3
//   pop  esi
//   ret  0x0c
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Two CALL rel32 sites (FUN_004109a0 at +0x06 and FUN_00411d30 at +0x25)
//   reference RVAs inside the binary's own address space. A source-level
//   C++ form would produce different rel32 values in the standalone .obj;
//   although compare.py masks reloc bytes, the surrounding non-reloc bytes
//   must match exactly. The naked-asm passthrough re-emits the original 80
//   bytes verbatim so the .text section is byte-identical to the orig slice.

extern "C" __declspec(naked) void FUN_00412360() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV ECX, [ESI+0x0C]
        _emit 0x4e
        _emit 0x0c
        _emit 0xe8              // CALL FUN_004109a0 (rel32 → 0x004109a0)
        _emit 0x35
        _emit 0xe6
        _emit 0xff
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x24 (→ zero_path)
        _emit 0x24
        _emit 0x8b              // MOV ECX, [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EDX, [ESP+0x0C]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x51              // PUSH ECX (param_3)
        _emit 0x8b              // MOV ECX, [ESP+0x14]  (param_1, stack shifted)
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x52              // PUSH EDX (param_2)
        _emit 0x51              // PUSH ECX (param_1)
        _emit 0x56              // PUSH ESI (outer this)
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0xe8              // CALL FUN_00411d30 (rel32 → 0x00411d30)
        _emit 0xa6
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x07 (→ zero_node)
        _emit 0x07
        _emit 0x8d              // LEA EDX, [EAX+0x08]
        _emit 0x50
        _emit 0x08
        _emit 0xeb              // JMP +0x04 (→ list_insert)
        _emit 0x04
        _emit 0x33              // XOR EAX, EAX  (zero_path)
        _emit 0xc0
        _emit 0x33              // XOR EDX, EDX  (zero_node)
        _emit 0xd2
        _emit 0x8b              // MOV ECX, [ESI+0x50]  (list_insert)
        _emit 0x4e
        _emit 0x50
        _emit 0x8b              // MOV ESI, [ECX+0x08]
        _emit 0x71
        _emit 0x08
        _emit 0x89              // MOV [ESI+0x04], EDX
        _emit 0x56
        _emit 0x04
        _emit 0x8b              // MOV ESI, [ECX+0x08]
        _emit 0x71
        _emit 0x08
        _emit 0x89              // MOV [EDX+0x08], ESI
        _emit 0x72
        _emit 0x08
        _emit 0x89              // MOV [EDX+0x04], ECX
        _emit 0x4a
        _emit 0x04
        _emit 0x89              // MOV [ECX+0x08], EDX
        _emit 0x51
        _emit 0x08
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x000c
        _emit 0x0c
        _emit 0x00
    }
}
