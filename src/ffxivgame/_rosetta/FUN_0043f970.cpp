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
// FUNCTION: ffxivgame 0x0003f970 — iterate range of 0x1C-byte objects,
//                                   calling FUN_0043f4b0(arg3, 0, -1) on each
//                                   (__cdecl, 3 args, void return, 42 B / 0x2A)
//
// void FUN_0043f970(char *begin, char *end, SomeType arg3)
//
// Iterates [begin, end) over 0x1C-byte-stride objects, calling FUN_0043f4b0
// (__thiscall, ECX = element) with fixed args (arg3, 0, -1) on each element.
// The EBX callee-save is deferred past the empty-range early exit — MSVC 2005
// /O2 deferred-save idiom: EBX only pushed when the loop body will execute.
//
// Asm (42 bytes @ orig RVA 0x0003f970):
//   56                   PUSH ESI
//   8b 74 24 08          MOV  ESI, dword ptr [ESP+0x8]    ; begin (arg1)
//   57                   PUSH EDI
//   8b 7c 24 10          MOV  EDI, dword ptr [ESP+0x10]   ; end   (arg2)
//   3b f7                CMP  ESI, EDI
//   74 19                JZ   exit
//   53                   PUSH EBX                         ; deferred save
//   8b 5c 24 18          MOV  EBX, dword ptr [ESP+0x18]   ; arg3
// loop:
//   6a ff                PUSH -0x1
//   6a 00                PUSH 0x0
//   53                   PUSH EBX
//   8b ce                MOV  ECX, ESI
//   e8 21 fb ff ff       CALL FUN_0043f4b0                ; rel32 reloc
//   83 c6 1c             ADD  ESI, 0x1C
//   3b f7                CMP  ESI, EDI
//   75 ed                JNZ  loop
//   5b                   POP  EBX
// exit:
//   5f                   POP  EDI
//   5e                   POP  ESI
//   c3                   RET
//
// The CALL rel32 reloc window (+0x1a..+0x1d) is masked by tools/compare.py.

extern "C" void FUN_0043f4b0();

extern "C" __declspec(naked) void FUN_0043f970() {
    __asm {
        // 0003f970: 56
        push    esi
        // 0003f971: 8b 74 24 08
        mov     esi, dword ptr [esp+0x8]
        // 0003f975: 57
        push    edi
        // 0003f976: 8b 7c 24 10
        mov     edi, dword ptr [esp+0x10]
        // 0003f97a: 3b f7
        cmp     esi, edi
        // 0003f97c: 74 19
        jz      exit_fn
        // 0003f97e: 53
        push    ebx
        // 0003f97f: 8b 5c 24 18
        mov     ebx, dword ptr [esp+0x18]
    iter:
        // 0003f983: 6a ff
        push    -1
        // 0003f985: 6a 00
        push    0
        // 0003f987: 53
        push    ebx
        // 0003f988: 8b ce
        mov     ecx, esi
        // 0003f98a: e8 21 fb ff ff
        call    FUN_0043f4b0
        // 0003f98f: 83 c6 1c
        add     esi, 0x1c
        // 0003f992: 3b f7
        cmp     esi, edi
        // 0003f994: 75 ed
        jnz     iter
        // 0003f996: 5b
        pop     ebx
    exit_fn:
        // 0003f997: 5f
        pop     edi
        // 0003f998: 5e
        pop     esi
        // 0003f999: c3
        ret
    }
}
