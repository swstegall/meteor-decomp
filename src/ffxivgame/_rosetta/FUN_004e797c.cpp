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
// FUNCTION: ffxivgame 0x004e797c — SEH-frame epilogue with conditional
//                                   flag-OR dispatch (70 B / 0x46)
//
// This is the tail of a __thiscall (or __stdcall) SEH-protected function.
// ESI = this, EDI = flags argument (single stack arg; callee cleans via RET 4).
// Returns ESI (this) in EAX.
//
// Body:
//   ECX = *(ESI) + *(*(ESI) + 4) + ESI   ; vtable-relative sub-object ptr
//   if (EDI != 0) {
//       EAX = ECX[8] | EDI;
//       if (ECX[0x28] == 0) EAX |= 4;
//       FUN_004e5130(EAX, 0);             ; callee-cleans 8 bytes
//   }
//   // SEH epilogue: uninstall frame, destruct local at [EBP-0x20], return this
//   [EBP-4]  = -1;                        ; mark SEH scope as done
//   FUN_004e6620(&local_at_ebp_minus_0x20);
//   FS:[0]   = [EBP-0xc];                 ; restore saved SEH chain head
//
// Calling convention: __stdcall / 1 stack arg (EDI), caller is the compiler's
// SEH dispatcher. RET 4 pops the arg from the caller's frame.
//
// Reloc-bearing sites:
//   off 0x1d  CALL rel32 → FUN_004e5130
//   off 0x2c  CALL rel32 → FUN_004e6620

extern "C" void FUN_004e5130();
extern "C" void FUN_004e6620();

extern "C" __declspec(naked) void FUN_004e797c() {
    __asm {
        mov     ecx, dword ptr [esi]
        mov     ecx, dword ptr [ecx + 4]
        add     ecx, esi
        test    edi, edi
        jz      epilogue
        mov     eax, dword ptr [ecx + 8]
        or      eax, edi
        cmp     dword ptr [ecx + 0x28], 0
        jnz     do_call
        or      eax, 4
    do_call:
        push    0
        push    eax
        call    FUN_004e5130
    epilogue:
        mov     dword ptr [ebp - 4], 0xffffffff
        lea     ecx, [ebp - 0x20]
        call    FUN_004e6620
        mov     eax, esi
        mov     ecx, dword ptr [ebp - 0xc]
        mov     dword ptr fs:[0], ecx
        pop     ecx
        pop     edi
        pop     esi
        pop     ebx
        mov     esp, ebp
        pop     ebp
        ret     4
    }
}
