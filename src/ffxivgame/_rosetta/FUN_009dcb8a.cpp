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
// FUNCTION: ffxivgame 0x005dcb8a — EH per-thread state predicate (19 B / 0x13)
//
// Located between __DestructExceptionObject (0x005dcb11), __AdjustPointer
// (0x005dcb65), and IsInExceptionSpec (0x005dcb9d) — this is part of the
// MSVC 2005 exception-handling runtime support compiled into the game binary.
//
// Asm (19 bytes @ RVA 0x005dcb8a):
//   e8 48 28 00 00        CALL    __getptd               ; get per-thread data → EAX
//   33 c9                 XOR     ECX,ECX                ; ECX = 0 (comparison operand)
//   39 88 90 00 00 00     CMP     dword ptr [EAX+0x90],ECX ; ptd->field_0x90 == 0?
//   0f 95 c1              SETNZ   CL                     ; CL = (field != 0)
//   8a c1                 MOV     AL,CL                  ; bool return value
//   c3                    RET
//
// Calling convention: __cdecl (bare RET, no argument cleanup).
// Return type: bool (written into AL via SETNZ CL / MOV AL, CL).
// No stack frame, no callee-saved registers, no security cookie.
//
// Semantics: return whether the DWORD at offset 0x90 in the MSVC per-thread
// data block (_ptiddata) is non-zero. Offset 0x90 holds an EH-related state
// flag (active exception handler pointer / nested exception depth) in the
// _ptiddata layout shipped with MSVC 2005's /MT CRT.
//
// MSVC 2005 /O2 generates XOR ECX,ECX + CMP [EAX+0x90],ECX (register form,
// 8 bytes) rather than the shorter CMP [EAX+0x90],0 (immediate form, 7 bytes)
// here; this is a known MSVC 2005 code-generation idiom for bool predicates
// on memory operands when the result feeds directly into SETNZ.

extern "C" void* __cdecl __getptd();

extern "C" bool __cdecl FUN_009dcb8a()
{
    return *(int*)((char*)__getptd() + 0x90) != 0;
}
