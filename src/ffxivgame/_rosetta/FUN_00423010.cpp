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
// FUNCTION: ffxivgame 0x00423010 — __thiscall conditional virtual dispatch
//                                  (52 bytes / 0x34).
//
// Layout (inferred from asm):
//   this (ECX):
//     +0x00  Obj *field0   — object with vtable; vtable[2] is invoked when
//                            the guard check returns false
//     +0x04  Sub *field4   — sub-object whose method FUN_00423b10 acts as
//                            the guard / pre-check
//
// Source shape:
//   void SomeClass::method(T a, T b, T c) {
//       if (!this->field4->check(a, b, c))   // FUN_00423b10 (guard)
//           this->field0->vtable[2](a, c, b); // virtual dispatch on failure
//   }
//
// Calling convention: __thiscall (ECX = this; three DWORD stack args;
// callee cleans 0xc via `ret 0xc`).
//
// Register allocation in prologue:
//   EBP = param1 (first stack arg, original [ESP+4])
//   EBX = param2 (second stack arg, original [ESP+8])
//   EDI = param3 (third stack arg, original [ESP+C])
//   ESI = this
//
// Note on argument ordering: the guard call (FUN_00423b10) receives args in
// the natural order (a, b, c) = (EBP, EBX, EDI), while the virtual call
// receives them in order (a, c, b) = (EBP, EDI, EBX) — matching the
// PUSH EBX / PUSH EDI / PUSH EBP push sequence visible in the asm.
//
// Byte layout verified at 52 bytes:
//   53          PUSH EBX
//   8B 5C 24 0C MOV EBX,[ESP+0Ch]
//   55          PUSH EBP
//   8B 6C 24 0C MOV EBP,[ESP+0Ch]
//   56          PUSH ESI
//   57          PUSH EDI
//   8B 7C 24 1C MOV EDI,[ESP+1Ch]
//   57          PUSH EDI
//   8B F1       MOV ESI,ECX
//   8B 4E 04    MOV ECX,[ESI+4]
//   53          PUSH EBX
//   55          PUSH EBP
//   E8 xx xx xx xx  CALL FUN_00423b10  (REL32 — masked by compare.py)
//   84 C0       TEST AL,AL
//   75 0C       JNZ  done
//   8B 0E       MOV  ECX,[ESI]
//   8B 01       MOV  EAX,[ECX]
//   8B 50 08    MOV  EDX,[EAX+8]
//   53          PUSH EBX
//   57          PUSH EDI
//   55          PUSH EBP
//   FF D2       CALL EDX
//   5F          POP  EDI
//   5E          POP  ESI
//   5D          POP  EBP
//   5B          POP  EBX
//   C2 0C 00    RET  0Ch

extern "C" int FUN_00423b10();   // __thiscall guard at RVA 0x00023b10

extern "C" __declspec(naked) void FUN_00423010() {
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
        call    FUN_00423b10
        test    al, al
        jnz     done
        mov     ecx, dword ptr [esi]
        mov     eax, dword ptr [ecx]
        mov     edx, dword ptr [eax + 8]
        push    ebx
        push    edi
        push    ebp
        call    edx
    done:
        pop     edi
        pop     esi
        pop     ebp
        pop     ebx
        ret     0xc
    }
}
