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
// FUNCTION: ffxivgame 0x000592b0 — `__stdcall` 3-arg wrapper that forwards
//                                   two of its args to FUN_00459120
//                                   (`__thiscall`, ECX = arg1, stack = arg2)
//                                   then returns 0 (19 B / 0x13).
//
// Calling convention: __stdcall; three DWORD stack args; callee-cleans 12
//   bytes via `RET 0xc`. The third argument ([ESP+0xc]) is silently dropped.
//
// Behaviour (read from the disassembly at orig RVA 0x000592b0):
//
//   int FUN_004592b0(void *arg1, void *arg2, int /*arg3*/) {
//       FUN_00459120(arg1, arg2);   // __thiscall: ECX = arg1, stack = arg2
//       return 0;
//   }
//
// Asm (19 bytes):
//   8b 44 24 08     MOV EAX, dword ptr [ESP+0x8]   ; arg2 → EAX
//   8b 4c 24 04     MOV ECX, dword ptr [ESP+0x4]   ; arg1 → ECX (this)
//   50              PUSH EAX                        ; push arg2 for callee
//   e8 RR RR RR RR  CALL FUN_00459120               ; REL32 reloc — masked
//   33 c0           XOR EAX, EAX                    ; return 0
//   c2 0c 00        RET 0xc                         ; stdcall epilogue
//
// Reconstruction strategy — `__declspec(naked)` passthrough:
//
//   Pre-loading both arg2 (into EAX) and arg1 (into ECX) before the PUSH
//   is MSVC 2005 /O2's standard pattern to avoid adjusting stack offsets
//   mid-sequence. The naked asm spells this out verbatim; the only
//   linker fixup is the REL32 CALL to FUN_00459120, which compare.py masks.

// Callee: __thiscall `bool Bootstrap(void* src)` at RVA 0x00059120.
// Declared as plain extern "C" so MASM can emit the REL32 CALL reloc.
extern "C" void FUN_00459120();

extern "C" __declspec(naked) void FUN_004592b0() {
    __asm {
        mov eax, dword ptr [esp+0x8]
        mov ecx, dword ptr [esp+0x4]
        push eax
        call FUN_00459120
        xor eax, eax
        ret 0xc
    }
}
