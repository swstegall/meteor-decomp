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
// FUNCTION: ffxivgame 0x0045a0c0 — thin wrapper around FUN_00459f00 (base64
//                                   encode / string-assign) that returns its
//                                   first argument (38 B / 0x26, __cdecl,
//                                   3 args, no /GS frame).
//
// Signature (reconstructed from disassembly at orig RVA 0x0005a0c0):
//
//   __cdecl void* FUN_0045a0c0(void* out, const void* in, int len);
//
// Behaviour:
//   Calls FUN_00459f00(out, in, len) then returns out regardless of the
//   callee's return value.
//
// Frame:
//   No EBP frame (/Oy).  One callee-save: ESI holds `out`.
//   Initial PUSH ECX / final POP ECX are the MSVC 2005 idiom for allocating
//   a single DWORD on the stack; the slot (at [ESP+0x10] after the three
//   argument pushes) is zeroed via MOV [ESP+0x10],0 immediately before the
//   CALL.  The callee cleanup is ADD ESP,0xc (3 × DWORD = the three pushed
//   args); the PUSH ECX slot is reclaimed by POP ECX in the epilogue.
//
// Reconstruction strategy — naked-asm passthrough:
//   The exact instruction scheduling (zero-of-local deferred past the three
//   argument pushes) is a compiler artefact that cannot be reproduced
//   reliably from a high-level source rewrite.  The only linker relocation
//   is the REL32 offset of the CALL to FUN_00459f00 at +0x19; compare.py
//   masks that field.
//
// Asm (38 bytes @ orig RVA 0x0005a0c0):
//   51                       PUSH ECX
//   8b 44 24 10              MOV  EAX, dword ptr [ESP+0x10]  ; arg3 (len)
//   8b 4c 24 0c              MOV  ECX, dword ptr [ESP+0x0c]  ; arg2 (in)
//   56                       PUSH ESI
//   8b 74 24 0c              MOV  ESI, dword ptr [ESP+0x0c]  ; arg1 (out)
//   50                       PUSH EAX                        ; push len
//   51                       PUSH ECX                        ; push in
//   56                       PUSH ESI                        ; push out
//   c7 44 24 10 00 00 00 00  MOV  dword ptr [ESP+0x10],0x0   ; zero ECX slot
//   e8 RR RR RR RR           CALL FUN_00459f00               ; (reloc)
//   83 c4 0c                 ADD  ESP,0xc
//   8b c6                    MOV  EAX,ESI                    ; return out
//   5e                       POP  ESI
//   59                       POP  ECX
//   c3                       RET

extern "C" void FUN_00459f00();   // forward declaration for the CALL relocation

extern "C" __declspec(naked) void __cdecl FUN_0045a0c0(void *, const void *, int) {
    __asm {
        // 0005a0c0: 51              PUSH ECX
        push ecx
        // 0005a0c1: 8b 44 24 10    MOV EAX, [ESP+0x10]
        mov  eax, dword ptr [esp + 0x10]
        // 0005a0c5: 8b 4c 24 0c    MOV ECX, [ESP+0x0c]
        mov  ecx, dword ptr [esp + 0x0c]
        // 0005a0c9: 56              PUSH ESI
        push esi
        // 0005a0ca: 8b 74 24 0c    MOV ESI, [ESP+0x0c]
        mov  esi, dword ptr [esp + 0x0c]
        // 0005a0ce: 50              PUSH EAX
        push eax
        // 0005a0cf: 51              PUSH ECX
        push ecx
        // 0005a0d0: 56              PUSH ESI
        push esi
        // 0005a0d1: c7 44 24 10 00 00 00 00   MOV [ESP+0x10],0
        mov  dword ptr [esp + 0x10], 0
        // 0005a0d9: e8 RR RR RR RR  CALL FUN_00459f00  (reloc)
        call FUN_00459f00
        // 0005a0de: 83 c4 0c        ADD ESP,0xc
        add  esp, 0x0c
        // 0005a0e1: 8b c6           MOV EAX,ESI
        mov  eax, esi
        // 0005a0e3: 5e              POP ESI
        pop  esi
        // 0005a0e4: 59              POP ECX
        pop  ecx
        // 0005a0e5: c3              RET
        ret
    }
}
