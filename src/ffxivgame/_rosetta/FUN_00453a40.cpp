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
// FUNCTION: ffxivgame 0x00053a40 — wide-string compare with SSO string unwrap
//                                  (227 B / 0xe3, __cdecl, /GS + EH3 SEH frame).
//
// Asm shape:
//
//   bool FUN_00453a40(arg0, arg1, arg2);
//
//   /GS + EH3-style SEH prologue:
//     PUSH -1                         ; SEH state = -1
//     PUSH scope_table (0xe58240)     ; EH3 scope table ptr
//     MOV  EAX, FS:[0]               ; save old FS:[0]
//     PUSH EAX
//     SUB  ESP, 0x50                  ; local frame
//     MOV  EAX, [__security_cookie]
//     XOR  EAX, ESP
//     MOV  [ESP+0x4c], EAX            ; primary /GS cookie slot
//     PUSH ESI
//     PUSH EDI
//     MOV  EAX, [__security_cookie]
//     XOR  EAX, ESP
//     PUSH EAX                        ; secondary EH cookie
//     LEA  EAX, [ESP+0x5c]
//     MOV  FS:[0], EAX                ; install EH frame
//
//   Body: constructs an SSO string on the stack, calls a comparison
//   helper (0x009d77a6), then on success returns (ESI, EDI) pair; on
//   failure returns (0, 0). If the buffer was heap-allocated (capacity
//   >= 8) a deallocation call (0x0044d350) is made on both paths.
//
// Reloc-bearing sites (wildcarded by compare.py):
//   +0x03  scope_table ptr          (0xe58240, .rdata)
//   +0x12  __security_cookie load   (0x012ea8b0, .data)
//   +0x1f  __security_cookie load   (0x012ea8b0, .data)
//   +0x51  CALL FUN_00449000        (rel32)
//   +0x67  CALL 0x009d77a6          (rel32)
//   +0x9c  CALL 0x0044d350          (rel32, success heap-free)
//   +0xbb  CALL 0x0044d350          (rel32, failure heap-free)
//   +0xdb  CALL __security_check_cookie (rel32, 0x009d20f4)

extern "C" __declspec(naked) void FUN_00453a40() {
    __asm {
        // /GS + EH3 prolog
        _emit 0x6a  // PUSH -1
        _emit 0xff
        _emit 0x68  // PUSH 0xe58240 (scope table)
        _emit 0x40
        _emit 0x82
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x83  // SUB ESP, 0x50
        _emit 0xec
        _emit 0x50
        _emit 0xa1  // MOV EAX, [0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x89  // MOV [ESP+0x4c], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0xa1  // MOV EAX, [0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX (secondary EH cookie)
        _emit 0x8d  // LEA EAX, [ESP+0x5c]
        _emit 0x44
        _emit 0x24
        _emit 0x5c
        _emit 0x64  // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // Body
        _emit 0x8b  // MOV ECX, [ESP+0x6c]  (arg0)
        _emit 0x4c
        _emit 0x24
        _emit 0x6c
        _emit 0x33  // XOR EAX, EAX
        _emit 0xc0
        _emit 0xc7  // MOV [ESP+0x54], 0x7
        _emit 0x44
        _emit 0x24
        _emit 0x54
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89  // MOV [ESP+0x50], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x50
        _emit 0x66  // MOV word ptr [ESP+0x40], AX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0x89  // MOV [ESP+0x64], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x64
        _emit 0x8d  // LEA EAX, [ESP+0x3c]
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL 0x00449000
        _emit 0x6b
        _emit 0x55
        _emit 0xff
        _emit 0xff
        // Post-call: check SSO size and conditionally use inline buffer
        _emit 0x83  // CMP [ESP+0x54], 0x8
        _emit 0x7c
        _emit 0x24
        _emit 0x54
        _emit 0x08
        _emit 0x8b  // MOV EAX, [ESP+0x40]
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0x73  // JNC +4 (skip LEA if size >= 8, use heap ptr)
        _emit 0x04
        _emit 0x8d  // LEA EAX, [ESP+0x40] (use inline buffer if size < 8)
        _emit 0x44
        _emit 0x24
        _emit 0x40
        // Compare call
        _emit 0x8d  // LEA ECX, [ESP+0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x51  // PUSH ECX
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL 0x009d77a6
        _emit 0xf7
        _emit 0x3c
        _emit 0x58
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x8b  // MOV EAX, [ESP+0x54]
        _emit 0x44
        _emit 0x24
        _emit 0x54
        _emit 0xc7  // MOV [ESP+0x64], -1
        _emit 0x44
        _emit 0x24
        _emit 0x64
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x75  // JNZ +0x27 (not equal → failure path)
        _emit 0x27
        // Success path
        _emit 0x83  // CMP EAX, 0x8
        _emit 0xf8
        _emit 0x08
        _emit 0x8b  // MOV ESI, [ESP+0x2c]
        _emit 0x74
        _emit 0x24
        _emit 0x2c
        _emit 0x8b  // MOV EDI, [ESP+0x30]
        _emit 0x7c
        _emit 0x24
        _emit 0x30
        _emit 0x72  // JC +0x14 (skip free if size < 8)
        _emit 0x14
        _emit 0x8d  // LEA EDX, [EAX+EAX+0x2]
        _emit 0x54
        _emit 0x00
        _emit 0x02
        _emit 0x8b  // MOV EAX, [ESP+0x40]
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0x6a  // PUSH 0xc
        _emit 0x0c
        _emit 0x52  // PUSH EDX
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL 0x0044d350
        _emit 0x70
        _emit 0x98
        _emit 0xff
        _emit 0xff
        _emit 0x83  // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x8b  // MOV EAX, ESI
        _emit 0xc6
        _emit 0x8b  // MOV EDX, EDI
        _emit 0xd7
        _emit 0xeb  // JMP +0x1d (to epilog)
        _emit 0x1d
        // Failure path
        _emit 0x83  // CMP EAX, 0x8
        _emit 0xf8
        _emit 0x08
        _emit 0x72  // JC +0x14 (skip free if size < 8)
        _emit 0x14
        _emit 0x8b  // MOV EDX, [ESP+0x40]
        _emit 0x54
        _emit 0x24
        _emit 0x40
        _emit 0x6a  // PUSH 0xc
        _emit 0x0c
        _emit 0x8d  // LEA ECX, [EAX+EAX+0x2]
        _emit 0x4c
        _emit 0x00
        _emit 0x02
        _emit 0x51  // PUSH ECX
        _emit 0x52  // PUSH EDX
        _emit 0xe8  // CALL 0x0044d350
        _emit 0x51
        _emit 0x98
        _emit 0xff
        _emit 0xff
        _emit 0x83  // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x33  // XOR EAX, EAX
        _emit 0xc0
        _emit 0x33  // XOR EDX, EDX
        _emit 0xd2
        // Epilog
        _emit 0x8b  // MOV ECX, [ESP+0x5c]  (old FS:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x5c
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
        _emit 0x8b  // MOV ECX, [ESP+0x4c]
        _emit 0x4c
        _emit 0x24
        _emit 0x4c
        _emit 0x33  // XOR ECX, ESP
        _emit 0xcc
        _emit 0xe8  // CALL __security_check_cookie (0x009d20f4)
        _emit 0xd5
        _emit 0xe5
        _emit 0x57
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x5c
        _emit 0xc4
        _emit 0x5c
        _emit 0xc3  // RET
    }
}
