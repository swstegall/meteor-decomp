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
// FUNCTION: ffxivgame 0x0004e400 — __thiscall forwarding wrapper that
//                                  computes an element address (55 bytes).
//
//   int __thiscall FUN_0044e400(void *this, int a0, int a1, int a2)
//     stack layout (after RET, ECX = this):
//       [ESP+0x04] : int a0   (param_1)
//       [ESP+0x08] : int a1   (param_2)
//       [ESP+0x0c] : int a2   (param_3)
//     returns: a0 + a1 * 0x54   (EAX)
//
// Behaviour (read from the orig bytes at RVA 0x0004e400, 55 bytes):
//   - Loads the three stack args into EDX/ESI/EDI and a local byte = 0.
//   - Calls the __cdecl helper FUN_0044e360 with 6 args, right-to-left:
//       FUN_0044e360(a0, a1, a2, this, a2, local_byte)
//     and discards its result (`add esp,0x18` cleans the 24 byte frame).
//   - Returns a0 + a1 * 0x54 (0x54 is the per-record stride).
//
// The `mov byte [esp+8],0` / `mov eax,[esp+8]` pair is MSVC materialising
// a zero-initialised stack local and passing it (as a dword) to the helper.
//
// Calling convention: __thiscall (ECX = this; three DWORD stack args;
// callee cleans 0xc via `ret 0xc`). The inner helper is __cdecl (caller
// cleans 0x18 = 6 args).
//
// Asm (55 bytes):
//   51                 PUSH ECX                  ; reserve local slot
//   8b 54 24 10        MOV  EDX, [ESP+0x10]      ; a2
//   56                 PUSH ESI
//   8b 74 24 10        MOV  ESI, [ESP+0x10]      ; a1
//   57                 PUSH EDI
//   8b 7c 24 10        MOV  EDI, [ESP+0x10]      ; a0
//   c6 44 24 08 00     MOV  byte [ESP+8], 0      ; local = 0
//   8b 44 24 08        MOV  EAX, [ESP+8]
//   50                 PUSH EAX                  ; arg6 = local
//   8b 44 24 1c        MOV  EAX, [ESP+0x1c]      ; a2
//   52                 PUSH EDX                  ; arg5 = a2
//   51                 PUSH ECX                  ; arg4 = this
//   50                 PUSH EAX                  ; arg3 = a2
//   56                 PUSH ESI                  ; arg2 = a1
//   57                 PUSH EDI                  ; arg1 = a0
//   e8 RR RR RR RR     CALL FUN_0044e360
//   8b c6              MOV  EAX, ESI             ; a1
//   6b c0 54           IMUL EAX, EAX, 0x54
//   83 c4 18           ADD  ESP, 0x18            ; clean 6 cdecl args
//   03 c7              ADD  EAX, EDI             ; + a0
//   5f                 POP  EDI
//   5e                 POP  ESI
//   59                 POP  ECX
//   c2 0c 00           RET  0xc
//
// Naked __asm so the PUSH/MOV interleave, the imm8-form IMUL, and the
// `ret 0xc` epilogue pin to the orig encoding. The single REL32 callsite
// (FUN_0044e360) is masked out of the byte diff by tools/compare.py.

extern "C" void FUN_0044e360();                 // __cdecl helper

extern "C" __declspec(naked) void FUN_0044e400() {
    __asm {
        push    ecx
        mov     edx, dword ptr [esp + 0x10]
        push    esi
        mov     esi, dword ptr [esp + 0x10]
        push    edi
        mov     edi, dword ptr [esp + 0x10]
        mov     byte ptr [esp + 8], 0
        mov     eax, dword ptr [esp + 8]
        push    eax
        mov     eax, dword ptr [esp + 0x1c]
        push    edx
        push    ecx
        push    eax
        push    esi
        push    edi
        call    FUN_0044e360
        mov     eax, esi
        imul    eax, eax, 0x54
        add     esp, 0x18
        add     eax, edi
        pop     edi
        pop     esi
        pop     ecx
        ret     0xc
    }
}
