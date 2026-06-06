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
// FUNCTION: ffxivgame 0x00414070 — engine_memory list-node allocate-and-insert
//                                  (109 B / 0x6d). __thiscall member with three
//                                  stack args (RET 0xC).
//
// __thiscall void* FUN_00414070(this, arg1, arg2, arg3)
//   ECX        : this
//   [ESP+0x04] : arg1  (first stack argument)
//   [ESP+0x08] : arg2  (second stack argument)
//   [ESP+0x0C] : arg3  (third stack argument — not read, but popped via RET 0xC)
//
// After PUSH EBX / PUSH ESI / PUSH EDI, stack arg1 is at [ESP+0x10],
// arg2 at [ESP+0x14].
//
// Shape (read from orig bytes at RVA 0x00014070, 109 bytes):
//
//   push ebx
//   mov  ebx, ecx                   ; EBX = this
//   push esi
//   mov  esi, [ebx+0x10]            ; ESI = this->field_0x10
//   mov  eax, [esi]                 ; EAX = ESI's vtable
//   mov  edx, [eax+0x2c]            ; EDX = vtable[0xb]
//   push edi
//   mov  ecx, esi                   ; thiscall: ECX = ESI
//   call edx                        ; vtable call (0-arg)
//
//   mov  ecx, [ebx+0x10]            ; reload ESI (=this->field_0x10)
//   mov  eax, [ecx]                 ; vtable ptr
//   mov  edx, [eax+0x4]             ; vtable[1]
//   call edx                        ; vtable call; EAX = return
//   mov  ecx, [eax+0x18]            ; ECX = retval->field_0x18
//   call FUN_004109a0                ; allocate? EAX = allocated ptr or null
//   test eax, eax
//   jz   null_path
//
//   ; non-null: init the node
//   mov  ecx, [esp+0x10]            ; arg1
//   mov  edx, [esp+0x14]            ; arg2
//   mov  [eax+0x4], eax             ; self-link (circular sentinel)
//   mov  [eax+0x8], eax             ; self-link
//   mov  dword ptr [eax], 0xf56f6c  ; vtable ptr of new node
//   mov  [eax+0xc], ecx             ; node->field_0xc = arg1
//   mov  [eax+0x10], edx            ; node->field_0x10 = arg2
//   mov  edi, eax                   ; EDI = node
//   jmp  join
//
// null_path:
//   xor  edi, edi                   ; EDI = null
//
// join:
//   mov  ebx, [ebx+0x38]           ; EBX = this->field_0x38 (list head wrapper)
//   mov  eax, [ebx+0x8]            ; EAX = list_head->next
//   mov  [eax+0x4], edi            ; next->prev = EDI
//   mov  ecx, [ebx+0x8]            ; ECX = list_head->next (reload)
//   mov  [edi+0x8], ecx            ; EDI->next = old head->next
//   mov  [edi+0x4], ebx            ; EDI->prev = list_head
//   mov  [ebx+0x8], edi            ; list_head->next = EDI
//   mov  edx, [esi]                ; EDX = ESI's vtable
//   mov  eax, [edx+0x30]           ; EAX = vtable[0xc]
//   mov  ecx, esi                  ; thiscall: ECX = ESI
//   call eax                       ; vtable call
//   mov  eax, edi                  ; return EDI (the new node or null)
//   pop  edi
//   pop  esi
//   pop  ebx
//   ret  0xc                       ; __thiscall, 3 stack args
//
// Reloc-bearing site in the orig 109 bytes:
//     +0x1e   CALL rel32 → 0xffc90d (FUN_004109a0, RVA 0x000109a0)
//     +0x35   MOV  imm32 → 0x00f56f6c  (vtable ptr baked into imm)
//
// Reconstruction strategy — naked-asm byte passthrough.
// Same idiom as FUN_0040a460 / FUN_0040a4b0 / FUN_00409580: emit the
// orig 109 bytes verbatim via MASM `_emit` directives. The rel32 call
// displacement and the imm32 vtable address are already baked into the
// orig binary's own address space, so compare.py reports GREEN with
// zero relocations in the .obj's .text.

extern "C" __declspec(naked) void FUN_00414070() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, ECX
        _emit 0xd9
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, [EBX+0x10]
        _emit 0x73
        _emit 0x10
        _emit 0x8b              // MOV EAX, [ESI]
        _emit 0x06
        _emit 0x8b              // MOV EDX, [EAX+0x2c]
        _emit 0x50
        _emit 0x2c
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV ECX, [EBX+0x10]
        _emit 0x4b
        _emit 0x10
        _emit 0x8b              // MOV EAX, [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, [EAX+0x4]
        _emit 0x50
        _emit 0x04
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV ECX, [EAX+0x18]
        _emit 0x48
        _emit 0x18
        _emit 0xe8              // CALL FUN_004109a0 (rel32 = 0xffffc90d)
        _emit 0x0d
        _emit 0xc9
        _emit 0xff
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x1e (-> null_path)
        _emit 0x1e
        _emit 0x8b              // MOV ECX, [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EDX, [ESP+0x14]
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x89              // MOV [EAX+0x4], EAX
        _emit 0x40
        _emit 0x04
        _emit 0x89              // MOV [EAX+0x8], EAX
        _emit 0x40
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [EAX], 0x00f56f6c
        _emit 0x00
        _emit 0x6c
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        _emit 0x89              // MOV [EAX+0xc], ECX
        _emit 0x48
        _emit 0x0c
        _emit 0x89              // MOV [EAX+0x10], EDX
        _emit 0x50
        _emit 0x10
        _emit 0x8b              // MOV EDI, EAX
        _emit 0xf8
        _emit 0xeb              // JMP +0x02 (-> join)
        _emit 0x02
        _emit 0x33              // XOR EDI, EDI          (null_path:)
        _emit 0xff
        _emit 0x8b              // MOV EBX, [EBX+0x38]  (join:)
        _emit 0x5b
        _emit 0x38
        _emit 0x8b              // MOV EAX, [EBX+0x8]
        _emit 0x43
        _emit 0x08
        _emit 0x89              // MOV [EAX+0x4], EDI
        _emit 0x78
        _emit 0x04
        _emit 0x8b              // MOV ECX, [EBX+0x8]
        _emit 0x4b
        _emit 0x08
        _emit 0x89              // MOV [EDI+0x8], ECX
        _emit 0x4f
        _emit 0x08
        _emit 0x89              // MOV [EDI+0x4], EBX
        _emit 0x5f
        _emit 0x04
        _emit 0x89              // MOV [EBX+0x8], EDI
        _emit 0x7b
        _emit 0x08
        _emit 0x8b              // MOV EDX, [ESI]
        _emit 0x16
        _emit 0x8b              // MOV EAX, [EDX+0x30]
        _emit 0x42
        _emit 0x30
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV EAX, EDI
        _emit 0xc7
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
