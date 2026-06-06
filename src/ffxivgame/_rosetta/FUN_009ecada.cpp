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
// FUNCTION: ffxivgame 0x009ecada — FP-value classification + rescaling helper
//                                  (327 B / 0x147), no standard prologue.
//
// Inspection (read from disassembly at RVA 0x005ecada):
//
//   No PUSH EBP / MOV EBP,ESP prologue — this function operates entirely on
//   the caller's stack frame via EBP-relative addressing.  Calling convention
//   is effectively frameless-__cdecl; the plain `RET` at +0x6f returns to
//   the caller with no stack adjustment.
//
//   Logical shape (EBP offsets are the caller's frame slots):
//
//     if ([0x01363f14] != 0) goto epilogue;     // global-flag guard
//     FST [EBP-0x2D0]                            // spill current ST0
//     type = [EBP-0x90]                          // load type byte
//     switch (type) {
//       case 0:   // FPU control / status word path
//         ctrl = [EBP-0xA4] & 0x20;
//         if (ctrl != 0) goto epilogue;
//         if (FSTSW() & 0x20) { result = 8; goto cont; }
//         goto epilogue;
//       case 0xfe:  // check for denormal / infinity
//         exponent = [EBP-0x2CA] & 0x7FF0;
//         if (exponent == 0)    goto denormal_path;
//         if (exponent == 0x7FF0) goto infinity_path;
//         goto type0_path;
//       case 0xff:  // similar but only check for infinity
//         exponent = [EBP-0x2CA] & 0x7FF0;
//         if (exponent == 0x7FF0) goto infinity_path;
//         goto type0_path;
//       default:   // normal numeric type byte
//         result = signextend(type);
//         goto cont;
//     }
//     denormal_path:   result=4; scale and compare against threshold
//     infinity_path:   result=3; scale and compare against threshold
//   cont:
//     save/copy double args, call FUN_009f713a(mode, &result, &ctrl), restore
//   epilogue:
//     FLDCW [EBP-0xA4]; WAIT; RET
//
//   Reloc-bearing sites (absolute VAs baked into orig image):
//     +0x02  DIR32 → 0x01363f14  ([g_fpclass_guard])
//     +0x45  DIR32 → 0x0108c558  (scaling double A)
//     +0x55  DIR32 → 0x0108c548  (threshold double A)
//     +0x61  DIR32 → 0x0108c568  (multiplier double A)
//     +0x69  DIR32 → 0x0108c550  (scaling double B)
//     +0x79  DIR32 → 0x0108c540  (threshold double B)
//     +0x85  DIR32 → 0x0108c560  (multiplier double B)
//     +0xe5  REL32 → 0x009f713a  (CALL FUN_009f713a)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The absence of a frame prologue, the seven absolute DIR32 fixups into
//   .data/.rdata, the single outbound REL32 CALL, and the complex x87
//   FP classification logic make a source-level MSVC 2005 /O2 rewrite
//   highly unlikely to reproduce every branch and address byte-for-byte.
//   The naked-asm passthrough is the same approach taken by FUN_004014b0,
//   FUN_00408f10, FUN_00405080, and the rest of the _rosetta siblings.

extern "C" __declspec(naked) void FUN_009ecada() {
    __asm {
        // 005ecada  CMP dword ptr [0x01363f14],0x0
        _emit 0x83
        _emit 0x3d
        _emit 0x14
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0x00
        // 005ecae1  JNZ 0x009ecb32
        _emit 0x75
        _emit 0x4f
        // 005ecae3  FST double ptr [EBP+0xfffffd30]
        _emit 0xdd
        _emit 0x95
        _emit 0x30
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 005ecae9  MOV AL,byte ptr [EBP+0xffffff70]
        _emit 0x8a
        _emit 0x85
        _emit 0x70
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 005ecaef  OR AL,AL
        _emit 0x0a
        _emit 0xc0
        // 005ecaf1  JZ 0x009ecb0d
        _emit 0x74
        _emit 0x1a
        // 005ecaf3  CMP AL,0xff
        _emit 0x3c
        _emit 0xff
        // 005ecaf5  JZ 0x009ecb52
        _emit 0x74
        _emit 0x5b
        // 005ecaf7  CMP AL,0xfe
        _emit 0x3c
        _emit 0xfe
        // 005ecaf9  JZ 0x009ecb3a
        _emit 0x74
        _emit 0x3f
        // 005ecafb  OR AL,AL
        _emit 0x0a
        _emit 0xc0
        // 005ecafd  JZ 0x009ecb32
        _emit 0x74
        _emit 0x33
        // 005ecaff  MOVSX EAX,AL
        _emit 0x0f
        _emit 0xbe
        _emit 0xc0
        // 005ecb02  MOV dword ptr [EBP+0xffffff72],EAX
        _emit 0x89
        _emit 0x85
        _emit 0x72
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 005ecb08  JMP 0x009ecbbf
        _emit 0xe9
        _emit 0xb2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005ecb0d  MOV AX,word ptr [EBP+0xffffff5c]
        _emit 0x66
        _emit 0x8b
        _emit 0x85
        _emit 0x5c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 005ecb14  AND AX,0x20
        _emit 0x66
        _emit 0x83
        _emit 0xe0
        _emit 0x20
        // 005ecb18  JNZ 0x009ecb32
        _emit 0x75
        _emit 0x18
        // 005ecb1a  FSTSW AX
        _emit 0x9b
        _emit 0xdf
        _emit 0xe0
        // 005ecb1d  AND AX,0x20
        _emit 0x66
        _emit 0x83
        _emit 0xe0
        _emit 0x20
        // 005ecb21  JZ 0x009ecb32
        _emit 0x74
        _emit 0x0f
        // 005ecb23  MOV dword ptr [EBP+0xffffff72],0x8
        _emit 0xc7
        _emit 0x85
        _emit 0x72
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005ecb2d  JMP 0x009ecbbf
        _emit 0xe9
        _emit 0x8d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005ecb32  FLDCW word ptr [EBP+0xffffff5c]
        _emit 0xd9
        _emit 0xad
        _emit 0x5c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 005ecb38  WAIT
        _emit 0x9b
        // 005ecb39  RET
        _emit 0xc3
        // 005ecb3a  MOV AX,word ptr [EBP+0xfffffd36]
        _emit 0x66
        _emit 0x8b
        _emit 0x85
        _emit 0x36
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 005ecb41  AND AX,0x7ff0
        _emit 0x66
        _emit 0x25
        _emit 0xf0
        _emit 0x7f
        // 005ecb45  OR AX,AX
        _emit 0x66
        _emit 0x0b
        _emit 0xc0
        // 005ecb48  JZ 0x009ecb65
        _emit 0x74
        _emit 0x1b
        // 005ecb4a  CMP AX,0x7ff0
        _emit 0x66
        _emit 0x3d
        _emit 0xf0
        _emit 0x7f
        // 005ecb4e  JZ 0x009ecb93
        _emit 0x74
        _emit 0x43
        // 005ecb50  JMP 0x009ecb0d
        _emit 0xeb
        _emit 0xbb
        // 005ecb52  MOV AX,word ptr [EBP+0xfffffd36]
        _emit 0x66
        _emit 0x8b
        _emit 0x85
        _emit 0x36
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 005ecb59  AND AX,0x7ff0
        _emit 0x66
        _emit 0x25
        _emit 0xf0
        _emit 0x7f
        // 005ecb5d  CMP AX,0x7ff0
        _emit 0x66
        _emit 0x3d
        _emit 0xf0
        _emit 0x7f
        // 005ecb61  JZ 0x009ecb93
        _emit 0x74
        _emit 0x30
        // 005ecb63  JMP 0x009ecb0d
        _emit 0xeb
        _emit 0xa8
        // 005ecb65  MOV dword ptr [EBP+0xffffff72],0x4
        _emit 0xc7
        _emit 0x85
        _emit 0x72
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005ecb6f  FLD double ptr [0x0108c558]
        _emit 0xdd
        _emit 0x05
        _emit 0x58
        _emit 0xc5
        _emit 0x08
        _emit 0x01
        // 005ecb75  FXCH
        _emit 0xd9
        _emit 0xc9
        // 005ecb77  FSCALE
        _emit 0xd9
        _emit 0xfd
        // 005ecb79  FSTP ST1
        _emit 0xdd
        _emit 0xd9
        // 005ecb7b  FLD ST0
        _emit 0xd9
        _emit 0xc0
        // 005ecb7d  FABS
        _emit 0xd9
        _emit 0xe1
        // 005ecb7f  FCOMP double ptr [0x0108c548]
        _emit 0xdc
        _emit 0x1d
        _emit 0x48
        _emit 0xc5
        _emit 0x08
        _emit 0x01
        // 005ecb85  FSTSW AX
        _emit 0x9b
        _emit 0xdf
        _emit 0xe0
        // 005ecb88  SAHF
        _emit 0x9e
        // 005ecb89  JNC 0x009ecbbf
        _emit 0x73
        _emit 0x34
        // 005ecb8b  FMUL double ptr [0x0108c568]
        _emit 0xdc
        _emit 0x0d
        _emit 0x68
        _emit 0xc5
        _emit 0x08
        _emit 0x01
        // 005ecb91  JMP 0x009ecbbf
        _emit 0xeb
        _emit 0x2c
        // 005ecb93  MOV dword ptr [EBP+0xffffff72],0x3
        _emit 0xc7
        _emit 0x85
        _emit 0x72
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005ecb9d  FLD double ptr [0x0108c550]
        _emit 0xdd
        _emit 0x05
        _emit 0x50
        _emit 0xc5
        _emit 0x08
        _emit 0x01
        // 005ecba3  FXCH
        _emit 0xd9
        _emit 0xc9
        // 005ecba5  FSCALE
        _emit 0xd9
        _emit 0xfd
        // 005ecba7  FSTP ST1
        _emit 0xdd
        _emit 0xd9
        // 005ecba9  FLD ST0
        _emit 0xd9
        _emit 0xc0
        // 005ecbab  FABS
        _emit 0xd9
        _emit 0xe1
        // 005ecbad  FCOMP double ptr [0x0108c540]
        _emit 0xdc
        _emit 0x1d
        _emit 0x40
        _emit 0xc5
        _emit 0x08
        _emit 0x01
        // 005ecbb3  FSTSW AX
        _emit 0x9b
        _emit 0xdf
        _emit 0xe0
        // 005ecbb6  SAHF
        _emit 0x9e
        // 005ecbb7  JBE 0x009ecbbf
        _emit 0x76
        _emit 0x06
        // 005ecbb9  FMUL double ptr [0x0108c560]
        _emit 0xdc
        _emit 0x0d
        _emit 0x60
        _emit 0xc5
        _emit 0x08
        _emit 0x01
        // 005ecbbf  PUSH ESI
        _emit 0x56
        // 005ecbc0  PUSH EDI
        _emit 0x57
        // 005ecbc1  MOV EBX,dword ptr [EBP+0xffffff6c]
        _emit 0x8b
        _emit 0x9d
        _emit 0x6c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 005ecbc7  INC EBX
        _emit 0x43
        // 005ecbc8  MOV dword ptr [EBP+0xffffff76],EBX
        _emit 0x89
        _emit 0x9d
        _emit 0x76
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 005ecbce  TEST byte ptr [EBP+0xfffffd38],0x1
        _emit 0xf6
        _emit 0x85
        _emit 0x38
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x01
        // 005ecbd5  JNZ 0x009ecbf1
        _emit 0x75
        _emit 0x1a
        // 005ecbd7  CLD
        _emit 0xfc
        // 005ecbd8  LEA ESI,[EBP+0x8]
        _emit 0x8d
        _emit 0x75
        _emit 0x08
        // 005ecbdb  LEA EDI,[EBP+0xffffff7a]
        _emit 0x8d
        _emit 0xbd
        _emit 0x7a
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 005ecbe1  MOVSD
        _emit 0xa5
        // 005ecbe2  MOVSD
        _emit 0xa5
        // 005ecbe3  CMP byte ptr [EBX+0xc],0x1
        _emit 0x80
        _emit 0x7b
        _emit 0x0c
        _emit 0x01
        // 005ecbe7  JZ 0x009ecbf1
        _emit 0x74
        _emit 0x08
        // 005ecbe9  LEA ESI,[EBP+0x10]
        _emit 0x8d
        _emit 0x75
        _emit 0x10
        // 005ecbec  LEA EDI,[EBP-0x7e]
        _emit 0x8d
        _emit 0x7d
        _emit 0x82
        // 005ecbef  MOVSD
        _emit 0xa5
        // 005ecbf0  MOVSD
        _emit 0xa5
        // 005ecbf1  FSTP double ptr [EBP-0x76]
        _emit 0xdd
        _emit 0x5d
        _emit 0x8a
        // 005ecbf4  LEA EAX,[EBP+0xffffff72]
        _emit 0x8d
        _emit 0x85
        _emit 0x72
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 005ecbfa  LEA EBX,[EBP+0xffffff5c]
        _emit 0x8d
        _emit 0x9d
        _emit 0x5c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 005ecc00  PUSH EBX
        _emit 0x53
        // 005ecc01  PUSH EAX
        _emit 0x50
        // 005ecc02  MOV EBX,dword ptr [EBP+0xffffff6c]
        _emit 0x8b
        _emit 0x9d
        _emit 0x6c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 005ecc08  MOV AL,byte ptr [EBX+0xe]
        _emit 0x8a
        _emit 0x43
        _emit 0x0e
        // 005ecc0b  MOVSX EAX,AL
        _emit 0x0f
        _emit 0xbe
        _emit 0xc0
        // 005ecc0e  PUSH EAX
        _emit 0x50
        // 005ecc0f  CALL 0x009f713a
        _emit 0xe8
        _emit 0x26
        _emit 0xa5
        _emit 0x00
        _emit 0x00
        // 005ecc14  ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 005ecc17  POP EDI
        _emit 0x5f
        // 005ecc18  POP ESI
        _emit 0x5e
        // 005ecc19  FLD double ptr [EBP-0x76]
        _emit 0xdd
        _emit 0x45
        _emit 0x8a
        // 005ecc1c  JMP 0x009ecb32
        _emit 0xe9
        _emit 0x11
        _emit 0xff
        _emit 0xff
        _emit 0xff
    }
}
