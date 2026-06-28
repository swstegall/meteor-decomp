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
// FUNCTION: ffxivgame 0x000429d0 — FUN_004429d0 (205 B / 0xcd)
//
// Calling convention: __thiscall (ECX = this), 1 stack arg, RET 4.
//
// Frame: MSVC 2005 EH3-style SEH prolog with /GS cookie.
//   No __security_check_cookie at exit (no local buffer ≥5 bytes;
//   cookie is present only because of the EH3 frame format).
//
// Logic:
//   EDI = this
//   if (this->field_1c == 0) {
//       FUN_004432c0(&this->field_8, arg0);
//       return;
//   }
//   // else branch
//   local3 = FUN_00b8f6f0(&local3, this->field_1c, 0,
//                          arg0->field_c, CDQ(arg0->field_c), 1);
//   arg0->field_20->FUN_00443860(ret_b8f6f0);   // EH state → 0 around this
//   FUN_00b527c0(&local3);                        // destruct local3
//   arg0->field_20->FUN_00443a10(arg0->field_8,
//                                arg0->field_0_f,
//                                &arg0->field_10,
//                                0,
//                                arg0->field_4_f);
//   this->field_20 = FUN_009d5725(0);
//
// Reloc-bearing sites (masked by compare.py):
//   +0x03  PUSH offset g_scope_table_4429d0   (DIR32 → .rdata 0x00e57216)
//   +0x09  MOV EAX, __security_cookie         (DIR32 → .data  0x012ea8b0)
//   +0x36  CALL FUN_004432c0                  (REL32 → .text  0x004432c0)
//   +0x63  CALL FUN_00b8f6f0                  (REL32 → .text  0x00b8f6f0)
//   +0x77  CALL FUN_00443860                  (REL32 → .text  0x00443860)
//   +0x88  CALL FUN_00b527c0                  (REL32 → .text  0x00b527c0)
//   +0xa7  CALL FUN_00443a10                  (REL32 → .text  0x00443a10)
//   +0xae  CALL FUN_009d5725                  (REL32 → .text  0x009d5725)

extern "C" {
    extern unsigned __security_cookie;          // .data 0x012ea8b0
    extern int g_scope_table_4429d0;            // .rdata 0x00e57216
    void FUN_004432c0();                        // short-path: init from arg
    void FUN_00b8f6f0();                        // 6-arg int64-like ctor
    void FUN_00443860();                        // thiscall, 1 stack arg, RET 4
    void FUN_00b527c0();                        // thiscall destructor, RET 0
    void FUN_00443a10();                        // thiscall, 5 stack args, RET 0x14
    void FUN_009d5725();                        // cdecl, 1 arg
}

extern "C" __declspec(naked) void FUN_004429d0() {
    __asm {
        // --- EH3 / /GS prolog -------------------------------------------
        push    -1
        push    offset g_scope_table_4429d0
        mov     eax, fs:[0]
        push    eax
        sub     esp, 0x10
        push    esi
        push    edi
        mov     eax, __security_cookie
        xor     eax, esp
        push    eax
        lea     eax, [esp + 0x1c]
        mov     fs:[0], eax

        // --- branch on this->field_1c ------------------------------------
        mov     edi, ecx
        mov     ecx, [edi + 0x1c]
        test    ecx, ecx
        jnz     short else_branch

        // --- short path: field_1c == 0 -----------------------------------
        mov     eax, [esp + 0x2c]
        push    eax
        lea     ecx, [edi + 0x8]
        call    FUN_004432c0

        mov     ecx, [esp + 0x1c]
        mov     fs:[0], ecx
        pop     ecx
        pop     edi
        pop     esi
        add     esp, 0x1c
        ret     4

        // --- else path: field_1c != 0 ------------------------------------
    else_branch:
        mov     esi, [esp + 0x2c]
        mov     eax, [esi + 0x0c]
        push    0x1
        cdq
        push    edx
        push    eax
        push    0x0
        push    ecx
        lea     ecx, [esp + 0x20]
        push    ecx
        call    FUN_00b8f6f0
        add     esp, 0x18

        mov     ecx, [esi + 0x20]
        push    eax
        mov     dword ptr [esp + 0x28], 0x0
        call    FUN_00443860

        lea     ecx, [esp + 0x0c]
        mov     dword ptr [esp + 0x24], 0xffffffff
        call    FUN_00b527c0

        fld     dword ptr [esi + 0x4]
        mov     eax, [esi + 0x8]
        push    ecx
        fstp    dword ptr [esp]
        push    0x0
        fld     dword ptr [esi]
        lea     edx, [esi + 0x10]
        push    edx
        push    ecx
        mov     ecx, [esi + 0x20]
        fstp    dword ptr [esp]
        push    eax
        call    FUN_00443a10

        push    0x0
        call    FUN_009d5725
        add     esp, 0x4
        mov     [edi + 0x20], eax

        mov     ecx, [esp + 0x1c]
        mov     fs:[0], ecx
        pop     ecx
        pop     edi
        pop     esi
        add     esp, 0x1c
        ret     4
    }
}
