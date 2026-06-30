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
// FUNCTION: ffxivgame 0x0006b660 — _ASN1_TIME_check (38 B / 0x26)
//
// Dispatches an ASN1_TIME to either the GENERALIZEDTIME or UTCTIME
// validator by reading the type tag at offset 4 of the argument struct:
//   type == 0x18 (V_ASN1_GENERALIZEDTIME = 24) → tail-JMP to FUN_004812c0
//   type == 0x17 (V_ASN1_UTCTIME          = 23) → tail-JMP to FUN_00481560
//   otherwise                                   → return 0
//
// Calling convention: __cdecl — one pointer arg, int return, plain RET.
// Frame: none (/Oy — no locals).
//
// Asm (38 bytes @ orig RVA 0x0006b660):
//   8b 4c 24 04          MOV ECX, dword ptr [ESP+0x4]      ; t
//   8b 41 04             MOV EAX, dword ptr [ECX+0x4]      ; t->type
//   83 f8 18             CMP EAX, 0x18
//   75 09                JNZ short +9                      ; skip to 0x17 test
//   89 4c 24 04          MOV dword ptr [ESP+0x4], ECX      ; arg for tail call
//   e9 4b 5c 01 00       JMP FUN_004812c0                  ; (reloc)
//   83 f8 17             CMP EAX, 0x17
//   75 09                JNZ short +9                      ; skip to return 0
//   89 4c 24 04          MOV dword ptr [ESP+0x4], ECX      ; arg for tail call
//   e9 dd 5e 01 00       JMP FUN_00481560                  ; (reloc)
//   33 c0                XOR EAX, EAX
//   c3                   RET
//
// The two tail calls overwrite [ESP+4] with ECX (same value) and JMP so
// the callee reads t from [ESP+4] and returns straight to our caller.
// Using __declspec(naked) to preserve the JNZ short-form (75 09) and the
// two 5-byte E9 JMPs; both JMP relocations are linker-fixed wildcards.

extern "C" void FUN_004812c0();   // ASN1_GENERALIZEDTIME_check target
extern "C" void FUN_00481560();   // ASN1_UTCTIME_check target

extern "C" __declspec(naked) void __cdecl FUN_0046b660(void *) {
    __asm {
        // 0006b660: 8b 4c 24 04   MOV ECX, dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0006b664: 8b 41 04      MOV EAX, dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 0006b667: 83 f8 18      CMP EAX, 0x18
        _emit 0x83
        _emit 0xf8
        _emit 0x18
        // 0006b66a: 75 09         JNZ short +9  (skip MOV+JMP to 0x17 test)
        _emit 0x75
        _emit 0x09
        // 0006b66c: 89 4c 24 04   MOV dword ptr [ESP+0x4], ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0006b670: e9 ...        JMP FUN_004812c0  (5-byte near JMP, linker reloc)
        jmp FUN_004812c0
        // 0006b675: 83 f8 17      CMP EAX, 0x17
        _emit 0x83
        _emit 0xf8
        _emit 0x17
        // 0006b678: 75 09         JNZ short +9  (skip MOV+JMP to XOR EAX,EAX)
        _emit 0x75
        _emit 0x09
        // 0006b67a: 89 4c 24 04   MOV dword ptr [ESP+0x4], ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0006b67e: e9 ...        JMP FUN_00481560  (5-byte near JMP, linker reloc)
        jmp FUN_00481560
        // 0006b683: 33 c0         XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0006b685: c3            RET
        _emit 0xc3
    }
}
