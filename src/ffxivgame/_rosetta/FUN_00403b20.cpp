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
// FUNCTION: ffxivgame 0x00003b20 — name → value table lookup using
// case-insensitive string compare (__cdecl, 77 B).
//
// Linear scan over an array of `(const char* name, int value)` pairs.
// For each entry, compares `obj->str()` against `entry->name` with
// `_stricmp`; on the first match writes the entry's `value` through
// the `out` pointer and returns `true`. Returns `false` if the table
// is exhausted (or empty) without a match.
//
// Asm shape (per asm/ffxivgame/00003b20_FUN_00403b20.s):
//
//   PUSH EBX                         ; save EBX
//   MOV  EBX, [ESP + 0xc]            ; EBX = count
//   PUSH EBP                         ; save EBP
//   PUSH ESI                         ; save ESI
//   XOR  ESI, ESI                    ; i = 0
//   TEST EBX, EBX
//   PUSH EDI                         ; save EDI (between TEST and JBE)
//   JBE  end_fail                    ; if (count == 0) → return false
//   MOV  EBP, [ESP + 0x1c]           ; EBP = obj
//   MOV  EDI, [ESP + 0x14]           ; EDI = table
// loop:
//   MOV  EAX, [EDI + ESI*8]          ; entry.name
//   PUSH EAX                         ;   arg2 of _stricmp
//   MOV  ECX, EBP                    ; this = obj
//   CALL FUN_00445210                ; obj->str()       (__thiscall, 0 args)
//   PUSH EAX                         ;   arg1 of _stricmp
//   CALL _stricmp                    ; __stricmp(a, b)
//   ADD  ESP, 0x8
//   TEST EAX, EAX
//   JZ   found                       ; if (cmp == 0) → match
//   ADD  ESI, 0x1                    ; ++i
//   CMP  ESI, EBX
//   JC   loop                        ; while (i < count)
// end_fail:
//   POP  EDI / ESI / EBP
//   XOR  AL, AL                      ; return false
//   POP  EBX
//   RET
// found:
//   MOV  ECX, [EDI + ESI*8 + 4]      ; entry.value
//   MOV  EDX, [ESP + 0x20]           ; out
//   POP  EDI / ESI / EBP
//   MOV  [EDX], ECX                  ; *out = value
//   MOV  AL, 1                       ; return true
//   POP  EBX
//   RET
//
// Calling conventions:
//   - This function: __cdecl, returns bool in AL.
//   - obj->str(): __thiscall, no stack args, returns const char* in
//     EAX. The actual target at 0x00445210 is the canonical 3-byte
//     field-getter cluster (`mov eax, [ecx]; ret`) — so `str()` is a
//     simple "return the first member" accessor on `obj`.
//   - _stricmp: __cdecl, 2 stack args, caller cleans 8 bytes.
//
// Stack frame: 16 bytes (PUSH EBX/EBP/ESI/EDI), no locals.

#include <string.h>      // _stricmp

namespace {

struct Entry {
    const char* name;
    int         value;
};

// Forward-declared (not defined) so MSVC does not inline the getter
// into `mov eax, [ecx]` at this call site — we want an explicit CALL
// to 0x00445210 (the canonical "return field at offset 0" thunk).
class Holder {
    const char* str_;
public:
    const char* str();
};

} // namespace

extern "C" bool __cdecl FUN_00403b20(
    const Entry*  table,
    unsigned int  count,
    Holder*       obj,
    int*          out)
{
    for (unsigned int i = 0; i < count; ++i) {
        if (_stricmp(obj->str(), table[i].name) == 0) {
            *out = table[i].value;
            return true;
        }
    }
    return false;
}
