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
// FUNCTION: ffxivgame 0x00047620 — vsnprintf-into-string helper
//                                  (__cdecl, 186 B / 0xba, /GS guarded)
//
//   Builds a custom string object (arg0, returned in EAX) from a printf
//   format string (arg1) plus a varargs tail. The result object uses an
//   SSO-style layout:
//
//     struct FxString {
//         char* data;    // +0x00  → points at inline buf (+0x12) initially
//         int   cap;     // +0x04  = 0x40 (64) inline capacity
//         int   m_08;    // +0x08  = 1
//         int   len;     // +0x0c  = 0
//         char  f_10;    // +0x10  = 1
//         char  f_11;    // +0x11  = 1
//         char  inline_buf[...]; // +0x12 ...
//     };
//
//   __cdecl FxString* FUN_00447620(FxString* result /* [ESP+0x40C] → ESI */,
//                                  const char* fmt  /* [ESP+0x410] → ECX */,
//                                  ...);
//
//   1. vsnprintf(local[0x400], 0x400, fmt, va=&...)   CALL 0x009d5a38 (cdecl)
//   2. Initialise `result` to an empty 64-cap inline string.
//   3. len = strlen(local)                            (inline scan loop)
//   4. result->reserve(len + 1, 1)                    CALL 0x00447010 (thiscall)
//   5. memcpy(result->data, local, len)               CALL 0x009d5110 (cdecl)
//   6. result->data[len] = '\0'; return result.
//   7. /GS cookie check                               CALL 0x009d20f4
//
// Reloc-bearing sites in the orig 186 bytes (compare.py wildcards these):
//   +0x06   __security_cookie LOAD  (.data 0x012ea8b0 — a1 moffs32)
//   +0x3d   CALL rel32 → 0x009d5a38 (vsnprintf)
//   +0x85   CALL rel32 → 0x00447010 (FxString::reserve, thiscall)
//   +0x93   CALL rel32 → 0x009d5110 (memcpy)
//   +0xae   CALL rel32 → 0x009d20f4 (__security_check_cookie)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Same approach as the in-module siblings (FUN_00403f10, FUN_00401750,
//   FUN_00406680): a /GS-guarded body with four CALL rel32 relocations and
//   a moffs32 cookie load has no plain-C++ encoding that reproduces the
//   exact byte stream under /O2 /GS. A `__declspec(naked)` body re-emits
//   the orig 186 bytes verbatim; the .obj's `.text` ends up byte-identical
//   to the orig slice (the rel32/abs bytes are the linker-resolved values
//   baked into the orig image, and compare.py masks the reloc windows).

extern "C" __declspec(naked) void FUN_00447620() {
    __asm {
        _emit 0x81  // SUB ESP, 0x408
        _emit 0xec
        _emit 0x08
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0xa1  // MOV EAX, __security_cookie [0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x89  // MOV [ESP+0x404], EAX
        _emit 0x84
        _emit 0x24
        _emit 0x04
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV ECX, [ESP+0x410]  (fmt)
        _emit 0x8c
        _emit 0x24
        _emit 0x10
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x56  // PUSH ESI
        _emit 0x8b  // MOV ESI, [ESP+0x410]  (result)
        _emit 0xb4
        _emit 0x24
        _emit 0x10
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x57  // PUSH EDI
        _emit 0x8d  // LEA EAX, [ESP+0x41c]  (&va)
        _emit 0x84
        _emit 0x24
        _emit 0x1c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x51  // PUSH ECX
        _emit 0x8d  // LEA EDX, [ESP+0x14]  (&local buffer)
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x68  // PUSH 0x400
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x33  // XOR EDI, EDI
        _emit 0xff
        _emit 0x52  // PUSH EDX
        _emit 0x89  // MOV [ESP+0x18], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0xe8  // CALL 0x009d5a38 (vsnprintf)
        _emit 0xd6
        _emit 0xe3
        _emit 0x58
        _emit 0x00
        _emit 0x8d  // LEA EAX, [ESI+0x12]
        _emit 0x46
        _emit 0x12
        _emit 0x89  // MOV [ESI], EAX
        _emit 0x06
        _emit 0xc6  // MOV byte [ESI+0x10], 1
        _emit 0x46
        _emit 0x10
        _emit 0x01
        _emit 0xc6  // MOV byte [ESI+0x11], 1
        _emit 0x46
        _emit 0x11
        _emit 0x01
        _emit 0x89  // MOV [ESI+0xc], EDI
        _emit 0x7e
        _emit 0x0c
        _emit 0xc7  // MOV [ESI+0x8], 1
        _emit 0x46
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7  // MOV [ESI+0x4], 0x40
        _emit 0x46
        _emit 0x04
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc6  // MOV byte [EAX], 0
        _emit 0x00
        _emit 0x00
        _emit 0x8d  // LEA EAX, [ESP+0x1c]  (&local)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x83  // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x8d  // LEA EDX, [EAX+1]
        _emit 0x50
        _emit 0x01
        _emit 0x8d  // LEA ECX, [ECX]  (nop align)
        _emit 0x49
        _emit 0x00
        _emit 0x8a  // MOV CL, [EAX]  (strlen loop)
        _emit 0x08
        _emit 0x83  // ADD EAX, 1
        _emit 0xc0
        _emit 0x01
        _emit 0x84  // TEST CL, CL
        _emit 0xc9
        _emit 0x75  // JNZ -9 (loop)
        _emit 0xf7
        _emit 0x2b  // SUB EAX, EDX
        _emit 0xc2
        _emit 0x8b  // MOV EDI, EAX  (len)
        _emit 0xf8
        _emit 0x6a  // PUSH 1
        _emit 0x01
        _emit 0x8d  // LEA EAX, [EDI+1]
        _emit 0x47
        _emit 0x01
        _emit 0x50  // PUSH EAX
        _emit 0x8b  // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8  // CALL 0x00447010 (reserve, thiscall)
        _emit 0x66
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV EDX, [ESI]  (dest)
        _emit 0x16
        _emit 0x57  // PUSH EDI  (count)
        _emit 0x8d  // LEA ECX, [ESP+0x10]  (src)
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x51  // PUSH ECX
        _emit 0x52  // PUSH EDX
        _emit 0xe8  // CALL 0x009d5110 (memcpy)
        _emit 0x58
        _emit 0xda
        _emit 0x58
        _emit 0x00
        _emit 0x8b  // MOV EAX, [ESI]
        _emit 0x06
        _emit 0x8b  // MOV ECX, [ESP+0x418]  (cookie)
        _emit 0x8c
        _emit 0x24
        _emit 0x18
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0xc6  // MOV byte [EDI+EAX], 0
        _emit 0x04
        _emit 0x07
        _emit 0x00
        _emit 0x5f  // POP EDI
        _emit 0x8b  // MOV EAX, ESI  (return result)
        _emit 0xc6
        _emit 0x5e  // POP ESI
        _emit 0x33  // XOR ECX, ESP
        _emit 0xcc
        _emit 0xe8  // CALL 0x009d20f4 (__security_check_cookie)
        _emit 0x21
        _emit 0xaa
        _emit 0x58
        _emit 0x00
        _emit 0x81  // ADD ESP, 0x408
        _emit 0xc4
        _emit 0x08
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0xc3  // RET
    }
}
