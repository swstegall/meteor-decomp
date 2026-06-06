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
// FUNCTION: ffxivgame 0x00460f30 — __cdecl wrapper that allocates 4 bytes via
//                                  __alloca_probe, calls FUN_00460bb0 with the
//                                  first arg in ECX and 0 as a stack arg (EDI
//                                  aimed at the alloca'd local), then returns
//                                  the local value masked by the sign of the
//                                  sub-call's return (55 bytes).
//
// Calling convention: __cdecl (one DWORD arg; plain RET — caller cleans).
// Frame:
//   MOV EAX, 4; CALL __alloca_probe   ; runtime-alloca 4 bytes
//   PUSH EDI                           ; callee-save
//
// The branchless return at the tail:
//   SETLE CL  →  CL = (rv <= 0) ? 1 : 0
//   SUB ECX,1 →  ECX = (rv <= 0) ? 0 : 0xFFFFFFFF
//   AND ECX,[ESP]  →  ECX &= local_var
//   return ECX     →  0 if rv <= 0, local_var if rv > 0
//
// EDI is set to &local_var before CALL FUN_00460bb0; the callee may write
// the result there.  local_var is zeroed immediately before the call.
//
// Both CALL sites are REL32 — masked by tools/compare.py.

extern "C" void _alloca_probe(void);
extern "C" int  FUN_00460bb0(int);

extern "C" __declspec(naked) void FUN_00460f30() {
    __asm {
        mov     eax, 4
        call    _alloca_probe
        mov     ecx, dword ptr [esp + 0x8]
        push    edi
        push    0
        lea     edi, [esp + 0x8]
        mov     dword ptr [esp + 0x8], 0
        call    FUN_00460bb0
        add     esp, 0x4
        xor     ecx, ecx
        test    eax, eax
        setle   cl
        pop     edi
        sub     ecx, 0x1
        and     ecx, dword ptr [esp]
        mov     eax, ecx
        pop     ecx
        ret
    }
}
