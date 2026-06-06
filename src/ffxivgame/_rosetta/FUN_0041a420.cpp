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
// FUNCTION: ffxivgame 0x0041a420 — __thiscall member write/flush helper
//           (4 stack args + implicit this in ECX; returns bool in AL; 95 bytes)
//
// Calling convention: __thiscall (ECX = this → EDI throughout).
// Stack args (right-to-left, 0x10 bytes cleaned by callee via RET 0x10):
//   [ESP+0x04]  arg1 — start offset / base position
//   [ESP+0x08]  arg2 — span length; if zero, defaults to this->field14 - arg1
//   [ESP+0x0c]  arg3 — passed through to FUN_00419dd0
//   [ESP+0x10]  arg4 — passed through to FUN_009d4600 (memcpy-like)
//
// Behaviour sketch:
//   1. If arg2 == 0, compute it as this->field14 - arg1 (end - start).
//   2. Call this->FUN_00419dd0(arg1, computed_span, arg3)  [thiscall, ret 0x0c]
//   3. Call FUN_009d4600(ret, arg4, computed_span)         [__cdecl memcpy variant]
//   4. Test this->field24; if field28 == field24, call FUN_009d22b4.
//   5. Call FUN_00423370(obj, this->field1c, this->field24) [thiscall, ret 0x08]
//   6. Return AL = 1.
//
// Disassembly (95 bytes, RVA 0x0001a420):
//   8b 44 24 04              MOV   EAX, [ESP+0x4]
//   56                       PUSH  ESI
//   8b 74 24 0c              MOV   ESI, [ESP+0xc]
//   85 f6                    TEST  ESI, ESI
//   57                       PUSH  EDI
//   8b f9                    MOV   EDI, ECX
//   75 05                    JNE   skip_default     (+5)
//   8b 77 14                 MOV   ESI, [EDI+0x14]
//   2b f0                    SUB   ESI, EAX
//   skip_default:
//   8b 4c 24 14              MOV   ECX, [ESP+0x14]
//   51                       PUSH  ECX
//   56                       PUSH  ESI
//   50                       PUSH  EAX
//   8b cf                    MOV   ECX, EDI
//   e8 ?? ?? ?? ??           CALL  FUN_00419dd0
//   8b 54 24 18              MOV   EDX, [ESP+0x18]
//   56                       PUSH  ESI
//   52                       PUSH  EDX
//   50                       PUSH  EAX
//   e8 ?? ?? ?? ??           CALL  FUN_009d4600
//   8b 47 24                 MOV   EAX, [EDI+0x24]
//   83 c4 0c                 ADD   ESP, 0xc
//   85 c0                    TEST  EAX, EAX
//   74 07                    JE    call_22b4        (+7)
//   8b 4f 28                 MOV   ECX, [EDI+0x28]
//   2b c8                    SUB   ECX, EAX
//   75 05                    JNE   after_22b4       (+5)
//   call_22b4:
//   e8 ?? ?? ?? ??           CALL  FUN_009d22b4
//   after_22b4:
//   8b 57 24                 MOV   EDX, [EDI+0x24]
//   8b 47 1c                 MOV   EAX, [EDI+0x1c]
//   8b 0d ?? ?? ?? ??        MOV   ECX, [DAT_0132987c]
//   52                       PUSH  EDX
//   50                       PUSH  EAX
//   e8 ?? ?? ?? ??           CALL  FUN_00423370
//   5f                       POP   EDI
//   b0 01                    MOV   AL, 1
//   5e                       POP   ESI
//   c2 10 00                 RET   0x10
//
// Reloc-bearing sites (4 × CALL rel32 + 1 × MOV [DIR32]):
//   +0x1e  CALL rel32 → FUN_00419dd0 (VA 0x00419dd0)
//   +0x27  CALL rel32 → FUN_009d4600 (VA 0x009d4600)
//   +0x40  CALL rel32 → FUN_009d22b4 (VA 0x009d22b4)
//   +0x53  CALL rel32 → FUN_00423370 (VA 0x00423370)
//   +0x4b  MOV  DIR32 → DAT_0132987c (VA 0x0132987c)

extern "C" {
    void FUN_00419dd0();   // __thiscall, 3 stack args, callee cleans 0xc
    void FUN_009d4600();   // __cdecl memcpy-like, 3 args
    void FUN_009d22b4();   // 0-arg helper
    void FUN_00423370();   // __thiscall, 2 stack args, callee cleans 0x8

    extern int DAT_0132987c;
}

extern "C" __declspec(naked) void FUN_0041a420() {
    __asm {
        mov   eax, dword ptr [esp + 4]
        push  esi
        mov   esi, dword ptr [esp + 0xc]
        test  esi, esi
        push  edi
        mov   edi, ecx
        jne   skip_default
        mov   esi, dword ptr [edi + 0x14]
        sub   esi, eax
    skip_default:
        mov   ecx, dword ptr [esp + 0x14]
        push  ecx
        push  esi
        push  eax
        mov   ecx, edi
        call  FUN_00419dd0
        mov   edx, dword ptr [esp + 0x18]
        push  esi
        push  edx
        push  eax
        call  FUN_009d4600
        mov   eax, dword ptr [edi + 0x24]
        add   esp, 0xc
        test  eax, eax
        je    call_22b4
        mov   ecx, dword ptr [edi + 0x28]
        sub   ecx, eax
        jne   after_22b4
    call_22b4:
        call  FUN_009d22b4
    after_22b4:
        mov   edx, dword ptr [edi + 0x24]
        mov   eax, dword ptr [edi + 0x1c]
        mov   ecx, dword ptr [DAT_0132987c]
        push  edx
        push  eax
        call  FUN_00423370
        pop   edi
        mov   al, 1
        pop   esi
        ret   0x10
    }
}
