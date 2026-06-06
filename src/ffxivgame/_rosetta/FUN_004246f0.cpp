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
// FUNCTION: ffxivgame 0x000246f0 — global linked-list virtual broadcast
//                                  (__cdecl, 1 arg, 36 B / 0x24)
//
// void FUN_004246f0(void *param_1)
//
// Walks the singly-linked list whose head is the global pointer at
// 0x01329958. For each node it loads the node's vtable (*node), dispatches
// the __thiscall method at vtable+0x20 (slot 8) with this=node and the one
// stack argument param_1, then advances to node->next at offset +4. The
// loop terminates when the next pointer is null.
//
// MSVC 2005 /O2 /Oy register allocation: ESI holds the current node and is
// loaded + null-tested *before* the EDI callee-save is pushed — the param_1
// stack load into EDI is deferred past the JZ so an empty list skips the
// PUSH EDI / load entirely. This deferred-save shape (PUSH ESI at entry,
// PUSH EDI only on the non-empty path, with the JZ landing on the lone
// POP ESI / RET tail) is fragile to reproduce from source-level C++, so the
// function is encoded as __declspec(naked) emitting the asm verbatim.
//
// The only relocated field is the 4-byte absolute address of the list head
// (MOV ESI, [0x01329958]); referencing the extern below makes the assembler
// emit the DIR32 fixup that compare.py masks.
//
// Asm (36 bytes @ orig RVA 0x000246f0):
//   56              PUSH ESI
//   8b 35 <head>    MOV ESI, dword ptr [g_head]      ; head (reloc)
//   85 f6           TEST ESI, ESI
//   74 17           JZ short  → POP ESI / RET
//   57              PUSH EDI
//   8b 7c 24 0c     MOV EDI, dword ptr [ESP+0xc]     ; param_1
// loop:
//   8b 06           MOV EAX, dword ptr [ESI]         ; node vtable
//   8b 50 20        MOV EDX, dword ptr [EAX+0x20]    ; vtable[8]
//   57              PUSH EDI                          ; arg = param_1
//   8b ce           MOV ECX, ESI                      ; this = node
//   ff d2           CALL EDX
//   8b 76 04        MOV ESI, dword ptr [ESI+0x4]     ; node = node->next
//   85 f6           TEST ESI, ESI
//   75 ef           JNZ short → loop
//   5f              POP EDI
//   5e              POP ESI
//   c3              RET

extern "C" void *DAT_01329958;   // [0x01329958] list head (DIR32 reloc)

extern "C" __declspec(naked) void __cdecl FUN_004246f0(void * /*param_1*/)
{
    __asm {
        // 000246f0: 56            PUSH ESI
        _emit 0x56
        // 000246f1: 8b 35 RR RR RR RR  MOV ESI, [DAT_01329958]  (reloc)
        mov esi, dword ptr [DAT_01329958]
        // 000246f7: 85 f6         TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 000246f9: 74 17         JZ short → POP ESI
        _emit 0x74
        _emit 0x17
        // 000246fb: 57            PUSH EDI
        _emit 0x57
        // 000246fc: 8b 7c 24 0c   MOV EDI, [ESP+0xc]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 00024700: 8b 06         MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 00024702: 8b 50 20      MOV EDX, [EAX+0x20]
        _emit 0x8b
        _emit 0x50
        _emit 0x20
        // 00024705: 57            PUSH EDI
        _emit 0x57
        // 00024706: 8b ce         MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00024708: ff d2         CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0002470a: 8b 76 04      MOV ESI, [ESI+0x4]
        _emit 0x8b
        _emit 0x76
        _emit 0x04
        // 0002470d: 85 f6         TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0002470f: 75 ef         JNZ short → 0x00024700
        _emit 0x75
        _emit 0xef
        // 00024711: 5f            POP EDI
        _emit 0x5f
        // 00024712: 5e            POP ESI
        _emit 0x5e
        // 00024713: c3            RET
        _emit 0xc3
    }
}
