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
// FUNCTION: ffxivgame 0x00013140 — __thiscall, 3 stack args, 136 bytes (0x88)
//
// Acquires a mutex-like vtable lock, calls a virtual allocation method via the
// sub-object at this+4's vtable slot 3, optionally allocates a
// ReceivableHeapBlock via FUN_00412a80, inserts it into a doubly-linked
// list at this->field_0x24, releases the lock, and returns a pointer
// to the block+4 (or NULL on failure).
//
// Calling convention: __thiscall (ECX = this), 3 stack args (RET 0xC).
// Callee-saves used: EBX, ESI, EDI.
//
// Register layout:
//   ESI = this  (cached in callee-save; ECX clobbered by virtual calls)
//   EBX = iVar3 (allocation result from FUN_00412a80; 0 on failure)
//   EDI = iVar2 (return value of vtable[3] call)
//
// Asm (136 bytes @ orig RVA 0x00013140):
//   53                  PUSH EBX
//   56                  PUSH ESI
//   8B F1               MOV ESI, ECX
//   8B 06               MOV EAX, [ESI]
//   8B 50 2C            MOV EDX, [EAX+0x2c]
//   57                  PUSH EDI
//   FF D2               CALL EDX                    ; (*this->vtable[0xb])()  [lock]
//   8B 54 24 18         MOV EDX, [ESP+0x18]          ; param_3
//   8B 4E 04            MOV ECX, [ESI+4]
//   8B 01               MOV EAX, [ECX]
//   8B 40 0C            MOV EAX, [EAX+0xc]
//   52                  PUSH EDX                    ; param_3
//   8B 54 24 18         MOV EDX, [ESP+0x18]          ; param_2
//   52                  PUSH EDX                    ; param_2
//   8B 54 24 18         MOV EDX, [ESP+0x18]          ; param_1
//   52                  PUSH EDX                    ; param_1
//   33 DB               XOR EBX, EBX                ; iVar3 = 0
//   FF D0               CALL EAX                    ; (*this->field4->vtable[3])(p1,p2,p3)
//   8B F8               MOV EDI, EAX                ; iVar2 = result
//   85 FF               TEST EDI, EDI
//   74 3D               JZ  unlock                  ; if !iVar2 skip alloc
//   8B 4E 10            MOV ECX, [ESI+0x10]
//   E8 ?? ?? ?? ??      CALL FUN_004109a0           ; try-lock/acquire (REL32)
//   85 C0               TEST EAX, EAX
//   74 0D               JZ  alloc_fail
//   53                  PUSH EBX                    ; 0
//   53                  PUSH EBX                    ; 0
//   57                  PUSH EDI                    ; iVar2
//   56                  PUSH ESI                    ; this
//   8B C8               MOV ECX, EAX
//   E8 ?? ?? ?? ??      CALL FUN_00412a80           ; construct block (REL32)
//   EB 02               JMP  after_alloc
//  alloc_fail:
//   33 C0               XOR EAX, EAX
//  after_alloc:
//   85 C0               TEST EAX, EAX               ; iVar3 = EAX
//   8B D8               MOV EBX, EAX
//   74 05               JZ  list_insert_null
//   83 C0 08            ADD EAX, 8                  ; iVar2 = iVar3 + 8
//   EB 02               JMP  do_insert
//  list_insert_null:
//   33 C0               XOR EAX, EAX                ; iVar2 = 0
//  do_insert:
//   8B 4E 24            MOV ECX, [ESI+0x24]          ; iVar1 = this->field_9
//   8B 51 08            MOV EDX, [ECX+8]             ; EDX = *(iVar1+8)
//   89 42 04            MOV [EDX+4], EAX             ; *(*(iVar1+8)+4) = iVar2
//   8B 51 08            MOV EDX, [ECX+8]
//   89 50 08            MOV [EAX+8], EDX             ; *(iVar2+8) = *(iVar1+8)
//   89 48 04            MOV [EAX+4], ECX             ; *(iVar2+4) = iVar1
//   89 41 08            MOV [ECX+8], EAX             ; *(iVar1+8) = iVar2
//  unlock:
//   8B 06               MOV EAX, [ESI]
//   8B 50 30            MOV EDX, [EAX+0x30]
//   8B CE               MOV ECX, ESI
//   FF D2               CALL EDX                    ; (*this->vtable[0xc])()  [unlock]
//   85 DB               TEST EBX, EBX
//   74 09               JZ  ret_null
//   5F                  POP EDI
//   5E                  POP ESI
//   8D 43 04            LEA EAX, [EBX+4]            ; return iVar3 + 4
//   5B                  POP EBX
//   C2 0C 00            RET 0xC
//  ret_null:
//   5F                  POP EDI
//   5E                  POP ESI
//   33 C0               XOR EAX, EAX                ; return 0
//   5B                  POP EBX
//   C2 0C 00            RET 0xC

extern "C" void FUN_004109a0();
extern "C" void FUN_00412a80();

extern "C" __declspec(naked) void FUN_00413140()
{
    __asm {
        push ebx
        push esi
        mov  esi, ecx
        mov  eax, [esi]
        mov  edx, [eax+0x2c]
        push edi
        call edx
        mov  edx, [esp+0x18]
        mov  ecx, [esi+4]
        mov  eax, [ecx]
        mov  eax, [eax+0x0c]
        push edx
        mov  edx, [esp+0x18]
        push edx
        mov  edx, [esp+0x18]
        push edx
        xor  ebx, ebx
        call eax
        mov  edi, eax
        test edi, edi
        jz   unlock
        mov  ecx, [esi+0x10]
        call FUN_004109a0
        test eax, eax
        jz   alloc_fail
        push ebx
        push ebx
        push edi
        push esi
        mov  ecx, eax
        call FUN_00412a80
        jmp  after_alloc
    alloc_fail:
        xor  eax, eax
    after_alloc:
        test eax, eax
        mov  ebx, eax
        jz   list_insert_null
        add  eax, 8
        jmp  do_insert
    list_insert_null:
        xor  eax, eax
    do_insert:
        mov  ecx, [esi+0x24]
        mov  edx, [ecx+8]
        mov  [edx+4], eax
        mov  edx, [ecx+8]
        mov  [eax+8], edx
        mov  [eax+4], ecx
        mov  [ecx+8], eax
    unlock:
        mov  eax, [esi]
        mov  edx, [eax+0x30]
        mov  ecx, esi
        call edx
        test ebx, ebx
        jz   ret_null
        pop  edi
        pop  esi
        lea  eax, [ebx+4]
        pop  ebx
        ret  0x0c
    ret_null:
        pop  edi
        pop  esi
        xor  eax, eax
        pop  ebx
        ret  0x0c
    }
}
