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
// FUNCTION: ffxivgame 0x00061300 — public __cdecl entry point that bridges
//                                  stack arguments to the register-threaded
//                                  dispatch core FUN_00461050 (23 B / 0x17).
//
// Calling convention: __cdecl (bare RET, 2 stack args: arg1 = node ptr,
//                              arg2 = ctx / event struct).
//
// FUN_00461050 uses a register-threading calling convention: it expects
// its two "structural" parameters in EDI (node ptr) and ESI (ctx) rather
// than on the stack. This wrapper bridges the gap:
//
//   1. Save ESI, EDI (callee-saved in the MSVC 2005 ABI).
//   2. Load arg2 → ESI, arg1 → EDI (re-using the same [ESP+0xc] offset
//      relative to the current top-of-stack, which shifts by 4 each time
//      a register is pushed).
//   3. Push 0 as the "free-node flag" ([esp+0x14] inside the callee once
//      its 8-byte local frame is allocated via the alloca probe).
//   4. CALL FUN_00461050 (with ESI/EDI already holding the live ptrs).
//   5. Clean up the one pushed arg (ADD ESP,4), restore EDI/ESI, RET.
//
// Asm (23 bytes @ orig RVA 0x00061300):
//   56                  PUSH ESI
//   8b 74 24 0c         MOV  ESI, dword ptr [ESP+0xc]  ; ESI = arg2
//   57                  PUSH EDI
//   8b 7c 24 0c         MOV  EDI, dword ptr [ESP+0xc]  ; EDI = arg1
//   6a 00               PUSH 0x0                        ; flag = 0
//   e8 3f fd ff ff      CALL FUN_00461050               ; rel32 reloc
//   83 c4 04            ADD  ESP, 0x4                   ; cdecl cleanup
//   5f                  POP  EDI
//   5e                  POP  ESI
//   c3                  RET

extern "C" void FUN_00461050();

extern "C" __declspec(naked) void FUN_00461300() {
    __asm {
        push    esi
        mov     esi, dword ptr [esp + 0xc]
        push    edi
        mov     edi, dword ptr [esp + 0xc]
        push    0
        call    FUN_00461050
        add     esp, 4
        pop     edi
        pop     esi
        ret
    }
}
