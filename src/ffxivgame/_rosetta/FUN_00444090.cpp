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
// FUNCTION: ffxivgame 0x00044090 — __thiscall linear search through an element
//                                  array; returns pointer to matching element
//                                  (81 B / 0x51)
//
// Calling convention: __thiscall (ECX = this; one DWORD stack arg; callee
//                     cleans via ret 4)
//
// Inferred this layout:
//   +0x04  int   m_count       ; actual iteration count = m_count * 32
//   +0x08  void* m_items       ; pointer to element array
//
// Each element is 0xBC (188) bytes; within each element:
//   +0x04  Buf   key           ; equality-tested via FUN_00445d20
//
// Logic:
//   int limit = m_count << 5;
//   if (limit <= 0) return NULL;
//   for (int i = 0, off = 0; i < limit; i++, off += 0xBC) {
//       Buf* p = (Buf*)((char*)m_items + off + 4);
//       if (p->equals(key))          // FUN_00445d20
//           return (char*)m_items + i * 0xBC;
//   }
//   return NULL;
//
// Register map:
//   EBP = this (loaded from ECX at entry)
//   EDI = limit (m_count << 5, precomputed before loop)
//   ESI = i    (element index, 0 .. limit-1)
//   EBX = off  (byte offset accumulator, 0, 0xBC, 0x1BC, ...)
//
// Epilogue — null path (0x000440c6..0x000440cc):
//   POP EDI  POP ESI  POP EBP  XOR EAX,EAX  POP EBX  RET 4
//
// Epilogue — found path (0x000440cf..0x000440e1):
//   MOV EAX,ESI ; IMUL EAX,EAX,0xBC ; ADD EAX,[EBP+8]
//   POP EDI  POP ESI  POP EBP  POP EBX  RET 4
//
// External: FUN_00445d20 — __thiscall Buf equality test (ret 4)

extern "C" void FUN_00445d20();

extern "C" __declspec(naked) void FUN_00444090() {
    __asm {
        push    ebx
        push    ebp
        push    esi
        push    edi
        mov     ebp, ecx                ; this = EBP (thiscall)
        mov     edi, [ebp+4]            ; EDI = m_count
        shl     edi, 5                  ; EDI = m_count * 32 (limit)
        xor     esi, esi                ; ESI = 0  (i)
        test    edi, edi
        jle     SHORT done_null         ; if limit <= 0 → return NULL
        xor     ebx, ebx                ; EBX = 0  (off)
    loop_top:
        mov     eax, [esp+0x14]         ; EAX = stack arg (key)
        mov     ecx, [ebp+8]            ; ECX = m_items
        push    eax                     ; push key for FUN_00445d20
        lea     ecx, [ecx+ebx+4]       ; ECX = &items[i].key  (this for call)
        call    FUN_00445d20
        test    al, al
        jnz     SHORT found             ; match → return pointer to element
        add     esi, 1                  ; i++
        add     ebx, 0BCh               ; off += 0xBC
        cmp     esi, edi                ; i < limit ?
        jl      loop_top
    done_null:
        pop     edi
        pop     esi
        pop     ebp
        xor     eax, eax
        pop     ebx
        ret     4
    found:
        mov     eax, esi                ; EAX = i
        imul    eax, eax, 0BCh          ; EAX = i * 0xBC
        add     eax, [ebp+8]            ; EAX = m_items + i * 0xBC
        pop     edi
        pop     esi
        pop     ebp
        pop     ebx
        ret     4
    }
}
