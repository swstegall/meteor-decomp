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
// FUNCTION: ffxivgame 0x0043b590 — __thiscall thread-safe clear of two
//                                   DWORD fields under an embedded
//                                   CRITICAL_SECTION (32 B / 0x20).
//
// Locks the CRITICAL_SECTION embedded at this+0x24, zeroes the two DWORD
// fields at this+0x1c and this+0x20, then unlocks.  The zeroing happens
// inside the lock so readers see consistent state.
//
// Object layout (offsets touched):
//   [this + 0x1c]  DWORD  field_1c  — zeroed under lock
//   [this + 0x20]  DWORD  field_20  — zeroed under lock
//   [this + 0x24]  CRITICAL_SECTION cs  (24 B embedded)
//
// Calling convention: __thiscall (ECX = this); no stack args; void return.
// Callee-saves pushed/popped: ESI, EDI.
//
// Disassembly (verbatim, RVA 0x0003b590 — 32 bytes):
//
//   0003b590:  56                    PUSH ESI
//   0003b591:  57                    PUSH EDI
//   0003b592:  8b f1                 MOV ESI,ECX
//   0003b594:  8d 7e 24              LEA EDI,[ESI+0x24]
//   0003b597:  57                    PUSH EDI
//   0003b598:  ff 15 6c e1 f3 00     CALL dword ptr [EnterCriticalSection]
//   0003b59e:  33 c0                 XOR EAX,EAX
//   0003b5a0:  57                    PUSH EDI
//   0003b5a1:  89 46 20              MOV dword ptr [ESI+0x20],EAX
//   0003b5a4:  89 46 1c              MOV dword ptr [ESI+0x1c],EAX
//   0003b5a7:  ff 15 68 e1 f3 00     CALL dword ptr [LeaveCriticalSection]
//   0003b5ad:  5f                    POP EDI
//   0003b5ae:  5e                    POP ESI
//   0003b5af:  c3                    RET
//
// Reloc-bearing sites (compare.py masks these 4-byte windows):
//   +0x0a   DIR32 → [0x00f3e16c]  (IAT: kernel32!EnterCriticalSection)
//   +0x19   DIR32 → [0x00f3e168]  (IAT: kernel32!LeaveCriticalSection)
//
// Reconstruction strategy — naked-asm with __declspec(dllimport):
//   __declspec(dllimport) forces the ff 15 [__imp_X] indirect-call
//   encoding (6 bytes) with a DIR32 reloc that compare.py masks.
//   Pattern mirrors FUN_00414640 (InitializeCriticalSection) and
//   FUN_0043bc60 (DeleteCriticalSection).

extern "C" {

__declspec(dllimport) void __stdcall EnterCriticalSection(void *lpCriticalSection);
__declspec(dllimport) void __stdcall LeaveCriticalSection(void *lpCriticalSection);

__declspec(naked) void FUN_0043b590()
{
    __asm {
        // 0003b590: 56
        push    esi
        // 0003b591: 57
        push    edi
        // 0003b592: 8b f1
        mov     esi, ecx
        // 0003b594: 8d 7e 24
        lea     edi, [esi + 0x24]
        // 0003b597: 57
        push    edi
        // 0003b598: ff 15 RR RR RR RR
        call    dword ptr [EnterCriticalSection]
        // 0003b59e: 33 c0
        xor     eax, eax
        // 0003b5a0: 57
        push    edi
        // 0003b5a1: 89 46 20
        mov     dword ptr [esi + 0x20], eax
        // 0003b5a4: 89 46 1c
        mov     dword ptr [esi + 0x1c], eax
        // 0003b5a7: ff 15 RR RR RR RR
        call    dword ptr [LeaveCriticalSection]
        // 0003b5ad: 5f
        pop     edi
        // 0003b5ae: 5e
        pop     esi
        // 0003b5af: c3
        ret
    }
}

} // extern "C"
