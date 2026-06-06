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
// FUNCTION: ffxivgame 0x0044daa0 — query helper returning bit 0x10 (35 B)
//
// Loads a global DWORD (a handle/state value) at 0x0132cf4c, then calls a
// __stdcall helper FUN_009d00ba(g, &out0, &out1) which fills two output
// locals. Returns out0 & 0x10 — testing a single status bit.
//
// Calling convention: __cdecl, no args, int return.
// Frame: SUB ESP,8 for the two output locals (no frame pointer, /Oy).
//
// Asm (35 bytes @ orig RVA 0x0044daa0):
//   8b 15 4c cf 32 01   MOV EDX, [g_0132cf4c]   ; hoisted global load
//   83 ec 08            SUB ESP, 8              ; out0 @ [ESP], out1 @ [ESP+4]
//   8d 44 24 04         LEA EAX, [ESP+4]        ; &out1
//   50                  PUSH EAX                ; arg3
//   8d 4c 24 04         LEA ECX, [ESP+4]        ; &out0 (ESP shifted by push)
//   51                  PUSH ECX                ; arg2
//   52                  PUSH EDX                ; arg1 = global
//   e8 RR RR RR RR      CALL FUN_009d00ba       ; __stdcall, RET 0xC (reloc)
//   8b 04 24            MOV EAX, [ESP]          ; out0
//   83 e0 10            AND EAX, 0x10
//   83 c4 08            ADD ESP, 8
//   c3                  RET

extern "C" int  g_0132cf4c;   // [0x0132cf4c]
extern "C" void __stdcall FUN_009d00ba(int handle, int* out0, int* out1);

extern "C" int __cdecl FUN_0044daa0()
{
    int out0;
    int out1;
    FUN_009d00ba(g_0132cf4c, &out0, &out1);
    return out0 & 0x10;
}
