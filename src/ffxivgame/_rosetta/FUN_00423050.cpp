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
// FUNCTION: ffxivgame 0x00023050 — __thiscall conditional virtual dispatch
//                                  (52 bytes).
//
// Layout (inferred from asm):
//   This (ECX):
//     +0x00  Inner *m_inner  (object with vtable; target of virtual call)
//     +0x04  Helper *m_helper (object used as `this` for FUN_00423710)
//
// Source shape (inferred):
//
//   void Class::Method(T a, T b, T c) {
//       if (!this->m_helper->FUN_00423710(a, b, c)) {
//           this->m_inner->vtable[3](a, c, b);  // args 2/3 swapped
//       }
//   }
//
// Calling convention: __thiscall (ECX = this; three DWORD stack args;
// callee cleans 0xc via `ret 0xc`).
//
// Asm (52 bytes / 0x34):
//   53                    PUSH EBX
//   8b 5c 24 0c           MOV  EBX, [ESP+0xc]       ; arg2
//   55                    PUSH EBP
//   8b 6c 24 0c           MOV  EBP, [ESP+0xc]       ; arg1
//   56                    PUSH ESI
//   57                    PUSH EDI
//   8b 7c 24 1c           MOV  EDI, [ESP+0x1c]      ; arg3
//   57                    PUSH EDI                  ; push arg3 for CALL
//   8b f1                 MOV  ESI, ECX             ; save this
//   8b 4e 04              MOV  ECX, [ESI+4]         ; ECX = m_helper
//   53                    PUSH EBX                  ; push arg2
//   55                    PUSH EBP                  ; push arg1
//   e8 a3 06 00 00        CALL FUN_00423710
//   84 c0                 TEST AL, AL
//   75 0c                 JNZ  skip                 ; if true, skip
//   8b 0e                 MOV  ECX, [ESI]           ; ECX = m_inner
//   8b 01                 MOV  EAX, [ECX]           ; EAX = vtable
//   8b 50 0c              MOV  EDX, [EAX+0xc]       ; EDX = vtable[3]
//   53                    PUSH EBX                  ; push arg2
//   57                    PUSH EDI                  ; push arg3
//   55                    PUSH EBP                  ; push arg1
//   ff d2                 CALL EDX
// skip:
//   5f                    POP  EDI
//   5e                    POP  ESI
//   5d                    POP  EBP
//   5b                    POP  EBX
//   c2 0c 00              RET  0xc
//
// Naked __asm pinned so the interleaved PUSH/MOV scheduling in the
// prologue and the arg-order swap (args 2 & 3 transposed) in the virtual
// call are preserved byte-for-byte.  The REL32 to FUN_00423710 is masked
// by tools/compare.py.

extern "C" bool FUN_00423710();   // __thiscall; 3 stack args; returns bool in AL

extern "C" __declspec(naked) void FUN_00423050() {
    __asm {
        push    ebx
        mov     ebx, dword ptr [esp + 0xc]
        push    ebp
        mov     ebp, dword ptr [esp + 0xc]
        push    esi
        push    edi
        mov     edi, dword ptr [esp + 0x1c]
        push    edi
        mov     esi, ecx
        mov     ecx, dword ptr [esi + 4]
        push    ebx
        push    ebp
        call    FUN_00423710
        test    al, al
        jnz     skip
        mov     ecx, dword ptr [esi]
        mov     eax, dword ptr [ecx]
        mov     edx, dword ptr [eax + 0xc]
        push    ebx
        push    edi
        push    ebp
        call    edx
    skip:
        pop     edi
        pop     esi
        pop     ebp
        pop     ebx
        ret     0xc
    }
}
