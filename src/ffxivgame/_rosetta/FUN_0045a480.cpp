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
// FUNCTION: ffxivgame 0x0005a480 — thin forwarder to FUN_0045a1d0, returns param1
//                                   (__cdecl, 3 args, 38 B / 0x26)
//
// Calls FUN_0045a1d0(param1, param2, param3) and returns param1 unchanged.
//
// Frame notes:
//   PUSH ECX at entry is MSVC /O2's "sub esp,4" substitution — allocates a
//   4-byte local slot on the stack rather than issuing a real SUB.  The slot
//   is zeroed by MOV [ESP+0x10], 0 just before the CALL and then consumed by
//   POP ECX in the epilogue.  No frame pointer (/Oy).
//
// Calling convention: __cdecl — plain RET (no RET N).
//
// Asm (38 bytes @ orig RVA 0x0005a480):
//   51                       PUSH ECX              ; allocate local slot
//   8b 44 24 10              MOV EAX,[ESP+0x10]    ; arg3
//   8b 4c 24 0c              MOV ECX,[ESP+0x0c]    ; arg2
//   56                       PUSH ESI              ; callee-save ESI
//   8b 74 24 0c              MOV ESI,[ESP+0x0c]    ; arg1
//   50                       PUSH EAX              ; callee arg3
//   51                       PUSH ECX              ; callee arg2
//   56                       PUSH ESI              ; callee arg1
//   c7 44 24 10 00 00 00 00  MOV [ESP+0x10],0x0    ; zero local slot
//   e8 RR RR RR RR           CALL FUN_0045a1d0     ; (reloc)
//   83 c4 0c                 ADD ESP,0xc
//   8b c6                    MOV EAX,ESI           ; return param1
//   5e                       POP ESI
//   59                       POP ECX               ; pop local slot
//   c3                       RET

extern "C" void FUN_0045a1d0();   // forward declaration for the CALL relocation

extern "C" __declspec(naked) void * __cdecl FUN_0045a480(void *, void *, int) {
    __asm {
        // 0005a480: 51
        push ecx
        // 0005a481: 8b 44 24 10
        mov eax, dword ptr [esp+0x10]
        // 0005a485: 8b 4c 24 0c
        mov ecx, dword ptr [esp+0x0c]
        // 0005a489: 56
        push esi
        // 0005a48a: 8b 74 24 0c
        mov esi, dword ptr [esp+0x0c]
        // 0005a48e: 50
        push eax
        // 0005a48f: 51
        push ecx
        // 0005a490: 56
        push esi
        // 0005a491: c7 44 24 10 00 00 00 00
        mov dword ptr [esp+0x10], 0
        // 0005a499: e8 RR RR RR RR  (reloc)
        call FUN_0045a1d0
        // 0005a49e: 83 c4 0c
        add esp, 0x0c
        // 0005a4a1: 8b c6
        mov eax, esi
        // 0005a4a3: 5e
        pop esi
        // 0005a4a4: 59
        pop ecx
        // 0005a4a5: c3
        ret
    }
}
