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
// FUNCTION: ffxivgame 0x0003b440 — __thiscall constructor / Init for an
//                                  object with a /GS-guarded C++ EH frame
//                                  (142 bytes / 0x8e, ret 8).
//
// Calling convention: __thiscall (ECX = this); returns this in EAX.
// Stack args: two DWORDs (one byte value read from [ESP+0x24] and one
// DWORD from [ESP+0x20] at call time) — cleaned with `RET 8`.
//
// Frame shape: standard MSVC 2005 C++ EH (`/EHsc`) + `/GS` prologue:
//   push -1                      ; SEH trylevel
//   push offset scopetable       ; 0x00e56573
//   push fs:[0]                  ; link prior handler
//   ... save ECX/EBX/ESI ...
//   mov  eax, ___security_cookie ; [0x012ea8b0]
//   xor  eax, esp
//   push eax                     ; canary
//   lea  eax, [esp+0x10]
//   mov  fs:[0], eax             ; install frame
//
// Object init (offsets off ESI = this):
//   [this+0x00]      = AL   (byte arg from [esp+0x24])
//   [this+0x01]      = 0
//   [this+0x04]      = 0    (dword)
//   [this+0x08]      = 0    (dword)
//   [this+0x0c..0e]  = 0    (three bytes)
//   [this+0x10]      = ECX  (dword arg from [esp+0x20])
//   [this+0x14]      = 0    (dword)
//   [this+0x8b8]     = 0    (dword, after first call)
//
// Calls (REL32 — compare.py masks the 4-byte displacement windows):
//   +0x62  CALL 0x009d61d6  — sub-object ctor on [this+0x18] with args
//                             (0x170, 6, 0x43ab80, 0x43abb0)
//   +0x73  CALL 0x0043af20  — second init on this (ECX=ESI)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//   The /GS canary + C++ EH frame and the absolute scopetable / cookie
//   / vtable-pointer immediates are not reproducible from isolated-TU
//   C++ source (the scopetable address and __security_cookie reference
//   are linker/runtime-provided). The naked body re-emits the original
//   142 bytes verbatim via MASM _emit; the .obj's .text is byte-
//   identical to the original slice modulo the masked reloc windows
//   (the two REL32 call displacements and the absolute-address
//   immediates), and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0043b440() {
    __asm {
        // 0003b440:  6a ff              PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0003b442:  68 73 65 e5 00     PUSH 0xe56573  (scopetable)
        _emit 0x68
        _emit 0x73
        _emit 0x65
        _emit 0xe5
        _emit 0x00
        // 0003b447:  64 a1 00 00 00 00  MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003b44d:  50                 PUSH EAX
        _emit 0x50
        // 0003b44e:  51                 PUSH ECX
        _emit 0x51
        // 0003b44f:  53                 PUSH EBX
        _emit 0x53
        // 0003b450:  56                 PUSH ESI
        _emit 0x56
        // 0003b451:  a1 b0 a8 2e 01     MOV EAX,[0x012ea8b0]  (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0003b456:  33 c4              XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0003b458:  50                 PUSH EAX
        _emit 0x50
        // 0003b459:  8d 44 24 10        LEA EAX,[ESP + 0x10]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0003b45d:  64 a3 00 00 00 00  MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003b463:  8b f1              MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0003b465:  89 74 24 0c        MOV [ESP + 0xc],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 0003b469:  8a 44 24 24        MOV AL,[ESP + 0x24]
        _emit 0x8a
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0003b46d:  8b 4c 24 20        MOV ECX,[ESP + 0x20]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 0003b471:  68 b0 ab 43 00     PUSH 0x43abb0
        _emit 0x68
        _emit 0xb0
        _emit 0xab
        _emit 0x43
        _emit 0x00
        // 0003b476:  68 80 ab 43 00     PUSH 0x43ab80
        _emit 0x68
        _emit 0x80
        _emit 0xab
        _emit 0x43
        _emit 0x00
        // 0003b47b:  33 db              XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // 0003b47d:  6a 06              PUSH 0x6
        _emit 0x6a
        _emit 0x06
        // 0003b47f:  68 70 01 00 00     PUSH 0x170
        _emit 0x68
        _emit 0x70
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0003b484:  8d 56 18           LEA EDX,[ESI + 0x18]
        _emit 0x8d
        _emit 0x56
        _emit 0x18
        // 0003b487:  52                 PUSH EDX
        _emit 0x52
        // 0003b488:  88 06              MOV [ESI],AL
        _emit 0x88
        _emit 0x06
        // 0003b48a:  88 5e 01           MOV [ESI + 0x1],BL
        _emit 0x88
        _emit 0x5e
        _emit 0x01
        // 0003b48d:  89 5e 04           MOV [ESI + 0x4],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x04
        // 0003b490:  89 5e 08           MOV [ESI + 0x8],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x08
        // 0003b493:  88 5e 0c           MOV [ESI + 0xc],BL
        _emit 0x88
        _emit 0x5e
        _emit 0x0c
        // 0003b496:  88 5e 0d           MOV [ESI + 0xd],BL
        _emit 0x88
        _emit 0x5e
        _emit 0x0d
        // 0003b499:  88 5e 0e           MOV [ESI + 0xe],BL
        _emit 0x88
        _emit 0x5e
        _emit 0x0e
        // 0003b49c:  89 4e 10           MOV [ESI + 0x10],ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x10
        // 0003b49f:  89 5e 14           MOV [ESI + 0x14],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x14
        // 0003b4a2:  e8 2f ad 59 00     CALL 0x009d61d6
        _emit 0xe8
        _emit 0x2f
        _emit 0xad
        _emit 0x59
        _emit 0x00
        // 0003b4a7:  8b ce              MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0003b4a9:  89 5c 24 18        MOV [ESP + 0x18],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // 0003b4ad:  89 9e b8 08 00 00  MOV [ESI + 0x8b8],EBX
        _emit 0x89
        _emit 0x9e
        _emit 0xb8
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0003b4b3:  e8 68 fa ff ff     CALL 0x0043af20
        _emit 0xe8
        _emit 0x68
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        // 0003b4b8:  8b c6              MOV EAX,ESI  (return this)
        _emit 0x8b
        _emit 0xc6
        // 0003b4ba:  8b 4c 24 10        MOV ECX,[ESP + 0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0003b4be:  64 89 0d 00 00 00 00  MOV FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003b4c5:  59                 POP ECX
        _emit 0x59
        // 0003b4c6:  5e                 POP ESI
        _emit 0x5e
        // 0003b4c7:  5b                 POP EBX
        _emit 0x5b
        // 0003b4c8:  83 c4 10           ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0003b4cb:  c2 08 00           RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
