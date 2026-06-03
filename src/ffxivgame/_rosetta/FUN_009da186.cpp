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
// FUNCTION: ffxivgame 0x005da186 — FUN_009da186 (__cdecl, 26 B)
//
// A guarded-free helper: receives a raw payload pointer, validates the
// 8-byte block header immediately preceding it (sentinel word 0xdddd),
// and — if valid — forwards the header pointer to FUN_009d5c88 for
// deallocation.
//
// Logic:
//   if (p == NULL) return;
//   header = (char *)p - 8;
//   if (*(unsigned int *)header != 0xdddd) return;
//   FUN_009d5c88(header);
//
// Calling convention: __cdecl (bare RET, caller cleans args).
// FUN_009d5c88 is __cdecl; the one pushed arg is cleaned by the caller
// via POP ECX (MSVC 2005 idiom: cheaper than ADD ESP,4).
// Both early-exit jumps bypass the PUSH/CALL/POP block and land at RET.
//
// Asm (27 bytes):
//   8b 44 24 04              MOV  EAX, [ESP+4]
//   85 c0                    TEST EAX, EAX
//   74 12                    JZ   +0x12  →  RET
//   83 e8 08                 SUB  EAX, 0x8
//   81 38 dd dd 00 00        CMP  dword ptr [EAX], 0xdddd
//   75 07                    JNZ  +0x7   →  RET
//   50                       PUSH EAX
//   e8 RR RR RR RR           CALL FUN_009d5c88  (rel32 reloc)
//   59                       POP  ECX         ← arg cleanup
//   c3                       RET

extern "C" void FUN_009d5c88(void *);

extern "C" __declspec(naked) void FUN_009da186()
{
    __asm {
        mov  eax, dword ptr [esp + 4]
        test eax, eax
        jz   end
        sub  eax, 8
        cmp  dword ptr [eax], 0xdddd
        jnz  end
        push eax
        call FUN_009d5c88
        pop  ecx
    end:
        ret
    }
}
