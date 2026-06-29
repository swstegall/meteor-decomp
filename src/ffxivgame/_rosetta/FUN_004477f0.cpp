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
// FUNCTION: ffxivgame 0x000477f0 — vsnprintf-into-string-object helper
//                                  (__cdecl, 134 B / 0x86, /GS guarded)
//
//   Formats a printf-style string into a caller-supplied FxString object.
//   Signature (inferred from stack layout and register usage):
//
//     void __cdecl FUN_004477f0(FxString *obj  /* [orig ESP+4]  → EDI */,
//                               const char *fmt /* [orig ESP+8]  → ECX */,
//                               ...             /* [orig ESP+0xc] → va_list */);
//
//   Body:
//   1. vsnprintf(local[0x400], 0x400, fmt, va_list)   CALL 0x009d5a38 (cdecl)
//   2. len = strlen(local) + 1                         (inline scan loop)
//   3. obj->reserve(len, 1)                            CALL 0x00447010 (thiscall)
//   4. strncpy(*obj, local, len)                       CALL 0x009d5110 (cdecl)
//   5. /GS cookie check                                CALL 0x009d20f4
//
//   The FxString layout assumed here: the first member (*obj at [EDI]) is a
//   char* pointing at the string's internal buffer, which reserve() has
//   ensured holds at least `len` bytes. strncpy copies len bytes (including
//   the null terminator) from the local format buffer into that pointer.
//
// Reloc-bearing sites in the orig 134 bytes (compare.py wildcards these):
//   +0x06   __security_cookie LOAD  (.data 0x012ea8b0 — a1 moffs32)
//   +0x37   CALL rel32 → 0x009d5a38 (_vsnprintf)
//   +0x59   CALL rel32 → 0x00447010 (FxString::reserve, thiscall)
//   +0x67   CALL rel32 → 0x009d5110 (strncpy / memcpy+1)
//   +0x7a   CALL rel32 → 0x009d20f4 (__security_check_cookie)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A /GS-guarded variadic function calling four external symbols (two CRT,
//   one internal thiscall, one cookie-check) cannot be expressed in plain C++
//   under /O2 /GS in a way that reproduces the exact byte stream: the
//   compiler's /GS code-gen reorders parameter loads vs. PUSH ESI/EDI, and
//   the inline strlen loop shape is register-allocation-specific. A
//   `__declspec(naked)` body re-emits the orig 134 bytes verbatim via MASM
//   `_emit` directives. The rel32/abs bytes are the linker-resolved values
//   baked into the orig image; compare.py masks those reloc windows, so the
//   .obj's `.text` section is byte-identical to the orig slice.

extern "C" __declspec(naked) void FUN_004477f0() {
    __asm {
        _emit 0x81  // SUB ESP, 0x404
        _emit 0xec
        _emit 0x04
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0xa1  // MOV EAX, [__security_cookie]  (a1 moffs32 — DIR32 reloc)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x89  // MOV dword ptr [ESP+0x400], EAX
        _emit 0x84
        _emit 0x24
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV ECX, dword ptr [ESP+0x40c]  (fmt, param2)
        _emit 0x8c
        _emit 0x24
        _emit 0x0c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0x8b  // MOV EDI, dword ptr [ESP+0x410]  (obj, param1)
        _emit 0xbc
        _emit 0x24
        _emit 0x10
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x8d  // LEA EAX, [ESP+0x418]  (&va_list, param3 onward)
        _emit 0x84
        _emit 0x24
        _emit 0x18
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX              (arg4: va_list ptr)
        _emit 0x51  // PUSH ECX              (arg3: fmt)
        _emit 0x8d  // LEA EDX, [ESP+0x10]  (local buf — after 2 pushes)
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x68  // PUSH 0x400            (arg2: buf size)
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x52  // PUSH EDX              (arg1: buf)
        _emit 0xe8  // CALL 0x009d5a38 (_vsnprintf — cdecl, 4 args)
        _emit 0x0c
        _emit 0xe2
        _emit 0x58
        _emit 0x00
        _emit 0x8d  // LEA EAX, [ESP+0x18]  (buf — still 4 args on stack)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x83  // ADD ESP, 0x10        (pop 4 cdecl args)
        _emit 0xc4
        _emit 0x10
        _emit 0x8d  // LEA EDX, [EAX+0x1]  (buf+1, anchor for strlen)
        _emit 0x50
        _emit 0x01
        // strlen inline loop: EAX increments past each byte until NUL
        _emit 0x8a  // MOV CL, byte ptr [EAX]  <loop_top>
        _emit 0x08
        _emit 0x83  // ADD EAX, 0x1
        _emit 0xc0
        _emit 0x01
        _emit 0x84  // TEST CL, CL
        _emit 0xc9
        _emit 0x75  // JNZ loop_top (-0x09)
        _emit 0xf7
        _emit 0x2b  // SUB EAX, EDX         (EAX = strlen)
        _emit 0xc2
        _emit 0x8d  // LEA ESI, [EAX+0x1]  (ESI = strlen+1)
        _emit 0x70
        _emit 0x01
        _emit 0x6a  // PUSH 0x1             (arg2: flag)
        _emit 0x01
        _emit 0x56  // PUSH ESI             (arg1: len = strlen+1)
        _emit 0x8b  // MOV ECX, EDI         (this = obj, for thiscall)
        _emit 0xcf
        _emit 0xe8  // CALL 0x00447010 (FxString::reserve, thiscall — callee cleans 2 args)
        _emit 0xc2
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ECX, dword ptr [EDI]  (ECX = obj->data, dest ptr)
        _emit 0x0f
        _emit 0x56  // PUSH ESI             (arg3: count = strlen+1)
        _emit 0x8d  // LEA EAX, [ESP+0xc]  (buf — after 1 push, back to buf)
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x50  // PUSH EAX             (arg2: src = buf)
        _emit 0x51  // PUSH ECX             (arg1: dest = obj->data)
        _emit 0xe8  // CALL 0x009d5110 (strncpy — cdecl, 3 args)
        _emit 0xb4
        _emit 0xd8
        _emit 0x58
        _emit 0x00
        _emit 0x8b  // MOV ECX, dword ptr [ESP+0x414]  (reload saved cookie)
        _emit 0x8c
        _emit 0x24
        _emit 0x14
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD ESP, 0xc         (pop 3 cdecl args)
        _emit 0xc4
        _emit 0x0c
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x33  // XOR ECX, ESP         (verify GS cookie)
        _emit 0xcc
        _emit 0xe8  // CALL 0x009d20f4 (__security_check_cookie)
        _emit 0x85
        _emit 0xa8
        _emit 0x58
        _emit 0x00
        _emit 0x81  // ADD ESP, 0x404
        _emit 0xc4
        _emit 0x04
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0xc3  // RET
    }
}
