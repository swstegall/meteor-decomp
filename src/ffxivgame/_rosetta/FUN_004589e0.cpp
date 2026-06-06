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
// FUNCTION: ffxivgame 0x004589e0 — __thiscall reset/reinit helper
//                                  (74 B / 0x4a, 2 relocs)
//
// __thiscall int FUN_004589e0(this, int param1, int param2)
//   ECX = this, 2 stack args, callee-cleans 8 bytes via `ret 0x8`.
//
// Pseudo-source (logical structure):
//
//   int FUN_004589e0(SomeObj *this, int param1, int param2) {
//       if (this->field0) {
//           FUN_0045b8b0(this->field0);   // release old handle (__cdecl, 1 arg)
//           this->field0 = 0;
//       }
//       this->field0 = FUN_0045b890(0, &param1, param2);  // reinit (__cdecl, 3 args)
//       return this->field0 ? 0 : 0x28a4;   // 0 = success, 0x28a4 = failure
//   }
//
// The NEG/SBB/AND/ADD epilogue is MSVC 2005's idiom for:
//   result != 0  →  return 0       (success)
//   result == 0  →  return 0x28a4  (error code)
//
// The unusual stack trick at offset +0x21: after PUSH ECX (param2),
// LEA EDX, [ESP+0x8] captures the address of the saved-ECX stack slot
// (the one that held `this` on entry). After PUSH EDX and PUSH 0, the
// sequence MOV [ESP+0x10], EAX writes param1 into that same slot,
// turning EDX into a pointer-to-param1 on the stack. MSVC 2005 /O2
// reuses the prologue-saved ECX slot as storage for param1 to avoid an
// extra SUB ESP / local allocation.
//
// Reloc-bearing sites (4-byte rel32 windows, wildcarded by compare.py):
//   +0x0b  CALL FUN_0045b8b0   (rel32)
//   +0x2d  CALL FUN_0045b890   (rel32)

extern "C" {
    int FUN_0045b8b0();   // release/free: __cdecl, 1 arg, returns void (EAX ignored)
    int FUN_0045b890();   // reinit/create: __cdecl, 3 args, returns handle in EAX
}

extern "C" __declspec(naked) void FUN_004589e0() {
    __asm {
        push    ecx                          // 51  — save ECX (this)
        push    esi                          // 56  — save ESI
        mov     esi, ecx                     // 8b f1  — ESI = this
        mov     eax, dword ptr [esi]         // 8b 06  — EAX = this->field0
        test    eax, eax                     // 85 c0
        jz      skip_free                    // 74 0f  — if null, skip release
        push    eax                          // 50
        call    FUN_0045b8b0                 // e8 ?? ?? ?? ??
        add     esp, 4                       // 83 c4 04
        mov     dword ptr [esi], 0           // c7 06 00 00 00 00
    skip_free:
        mov     ecx, dword ptr [esp + 0x10]  // 8b 4c 24 10  — ECX = param2
        mov     eax, dword ptr [esp + 0xc]   // 8b 44 24 0c  — EAX = param1
        push    ecx                          // 51  — arg3: param2
        lea     edx, [esp + 0x8]             // 8d 54 24 08  — EDX = &saved-ECX slot
        push    edx                          // 52  — arg2: ptr (will point to param1)
        push    0                            // 6a 00  — arg1: 0
        mov     dword ptr [esp + 0x10], eax  // 89 44 24 10  — write param1 into that slot
        call    FUN_0045b890                 // e8 ?? ?? ?? ??
        mov     dword ptr [esi], eax         // 89 06  — this->field0 = result
        add     esp, 0xc                     // 83 c4 0c
        neg     eax                          // f7 d8  — set CF if EAX != 0
        sbb     eax, eax                     // 1b c0  — EAX = 0 or 0xFFFFFFFF
        and     eax, 0xffffd75c              // 25 5c d7 ff ff
        add     eax, 0x28a4                  // 05 a4 28 00 00
        pop     esi                          // 5e
        pop     ecx                          // 59
        ret     8                            // c2 08 00
    }
}
