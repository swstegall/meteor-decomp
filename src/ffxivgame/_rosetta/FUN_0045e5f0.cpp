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
// FUNCTION: ffxivgame 0x0045e5f0 — __cdecl 4-arg pointer-field
//                                  setter/update dispatcher (0x81 = 129 B).
//
// Asm shape (read from RVA 0x0005e5f0, 129 bytes of `.text`):
//
//   __cdecl int FUN_0045e5f0(
//       Obj  *p,       // [esp+4] at entry → ESI (used throughout)
//       void *newPtr,  // [esp+8] at entry (= [esp+10] after PUSH EDI)
//       int   mode,    // [esp+C] at entry (= [esp+14] after PUSH EDI) → EDI
//       int   extra    // [esp+10] at entry (= [esp+18] after PUSH EDI)
//   )
//
//   Logical behaviour:
//     if (!p) return 0;
//     if (mode != -1 && !p->field4) {
//         p->field4 = (void*)FUN_0045da80();
//         if (!p->field4) return 0;
//     }
//     if (p->field0) FUN_0046cae0(p->field0);
//     // Scheduler interleaving: TEST EDI,EDI / load newPtr / store / JZ
//     p->field0 = newPtr;
//     if (!mode) return 1;
//     if (mode == -1) {
//         if (!p->field4) return 1;
//         FUN_0045da90(p->field4);
//         p->field4 = NULL;
//         return mode + 2;   // == 1; compiler emits LEA EAX,[EDI+2] (3 B vs 5 B)
//     }
//     FUN_0046c460(p->field4, mode, extra);
//     return 1;
//
//   Calling convention: __cdecl (plain RET, caller cleans args).
//   Frame: PUSH ESI + PUSH EDI only — no SUB ESP, no SEH, no GS cookie.
//
//   Callees (all __cdecl, caller cleans stack):
//     FUN_0045da80 : 0 args, returns int in EAX
//     FUN_0046cae0 : 1 arg  (ADD ESP,4 after)
//     FUN_0045da90 : 1 arg  (ADD ESP,4 after)
//     FUN_0046c460 : 3 args in order (p->field4, mode, extra) (ADD ESP,0xC after)
//
//   Instruction-scheduling note: MSVC /O2 emits
//       TEST EDI,EDI  /  MOV EAX,[esp+10]  /  MOV [ESI],EAX  /  JZ
//   so newPtr is loaded and stored between the TEST and the conditional
//   branch that consumes the flags — a classic /O2 scheduling artefact
//   that C++ source recompilation may not reproduce, making naked asm
//   the safe vehicle.
//
//   Notable encoding: LEA EAX,[EDI+2] at byte offset +0x5a encodes the
//   return value 1 (when mode==-1) in 3 bytes vs the 5-byte MOV EAX,1.
//
//   Reloc-bearing CALL sites (4-byte displacements masked by compare.py):
//     offset +0x1d  CALL FUN_0045da80  (RVA 0x0005da80)
//     offset +0x33  CALL FUN_0046cae0  (RVA 0x0006cae0)
//     offset +0x52  CALL FUN_0045da90  (RVA 0x0005da90)
//     offset +0x71  CALL FUN_0046c460  (RVA 0x0006c460)

extern "C" {
    int  FUN_0045da80();
    void FUN_0046cae0(int);
    void FUN_0045da90(int);
    void FUN_0046c460(int, int, int);
}

extern "C" __declspec(naked) void FUN_0045e5f0() {
    __asm {
        // Prologue — save ESI; load arg1 (p) into ESI
        push    esi
        mov     esi, dword ptr [esp + 0x8]   ; ESI = p  (arg1)
        test    esi, esi
        jnz     loc_e5fd
        xor     eax, eax                     ; return 0
        pop     esi
        ret

    loc_e5fd:
        // Push EDI; load arg3 (mode) into EDI
        push    edi
        mov     edi, dword ptr [esp + 0x14]  ; EDI = mode  (arg3, after 2 pushes)
        cmp     edi, -1
        jz      loc_e61c                     ; mode == -1: skip init path
        cmp     dword ptr [esi + 4], 0       ; p->field4 == NULL?
        jnz     loc_e61c                     ; already init'd: skip
        call    FUN_0045da80                 ; allocate / init  (0 args, __cdecl)
        test    eax, eax
        mov     dword ptr [esi + 4], eax     ; p->field4 = result
        jnz     loc_e61c                     ; success: proceed
        pop     edi                          ; failure: return 0
        pop     esi
        ret

    loc_e61c:
        // Free old field0 if non-null, then install newPtr
        mov     eax, dword ptr [esi]         ; EAX = p->field0
        test    eax, eax
        jz      loc_e62b
        push    eax
        call    FUN_0046cae0                 ; free/release old ptr  (1 arg, __cdecl)
        add     esp, 4

    loc_e62b:
        // Scheduler interleaving: TEST sets flags, then load+store happen
        // before the JZ that consumes those flags (MSVC /O2 artefact).
        test    edi, edi                     ; mode == 0?
        mov     eax, dword ptr [esp + 0x10]  ; EAX = newPtr  (arg2, after 2 pushes)
        mov     dword ptr [esi], eax         ; p->field0 = newPtr
        jz      loc_e669                     ; mode == 0: return 1

        cmp     edi, -1
        jnz     loc_e657                     ; mode != -1: call updater

        // mode == -1: tear down field4
        mov     eax, dword ptr [esi + 4]     ; EAX = p->field4
        test    eax, eax
        jz      loc_e669                     ; field4 == NULL: return 1
        push    eax
        call    FUN_0045da90                 ; shutdown field4  (1 arg, __cdecl)
        add     esp, 4
        lea     eax, [edi + 2]              ; EAX = mode+2 == 1  (3-byte form)
        pop     edi
        mov     dword ptr [esi + 4], 0      ; p->field4 = NULL
        pop     esi
        ret

    loc_e657:
        // mode != 0 and mode != -1: call the update helper
        mov     ecx, dword ptr [esp + 0x18]  ; ECX = extra  (arg4, after 2 pushes)
        mov     edx, dword ptr [esi + 4]     ; EDX = p->field4
        push    ecx                          ; push extra  (arg3 of FUN_0046c460)
        push    edi                          ; push mode   (arg2 of FUN_0046c460)
        push    edx                          ; push field4 (arg1 of FUN_0046c460)
        call    FUN_0046c460                 ; (3 args, __cdecl)
        add     esp, 0xc

    loc_e669:
        pop     edi
        mov     eax, 1                       ; return 1
        pop     esi
        ret
    }
}

// vim: ts=4 sts=4 sw=4 et
