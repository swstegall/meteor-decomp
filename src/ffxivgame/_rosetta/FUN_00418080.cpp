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
// FUNCTION: ffxivgame 0x00018080 — conditional indirect-call thunk (19 B / 0x13).
//
// Behaviour read from the disassembly at orig RVA 0x00018080:
//
//   __cdecl void FUN_00418080(void* p, int flag) {
//       if (flag != 1)
//           g_pfn_00f3e164(p);
//   }
//
//   If the second argument equals 1 the function is a no-op; otherwise it
//   pushes the first argument and calls through the global function pointer
//   stored at absolute address 0x00f3e164. The callee cleans its own stack
//   slot (no ADD ESP after the CALL → callee is __stdcall). The outer
//   function returns via plain RET (no RET N) so it is __cdecl.
//
//   Asm shape (19 bytes total):
//     83 7c 24 08 01    CMP  dword ptr [ESP+0x8], 1
//     74 0b             JZ   .ret               ; flag==1 → skip
//     8b 44 24 04       MOV  EAX, [ESP+0x4]     ; load p
//     50                PUSH EAX                ; arg for callee
//     ff 15 ~~ ~~ ~~ ~~ CALL dword ptr [reloc]  ; indirect via g_pfn_00f3e164
//   .ret:
//     c3                RET
//
//   The 4 reloc bytes in the CALL operand are masked by tools/compare.py.

typedef void (__stdcall *PFN_00F3E164)(void*);
extern "C" PFN_00F3E164 g_pfn_00f3e164;

extern "C" void __cdecl FUN_00418080(void* p, int flag)
{
    if (flag != 1)
        g_pfn_00f3e164(p);
}
