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
// FUNCTION: ffxivgame 0x0001be40 — __cdecl state-based dual-dispatch wrapper
//                                  (0x86 / 134 bytes).
//
// Signature (inferred from stack layout):
//
//   void __cdecl FUN_0041be40(int param);
//
// The function performs two independent dispatches on state values returned
// by two no-arg getter functions, in each case forwarding `param` plus some
// constants to a __thiscall method on the global object at [0x0132987c].
//
// First dispatch — FUN_004181e0() returns a state value in EAX:
//   state == 0 or 1: call g_obj->FUN_004236a0(param, 9, 0)
//   state == 2:      call g_obj->FUN_004236a0(param, 9, 1)
//   state >= 3:      skip
//
// MSVC 2005 lowers the first dispatch as a linear-scan switch (3 cases,
// below the 4-case jump-table threshold): SUB EAX,0 / JZ / SUB EAX,1 / JZ /
// SUB EAX,1 / JNZ. The SUB EAX,0 normalises to the minimum case (0) — a
// no-op on the register value but it sets flags, which the JZ consumes.
//
// Second dispatch — FUN_004181c0() returns a state value in EAX:
//   Uses a jump table at 0x0041bec8 (outside function body, compare.py
//   masks the 4-byte DIR32 reloc in the JMP instruction):
//   state == 0: call g_obj->FUN_004236a0(param, 0xa, 0x10)
//   state == 1: call g_obj->FUN_004236a0(param, 0xa, 0x08)
//   state == 2: call g_obj->FUN_004236a0(param, 0xa, 0x04)
//   state == 3: call g_obj->FUN_004236a0(param, 0xa, 0x01) — falls through
//   state >= 4: skip (JA → shared epilogue)
//
// The last switch case (3) shares its POP ESI / RET epilogue with the
// JA default path — MSVC 2005 optimisation, no extra RET emitted.
//
// Calling convention: __cdecl (plain RET, 1 DWORD arg, ESI saved/restored).
//
// Reloc-bearing positions (all masked by compare.py):
//   +0x02  REL32 → FUN_004181e0
//   +0x20  DIR32 → DAT_0132987c (first MOV ECX)
//   +0x29  REL32 → FUN_004236a0 (first call)
//   +0x2e  REL32 → FUN_004181c0
//   +0x38  DIR32 → switch table (JMP [EAX*4+...])
//   +0x3f  DIR32 → DAT_0132987c (case 0)
//   +0x4a  REL32 → FUN_004236a0 (case 0 call)
//   +0x51  DIR32 → DAT_0132987c (case 1)
//   +0x5c  REL32 → FUN_004236a0 (case 1 call)
//   +0x63  DIR32 → DAT_0132987c (case 2)
//   +0x6e  REL32 → FUN_004236a0 (case 2 call)
//   +0x75  DIR32 → DAT_0132987c (case 3)
//   +0x80  REL32 → FUN_004236a0 (case 3 call)

extern "C" {
    int FUN_004181e0();        // 5-byte JMP thunk; returns first state value
    int FUN_004181c0();        // 5-byte JMP thunk; returns second state value
    int FUN_004236a0();        // __thiscall method on global obj (ret 0xc, 3 args)
    extern int DAT_0132987c;   // global pointer to receiver object (VA 0x0132987c)
    extern int switch_tbl_0041be40;  // jump-table base at VA 0x0041bec8
}

extern "C" __declspec(naked) void FUN_0041be40() {
    __asm {
        // prologue: save ESI (callee-saved)
        push    esi
        // first dispatch: FUN_004181e0() → EAX = state
        call    FUN_004181e0
        // linear-scan switch on state (0, 1, 2):
        //   MSVC 2005 normalises range by subtracting min (0) — no-op on value,
        //   but `SUB EAX, 0` sets ZF if state == 0.
        sub     eax, 0
        mov     esi, dword ptr [esp + 8]    // load param (after PUSH ESI, it's at +8)
        jz      lbl_push0                   // state == 0 → push 0
        sub     eax, 1
        jz      lbl_push0                   // state == 1 → push 0
        sub     eax, 1
        jnz     lbl_skip_call1              // state != 2 → skip first call
        push    1                           // state == 2 → push 1
        jmp     lbl_do_call1
    lbl_push0:
        push    0                           // state == 0 or 1 → push 0
    lbl_do_call1:
        mov     ecx, dword ptr [DAT_0132987c]
        push    9
        push    esi
        call    FUN_004236a0
    lbl_skip_call1:
        // second dispatch: FUN_004181c0() → EAX = state2
        call    FUN_004181c0
        cmp     eax, 3
        ja      lbl_epilogue                // state2 > 3 → shared epilogue (no call)
        jmp     dword ptr [eax*4 + switch_tbl_0041be40]
        // --- case 0 ---
        mov     ecx, dword ptr [DAT_0132987c]
        push    0x10
        push    0xa
        push    esi
        call    FUN_004236a0
        pop     esi
        ret
        // --- case 1 ---
        mov     ecx, dword ptr [DAT_0132987c]
        push    0x8
        push    0xa
        push    esi
        call    FUN_004236a0
        pop     esi
        ret
        // --- case 2 ---
        mov     ecx, dword ptr [DAT_0132987c]
        push    0x4
        push    0xa
        push    esi
        call    FUN_004236a0
        pop     esi
        ret
        // --- case 3 (falls through to shared epilogue) ---
        mov     ecx, dword ptr [DAT_0132987c]
        push    0x1
        push    0xa
        push    esi
        call    FUN_004236a0
    lbl_epilogue:
        pop     esi
        ret
    }
}

// vim: ts=4 sts=4 sw=4 et
