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
// FUNCTION: ffxivgame 0x00459270 — __stdcall HRESULT wrapper: calls
//                                   FUN_00458f30 (__thiscall, ECX=arg1)
//                                   to resolve a count from (arg2, &arg2),
//                                   then dispatches to vtable[3] of the
//                                   sub-object at arg1+0x8 with (arg2, count-1).
//                                   Returns S_OK on success, E_INVALIDARG
//                                   (0x80070057) if count == 0.
//
// Calling convention: __stdcall, 2 DWORD stack args; callee-cleans via RET 8.
//
// Asm (60 bytes = 0x3c):
//   8b 4c 24 08     MOV ECX, [ESP+0x8]           ; arg2 → ECX (pre-PUSH)
//   56              PUSH ESI
//   8b 74 24 08     MOV ESI, [ESP+0x8]            ; arg1 → ESI (post-PUSH)
//   8d 44 24 0c     LEA EAX, [ESP+0xc]            ; EAX = &arg2 (on stack)
//   50              PUSH EAX                      ; push &arg2
//   51              PUSH ECX                      ; push arg2
//   8b ce           MOV ECX, ESI                  ; this = arg1
//   e8 aa fc ff ff  CALL FUN_00458f30             ; __thiscall: count = fn(arg1,arg2,&arg2)
//   85 c0           TEST EAX, EAX
//   76 19           JBE fail                      ; if count == 0 → E_INVALIDARG
//   8b 4e 08        MOV ECX, [ESI+0x8]            ; ECX = *(arg1+8)  (sub-object)
//   8b 11           MOV EDX, [ECX]               ; EDX = vtable ptr
//   8b 52 0c        MOV EDX, [EDX+0xc]           ; EDX = vtable[3]
//   83 c0 ff        ADD EAX, -0x1                 ; count - 1
//   50              PUSH EAX                      ; push count-1
//   8b 44 24 10     MOV EAX, [ESP+0x10]           ; reload arg2 (may be modified)
//   50              PUSH EAX                      ; push arg2
//   ff d2           CALL EDX                      ; vtable[3](arg2, count-1)
//   33 c0           XOR EAX, EAX                  ; return S_OK
//   5e              POP ESI
//   c2 08 00        RET 0x8
// fail:
//   b8 57 00 07 80  MOV EAX, 0x80070057           ; return E_INVALIDARG
//   5e              POP ESI
//   c2 08 00        RET 0x8

extern "C" int FUN_00458f30();

extern "C" __declspec(naked) void FUN_00459270() {
    __asm {
        mov     ecx, dword ptr [esp+0x8]
        push    esi
        mov     esi, dword ptr [esp+0x8]
        lea     eax, [esp+0xc]
        push    eax
        push    ecx
        mov     ecx, esi
        call    FUN_00458f30
        test    eax, eax
        jbe     fail
        mov     ecx, dword ptr [esi+0x8]
        mov     edx, dword ptr [ecx]
        mov     edx, dword ptr [edx+0xc]
        add     eax, -1
        push    eax
        mov     eax, dword ptr [esp+0x10]
        push    eax
        call    edx
        xor     eax, eax
        pop     esi
        ret     0x8
    fail:
        mov     eax, 0x80070057
        pop     esi
        ret     0x8
    }
}
