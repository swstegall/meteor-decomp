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
// FUNCTION: ffxivgame 0x00019640 — FUN_00419640
//                                  (0xfb / 251 B, EH4-SEH wrapped, __cdecl).
//
// Behaviour read from asm/ffxivgame/00019640_FUN_00419640.s:
//
//   __cdecl void* FUN_00419640(void** out, int arg1, int arg2, int arg3)
//
//   EH4 prologue: PUSH -1 / PUSH scopetable(0xe55709) / PUSH FS:[0] /
//   SUB ESP,0x18 / PUSH EBX / PUSH ESI / cookie XOR ESP / LEA+install FS:[0].
//
//   Body:
//     trylevel = 0;
//     ECX = arg3; EAX = arg2; EBX = arg1;
//     // construct a local struct at [ESP+0x10] (base frame) from the three args
//     PUSH 0xf57bd8; store ECX; PUSH 0x10; LEA ECX,[ESP+0x18];
//     trylevel = 0; store EAX, EBX;
//     local_struct = FUN_0040e2d0(ECX, 0x10, 0xf57bd8);   // init local struct
//     PUSH local_struct_result; PUSH 0x38;
//     store result in [ESP+0x44];
//     p = FUN_00419c40(local_struct_result, 0x38);         // allocate 0x38-byte obj
//     ESI = p; ADD ESP,0x8;
//     store ESI at [ESP+0x38];
//     trylevel = 1;
//     if (p != NULL) {
//         FUN_0041b5b0(p, &local_struct);                  // ctor
//         p->vtable = 0xf58208;
//         ECX = p;
//     } else {
//         ECX = NULL;
//     }
//     ESI = *out;   // load output slot ptr
//     *ESI = ECX;   // store new obj (or NULL)
//     if (ECX->field_0x18 == 0) {
//         trylevel = 0; local_state = 1;
//         call ECX->vtable[0](1);                          // vtable dispatch
//         *ESI = NULL;
//         return ESI;           // success path
//     }
//     // failure path
//     trylevel = 0; local_state = 1;
//     tmp = [ESP+0x44];
//     if (tmp != NULL)
//         FUN_0041a570(0, arg1, 0, tmp);                   // cleanup
//     return ESI;
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function contains EH4 trylevel management, absolute data addresses
//   (0xf57bd8, 0xf58208 vtable), and PC-relative CALLs (0x0040e2d0,
//   0x00419c40, 0x0041b5b0, 0x0041a570) that cannot be reproduced by a
//   source-level C++ port in a standalone .obj without linker intervention.
//   Following the same strategy as FUN_00403a20 / FUN_00402a30 / FUN_004054d0:
//   emit all 251 bytes verbatim via _emit directives.

extern "C" __declspec(naked) void FUN_00419640() {
    __asm {
        // 00019640  PUSH -1
        _emit 0x6a
        _emit 0xff
        // 00019642  PUSH 0xe55709 (scope table)
        _emit 0x68
        _emit 0x09
        _emit 0x57
        _emit 0xe5
        _emit 0x00
        // 00019647  MOV EAX, FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001964d  PUSH EAX
        _emit 0x50
        // 0001964e  SUB ESP, 0x18
        _emit 0x83
        _emit 0xec
        _emit 0x18
        // 00019651  PUSH EBX
        _emit 0x53
        // 00019652  PUSH ESI
        _emit 0x56
        // 00019653  MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00019658  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 0001965a  PUSH EAX
        _emit 0x50
        // 0001965b  LEA EAX, [ESP+0x24]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0001965f  MOV FS:[0], EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00019665  MOV dword [ESP+0x0c], 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001966d  MOV ECX, [ESP+0x40]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        // 00019671  MOV EAX, [ESP+0x3c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        // 00019675  MOV EBX, [ESP+0x38]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x38
        // 00019679  PUSH 0xf57bd8
        _emit 0x68
        _emit 0xd8
        _emit 0x7b
        _emit 0xf5
        _emit 0x00
        // 0001967e  MOV [ESP+0x20], ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 00019682  PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 00019684  LEA ECX, [ESP+0x18]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00019688  MOV dword [ESP+0x34], 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00019690  MOV [ESP+0x20], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 00019694  MOV [ESP+0x28], EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x28
        // 00019698  CALL 0x0040e2d0
        _emit 0xe8
        _emit 0x33
        _emit 0x4c
        _emit 0xff
        _emit 0xff
        // 0001969d  PUSH EAX
        _emit 0x50
        // 0001969e  PUSH 0x38
        _emit 0x6a
        _emit 0x38
        // 000196a0  MOV [ESP+0x44], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x44
        // 000196a4  CALL 0x00419c40
        _emit 0xe8
        _emit 0x97
        _emit 0x05
        _emit 0x00
        _emit 0x00
        // 000196a9  MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 000196ab  ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 000196ae  MOV [ESP+0x38], ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x38
        // 000196b2  TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 000196b4  MOV dword [ESP+0x2c], 1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000196bc  JZ +0x16
        _emit 0x74
        _emit 0x16
        // 000196be  LEA EDX, [ESP+0x18]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 000196c2  PUSH EDX
        _emit 0x52
        // 000196c3  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000196c5  CALL 0x0041b5b0
        _emit 0xe8
        _emit 0xe6
        _emit 0x1e
        _emit 0x00
        _emit 0x00
        // 000196ca  MOV dword [ESI], 0xf58208
        _emit 0xc7
        _emit 0x06
        _emit 0x08
        _emit 0x82
        _emit 0xf5
        _emit 0x00
        // 000196d0  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000196d2  JMP +0x02
        _emit 0xeb
        _emit 0x02
        // 000196d4  XOR ECX, ECX
        _emit 0x33
        _emit 0xc9
        // 000196d6  MOV ESI, [ESP+0x34]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x34
        // 000196da  MOV [ESI], ECX
        _emit 0x89
        _emit 0x0e
        // 000196dc  CMP dword [ECX+0x18], 0
        _emit 0x83
        _emit 0x79
        _emit 0x18
        _emit 0x00
        // 000196e0  MOV dword [ESP+0x2c], 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000196e8  MOV dword [ESP+0x0c], 1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000196f0  JNZ +0x22
        _emit 0x75
        _emit 0x22
        // 000196f2  MOV EAX, [ECX]
        _emit 0x8b
        _emit 0x01
        // 000196f4  MOV EDX, [EAX]
        _emit 0x8b
        _emit 0x10
        // 000196f6  PUSH 1
        _emit 0x6a
        _emit 0x01
        // 000196f8  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000196fa  MOV dword [ESI], 0
        _emit 0xc7
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00019700  MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 00019702  MOV ECX, [ESP+0x24]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 00019706  MOV FS:[0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001970d  POP ECX
        _emit 0x59
        // 0001970e  POP ESI
        _emit 0x5e
        // 0001970f  POP EBX
        _emit 0x5b
        // 00019710  ADD ESP, 0x24
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        // 00019713  RET
        _emit 0xc3
        // 00019714  MOV EAX, [ESP+0x44]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x44
        // 00019718  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0001971a  JZ +0x0b
        _emit 0x74
        _emit 0x0b
        // 0001971c  PUSH EAX
        _emit 0x50
        // 0001971d  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0001971f  PUSH EBX
        _emit 0x53
        // 00019720  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 00019722  CALL 0x0041a570
        _emit 0xe8
        _emit 0x49
        _emit 0x0e
        _emit 0x00
        _emit 0x00
        // 00019727  MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 00019729  MOV ECX, [ESP+0x24]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 0001972d  MOV FS:[0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00019734  POP ECX
        _emit 0x59
        // 00019735  POP ESI
        _emit 0x5e
        // 00019736  POP EBX
        _emit 0x5b
        // 00019737  ADD ESP, 0x24
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        // 0001973a  RET
        _emit 0xc3
    }
}
