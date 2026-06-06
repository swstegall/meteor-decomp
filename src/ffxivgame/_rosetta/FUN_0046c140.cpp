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
// FUNCTION: ffxivgame 0x0046c140 — _X509_NAME_print_ex (57 bytes)
//
// OpenSSL X509_NAME_print_ex wrapper. When flags == 0 (XN_FLAG_COMPAT),
// delegates to a simpler X509_NAME_print at FUN_00482210; otherwise
// calls the full formatter FUN_0046bd60 (__thiscall, BIO *out in ECX)
// with a pre-built default ASN1_PCTX table at 0x0046b690 as arg1.
//
// Calling convention: __cdecl (plain RET; caller-cleans).
// Frame: none (no EBP frame, no callee-save pushes).
//
// Signature (inferred):
//   int __cdecl (void *out, void *nm, int indent, unsigned long flags)
//   [ESP+0x04] = out
//   [ESP+0x08] = nm
//   [ESP+0x0c] = indent
//   [ESP+0x10] = flags
//
// Branch shape:
//   if (flags == 0)
//       → FUN_00482210(out, nm, indent); return
//   else
//       → ECX=out; FUN_0046bd60(0x46b690, nm, indent, flags); return
//
// Asm (57 bytes):
//   8b 44 24 10   MOV  EAX,[ESP+0x10]        ; flags
//   85 c0         TEST EAX,EAX
//   8b 4c 24 08   MOV  ECX,[ESP+0x8]         ; nm
//   75 14         JNZ  else_branch
//   8b 44 24 0c   MOV  EAX,[ESP+0xc]         ; indent
//   8b 54 24 04   MOV  EDX,[ESP+0x4]         ; out
//   50            PUSH EAX                   ; push indent
//   51            PUSH ECX                   ; push nm
//   52            PUSH EDX                   ; push out
//   e8 ~ ~ ~ ~   CALL FUN_00482210           ; X509_NAME_print(out,nm,indent)
//   83 c4 0c      ADD  ESP,0xc
//   c3            RET
// else_branch:
//   50            PUSH EAX                   ; push flags
//   8b 44 24 10   MOV  EAX,[ESP+0x10]        ; indent (ESP shifted -4)
//   50            PUSH EAX                   ; push indent
//   51            PUSH ECX                   ; push nm
//   8b 4c 24 10   MOV  ECX,[ESP+0x10]        ; out (ESP shifted -12)
//   68 90 b6 46 00 PUSH 0x46b690             ; push default_pctx ptr
//   e8 ~ ~ ~ ~   CALL FUN_0046bd60           ; do_name_ex(this=out,...)
//   83 c4 10      ADD  ESP,0x10
//   c3            RET

extern "C" int FUN_00482210();
extern "C" int FUN_0046bd60();

extern "C" __declspec(naked) void FUN_0046c140() {
    __asm {
        mov     eax, dword ptr [esp + 0x10]
        test    eax, eax
        mov     ecx, dword ptr [esp + 0x8]
        jnz     else_branch
        mov     eax, dword ptr [esp + 0xc]
        mov     edx, dword ptr [esp + 0x4]
        push    eax
        push    ecx
        push    edx
        call    FUN_00482210
        add     esp, 0xc
        ret
    else_branch:
        push    eax
        mov     eax, dword ptr [esp + 0x10]
        push    eax
        push    ecx
        mov     ecx, dword ptr [esp + 0x10]
        push    0x46b690
        call    FUN_0046bd60
        add     esp, 0x10
        ret
    }
}
