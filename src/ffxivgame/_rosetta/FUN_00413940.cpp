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
// FUNCTION: ffxivgame 0x00413940 — __thiscall "detach block back to pool" helper
//                                  (90 B / 0x5a, no relocations, no SEH).
//
// Inspection (read from asm/ffxivgame/00013940_FUN_00413940.s):
//
//   __thiscall void FUN_00413940(this);
//
//   Body (ESI = this throughout):
//
//     block = this->fld34;                        // [ESI+0x34]
//     if (block == null) return;
//
//     // Call vtable[0](block, 0) — MSVC scalar-dtor-no-free idiom:
//     // tears down the block's content without operator delete.
//     block->vtable[0](block, 0);
//
//     // Get a list-manager via vtable[1] on this->fld14 (the owning space):
//     mgr = this->fld14->vtable[1](this->fld14);
//     list_head = mgr->fld18;                     // [ret+0x18]
//
//     // Spinlock acquire on list_head->fld04:
//     EDI = &list_head->fld04;
//     do {
//         ECX = 1; XCHG [EDI], ECX;              // atomic swap with 1
//     } while (ECX != 0);                         // retry until we see 0 (unlocked)
//
//     // Insert `block` after list_head->fld0c (standard doubly-linked insert):
//     P = list_head->fld0c;
//     N = P->fld04;
//     N->fld00 = block;
//     block->fld04 = N;
//     block->fld00 = P;
//     P->fld04 = block;
//
//     // Decrement the allocated-block counter and release the spinlock:
//     list_head->fld18 -= 1;
//     EDX = 0; XCHG [EDI], EDX;                  // atomic write 0 (unlock)
//
//     this->fld34 = null;
//
// Context: part of the SQEX::CDev::Engine::Memory::Alternative family.
//   `this` is a DetachableHeapBlock (see decomp-notes/types/ffxivgame/0x000139d0.md).
//   `this->fld14` = owning DetachableHeapSpace pointer.
//   `this->fld34` = pointer to an attached sub-block (or null if nothing attached).
//   Calling vtable[0](0) before reinsertion is the "dtor-no-free" pattern
//   confirmed by the sibling FUN_00414370 analysis in
//   decomp-notes/types/ffxivgame/0x00014370.md.
//
// No relocations: all memory accesses are register-relative; all CALLs
// use CALL EDX (indirect). Therefore the naked-asm byte passthrough
// produces a byte-identical .text section with no relocation entries.
//
// Instruction layout (90 bytes, RVA 0x00013940..0x00013999):
//
//   00013940:  56                     PUSH ESI
//   00013941:  8b f1                  MOV ESI,ECX
//   00013943:  8b 4e 34               MOV ECX,[ESI+0x34]
//   00013946:  85 c9                  TEST ECX,ECX
//   00013948:  74 4e                  JZ +0x4e  → 0x00413998
//   0001394a:  8b 01                  MOV EAX,[ECX]
//   0001394c:  8b 10                  MOV EDX,[EAX]
//   0001394e:  53                     PUSH EBX
//   0001394f:  57                     PUSH EDI
//   00013950:  6a 00                  PUSH 0
//   00013952:  ff d2                  CALL EDX
//   00013954:  8b 4e 14               MOV ECX,[ESI+0x14]
//   00013957:  8b 01                  MOV EAX,[ECX]
//   00013959:  8b 50 04               MOV EDX,[EAX+0x4]
//   0001395c:  ff d2                  CALL EDX
//   0001395e:  8b 40 18               MOV EAX,[EAX+0x18]
//   00013961:  8b 56 34               MOV EDX,[ESI+0x34]
//   00013964:  8d 78 04               LEA EDI,[EAX+0x4]
//   00013967:  b9 01 00 00 00         MOV ECX,0x1       ← spin:
//   0001396c:  8b df                  MOV EBX,EDI
//   0001396e:  87 0b                  XCHG [EBX],ECX
//   00013970:  85 c9                  TEST ECX,ECX
//   00013972:  75 f3                  JNZ -0xd  → spin
//   00013974:  8b 48 0c               MOV ECX,[EAX+0xc]
//   00013977:  8b 59 04               MOV EBX,[ECX+0x4]
//   0001397a:  89 13                  MOV [EBX],EDX
//   0001397c:  8b 59 04               MOV EBX,[ECX+0x4]
//   0001397f:  89 5a 04               MOV [EDX+0x4],EBX
//   00013982:  89 0a                  MOV [EDX],ECX
//   00013984:  89 51 04               MOV [ECX+0x4],EDX
//   00013987:  83 40 18 ff            ADD [EAX+0x18],-1
//   0001398b:  33 d2                  XOR EDX,EDX
//   0001398d:  87 17                  XCHG [EDI],EDX
//   0001398f:  5f                     POP EDI
//   00013990:  c7 46 34 00 00 00 00   MOV [ESI+0x34],0
//   00013997:  5b                     POP EBX
//   00013998:  5e                     POP ESI          ← done:
//   00013999:  c3                     RET

extern "C" __declspec(naked) void FUN_00413940() {
    __asm {
        push    esi
        mov     esi, ecx
        mov     ecx, dword ptr [esi + 0x34]
        test    ecx, ecx
        jz      done
        mov     eax, dword ptr [ecx]
        mov     edx, dword ptr [eax]
        push    ebx
        push    edi
        push    0
        call    edx
        mov     ecx, dword ptr [esi + 0x14]
        mov     eax, dword ptr [ecx]
        mov     edx, dword ptr [eax + 0x4]
        call    edx
        mov     eax, dword ptr [eax + 0x18]
        mov     edx, dword ptr [esi + 0x34]
        lea     edi, [eax + 0x4]
    spin:
        mov     ecx, 0x1
        mov     ebx, edi
        xchg    dword ptr [ebx], ecx
        test    ecx, ecx
        jnz     spin
        mov     ecx, dword ptr [eax + 0xc]
        mov     ebx, dword ptr [ecx + 0x4]
        mov     dword ptr [ebx], edx
        mov     ebx, dword ptr [ecx + 0x4]
        mov     dword ptr [edx + 0x4], ebx
        mov     dword ptr [edx], ecx
        mov     dword ptr [ecx + 0x4], edx
        add     dword ptr [eax + 0x18], -1
        xor     edx, edx
        xchg    dword ptr [edi], edx
        pop     edi
        mov     dword ptr [esi + 0x34], 0
        pop     ebx
    done:
        pop     esi
        ret
    }
}
