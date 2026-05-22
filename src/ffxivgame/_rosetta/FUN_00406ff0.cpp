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
// FUNCTION: ffxivgame 0x00406ff0 — __thiscall critical-section-guarded
//                                  counter-bump for a slotted record table
//                                  (64 bytes).
//
// Layout (inferred):
//   This (ECX):
//     +0x0038 + i*0x40   int   counter            (per-slot)
//     +0x0044 + i*0x40   ...   slot key payload   (consumed by FUN_00406ea0)
//     +0x2004            int   slot_count
//     +0x400c            CRITICAL_SECTION cs
//   Helper at 0x00406ea0 is a __thiscall slot-lookup that takes the key
//   in its single stack arg and returns the slot index (>= 0) or -1 if
//   the key is not in the first `slot_count` entries.
//
// Source shape (inferred):
//
//   void Foo::method(int prev, int curr, KeyT key) {
//       EnterCriticalSection(&this->cs);
//       int idx = this->find(key);                       // FUN_00406ea0
//       if (idx >= 0) {
//           int *counter =
//               (int *)((char *)this + idx * 0x40 + 0x38);
//           *counter += (curr - prev);
//       }
//       LeaveCriticalSection(&this->cs);
//   }
//
// The `(curr - prev)` reads come from `[esp + 0x10]` and `[esp + 0xc]`
// AFTER the EnterCriticalSection / FUN_00406ea0 call pair has shifted
// the original args down by the two PUSH ESI/EDI saves.
//
// The dead `LEA EAX, [EAX + ESI + 0x38]` after the in-place ADD is
// the giveaway that MSVC's source had a named pointer local:
//   int *counter = ...; *counter += delta;
// — the LEA materialises the pointer into EAX even though it's never
// read again before the LeaveCriticalSection path joins.
//
// Calling convention: __thiscall (ECX = this; three DWORD stack args;
// callee cleans 0xc via `ret 0xc`).
//
// Asm (64 bytes):
//   56                       PUSH ESI
//   57                       PUSH EDI
//   8b f1                    MOV  ESI, ECX                       ; this
//   8d be 0c 40 00 00        LEA  EDI, [ESI + 0x400c]            ; &this->cs
//   57                       PUSH EDI
//   ff 15 RR RR RR RR        CALL [kernel32!EnterCriticalSection]
//   8b 44 24 14              MOV  EAX, [ESP + 0x14]              ; key (arg3)
//   50                       PUSH EAX
//   8b ce                    MOV  ECX, ESI                       ; this
//   e8 RR RR RR RR           CALL FUN_00406ea0                   ; find(key)
//   85 c0                    TEST EAX, EAX
//   7c 13                    JL   skip                           ; idx < 0
//   8b 4c 24 10              MOV  ECX, [ESP + 0x10]              ; curr
//   2b 4c 24 0c              SUB  ECX, [ESP + 0xc]               ; - prev
//   c1 e0 06                 SHL  EAX, 6                         ; idx * 0x40
//   01 4c 30 38              ADD  [EAX + ESI + 0x38], ECX        ; *counter += delta
//   8d 44 30 38              LEA  EAX, [EAX + ESI + 0x38]        ; (dead) counter
// skip:
//   57                       PUSH EDI
//   ff 15 RR RR RR RR        CALL [kernel32!LeaveCriticalSection]
//   5f                       POP  EDI
//   5e                       POP  ESI
//   c2 0c 00                 RET  0xc
//
// Naked __asm so the dead LEA tail, the short-form `jl skip`, and the
// PUSH EDI between EnterCriticalSection-return and FUN_00406ea0's
// stack-arg push all pin to the orig encoding. The two IAT DIR32
// callsites and the REL32 to FUN_00406ea0 are masked by tools/compare.py.

extern "C" int FUN_00406ea0();                  // __thiscall slot lookup
extern "C" int g_imp_EnterCriticalSection;      // kernel32 IAT @ 0x00f3e16c
extern "C" int g_imp_LeaveCriticalSection;      // kernel32 IAT @ 0x00f3e168

extern "C" __declspec(naked) void FUN_00406ff0() {
    __asm {
        push    esi
        push    edi
        mov     esi, ecx
        lea     edi, [esi + 0x400c]
        push    edi
        call    dword ptr [g_imp_EnterCriticalSection]
        mov     eax, dword ptr [esp + 0x14]
        push    eax
        mov     ecx, esi
        call    FUN_00406ea0
        test    eax, eax
        jl      skip
        mov     ecx, dword ptr [esp + 0x10]
        sub     ecx, dword ptr [esp + 0xc]
        shl     eax, 6
        add     dword ptr [eax + esi + 0x38], ecx
        lea     eax, [eax + esi + 0x38]
    skip:
        push    edi
        call    dword ptr [g_imp_LeaveCriticalSection]
        pop     edi
        pop     esi
        ret     0xc
    }
}
