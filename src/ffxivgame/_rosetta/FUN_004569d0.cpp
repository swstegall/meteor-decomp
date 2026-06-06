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
// FUNCTION: ffxivgame 0x004569d0 — unknown (__cdecl void, 178 B / 0xb2)
//
// EH3/GS-guarded function. Takes one pointer argument (ECX = arg1 is
// loaded from [ESP+0x2c] and passed as `this` to the thiscall at
// 0x457500). Allocates a 20-byte local struct on the stack, zeroes it,
// calls FUN_00457500 to populate it, then checks a global at 0x0126701c:
//
//   - if global == -1: call FUN_00457270 with ECX=0x0132d0e0 and zero
//     two word fields at offsets +0x88/+0x8a of the returned pointer.
//   - else: push global, call via IAT slot 0xf3e2a4, write -1 to *EAX.
//
// Then iterates while local[+0x18] != 0 (loop body calls FUN_00456ef0 to
// retrieve an element, pushes result, calls __cdecl FUN_00456060, cleans
// up one arg). After the loop, calls FUN_00cc3b80 as a dtor on the local
// struct, restores the EH/GS frame, and returns.
//
// Naked-asm: the EH3 prolog (PUSH -1 / PUSH scope_table / PUSH FS:[0] /
// SUB ESP,0x14 / PUSH ESI / cookie XOR ESP / LEA FS:[0]) plus the
// mid-body EH-state write (MOV [ESP+0x28],ESI while ESP has been pushed
// one extra time) produce a shape that is impractical to coax out of
// plain C++ under /O2 /GS.
//
// Reloc-bearing sites (offsets within the 178-byte function body):
//   +0x03  PUSH scope_table (0x00e585c8)
//   +0x0d  MOV EAX, __security_cookie (0x012ea8b0)
//   +0x14  CALL FUN_00457500 rel32
//   +0x19  MOV EAX, [data_0126701c] (0x0126701c)
//   +0x23  MOV ECX, OFFSET data_0132d0e0 (0x0132d0e0)
//   +0x28  CALL FUN_00457270 rel32
//   +0x3e  CALL dword ptr [ext_f3e2a4] (0x00f3e2a4)
//   +0x4f  CALL FUN_00456ef0 rel32
//   +0x55  CALL FUN_00456060 rel32
//   +0x67  CALL FUN_00cc3b80 rel32

extern "C" {
    // .data — security cookie shared across the TU.
    extern unsigned __security_cookie;

    // .rdata — MSVC EH3 scope table / exception handler for this fn.
    extern int g_scope_table_004569d0;

    // .data — global checked against -1 (0x0126701c).
    extern int data_0126701c;

    // .data — global object whose address is loaded into ECX (0x0132d0e0).
    extern int data_0132d0e0;

    // .idata — IAT slot for the indirect call at 0x00f3e2a4.
    extern int ext_f3e2a4;

    // .text — internal call targets.
    int FUN_00457500();   // __thiscall, 1 stack arg, RET 4
    int FUN_00457270();   // __thiscall, no stack args
    int FUN_00456ef0();   // __thiscall, no stack args
    int FUN_00456060();   // __cdecl,    1 stack arg
    int FUN_00cc3b80();   // __thiscall, no stack args (dtor)
}

extern "C" __declspec(naked) void FUN_004569d0() {
    __asm {
        // --- EH3/GS prolog -------------------------------------------
        push    -1                              // 6a ff
        push    offset g_scope_table_004569d0  // 68 ?? ?? ?? ?? (reloc)
        mov     eax, fs:[0]                    // 64 a1 00 00 00 00
        push    eax                            // 50
        sub     esp, 0x14                      // 83 ec 14
        push    esi                            // 56
        mov     eax, __security_cookie         // a1 ?? ?? ?? ?? (reloc)
        xor     eax, esp                       // 33 c4
        push    eax                            // 50
        lea     eax, [esp + 0x1c]             // 8d 44 24 1c
        mov     fs:[0], eax                   // 64 a3 00 00 00 00

        // --- zero locals; ESI = 0 ------------------------------------
        xor     esi, esi                       // 33 f6
        mov     [esp + 0x0c], esi             // 89 74 24 0c
        mov     [esp + 0x10], esi             // 89 74 24 10
        mov     [esp + 0x14], esi             // 89 74 24 14
        mov     [esp + 0x18], esi             // 89 74 24 18

        // --- call FUN_00457500(this=arg1, &local_struct) --------------
        mov     ecx, [esp + 0x2c]             // 8b 4c 24 2c  (arg1)
        lea     eax, [esp + 0x08]             // 8d 44 24 08
        push    eax                            // 50
        mov     [esp + 0x28], esi             // 89 74 24 28  (EH state -> 0)
        call    FUN_00457500                   // e8 ?? ?? ?? ?? (reloc)

        // --- check global at 0x0126701c ------------------------------
        mov     eax, data_0126701c             // a1 ?? ?? ?? ?? (reloc)
        cmp     eax, -0x1                      // 83 f8 ff
        jnz     short branch_else              // 75 1a

        // --- branch: global == -1 ------------------------------------
        mov     ecx, offset data_0132d0e0      // b9 ?? ?? ?? ?? (reloc)
        call    FUN_00457270                   // e8 ?? ?? ?? ?? (reloc)
        mov     word ptr [eax + 0x88], si      // 66 89 b0 88 00 00 00
        mov     word ptr [eax + 0x8a], si      // 66 89 b0 8a 00 00 00
        jmp     short after_branch             // eb 0d

    branch_else:
        // --- branch: global != -1 ------------------------------------
        push    eax                            // 50
        call    dword ptr [ext_f3e2a4]         // ff 15 ?? ?? ?? ?? (reloc)
        mov     dword ptr [eax], 0xffffffff    // c7 00 ff ff ff ff

    after_branch:
        // --- loop while local[+0x18] != 0 ----------------------------
        cmp     dword ptr [esp + 0x18], esi   // 39 74 24 18
        jz      short exit_loop               // 74 19
        nop                                   // 90

    loop_start:
        lea     ecx, [esp + 0x08]             // 8d 4c 24 08
        call    FUN_00456ef0                   // e8 ?? ?? ?? ?? (reloc)
        push    eax                            // 50
        call    FUN_00456060                   // e8 ?? ?? ?? ?? (reloc)
        add     esp, 0x4                       // 83 c4 04
        cmp     dword ptr [esp + 0x18], esi   // 39 74 24 18
        jnz     short loop_start              // 75 e8

    exit_loop:
        // --- dtor on local struct ------------------------------------
        lea     ecx, [esp + 0x08]             // 8d 4c 24 08
        call    FUN_00cc3b80                   // e8 ?? ?? ?? ?? (reloc)

        // --- EH/GS epilog --------------------------------------------
        mov     ecx, [esp + 0x1c]             // 8b 4c 24 1c
        mov     fs:[0], ecx                   // 64 89 0d 00 00 00 00
        pop     ecx                            // 59
        pop     esi                            // 5e
        add     esp, 0x20                      // 83 c4 20
        ret                                    // c3
    }
}
