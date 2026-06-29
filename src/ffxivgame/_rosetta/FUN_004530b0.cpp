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
// FUNCTION: ffxivgame 0x000530b0 — `__cdecl` conditional format-and-write
//                                  helper for a logger struct (64 bytes).
//
// Calling convention: __cdecl (three DWORD stack args; RET with no cleanup).
//
// Arguments:
//   [ESP+0x4] : struct *s   — pointer to logger struct
//   [ESP+0x8] : void *fmt   — format string (arg2)
//   [ESP+0xc] : DWORD arg3  — first variadic arg (address taken, passed as va_list)
//
// Struct layout (inferred):
//   +0x00  DWORD  _pad        (not referenced here)
//   +0x04  void  *handle      (non-null check before writing)
//   ...
//   +0x0d  BYTE   enabled     (non-zero check before writing)
//   +0x0e  char   buf[...]    (output buffer; formatted in place by FUN_009d8cbd)
//
// Logic:
//   if (s->handle == NULL) return false;
//   if (!s->enabled)       return false;
//   FUN_009d8cbd(&s->buf, 0x2000, fmt, &arg3);   // vsnprintf-style format into buf
//   FUN_009d741c(s->handle, &s->buf);             // write buf to handle
//   return true;
//
// Register allocation notes:
//   ESI = s (arg1, held across both calls)
//   EDI = &s->buf (set once via LEA [ESI+0xe], reused for both call sites)
//   ECX = fmt (arg2, loaded from [ESP+0xc] BEFORE PUSH EDI to avoid the
//              larger [ESP+0x10] displacement that would be needed after)
//   EAX = &arg3 (va_list, computed via LEA [ESP+0x14] AFTER PUSH EDI)
//   EDX = s->handle (reloaded from [ESI+0x4] for the second call)
//
// Fail path (JZ fail × 2): both jumps are short (rel8 fits in a signed
// byte — distances 0x31 and 0x2b respectively); MSVC inline assembler
// reduces to the 74/75 form for forward refs within the same __asm block.
//
// ADD ESP, 0x18: cleans 24 bytes = 4 args to FUN_009d8cbd (16 B) +
//               2 args to FUN_009d741c (8 B) in one shot.
//
// Asm (64 bytes):
//   56                   PUSH ESI
//   8b 74 24 08          MOV  ESI, [ESP+0x8]        ; s = arg1
//   83 7e 04 00          CMP  dword ptr [ESI+0x4], 0
//   74 31                JZ   fail                   ; s->handle == NULL
//   80 7e 0d 00          CMP  byte ptr [ESI+0xd], 0
//   74 2b                JZ   fail                   ; !s->enabled
//   8b 4c 24 0c          MOV  ECX, [ESP+0xc]         ; ECX = fmt (preloaded before PUSH EDI)
//   57                   PUSH EDI
//   8d 44 24 14          LEA  EAX, [ESP+0x14]        ; EAX = &arg3 (va_list)
//   50                   PUSH EAX
//   51                   PUSH ECX                    ; fmt
//   8d 7e 0e             LEA  EDI, [ESI+0xe]         ; EDI = &s->buf
//   68 00 20 00 00       PUSH 0x2000                 ; buf size
//   57                   PUSH EDI                    ; buf
//   e8 ...               CALL FUN_009d8cbd           ; (rel32)
//   8b 56 04             MOV  EDX, [ESI+0x4]         ; s->handle
//   57                   PUSH EDI                    ; buf
//   52                   PUSH EDX                    ; handle
//   e8 ...               CALL FUN_009d741c           ; (rel32)
//   83 c4 18             ADD  ESP, 0x18              ; clean 6 args
//   5f                   POP  EDI
//   b0 01                MOV  AL, 1                  ; return true
//   5e                   POP  ESI
//   c3                   RET
// fail:
//   32 c0                XOR  AL, AL                 ; return false
//   5e                   POP  ESI
//   c3                   RET

extern "C" void FUN_009d8cbd();
extern "C" void FUN_009d741c();

extern "C" __declspec(naked) void FUN_004530b0()
{
    __asm {
        push    esi
        mov     esi, dword ptr [esp + 0x8]
        cmp     dword ptr [esi + 0x4], 0
        jz      fail
        cmp     byte ptr [esi + 0xd], 0
        jz      fail
        mov     ecx, dword ptr [esp + 0xc]
        push    edi
        lea     eax, [esp + 0x14]
        push    eax
        push    ecx
        lea     edi, [esi + 0xe]
        push    0x2000
        push    edi
        call    FUN_009d8cbd
        mov     edx, dword ptr [esi + 0x4]
        push    edi
        push    edx
        call    FUN_009d741c
        add     esp, 0x18
        pop     edi
        mov     al, 0x1
        pop     esi
        ret
    fail:
        xor     al, al
        pop     esi
        ret
    }
}
