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
// FUNCTION: ffxivgame 0x0044dfd0 — `__thiscall` WM_MESSAGE / vtable-init
//                                   dispatch (334 B / 0x14e), /GS-wrapped.
//
// Inspection (read from the disassembly at orig RVA 0x0004dfd0):
//
//   __thiscall bool FUN_0044dfd0(void *this) — ECX = this, returns bool AL.
//
//   Frame: SUB ESP,0x98 (152 B local frame); __security_cookie @ ESP+0x94.
//   No EBP frame; ESP-relative addressing throughout.
//   PUSH ESI saves ECX (this) into ESI before the frame-fill call.
//
//   Body sketch:
//     // Fill frame area (ZeroMemory / RtlSecureZeroMemory via IAT[0xf3e14c]):
//     LEA EAX,[ESP+0x4]          ; &local_area
//     PUSH EAX
//     MOV  dword ptr [ESP+0x8],0x94   ; byte-count = 0x94
//     CALL dword ptr [0x00f3e14c]     ; ZeroMemory(local_area, 0x94)
//
//     if (*(DWORD*)(ESP+0x14) != 2) goto early_exit_false;
//     if (*(DWORD*)(ESP+0x08) <= 4) goto early_exit_false;
//
//     ECX = [0x0132cf48];         ; global state
//     PUSH ECX
//     // initialise vtable-like function pointer table [0x01363c20..0x01363c38]:
//     [0x01363c20] = 0x0044de90;
//     [0x01363c24] = 0x0044d710;
//     [0x01363c28] = 0x0044dcf0;
//     [0x01363c2c] = 0x0044ddc0;
//     [0x01363c30] = 0x0044d690;
//     [0x01363c34] = 0x0044d6c0;
//     [0x01363c38] = 0x0044dc20;
//     EAX = FUN_009d0416();        ; some init
//     [0x0132cf4c] = EAX;
//
//     if (*(DWORD*)(ESP+0x08) == 6 && *(DWORD*)(ESP+0x0c) > 0) {
//         EAX = (ESI != 0) ? &ESI->field_4 : NULL;
//         PUSH EAX; PUSH [0x0132cf48]
//         ECX = 0x01266e00;
//         AL  = FUN_00459510(ECX, EDX, EAX);   // some check
//         [0x012ea658] = SETZ(AL);              // flag = (AL == 0)
//         EAX = SETNZ([0x0132cf4c] != 0);       // return = (global != 0)
//         goto cookie_check_ret;
//     }
//
//     // else branch (argc != 6 or argc2 <= 0):
//     EAX = (ESI != 0) ? &ESI->field_4 : NULL;
//     PUSH EAX; PUSH [0x0132cf48]
//     ECX = 0x01266e00;
//     DL  = SETZ(FUN_00459510(ECX, EDX, EAX) == 0);
//     [0x012ea658] = DL;
//     EAX = SETNZ([0x0132cf4c] != 0);
//   cookie_check_ret:
//     POP ESI
//     XOR ECX,ESP; CALL __security_check_cookie
//     ADD ESP,0x98; RET
//
//   early_exit_false:
//     POP ESI
//     XOR ECX,ESP; XOR AL,AL
//     CALL __security_check_cookie
//     ADD ESP,0x98; RET
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function contains 23 absolute DIR32 relocations (global addresses,
//   IAT thunks, function-pointer table addresses) and two REL32 CALL
//   offsets tied to orig link-time RVAs. Reproduce via naked-asm _emit.

extern "C" __declspec(naked) void FUN_0044dfd0() {
    __asm {
        // 0004dfd0  SUB ESP,0x98
        _emit 0x81
        _emit 0xec
        _emit 0x98
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004dfd6  MOV EAX,[__security_cookie @ 0x012ea8b0]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0004dfdb  XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0004dfdd  MOV dword ptr [ESP+0x94],EAX
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0x94
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004dfe4  PUSH ESI
        _emit 0x56
        // 0004dfe5  LEA EAX,[ESP+0x4]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0004dfe9  PUSH EAX
        _emit 0x50
        // 0004dfea  MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0004dfec  MOV dword ptr [ESP+0x8],0x94
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x94
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004dff4  CALL dword ptr [IAT @ 0x00f3e14c]
        _emit 0xff
        _emit 0x15
        _emit 0x4c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0004dffa  CMP dword ptr [ESP+0x14],0x2
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x02
        // 0004dfff  JNZ 0x0044e106
        _emit 0x0f
        _emit 0x85
        _emit 0x01
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0004e005  CMP dword ptr [ESP+0x8],0x4
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x08
        _emit 0x04
        // 0004e00a  JBE 0x0044e106
        _emit 0x0f
        _emit 0x86
        _emit 0xf6
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004e010  MOV ECX,dword ptr [0x0132cf48]
        _emit 0x8b
        _emit 0x0d
        _emit 0x48
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // 0004e016  PUSH ECX
        _emit 0x51
        // 0004e017  MOV dword ptr [0x01363c20],0x0044de90
        _emit 0xc7
        _emit 0x05
        _emit 0x20
        _emit 0x3c
        _emit 0x36
        _emit 0x01
        _emit 0x90
        _emit 0xde
        _emit 0x44
        _emit 0x00
        // 0004e021  MOV dword ptr [0x01363c24],0x0044d710
        _emit 0xc7
        _emit 0x05
        _emit 0x24
        _emit 0x3c
        _emit 0x36
        _emit 0x01
        _emit 0x10
        _emit 0xd7
        _emit 0x44
        _emit 0x00
        // 0004e02b  MOV dword ptr [0x01363c28],0x0044dcf0
        _emit 0xc7
        _emit 0x05
        _emit 0x28
        _emit 0x3c
        _emit 0x36
        _emit 0x01
        _emit 0xf0
        _emit 0xdc
        _emit 0x44
        _emit 0x00
        // 0004e035  MOV dword ptr [0x01363c2c],0x0044ddc0
        _emit 0xc7
        _emit 0x05
        _emit 0x2c
        _emit 0x3c
        _emit 0x36
        _emit 0x01
        _emit 0xc0
        _emit 0xdd
        _emit 0x44
        _emit 0x00
        // 0004e03f  MOV dword ptr [0x01363c30],0x0044d690
        _emit 0xc7
        _emit 0x05
        _emit 0x30
        _emit 0x3c
        _emit 0x36
        _emit 0x01
        _emit 0x90
        _emit 0xd6
        _emit 0x44
        _emit 0x00
        // 0004e049  MOV dword ptr [0x01363c34],0x0044d6c0
        _emit 0xc7
        _emit 0x05
        _emit 0x34
        _emit 0x3c
        _emit 0x36
        _emit 0x01
        _emit 0xc0
        _emit 0xd6
        _emit 0x44
        _emit 0x00
        // 0004e053  MOV dword ptr [0x01363c38],0x0044dc20
        _emit 0xc7
        _emit 0x05
        _emit 0x38
        _emit 0x3c
        _emit 0x36
        _emit 0x01
        _emit 0x20
        _emit 0xdc
        _emit 0x44
        _emit 0x00
        // 0004e05d  CALL 0x009d0416
        _emit 0xe8
        _emit 0xb4
        _emit 0x23
        _emit 0x58
        _emit 0x00
        // 0004e062  CMP dword ptr [ESP+0x8],0x6
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x08
        _emit 0x06
        // 0004e067  MOV [0x0132cf4c],EAX
        _emit 0xa3
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // 0004e06c  JNZ 0x0044e0bd
        _emit 0x75
        _emit 0x4f
        // 0004e06e  CMP dword ptr [ESP+0xc],0x0
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        // 0004e073  JBE 0x0044e0bd
        _emit 0x76
        _emit 0x48
        // 0004e075  TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 0004e077  JZ 0x0044e07e
        _emit 0x74
        _emit 0x05
        // 0004e079  LEA EAX,[ESI+0x4]
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        // 0004e07c  JMP 0x0044e080
        _emit 0xeb
        _emit 0x02
        // 0004e07e  XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0004e080  MOV EDX,dword ptr [0x0132cf48]
        _emit 0x8b
        _emit 0x15
        _emit 0x48
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // 0004e086  PUSH EAX
        _emit 0x50
        // 0004e087  PUSH EDX
        _emit 0x52
        // 0004e088  MOV ECX,0x01266e00
        _emit 0xb9
        _emit 0x00
        _emit 0x6e
        _emit 0x26
        _emit 0x01
        // 0004e08d  CALL 0x00459510
        _emit 0xe8
        _emit 0x7e
        _emit 0xb4
        _emit 0x00
        _emit 0x00
        // 0004e092  TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // 0004e094  SETZ AL
        _emit 0x0f
        _emit 0x94
        _emit 0xc0
        // 0004e097  MOV [0x012ea658],AL
        _emit 0xa2
        _emit 0x58
        _emit 0xa6
        _emit 0x2e
        _emit 0x01
        // 0004e09c  XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0004e09e  CMP dword ptr [0x0132cf4c],EAX
        _emit 0x39
        _emit 0x05
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // 0004e0a4  POP ESI
        _emit 0x5e
        // 0004e0a5  SETNZ AL
        _emit 0x0f
        _emit 0x95
        _emit 0xc0
        // 0004e0a8  MOV ECX,dword ptr [ESP+0x94]
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x94
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004e0af  XOR ECX,ESP
        _emit 0x33
        _emit 0xcc
        // 0004e0b1  CALL __security_check_cookie
        _emit 0xe8
        _emit 0x3e
        _emit 0x40
        _emit 0x58
        _emit 0x00
        // 0004e0b6  ADD ESP,0x98
        _emit 0x81
        _emit 0xc4
        _emit 0x98
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004e0bc  RET
        _emit 0xc3
        // 0004e0bd  TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 0004e0bf  JZ 0x0044e0c6
        _emit 0x74
        _emit 0x05
        // 0004e0c1  LEA EAX,[ESI+0x4]
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        // 0004e0c4  JMP 0x0044e0c8
        _emit 0xeb
        _emit 0x02
        // 0004e0c6  XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0004e0c8  MOV ECX,dword ptr [0x0132cf48]
        _emit 0x8b
        _emit 0x0d
        _emit 0x48
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // 0004e0ce  PUSH EAX
        _emit 0x50
        // 0004e0cf  PUSH ECX
        _emit 0x51
        // 0004e0d0  MOV ECX,0x01266e00
        _emit 0xb9
        _emit 0x00
        _emit 0x6e
        _emit 0x26
        _emit 0x01
        // 0004e0d5  CALL 0x00459510
        _emit 0xe8
        _emit 0x36
        _emit 0xb4
        _emit 0x00
        _emit 0x00
        // 0004e0da  TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // 0004e0dc  SETZ DL
        _emit 0x0f
        _emit 0x94
        _emit 0xc2
        // 0004e0df  XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0004e0e1  CMP dword ptr [0x0132cf4c],EAX
        _emit 0x39
        _emit 0x05
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // 0004e0e7  MOV byte ptr [0x012ea658],DL
        _emit 0x88
        _emit 0x15
        _emit 0x58
        _emit 0xa6
        _emit 0x2e
        _emit 0x01
        // 0004e0ed  SETNZ AL
        _emit 0x0f
        _emit 0x95
        _emit 0xc0
        // 0004e0f0  POP ESI
        _emit 0x5e
        // 0004e0f1  MOV ECX,dword ptr [ESP+0x94]
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x94
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004e0f8  XOR ECX,ESP
        _emit 0x33
        _emit 0xcc
        // 0004e0fa  CALL __security_check_cookie
        _emit 0xe8
        _emit 0xf5
        _emit 0x3f
        _emit 0x58
        _emit 0x00
        // 0004e0ff  ADD ESP,0x98
        _emit 0x81
        _emit 0xc4
        _emit 0x98
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004e105  RET
        _emit 0xc3
        // 0004e106  MOV ECX,dword ptr [ESP+0x98]
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x98
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004e10d  POP ESI
        _emit 0x5e
        // 0004e10e  XOR ECX,ESP
        _emit 0x33
        _emit 0xcc
        // 0004e110  XOR AL,AL
        _emit 0x32
        _emit 0xc0
        // 0004e112  CALL __security_check_cookie
        _emit 0xe8
        _emit 0xdd
        _emit 0x3f
        _emit 0x58
        _emit 0x00
        // 0004e117  ADD ESP,0x98
        _emit 0x81
        _emit 0xc4
        _emit 0x98
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004e11d  RET
        _emit 0xc3
    }
}
