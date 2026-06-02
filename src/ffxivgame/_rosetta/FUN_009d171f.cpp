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
// FUNCTION: ffxivgame 0x009d171f — __cdecl 2-arg initializer that writes a
//                                  hard-coded function pointer and a caller-
//                                  supplied value into a small struct (18 B).
//
// Disassembly (orig RVA 0x005d171f):
//
//   8b 44 24 04          MOV  EAX, dword ptr [ESP+4]    ; p = arg1
//   8b 4c 24 08          MOV  ECX, dword ptr [ESP+8]    ; val = arg2
//   c7 00 13 17 9d 00    MOV  dword ptr [EAX], 9d1713h  ; p->field0 = &helper
//   89 48 04             MOV  dword ptr [EAX+4], ECX    ; p->field1 = val
//   c3                   RET
//
// Calling convention: __cdecl (bare RET; caller cleans two stack args).
// No prologue / no callee-saves / no stack frame (consistent with /Oy).
//
// The constant 0x009d1713 is the virtual address of a 12-byte helper that
// lives immediately before this function in the .text section (between the
// end of __Toupper and the start of FUN_009d171f). That helper is not
// listed as a named symbol in the work-pool YAML; it is expressed here as
// a cast integer literal so the .obj carries the same immediate bytes as
// the original PE (no relocation entry required — the literal 0x009d1713
// matches the orig byte-for-byte).
//
// Struct layout inferred from the two stores:
//   offset 0 (void*)   — function-pointer / callback slot
//   offset 4 (int)     — integer payload

struct FUN_009d171f_Struct {
    void* field0;   // offset 0: function pointer or callback
    int   field1;   // offset 4: integer payload
};

extern "C" void __cdecl FUN_009d171f(FUN_009d171f_Struct* p, int val)
{
    p->field0 = reinterpret_cast<void*>(0x009d1713);
    p->field1 = val;
}
