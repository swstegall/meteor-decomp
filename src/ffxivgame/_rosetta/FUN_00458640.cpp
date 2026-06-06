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
// FUNCTION: ffxivgame 0x00058640 — __cdecl string-render loop +
//                                  MSVC inline strlen (62 B / 0x3e).
//
// Stack layout on entry (CDECL):
//   [ESP + 0x04]  arg1  — object context ptr, placed in EBP
//   [ESP + 0x08]  arg2  — char* string, placed in EDI; used for strlen result
//   [ESP + 0x0c]  arg3  — slot index, placed in EBX
//
// EAX convention: FUN_004580b0 uses a non-standard EAX-in parameter
//   (confirmed in its matched source: first thing it does is MOV ESI, EAX).
//   The caller passes arg1 in EAX immediately before the CALL.
//   Stack args to FUN_004580b0: (i=ESI, arg2=EDI, arg3=EBX, 0x0).
//   Caller cleans 0x10 bytes (4 dwords — the explicit stack args only).
//
// Body: loops i = 0 .. 3 (inclusive, JLE), calling FUN_004580b0 each time;
//       then returns strlen(arg2) via MSVC's classic inline idiom:
//         LEA EDX,[EAX+1]; MOV CL,[EAX]; INC EAX; TEST CL,CL; JNZ;
//         SUB EAX,EDX  → returns length in EAX.
//
// Prologue uses MSVC 2005's interleaved push+load pattern: push a callee-
// saved register then immediately load its value from the (just-updated)
// stack offset, reducing pipeline stalls.  Pop order in epilogue is
// EDI, ESI, EBP, (LEA EDX), EBX — the LEA is hoisted above POP EBX so
// EDX is ready for the strlen loop without a stall.
//
// Calling convention: __cdecl (plain RET; 3 stack args; caller cleans).
// Return value: int (strlen of arg2).
//
// Asm (62 bytes):
//   53                       PUSH EBX
//   8b 5c 24 10              MOV  EBX, [ESP+0x10]          ; arg3
//   55                       PUSH EBP
//   8b 6c 24 0c              MOV  EBP, [ESP+0x0c]          ; arg1
//   56                       PUSH ESI
//   57                       PUSH EDI
//   8b 7c 24 18              MOV  EDI, [ESP+0x18]          ; arg2
//   33 f6                    XOR  ESI, ESI                 ; i = 0
// loop_body:
//   6a 00                    PUSH 0x0
//   53                       PUSH EBX
//   57                       PUSH EDI
//   56                       PUSH ESI
//   8b c5                    MOV  EAX, EBP                 ; EAX = arg1
//   e8 RR RR RR RR           CALL FUN_004580b0
//   83 c6 01                 ADD  ESI, 0x1
//   83 c4 10                 ADD  ESP, 0x10
//   83 fe 03                 CMP  ESI, 0x3
//   7e e9                    JLE  loop_body
//   8b c7                    MOV  EAX, EDI
//   5f                       POP  EDI
//   5e                       POP  ESI
//   5d                       POP  EBP
//   8d 50 01                 LEA  EDX, [EAX+0x1]
//   5b                       POP  EBX
// strlen_loop:
//   8a 08                    MOV  CL, byte ptr [EAX]
//   83 c0 01                 ADD  EAX, 0x1
//   84 c9                    TEST CL, CL
//   75 f7                    JNZ  strlen_loop
//   2b c2                    SUB  EAX, EDX
//   c3                       RET

extern "C" void FUN_004580b0();   // EAX-in non-standard convention

extern "C" __declspec(naked) int FUN_00458640() {
    __asm {
        push    ebx
        mov     ebx, dword ptr [esp + 0x10]
        push    ebp
        mov     ebp, dword ptr [esp + 0x0c]
        push    esi
        push    edi
        mov     edi, dword ptr [esp + 0x18]
        xor     esi, esi
    loop_body:
        push    0x0
        push    ebx
        push    edi
        push    esi
        mov     eax, ebp
        call    FUN_004580b0
        add     esi, 0x1
        add     esp, 0x10
        cmp     esi, 0x3
        jle     loop_body
        mov     eax, edi
        pop     edi
        pop     esi
        pop     ebp
        lea     edx, [eax + 0x1]
        pop     ebx
    strlen_loop:
        mov     cl, byte ptr [eax]
        add     eax, 0x1
        test    cl, cl
        jnz     strlen_loop
        sub     eax, edx
        ret
    }
}
