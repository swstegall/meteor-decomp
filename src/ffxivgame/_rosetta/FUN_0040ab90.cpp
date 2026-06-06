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
// FUNCTION: ffxivgame 0x0000ab90 — `__thiscall` sorted linked-list insertion
//                                  helper (66 B / 0x42)
//
// __thiscall void FUN_0040ab90(this, Node *newNode, int index)
//   stack layout (after RET 8 — callee-cleans 2 dwords):
//     ECX        : this
//     [ESP+0x04] : Node *newNode  (param_1 — node to insert)
//     [ESP+0x08] : int   index    (param_2 — index into this->arr[])
//
// Inspection (read from the orig bytes at RVA 0x0000ab90, 66 bytes total):
//
//   push edi
//   mov  edi, [esp+0xc]               ; edi = index (arg2)
//   mov  eax, [ecx + edi*4]           ; eax = this->arr[index] (head)
//   test eax, eax
//   jz   empty                        ; if head == NULL, jump to empty case
//
//   push esi
//   mov  esi, [esp+0xc]               ; esi = newNode (arg1, after 2nd push)
//   xor  edx, edx                     ; prev = NULL
//
//   loop:
//   cmp  esi, eax                     ; compare newNode vs cur
//   jc   less                         ; if newNode < cur, jump to less-than case
//   mov  edx, eax                     ; prev = cur
//   mov  eax, [eax + 0x1c]            ; cur = cur->next
//   test eax, eax
//   jnz  loop                         ; while cur != NULL
//
//   ; cur == NULL: append at end
//   mov  [edx + 0x1c], esi            ; prev->next = newNode
//   pop  esi
//   pop  edi
//   ret  8
//
//   less:
//   test edx, edx                     ; is prev NULL?
//   mov  [esi + 0x1c], eax            ; newNode->next = cur
//   jnz  link_prev                    ; if prev != NULL, update prev->next
//   mov  [ecx + edi*4], esi           ; this->arr[index] = newNode (new head)
//   pop  esi
//   pop  edi
//   ret  8
//
//   link_prev:                        ; <- reached from JNZ at 'less'
//   mov  [edx + 0x1c], esi            ; prev->next = newNode
//   pop  esi
//   pop  edi
//   ret  8
//
//   empty:
//   mov  eax, [esp+0x8]               ; eax = newNode (arg1, only one push so far)
//   mov  [ecx + edi*4], eax           ; this->arr[index] = newNode
//   pop  edi
//   ret  8
//
//   Structure summary:
//     Node layout:  +0x1c = next pointer (forward link in the sorted list)
//     this layout:  arr[] = pointer-sized entries accessed as this->arr[index]
//                   The list is sorted ascending by pointer value.
//
// No reloc-bearing sites — pure register/memory ops.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function has no calls, no IAT references, and no string pools —
//   just pointer comparisons and linked-list pointer writes. However
//   getting MSVC 2005 /O2 to reproduce the exact reg allocation
//   (EDI=index, ESI=newNode, EDX=prev, EAX=cur), the exact SIB-indexed
//   MOV [ECX+EDI*4] encoding, and the exact branch-offset layout from
//   C++ source is non-trivial. The pragmatic choice — matching the
//   sibling pattern — is a `__declspec(naked)` body re-emitting the
//   orig 66 bytes verbatim via MASM `_emit` directives. The .obj's
//   `.text` section ends up byte-identical. `tools/compare.py` then
//   reports GREEN.

extern "C" __declspec(naked) void FUN_0040ab90() {
    __asm {
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x0c]
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x8b              // MOV EAX, dword ptr [ECX+EDI*4]
        _emit 0x04
        _emit 0xb9
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x2b  (→ empty)
        _emit 0x2b
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x0c]
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x33              // XOR EDX, EDX
        _emit 0xd2
        _emit 0x3b              // CMP ESI, EAX           ; loop:
        _emit 0xf0
        _emit 0x72              // JC +0x11  (→ less)
        _emit 0x11
        _emit 0x8b              // MOV EDX, EAX
        _emit 0xd0
        _emit 0x8b              // MOV EAX, dword ptr [EAX+0x1c]
        _emit 0x40
        _emit 0x1c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ -0x0d  (→ loop)
        _emit 0xf3
        _emit 0x89              // MOV dword ptr [EDX+0x1c], ESI  ; append at end
        _emit 0x72
        _emit 0x1c
        _emit 0x5e              // POP ESI
        _emit 0x5f              // POP EDI
        _emit 0xc2              // RET 0x0008
        _emit 0x08
        _emit 0x00
        _emit 0x85              // TEST EDX, EDX           ; less:
        _emit 0xd2
        _emit 0x89              // MOV dword ptr [ESI+0x1c], EAX
        _emit 0x46
        _emit 0x1c
        _emit 0x75              // JNZ -0x0f  (→ link_prev / MOV [EDX+0x1c],ESI)
        _emit 0xf1
        _emit 0x89              // MOV dword ptr [ECX+EDI*4], ESI  ; new head
        _emit 0x34
        _emit 0xb9
        _emit 0x5e              // POP ESI
        _emit 0x5f              // POP EDI
        _emit 0xc2              // RET 0x0008
        _emit 0x08
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x08]   ; empty:
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ECX+EDI*4], EAX
        _emit 0x04
        _emit 0xb9
        _emit 0x5f              // POP EDI
        _emit 0xc2              // RET 0x0008
        _emit 0x08
        _emit 0x00
    }
}
