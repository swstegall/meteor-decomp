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
// FUNCTION: ffxivgame 0x0001a480 — `__cdecl` 3-arg byte-fill loop (30 B).
//
// Fills the range [begin, end) with the byte value *src.
// If begin == end the function returns immediately without pushing ESI,
// so the PUSH/POP ESI pair is nested inside the taken-branch only.
//
// Asm shape (read from the orig PE at RVA 0x0001a480 — 30 bytes,
// no relocations, no IAT touches, no SEH):
//
//   0001a480:  8b 44 24 04          MOV  EAX, [ESP+0x4]    ; begin (dst)
//   0001a484:  8b 4c 24 08          MOV  ECX, [ESP+0x8]    ; end
//   0001a488:  3b c1                CMP  EAX, ECX
//   0001a48a:  74 11                JZ   done              ; begin == end → skip
//   0001a48c:  56                   PUSH ESI
//   0001a48d:  8b 74 24 10          MOV  ESI, [ESP+0x10]   ; src (after PUSH, +4)
//   0001a491:  8a 16                MOV  DL, [ESI]         ; DL = *src
//   0001a493:  88 10                MOV  [EAX], DL         ; *dst = DL
//   0001a495:  83 c0 01             ADD  EAX, 0x1          ; dst++
//   0001a498:  3b c1                CMP  EAX, ECX          ; dst == end?
//   0001a49a:  75 f5                JNZ  loop_body         ; no → repeat
//   0001a49c:  5e                   POP  ESI
//   0001a49d:  c3                   RET
//
// No relocations — direct asm mnemonics (same strategy as FUN_00403c40).

extern "C" __declspec(naked) void FUN_0041a480() {
    __asm {
        mov     eax, dword ptr [esp+0x4]        // 8b 44 24 04  (begin/dst)
        mov     ecx, dword ptr [esp+0x8]        // 8b 4c 24 08  (end)
        cmp     eax, ecx                        // 3b c1
        jz      done                            // 74 11

        push    esi                             // 56
        mov     esi, dword ptr [esp+0x10]       // 8b 74 24 10  (src; +4 from push)
    loop_body:
        mov     dl, byte ptr [esi]              // 8a 16
        mov     byte ptr [eax], dl              // 88 10
        add     eax, 0x1                        // 83 c0 01
        cmp     eax, ecx                        // 3b c1
        jnz     loop_body                       // 75 f5
        pop     esi                             // 5e
    done:
        ret                                     // c3
    }
}
