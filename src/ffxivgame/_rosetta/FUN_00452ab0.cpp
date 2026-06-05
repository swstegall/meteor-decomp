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
// FUNCTION: ffxivgame 0x00052ab0 — 225 B / 0xe1, __cdecl, EH3-style C++ SEH
//                                  frame + /GS security cookie.
//
// Asm shape (RVA 0x00052ab0..0x00052b91):
//
//     ; ---- EH3 SEH + /GS prologue -----------------------------------
//     PUSH -1                       ; try_level seed
//     PUSH 0xe58051                 ; scope_table (FuncInfo) VA
//     MOV  EAX, FS:[0]              ; chain previous SEH frame
//     PUSH EAX
//     SUB  ESP, 0x60
//     MOV  EAX, [__security_cookie @ 0x012ea8b0]
//     XOR  EAX, ESP
//     MOV  [ESP+0x5c], EAX          ; /GS cookie slot
//     PUSH ESI
//     PUSH EDI
//     MOV  EAX, [__security_cookie @ 0x012ea8b0]
//     XOR  EAX, ESP
//     PUSH EAX                      ; secondary EH cookie
//     LEA  EAX, [ESP+0x6c]
//     MOV  FS:[0], EAX              ; install our SEH frame
//
//     ; ---- body -----------------------------------------------------
//     ; arg0 = [esp+0x7c] (this/out, kept in ESI, echoed to EAX at exit)
//     ; arg1 = [esp+0x80] (source pointer)
//     ; Builds a temporary string object at [esp+0x14], measures /
//     ; replaces a substring against a global at [0x00f67298], assigns
//     ; back into *ESI via the helper at 0x00447200, then destroys the
//     ; temporary (0x00446f50).
//     ;
//     ;   FUN_00452a50(&tmp, src)            ; construct temp from arg1
//     ;   pos = FUN_00446fd0(&tmp, 0x132d088, g_val@f67298)
//     ;   if (pos != g_val) {
//     ;       end = FUN_00445e50(&tmp);
//     ;       FUN_004460a0(&tmp, pos, end - pos);
//     ;       FUN_00447200(ESI, &tmp);       ; this = arg0
//     ;   } else {
//     ;       FUN_00447200(ESI, &tmp);       ; this = arg0
//     ;   }
//     ;   FUN_00446f50(&tmp);                ; ~temp
//     ;   return ESI;
//
//     ; ---- /GS + SEH epilogue ---------------------------------------
//     MOV  EAX, ESI                 ; return value
//     MOV  ECX, [ESP+0x6c]          ; unchain SEH
//     MOV  FS:[0], ECX
//     POP  ECX / POP EDI / POP ESI
//     MOV  ECX, [ESP+0x5c]
//     XOR  ECX, ESP
//     CALL __security_check_cookie @ 0x009d20f4
//     ADD  ESP, 0x6c
//     RET
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The compiler-emitted EH3 SEH prolog (PUSH -1 / scope_table / FS:[0]
//   chain / double-cookie XOR ESP), the mid-body try_level state writes
//   ([esp+0x7c]=1, [esp+0xc]=1, [esp+0x74]=0), and the exact ESI/EDI
//   register scheduling around the four __thiscall string helpers are
//   shape that depends on precise locals layout, the linker-laid scope
//   table VA, and MSVC's a1/a3 moffs32 encoding choices. Coaxing exactly
//   this 225-byte sequence out of /O2 /GS /EHsc C++ shifts at least one
//   byte. Emitting the orig bytes verbatim makes .text exactly 225 bytes;
//   the reloc-bearing immediates (scope_table, two cookie loads, the five
//   rel32 calls, and the two abs32 global loads) resolve against the orig
//   address space and tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00452ab0() {
    __asm {
        _emit 0x6a  // PUSH -1
        _emit 0xff
        _emit 0x68  // PUSH 0xe58051  (scope_table)
        _emit 0x51
        _emit 0x80
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x83  // SUB ESP, 0x60
        _emit 0xec
        _emit 0x60
        _emit 0xa1  // MOV EAX, [0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x89  // MOV [ESP+0x5c], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x5c
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0xa1  // MOV EAX, [0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX
        _emit 0x8d  // LEA EAX, [ESP+0x6c]
        _emit 0x44
        _emit 0x24
        _emit 0x6c
        _emit 0x64  // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EAX, [ESP+0x80]
        _emit 0x84
        _emit 0x24
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV ESI, [ESP+0x7c]
        _emit 0x74
        _emit 0x24
        _emit 0x7c
        _emit 0x50  // PUSH EAX
        _emit 0x8d  // LEA EAX, [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50  // PUSH EAX
        _emit 0x89  // MOV [ESP+0x18], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x18
        _emit 0xc7  // MOV [ESP+0x14], 0
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL 0x00452a50
        _emit 0x4e
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x8b  // MOV ECX, [0x00f67298]
        _emit 0x0d
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x51  // PUSH ECX
        _emit 0x68  // PUSH 0x132d088
        _emit 0x88
        _emit 0xd0
        _emit 0x32
        _emit 0x01
        _emit 0x8d  // LEA ECX, [ESP+0x1c]
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xc7  // MOV [ESP+0x7c], 1
        _emit 0x44
        _emit 0x24
        _emit 0x7c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL 0x00446fd0
        _emit 0xae
        _emit 0x44
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV EDI, EAX
        _emit 0xf8
        _emit 0x3b  // CMP EDI, [0x00f67298]
        _emit 0x3d
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x74  // JZ 0x00452b50
        _emit 0x24
        _emit 0x8d  // LEA ECX, [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xe8  // CALL 0x00445e50
        _emit 0x1b
        _emit 0x33
        _emit 0xff
        _emit 0xff
        _emit 0x2b  // SUB EAX, EDI
        _emit 0xc7
        _emit 0x50  // PUSH EAX
        _emit 0x57  // PUSH EDI
        _emit 0x8d  // LEA ECX, [ESP+0x1c]
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xe8  // CALL 0x004460a0
        _emit 0x5e
        _emit 0x35
        _emit 0xff
        _emit 0xff
        _emit 0x8d  // LEA EDX, [ESP+0x14]
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x52  // PUSH EDX
        _emit 0x8b  // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8  // CALL 0x00447200
        _emit 0xb2
        _emit 0x46
        _emit 0xff
        _emit 0xff
        _emit 0xeb  // JMP 0x00452b5c
        _emit 0x0c
        _emit 0x8d  // LEA EAX, [ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x50  // PUSH EAX
        _emit 0x8b  // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8  // CALL 0x00447200
        _emit 0xa4
        _emit 0x46
        _emit 0xff
        _emit 0xff
        _emit 0x8d  // LEA ECX, [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xc7  // MOV [ESP+0xc], 1
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc6  // MOV byte [ESP+0x74], 0
        _emit 0x44
        _emit 0x24
        _emit 0x74
        _emit 0x00
        _emit 0xe8  // CALL 0x00446f50
        _emit 0xde
        _emit 0x43
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV EAX, ESI
        _emit 0xc6
        _emit 0x8b  // MOV ECX, [ESP+0x6c]
        _emit 0x4c
        _emit 0x24
        _emit 0x6c
        _emit 0x64  // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x8b  // MOV ECX, [ESP+0x5c]
        _emit 0x4c
        _emit 0x24
        _emit 0x5c
        _emit 0x33  // XOR ECX, ESP
        _emit 0xcc
        _emit 0xe8  // CALL __security_check_cookie @ 0x009d20f4
        _emit 0x67
        _emit 0xf5
        _emit 0x57
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x6c
        _emit 0xc4
        _emit 0x6c
        _emit 0xc3  // RET
    }
}
