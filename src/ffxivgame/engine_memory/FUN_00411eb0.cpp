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
// FUNCTION: ffxivgame 0x00411eb0 — SeparateHeapSpace::push_node (120 bytes / 0x78)
//           __thiscall with 1 stack arg (RET 0x4)
//
// ECX = this (an object whose field_0x10 is a heap-space object with vtable).
// param_1 = a node pointer (stack arg).
//
// Behaviour:
//   1. EDI = this->field_0x10 (space object)
//   2. Call vtable[0x2c/4=11] on EDI                 ; e.g. Lock()
//   3. Via param_1->vtable[5]()->vtable[1]() obtain piVar4 (a list node ptr)
//   4. Call piVar4->vtable[0](0)                     ; initialise node
//   5. Via this->field_0x10->vtable[1]() obtain an allocator result;
//      from that result->field_0xC get the spin-lock header (iVar5)
//   6. Spin-acquire the lock at iVar5->field_4 using XCHG (test-and-set 1)
//   7. Doubly-linked list insert: splice piVar4 before iVar5->field_0xC->next
//   8. Decrement iVar5->field_0x18 (count)
//   9. Release the lock (XCHG 0 into iVar5->field_4)
//  10. Call vtable[0x30/4=12] on EDI                 ; e.g. Unlock()
//
// Non-reproducibility:
//   The spin-lock loop uses XCHG [EBX], ECX (implicit atomic) and the
//   compiler copies EDX (lock-ptr) into EBX inside every loop iteration.
//   Plain C++ cannot reproduce the exact register layout and XCHG form;
//   naked-asm passthrough guarantees byte-identical output.

extern "C" __declspec(naked) void FUN_00411eb0()
{
    __asm {
        // 00011eb0: 53                  PUSH EBX
        push ebx
        // 00011eb1: 56                  PUSH ESI
        push esi
        // 00011eb2: 8b d9               MOV EBX, ECX
        mov  ebx, ecx
        // 00011eb4: 57                  PUSH EDI
        push edi
        // 00011eb5: 8b 7b 10            MOV EDI, [EBX + 0x10]
        mov  edi, dword ptr [ebx + 0x10]
        // 00011eb8: 8b 07               MOV EAX, [EDI]
        mov  eax, dword ptr [edi]
        // 00011eba: 8b 50 2c            MOV EDX, [EAX + 0x2c]
        mov  edx, dword ptr [eax + 0x2c]
        // 00011ebd: 8b cf               MOV ECX, EDI
        mov  ecx, edi
        // 00011ebf: ff d2               CALL EDX
        call edx
        // 00011ec1: 8b 4c 24 10         MOV ECX, [ESP + 0x10]
        mov  ecx, dword ptr [esp + 0x10]
        // 00011ec5: 8b 01               MOV EAX, [ECX]
        mov  eax, dword ptr [ecx]
        // 00011ec7: 8b 50 14            MOV EDX, [EAX + 0x14]
        mov  edx, dword ptr [eax + 0x14]
        // 00011eca: ff d2               CALL EDX
        call edx
        // 00011ecc: 8b 10               MOV EDX, [EAX]
        mov  edx, dword ptr [eax]
        // 00011ece: 8b c8               MOV ECX, EAX
        mov  ecx, eax
        // 00011ed0: 8b 42 04            MOV EAX, [EDX + 0x4]
        mov  eax, dword ptr [edx + 0x4]
        // 00011ed3: ff d0               CALL EAX
        call eax
        // 00011ed5: 8b f0               MOV ESI, EAX
        mov  esi, eax
        // 00011ed7: 8b 16               MOV EDX, [ESI]
        mov  edx, dword ptr [esi]
        // 00011ed9: 8b 02               MOV EAX, [EDX]
        mov  eax, dword ptr [edx]
        // 00011edb: 6a 00               PUSH 0
        push 0
        // 00011edd: 8b ce               MOV ECX, ESI
        mov  ecx, esi
        // 00011edf: ff d0               CALL EAX
        call eax
        // 00011ee1: 8b 4b 10            MOV ECX, [EBX + 0x10]
        mov  ecx, dword ptr [ebx + 0x10]
        // 00011ee4: 8b 11               MOV EDX, [ECX]
        mov  edx, dword ptr [ecx]
        // 00011ee6: 8b 42 04            MOV EAX, [EDX + 0x4]
        mov  eax, dword ptr [edx + 0x4]
        // 00011ee9: ff d0               CALL EAX
        call eax
        // 00011eeb: 8b 40 0c            MOV EAX, [EAX + 0xc]
        mov  eax, dword ptr [eax + 0xc]
        // 00011eee: 8d 50 04            LEA EDX, [EAX + 0x4]
        lea  edx, dword ptr [eax + 0x4]
        // 00011ef1: b9 01 00 00 00      MOV ECX, 1
    spin_loop:
        mov  ecx, 1
        // 00011ef6: 8b da               MOV EBX, EDX
        mov  ebx, edx
        // 00011ef8: 87 0b               XCHG [EBX], ECX
        xchg dword ptr [ebx], ecx
        // 00011efa: 85 c9               TEST ECX, ECX
        test ecx, ecx
        // 00011efc: 75 f3               JNZ spin_loop
        jnz  spin_loop
        // 00011efe: 8b 48 0c            MOV ECX, [EAX + 0xc]
        mov  ecx, dword ptr [eax + 0xc]
        // 00011f01: 8b 59 04            MOV EBX, [ECX + 0x4]
        mov  ebx, dword ptr [ecx + 0x4]
        // 00011f04: 89 33               MOV [EBX], ESI
        mov  dword ptr [ebx], esi
        // 00011f06: 8b 59 04            MOV EBX, [ECX + 0x4]
        mov  ebx, dword ptr [ecx + 0x4]
        // 00011f09: 89 5e 04            MOV [ESI + 0x4], EBX
        mov  dword ptr [esi + 0x4], ebx
        // 00011f0c: 89 0e               MOV [ESI], ECX
        mov  dword ptr [esi], ecx
        // 00011f0e: 89 71 04            MOV [ECX + 0x4], ESI
        mov  dword ptr [ecx + 0x4], esi
        // 00011f11: 83 40 18 ff         ADD [EAX + 0x18], -1
        add  dword ptr [eax + 0x18], -1
        // 00011f15: 33 c0               XOR EAX, EAX
        xor  eax, eax
        // 00011f17: 87 02               XCHG [EDX], EAX
        xchg dword ptr [edx], eax
        // 00011f19: 8b 17               MOV EDX, [EDI]
        mov  edx, dword ptr [edi]
        // 00011f1b: 8b 42 30            MOV EAX, [EDX + 0x30]
        mov  eax, dword ptr [edx + 0x30]
        // 00011f1e: 8b cf               MOV ECX, EDI
        mov  ecx, edi
        // 00011f20: ff d0               CALL EAX
        call eax
        // 00011f22: 5f                  POP EDI
        pop  edi
        // 00011f23: 5e                  POP ESI
        pop  esi
        // 00011f24: 5b                  POP EBX
        pop  ebx
        // 00011f25: c2 04 00            RET 4
        ret  4
    }
}
