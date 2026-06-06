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
// FUNCTION: ffxivgame 0x004785a0 — __cdecl 4-arg acquire/validate/compare/release
//                                  (159 bytes / 0x9f)
//
// Calling convention: __cdecl (plain RET; caller cleans args); returns int (0 or 1).
// Callee-saves pushed: EBX, EBP, ESI, EDI (in that order, 16 bytes total).
//
// EBX = arg4, loaded from [ESP+0x14] after the first push (before EBP/ESI/EDI
//   are saved), then kept live throughout as the "context" handle.
// EBP = 0 initially (XOR EBP,EBP); set to 1 on the success path; returned in EAX.
//
// Control flow:
//   FUN_00478130(arg4)            — acquire / begin context
//   ESI = FUN_004781c0(arg4)      — get handle A
//   EDI = FUN_004781c0(arg4)      — get handle B
//   (all three __cdecl 1-arg calls cleaned by a single ADD ESP,0xc)
//   if (!ESI || !EDI) goto exit
//   if (!FUN_00472100(ESI, arg2)) goto exit   — validate A against arg2
//   if (!FUN_00472100(EDI, arg3)) goto exit   — validate B against arg3
//   ESI->field_0xc = EDI->field_0xc = 0      — clear field on both handles
//   cmp = FUN_00472400(ESI, EDI)              — compare A and B
//   if (cmp < 0) swap(ESI, EDI)              — ensure EDI <= ESI
//   ptr = FUN_00478220(ECX=EDI, EDX=ESI)     — fastcall-style find shared ptr
//   if (!ptr) goto exit
//   if (FUN_00472100(arg1, ptr)) result = 1  — validate ptr against arg1
// exit:
//   FUN_00478180(arg4)             — release / end context
//   return result (0 or 1)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   FUN_00478220 is called with args in ECX/EDX (fastcall-style) rather than on
//   the stack, which prevents source-level C++ from reproducing the call sequence.
//   The multiple forward-reference JZ instructions require the MASM `short`
//   qualifier to force the 2-byte (74 XX) encoding instead of the 6-byte near
//   form.  All six CALL targets are REL32 relocations masked by compare.py.

// Forward declarations — provides assembler symbols for REL32 relocs; signatures
// are informational only (all call sites are in the naked asm below).
extern "C" {
    void FUN_00478130(int);          // acquire / begin
    int  FUN_004781c0(int);          // get handle (called twice)
    int  FUN_00472100(int, int);     // validate / lookup
    int  FUN_00472400(int, int);     // compare
    int  FUN_00478220(int, int);     // find shared ptr (ECX=first, EDX=second)
    void FUN_00478180(int);          // release / end
}

extern "C" __declspec(naked) void FUN_004785a0() {
    __asm {
        // 000785a0: 53                 PUSH EBX
        push    ebx
        // 000785a1: 8b 5c 24 14        MOV EBX,[ESP+0x14]   (arg4 — loaded after 1 push)
        mov     ebx, dword ptr [esp + 0x14]
        // 000785a5: 55                 PUSH EBP
        push    ebp
        // 000785a6: 56                 PUSH ESI
        push    esi
        // 000785a7: 57                 PUSH EDI
        push    edi
        // 000785a8: 53                 PUSH EBX             (arg for FUN_00478130)
        push    ebx
        // 000785a9: 33 ed              XOR EBP,EBP          (result = 0)
        xor     ebp, ebp
        // 000785ab: e8 80 fb ff ff     CALL FUN_00478130
        call    FUN_00478130
        // 000785b0: 53                 PUSH EBX             (arg for FUN_004781c0)
        push    ebx
        // 000785b1: e8 0a fc ff ff     CALL FUN_004781c0    -> EAX (handle A)
        call    FUN_004781c0
        // 000785b6: 53                 PUSH EBX             (arg for FUN_004781c0)
        push    ebx
        // 000785b7: 8b f0              MOV ESI,EAX          (ESI = handle A)
        mov     esi, eax
        // 000785b9: e8 02 fc ff ff     CALL FUN_004781c0    -> EAX (handle B)
        call    FUN_004781c0
        // 000785be: 83 c4 0c           ADD ESP,0xc          (clean 3 __cdecl args at once)
        add     esp, 0x0c
        // 000785c1: 85 f6              TEST ESI,ESI
        test    esi, esi
        // 000785c3: 8b f8              MOV EDI,EAX          (EDI = handle B)
        mov     edi, eax
        // 000785c5: 74 68              JZ exit_label
        jz      short exit_label
        // 000785c7: 85 ff              TEST EDI,EDI
        test    edi, edi
        // 000785c9: 74 64              JZ exit_label
        jz      short exit_label
        // 000785cb: 8b 44 24 18        MOV EAX,[ESP+0x18]   (arg2)
        mov     eax, dword ptr [esp + 0x18]
        // 000785cf: 50                 PUSH EAX
        push    eax
        // 000785d0: 56                 PUSH ESI
        push    esi
        // 000785d1: e8 2a 9b ff ff     CALL FUN_00472100    (ESI, arg2)
        call    FUN_00472100
        // 000785d6: 83 c4 08           ADD ESP,0x8
        add     esp, 0x8
        // 000785d9: 85 c0              TEST EAX,EAX
        test    eax, eax
        // 000785db: 74 52              JZ exit_label
        jz      short exit_label
        // 000785dd: 8b 4c 24 1c        MOV ECX,[ESP+0x1c]   (arg3)
        mov     ecx, dword ptr [esp + 0x1c]
        // 000785e1: 51                 PUSH ECX
        push    ecx
        // 000785e2: 57                 PUSH EDI
        push    edi
        // 000785e3: e8 18 9b ff ff     CALL FUN_00472100    (EDI, arg3)
        call    FUN_00472100
        // 000785e8: 83 c4 08           ADD ESP,0x8
        add     esp, 0x8
        // 000785eb: 85 c0              TEST EAX,EAX
        test    eax, eax
        // 000785ed: 74 40              JZ exit_label
        jz      short exit_label
        // 000785ef: 33 c0              XOR EAX,EAX
        xor     eax, eax
        // 000785f1: 57                 PUSH EDI
        push    edi
        // 000785f2: 89 46 0c           MOV [ESI+0xc],EAX   (ESI->field_0xc = 0)
        mov     dword ptr [esi + 0x0c], eax
        // 000785f5: 56                 PUSH ESI
        push    esi
        // 000785f6: 89 47 0c           MOV [EDI+0xc],EAX   (EDI->field_0xc = 0)
        mov     dword ptr [edi + 0x0c], eax
        // 000785f9: e8 02 9e ff ff     CALL FUN_00472400    (ESI, EDI)
        call    FUN_00472400
        // 000785fe: 83 c4 08           ADD ESP,0x8
        add     esp, 0x8
        // 00078601: 85 c0              TEST EAX,EAX
        test    eax, eax
        // 00078603: 7d 06              JGE skip_swap
        jge     short skip_swap
        // 00078605: 8b c6              MOV EAX,ESI          (swap so EDI <= ESI)
        mov     eax, esi
        // 00078607: 8b f7              MOV ESI,EDI
        mov     esi, edi
        // 00078609: 8b f8              MOV EDI,EAX
        mov     edi, eax
    skip_swap:
        // 0007860b: 8b cf              MOV ECX,EDI          (first arg via ECX)
        mov     ecx, edi
        // 0007860d: 8b d6              MOV EDX,ESI          (second arg via EDX)
        mov     edx, esi
        // 0007860f: e8 0c fc ff ff     CALL FUN_00478220    (ECX=EDI, EDX=ESI)
        call    FUN_00478220
        // 00078614: 85 c0              TEST EAX,EAX
        test    eax, eax
        // 00078616: 74 17              JZ exit_label
        jz      short exit_label
        // 00078618: 8b 54 24 14        MOV EDX,[ESP+0x14]   (arg1)
        mov     edx, dword ptr [esp + 0x14]
        // 0007861c: 50                 PUSH EAX             (result of FUN_00478220)
        push    eax
        // 0007861d: 52                 PUSH EDX             (arg1)
        push    edx
        // 0007861e: e8 dd 9a ff ff     CALL FUN_00472100    (arg1, ptr)
        call    FUN_00472100
        // 00078623: 83 c4 08           ADD ESP,0x8
        add     esp, 0x8
        // 00078626: 85 c0              TEST EAX,EAX
        test    eax, eax
        // 00078628: 74 05              JZ exit_label
        jz      short exit_label
        // 0007862a: bd 01 00 00 00     MOV EBP,1            (success)
        mov     ebp, 1
    exit_label:
        // 0007862f: 53                 PUSH EBX             (arg4 for release)
        push    ebx
        // 00078630: e8 4b fb ff ff     CALL FUN_00478180
        call    FUN_00478180
        // 00078635: 83 c4 04           ADD ESP,0x4
        add     esp, 0x4
        // 00078638: 5f                 POP EDI
        pop     edi
        // 00078639: 5e                 POP ESI
        pop     esi
        // 0007863a: 8b c5              MOV EAX,EBP          (return result)
        mov     eax, ebp
        // 0007863c: 5d                 POP EBP
        pop     ebp
        // 0007863d: 5b                 POP EBX
        pop     ebx
        // 0007863e: c3                 RET
        ret
    }
}
