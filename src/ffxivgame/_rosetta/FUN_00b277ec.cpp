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
// FUNCTION: ffxivgame 0x007277ec — SEH-frame epilogue with EBX-based ESP restore
//                                  (__stdcall, 22 B, 0x3c bytes of stack args)
//
// This is the tail of a __stdcall function that uses MSVC 2005 structured
// exception handling (SEH via FS:[0]) and an EBX-based ESP anchor (the
// canonical MSVC 2005 _alloca / variable-frame epilogue idiom).
//
// Asm (22 bytes @ orig RVA 0x007277ec):
//   8b 4d f4                MOV  ECX, dword ptr [EBP-0xc]   ; saved FS:[0]
//   64 89 0d 00 00 00 00    MOV  dword ptr FS:[0x0], ECX    ; unlink SEH frame
//   5f                      POP  EDI                        ; restore callee-saved
//   5e                      POP  ESI
//   5b                      POP  EBX                        ; EBX = saved ESP anchor
//   8b e5                   MOV  ESP, EBP                   ; standard frame teardown
//   5d                      POP  EBP                        ; restore EBP
//   8b e3                   MOV  ESP, EBX                   ; skip alloca'd region
//   5b                      POP  EBX                        ; restore real saved EBX
//   c2 3c 00                RET  0x3c                       ; callee cleans 60 bytes

extern "C" __declspec(naked) void FUN_00b277ec()
{
    __asm {
        mov  ecx, dword ptr [ebp - 0xc]
        mov  dword ptr fs:[0], ecx
        pop  edi
        pop  esi
        pop  ebx
        mov  esp, ebp
        pop  ebp
        mov  esp, ebx
        pop  ebx
        ret  0x3c
    }
}
