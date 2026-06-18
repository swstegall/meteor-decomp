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
// FUNCTION: ffxivgame 0x004236e0 — __thiscall set-if-changed + virtual notify
//                                  (44 B / 0x2C)
//
// void __thiscall FUN_004236e0(unsigned int index, int value)
//
//   ECX → this  (a struct with two sub-object pointers at offsets 0 and 4)
//   arg1 (first  stack param, [ESP+4] at entry) → index
//   arg2 (second stack param, [ESP+8] at entry) → value
//
// Pseudo-C:
//
//   void FUN_004236e0(unsigned int index, int value) {    // __thiscall
//       if (!this->field4->FUN_00424010(index, value)) {
//           // field0 has a vtable; call virtual[9] (offset 0x24) with same args
//           (this->field0->vtable[9])(index, value);
//       }
//   }
//
// Asm (44 bytes @ orig RVA 0x000236e0):
//
//   53              PUSH EBX
//   8b 5c 24 08     MOV  EBX, [ESP+0x8]          ; EBX = index (arg1)
//   56              PUSH ESI
//   57              PUSH EDI
//   8b 7c 24 14     MOV  EDI, [ESP+0x14]          ; EDI = value (arg2)
//   8b f1           MOV  ESI, ECX                 ; ESI = this
//   8b 4e 04        MOV  ECX, [ESI+0x4]           ; ECX = this->field4
//   57              PUSH EDI                       ; push value
//   53              PUSH EBX                       ; push index
//   e8 19 09 00 00  CALL FUN_00424010              ; field4->set_if_changed(index, value)
//   84 c0           TEST AL, AL
//   75 0b           JNZ  done                      ; if changed, nothing more to do
//   8b 0e           MOV  ECX, [ESI]               ; ECX = this->field0
//   8b 01           MOV  EAX, [ECX]               ; EAX = vtable of field0
//   8b 50 24        MOV  EDX, [EAX+0x24]          ; EDX = vtable[9]
//   57              PUSH EDI                       ; push value
//   53              PUSH EBX                       ; push index
//   ff d2           CALL EDX                       ; field0->vtable[9](index, value)
//   5f              POP  EDI
//   5e              POP  ESI
//   5b              POP  EBX
//   c2 08 00        RET  0x8
//
// Reconstruction: __declspec(naked) with inline assembly. The indirect CALL
// through EDX (vtable dispatch) and the precise interleaving of register
// saves vs. argument loads (EBX before ESI/EDI saves, EDI read after all
// three saves) pin the encoding; naked-asm reproduces this exactly. The
// CALL rel32 to FUN_00424010 carries one relocation masked by compare.py.

extern "C" void FUN_00424010(void);

extern "C" __declspec(naked) void FUN_004236e0() {
    __asm {
        push    ebx
        mov     ebx, dword ptr [esp + 0x8]
        push    esi
        push    edi
        mov     edi, dword ptr [esp + 0x14]
        mov     esi, ecx
        mov     ecx, dword ptr [esi + 0x4]
        push    edi
        push    ebx
        call    FUN_00424010
        test    al, al
        jnz     done
        mov     ecx, dword ptr [esi]
        mov     eax, dword ptr [ecx]
        mov     edx, dword ptr [eax + 0x24]
        push    edi
        push    ebx
        call    edx
    done:
        pop     edi
        pop     esi
        pop     ebx
        ret     8
    }
}
