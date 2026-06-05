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
// FUNCTION: ffxivgame 0x0005ce10 — __cdecl RSA-pointer setter: calls the
//                                  non-standard-convention helper FUN_0045ccc0,
//                                  stores the new RSA* into the object at +0x14,
//                                  increments its refcount via _RSA_up_ref, and
//                                  returns 1 if the pointer is non-null (58 B / 0x3a).
//
// Calling convention: __cdecl (bare RET; two DWORD stack args).
//   [ESP+4]  arg1 — pointer to the owning object (struct with RSA* at +0x14)
//   [ESP+8]  arg2 — new RSA* value (int-sized)
//
// FUN_0045ccc0 non-standard convention (documented in sibling FUN_0045cdc0):
//   ESI  — register arg: the object pointer (already in ESI from our load)
//   ECX  — register arg: null (zeroed immediately before the call)
//   stack arg1 (leftmost) = 6  (operation mode flag)
//   stack arg2 (rightmost) = -1
//   Caller cleans the two stack args via ADD ESP, 8 (callee does plain RET
//   + POP ECX in its epilogue to discard only its internal alloca slot).
//
// _RSA_up_ref is OpenSSL's RSA_up_ref (increments refcount); __cdecl(int rsa).
//
// Source shape (inferred):
//
//   int FUN_0045ce10(ObjWithRSA *pObj, int rsa) {
//       if (!FUN_0045ccc0(/* ESI=pObj, ECX=0, */ 6, -1))
//           return 0;
//       int bNonZero = (rsa != 0) ? 1 : 0;
//       pObj->rsa = rsa;                 // [pObj+0x14] = rsa
//       if (bNonZero)
//           _RSA_up_ref(rsa);
//       return bNonZero;
//   }
//
// Asm (58 bytes, two REL32 call offsets masked by tools/compare.py):
//
//   56             PUSH ESI
//   8b 74 24 08    MOV  ESI, [ESP+0x8]      ; arg1 = pObj
//   6a ff          PUSH -1                   ; stack arg2 for FUN_0045ccc0
//   6a 06          PUSH 6                    ; stack arg1 for FUN_0045ccc0
//   33 c9          XOR  ECX, ECX             ; register arg ECX = 0
//   e8 ?? ?? ?? ?? CALL FUN_0045ccc0
//   83 c4 08       ADD  ESP, 0x8             ; caller-clean 2 stack args
//   85 c0          TEST EAX, EAX
//   75 02          JNZ  +2                   ; if result != 0, continue
//   5e             POP  ESI
//   c3             RET
//   8b 44 24 0c    MOV  EAX, [ESP+0xc]      ; arg2 = rsa
//   33 c9          XOR  ECX, ECX
//   85 c0          TEST EAX, EAX
//   0f 95 c1       SETNZ CL                  ; CL = (rsa != 0)
//   89 46 14       MOV  [ESI+0x14], EAX      ; pObj->rsa = rsa
//   8b f1          MOV  ESI, ECX             ; ESI = bNonZero
//   85 f6          TEST ESI, ESI
//   74 09          JZ   +9                   ; if bNonZero == 0, skip
//   50             PUSH EAX                  ; push rsa for _RSA_up_ref
//   e8 ?? ?? ?? ?? CALL _RSA_up_ref
//   83 c4 04       ADD  ESP, 0x4
//   8b c6          MOV  EAX, ESI             ; return bNonZero
//   5e             POP  ESI
//   c3             RET

extern "C" void FUN_0045ccc0();    // non-standard: ESI + ECX register args + 2 caller-cleaned stack args
extern "C" int  _RSA_up_ref(int);  // __cdecl

extern "C" __declspec(naked) void FUN_0045ce10() {
    __asm {
        push    esi
        mov     esi, dword ptr [esp + 8]       // arg1 = pObj
        push    -1
        push    6
        xor     ecx, ecx
        call    FUN_0045ccc0
        add     esp, 8
        test    eax, eax
        jnz     cont
        pop     esi
        ret
    cont:
        mov     eax, dword ptr [esp + 0xc]     // arg2 = rsa
        xor     ecx, ecx
        test    eax, eax
        setnz   cl
        mov     dword ptr [esi + 0x14], eax
        mov     esi, ecx
        test    esi, esi
        jz      done
        push    eax
        call    _RSA_up_ref
        add     esp, 4
    done:
        mov     eax, esi
        pop     esi
        ret
    }
}
