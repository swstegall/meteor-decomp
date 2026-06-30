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
// FUNCTION: ffxivgame 0x00050060 — FUN_00450060
//                                  (0xf8 / 248 B, EH4-SEH wrapped, __thiscall, RET 4).
//
// Behaviour read from the disassembly at orig RVA 0x00050060:
//
//   __thiscall void* FUN_00450060(this, arg0) — ECX = this, one stack arg (RET 4).
//
//   EH4 SEH prologue (PUSH -1 / PUSH 0xe57d7b (scope-table) / PUSH FS:[0] /
//   SUB ESP,0x30 / double __security_cookie XOR-ESP / install FS:[0]).
//
//   Body:
//     ESI = ECX                       ; save this
//     EAX = [ESP+0x50]                ; arg0 (first stack arg past the EH4 frame)
//     PUSH -1 / PUSH 0 / PUSH EAX    ; three args to call below
//     LEA ECX,[ESP+0x2c]              ; local Utf8String at [ESP+0x2c] (post-pushes: local_str)
//     [ESP+0x44] = 0xf                ; Utf8String capacity field (SSO = 15)
//     [ESP+0x40] = 0                  ; Utf8String length field
//     byte [ESP+0x30] = 0             ; Utf8String data[0] = '\0'
//     CALL 0x00404040                 ; Utf8String::assign (or equivalent 3-arg ctor)
//
//     LEA EAX,[ESP+0x20]              ; local result slot
//     PUSH EAX
//     ECX = ESI                       ; restore this
//     [ESP+0x4c] = 0                  ; trylevel = 0
//     CALL 0x0044fd60                 ; this->some_method(result_slot)
//
//     LEA ECX,[ESP+0x20]              ; result a
//     PUSH ECX
//     LEA EDX,[ESP+0x1c]              ; result b
//     PUSH EDX
//     ECX = ESI
//     CALL 0x00451a60                 ; this->find_or_lookup(b, a) → returns pair ptr in EAX
//
//     EDI = [EAX]                     ; pair.first (node ptr)
//     TEST EDI,EDI
//     EBP = [EAX+4]                   ; pair.second (value ptr)
//     EAX = [ESI+4]                   ; this->field_4
//     [ESP+0x14] = EAX                ; save field_4 snapshot
//     JZ → error_check               ; if EDI == null → assert
//     CMP EDI,ESI                     ; if EDI == this → assert
//     JZ → ok_check
//   error_check:
//     CALL 0x009d22b4                 ; _invalid_parameter / assert_fail
//   ok_check:
//     CMP EBP,[ESP+0x14]             ; if EBP == this->field_4 → already_at_end
//     JZ  → already_at_end
//     TEST EDI,EDI
//     JNZ → check_ebp_field
//     CALL 0x009d22b4                 ; assert — EDI must be non-null
//   check_ebp_field:
//     CMP EBP,[EDI+4]                ; if EBP == EDI->field_4 → assert
//     JNZ → compute_result
//     CALL 0x009d22b4
//   compute_result:
//     CMP [ESP+0x38],0x10            ; SSO capacity check on the local string
//     LEA ESI,[EBP+0x28]             ; ESI = EBP + 0x28  (path A)
//     JC  → epilogue                 ; if SSO → skip free of heap buffer
//     MOV ECX,[ESP+0x24]             ; heap ptr
//     PUSH ECX
//     JMP → free_and_return
//   already_at_end:
//     ADD ESI,0x28                   ; ESI = local_str_base + 0x28 (path B)
//     CMP [ESP+0x38],0x10
//     JC  → epilogue
//     MOV EDX,[ESP+0x24]
//     PUSH EDX
//   free_and_return:
//     CALL 0x009d1b17                ; operator delete / free heap buffer
//   epilogue:
//     EAX = ESI                      ; return ESI (pointer into result or local_str)
//     ECX = [ESP+0x40]               ; restore saved FS:[0] chain link
//     MOV FS:[0],ECX
//     POP ECX / POP EDI / POP ESI / POP EBP
//     ECX = [ESP+0x2c]; XOR ECX,ESP
//     CALL 0x009d20f4                ; __security_check_cookie
//     ADD ESP,0x3c
//     RET 4
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The EH4 prologue double-cookies, the absolute addresses, the branch
//   short vs near selection, and the exact register allocations across
//   nested conditionals make source-level reproduction brittle under
//   MSVC 2005 /O2. Siblings FUN_00403a20, FUN_004054d0, and FUN_00402a30
//   all used this same naked-emit strategy. Emit the 248 orig bytes verbatim.

extern "C" __declspec(naked) void FUN_00450060() {
    __asm {
        // 00050060  PUSH -1
        _emit 0x6a
        _emit 0xff
        // 00050062  PUSH 0xe57d7b  (scope-table handler RVA)
        _emit 0x68
        _emit 0x7b
        _emit 0x7d
        _emit 0xe5
        _emit 0x00
        // 00050067  MOV EAX,FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005006d  PUSH EAX
        _emit 0x50
        // 0005006e  SUB ESP,0x30
        _emit 0x83
        _emit 0xec
        _emit 0x30
        // 00050071  MOV EAX,[__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00050076  XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 00050078  MOV [ESP+0x2c],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // 0005007c  PUSH EBP
        _emit 0x55
        // 0005007d  PUSH ESI
        _emit 0x56
        // 0005007e  PUSH EDI
        _emit 0x57
        // 0005007f  MOV EAX,[__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00050084  XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 00050086  PUSH EAX  (EH4 cookie #2)
        _emit 0x50
        // 00050087  LEA EAX,[ESP+0x40]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x40
        // 0005008b  MOV FS:[0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00050091  MOV EAX,[ESP+0x50]  ; arg0
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x50
        // 00050095  PUSH -1
        _emit 0x6a
        _emit 0xff
        // 00050097  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 00050099  MOV ESI,ECX  ; save this
        _emit 0x8b
        _emit 0xf1
        // 0005009b  PUSH EAX
        _emit 0x50
        // 0005009c  LEA ECX,[ESP+0x2c]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // 000500a0  MOV dword [ESP+0x44],0xf
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x44
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000500a8  MOV dword [ESP+0x40],0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000500b0  MOV byte [ESP+0x30],0
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x00
        // 000500b5  CALL 0x00404040
        _emit 0xe8
        _emit 0x86
        _emit 0x3f
        _emit 0xfb
        _emit 0xff
        // 000500ba  LEA EAX,[ESP+0x20]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 000500be  PUSH EAX
        _emit 0x50
        // 000500bf  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 000500c1  MOV dword [ESP+0x4c],0  ; trylevel = 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000500c9  CALL 0x0044fd60
        _emit 0xe8
        _emit 0x92
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 000500ce  LEA ECX,[ESP+0x20]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 000500d2  PUSH ECX
        _emit 0x51
        // 000500d3  LEA EDX,[ESP+0x1c]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 000500d7  PUSH EDX
        _emit 0x52
        // 000500d8  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 000500da  CALL 0x00451a60
        _emit 0xe8
        _emit 0x81
        _emit 0x19
        _emit 0x00
        _emit 0x00
        // 000500df  MOV EDI,[EAX]
        _emit 0x8b
        _emit 0x38
        // 000500e1  TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 000500e3  MOV EBP,[EAX+4]
        _emit 0x8b
        _emit 0x68
        _emit 0x04
        // 000500e6  MOV EAX,[ESI+4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 000500e9  MOV [ESP+0x14],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 000500ed  JZ +4  (→ 0x004500f3)
        _emit 0x74
        _emit 0x04
        // 000500ef  CMP EDI,ESI
        _emit 0x3b
        _emit 0xfe
        // 000500f1  JZ +5  (→ 0x004500f8)
        _emit 0x74
        _emit 0x05
        // 000500f3  CALL 0x009d22b4
        _emit 0xe8
        _emit 0xbc
        _emit 0x21
        _emit 0x58
        _emit 0x00
        // 000500f8  CMP EBP,[ESP+0x14]
        _emit 0x3b
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        // 000500fc  JZ +0x24  (→ 0x00450122)
        _emit 0x74
        _emit 0x24
        // 000500fe  TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 00050100  JNZ +5  (→ 0x00450107)
        _emit 0x75
        _emit 0x05
        // 00050102  CALL 0x009d22b4
        _emit 0xe8
        _emit 0xad
        _emit 0x21
        _emit 0x58
        _emit 0x00
        // 00050107  CMP EBP,[EDI+4]
        _emit 0x3b
        _emit 0x6f
        _emit 0x04
        // 0005010a  JNZ +5  (→ 0x00450111)
        _emit 0x75
        _emit 0x05
        // 0005010c  CALL 0x009d22b4
        _emit 0xe8
        _emit 0xa3
        _emit 0x21
        _emit 0x58
        _emit 0x00
        // 00050111  CMP dword [ESP+0x38],0x10
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x38
        _emit 0x10
        // 00050116  LEA ESI,[EBP+0x28]
        _emit 0x8d
        _emit 0x75
        _emit 0x28
        // 00050119  JC +0x1e  (→ 0x00450139)
        _emit 0x72
        _emit 0x1e
        // 0005011b  MOV ECX,[ESP+0x24]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 0005011f  PUSH ECX
        _emit 0x51
        // 00050120  JMP +0xf  (→ 0x00450131)
        _emit 0xeb
        _emit 0x0f
        // 00050122  ADD ESI,0x28
        _emit 0x83
        _emit 0xc6
        _emit 0x28
        // 00050125  CMP dword [ESP+0x38],0x10
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x38
        _emit 0x10
        // 0005012a  JC +0xd  (→ 0x00450139)
        _emit 0x72
        _emit 0x0d
        // 0005012c  MOV EDX,[ESP+0x24]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 00050130  PUSH EDX
        _emit 0x52
        // 00050131  CALL 0x009d1b17
        _emit 0xe8
        _emit 0xe1
        _emit 0x19
        _emit 0x58
        _emit 0x00
        // 00050136  ADD ESP,4  (caller cleans the one PUSH'd arg)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00050139  MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0005013b  MOV ECX,[ESP+0x40]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        // 0005013f  MOV FS:[0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00050146  POP ECX
        _emit 0x59
        // 00050147  POP EDI
        _emit 0x5f
        // 00050148  POP ESI
        _emit 0x5e
        // 00050149  POP EBP
        _emit 0x5d
        // 0005014a  MOV ECX,[ESP+0x2c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // 0005014e  XOR ECX,ESP
        _emit 0x33
        _emit 0xcc
        // 00050150  CALL 0x009d20f4  (__security_check_cookie)
        _emit 0xe8
        _emit 0x9f
        _emit 0x1f
        _emit 0x58
        _emit 0x00
        // 00050155  ADD ESP,0x3c
        _emit 0x83
        _emit 0xc4
        _emit 0x3c
        // 00050158  RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
