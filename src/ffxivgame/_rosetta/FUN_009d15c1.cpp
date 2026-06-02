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
// FUNCTION: ffxivgame 0x009d15c1 — __cdecl reference-count decrement
//                                  and linked-list cleanup
//                                  (RVA 0x005d15c1, 62 bytes / 0x3e)
//
// Call shape:
//   void FUN_009d15c1(SomeStruct *p)    // __cdecl, one stack arg
//     if (p->field_0x4 != 0) {
//         --*(byte *)(p->field_0x4 + 0x1363DA4);
//         if (*(byte *)(p->field_0x4 + 0x1363DA4) > 0)
//             return;
//     }
//     FUN_009d152b(p);
//     q = (SomeOtherStruct *)(p->field_0x24);
//     if (q != nullptr) {
//         q->SomeMethod();   // FUN_004e3580 via __thiscall (ECX = q)
//         FUN_009d1b17(q);
//     }
//
// Calling convention: __cdecl (plain RET; caller-cleanup pushes for all
// three sibling calls visible via POP ECX after each).
// Frame: no frame pointer — only ESI is saved/restored.
//
// Asm (62 bytes including the shared RET at 0x9d15fe):
//   56                       PUSH ESI
//   8b 74 24 08              MOV  ESI, [ESP+8]            ; p
//   83 7e 04 00              CMP  dword ptr [ESI+4], 0
//   76 15                    JBE  short loc_e1            ; branch if 0
//   8b 46 04                 MOV  EAX, [ESI+4]
//   fe 88 a4 3d 36 01        DEC  byte ptr [EAX+0x1363da4]
//   8b 46 04                 MOV  EAX, [ESI+4]
//   80 b8 a4 3d 36 01 00     CMP  byte ptr [EAX+0x1363da4], 0
//   7f 1c                    JG   short loc_fd            ; refcount > 0
// loc_e1:
//   56                       PUSH ESI
//   e8 RR RR RR RR           CALL FUN_009d152b
//   8b 76 24                 MOV  ESI, [ESI+0x24]
//   85 f6                    TEST ESI, ESI
//   59                       POP  ECX
//   74 0e                    JE   short loc_fd
//   8b ce                    MOV  ECX, ESI
//   e8 RR RR RR RR           CALL FUN_004e3580
//   56                       PUSH ESI
//   e8 RR RR RR RR           CALL FUN_009d1b17
//   59                       POP  ECX
// loc_fd:
//   5e                       POP  ESI
//   c3                       RET

extern "C" void FUN_009d152b();
extern "C" void FUN_004e3580();
extern "C" void FUN_009d1b17();

extern "C" __declspec(naked) void FUN_009d15c1() {
    __asm {
        push    esi
        mov     esi, dword ptr [esp+8]
        cmp     dword ptr [esi+4], 0
        jbe     loc_e1
        mov     eax, dword ptr [esi+4]
        dec     byte ptr [eax+0x1363DA4]
        mov     eax, dword ptr [esi+4]
        cmp     byte ptr [eax+0x1363DA4], 0
        jg      loc_fd
    loc_e1:
        push    esi
        call    FUN_009d152b
        mov     esi, dword ptr [esi+0x24]
        test    esi, esi
        pop     ecx
        je      loc_fd
        mov     ecx, esi
        call    FUN_004e3580
        push    esi
        call    FUN_009d1b17
        pop     ecx
    loc_fd:
        pop     esi
        ret
    }
}
