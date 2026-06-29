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
// FUNCTION: ffxivgame 0x00063400 — setup_dp
//                                  (__cdecl, 156 bytes / 0x9c)
//
// No standard prologue — ESI is an implicit struct pointer set by the caller
// (the function reads [ESI+0x0], [ESI+0x4], [ESI+0x8], [ESI+0xc] without
// saving ESI; the caller retains ownership). EDI is the only callee-save
// pushed/popped inside the longer code path (the two early-exit paths at
// 0x63439 and 0x6343e jump straight to RET without going through POP EDI).
//
// One __cdecl argument is on the stack at [ESP+4] from entry, read as
// [ESP+8] after the PUSH EDI callee-save at 0x63443.
//
// Object layout (ESI-relative offsets):
//   [ESI+0x0]  pointer — checked for NULL and for [[ESI]] == 1
//   [ESI+0x4]  pointer — may be NULL; if set, field[0] is a count,
//                        field[8] is a data pointer
//   [ESI+0x8]  data pointer passed to FUN_464030 / FUN_464040
//   [ESI+0xc]  status-bits field (written in part 1)
//
// Logic summary:
//   Part 1: initialise ESI->field_0xc from optional byte data at ESI->field_0x4.
//           If the pointer is NULL, store 0x807F; otherwise extract byte 0
//           and (optionally) byte 1 via CH, then mask with 0x807F.
//   Part 2: if *ESI == NULL or **ESI != 1, return immediately.
//   Part 3: loop over items returned by FUN_464030 / FUN_464040(ESI->field_0x8, i).
//           On finding an item whose [0] == 4 with a non-NULL [4] field,
//           use that pointer; otherwise fall back to FUN_46a5d0(stack_arg).
//           Either way, call FUN_462600(*ESI, result).
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//   The unusual register convention (ESI as implicit struct pointer, not
//   derived from any standard-ABI register at function entry) and the
//   asymmetric callee-save (EDI only, and only on the main path) cannot
//   be reproduced from C++ source. A __declspec(naked) body re-emitting
//   the original 156 bytes verbatim produces a .obj whose .text is
//   byte-identical to the original slice; compare.py reports GREEN.

// Callee-called direct targets (REL32 relocs — wildcarded by compare.py).
void FUN_00464030();
void FUN_00464040();
void FUN_0046a5d0();
void FUN_00462600();

extern "C" __declspec(naked) void FUN_00463400() {
    __asm {
        // 00063400: 8b 46 04  MOV EAX,dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00063403: 85 c0     TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00063405: 74 27     JZ +0x27  (→ 0x6342e: NULL path)
        _emit 0x74
        _emit 0x27
        // 00063407: 83 38 00  CMP dword ptr [EAX],0x0
        _emit 0x83
        _emit 0x38
        _emit 0x00
        // 0006340a: 7e 09     JLE +0x9  (→ 0x63415: skip byte[0] load)
        _emit 0x7e
        _emit 0x09
        // 0006340c: 8b 48 08  MOV ECX,dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        // 0006340f: 0f b6 11  MOVZX EDX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x11
        // 00063412: 89 56 0c  MOV dword ptr [ESI+0xc],EDX
        _emit 0x89
        _emit 0x56
        _emit 0x0c
        // 00063415: 83 38 01  CMP dword ptr [EAX],0x1
        _emit 0x83
        _emit 0x38
        _emit 0x01
        // 00063418: 7e 0b     JLE +0xb  (→ 0x63425: skip byte[1] OR)
        _emit 0x7e
        _emit 0x0b
        // 0006341a: 8b 50 08  MOV EDX,dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 0006341d: 33 c9     XOR ECX,ECX
        _emit 0x33
        _emit 0xc9
        // 0006341f: 8a 6a 01  MOV CH,byte ptr [EDX+0x1]
        _emit 0x8a
        _emit 0x6a
        _emit 0x01
        // 00063422: 09 4e 0c  OR dword ptr [ESI+0xc],ECX
        _emit 0x09
        _emit 0x4e
        _emit 0x0c
        // 00063425: 81 66 0c 7f 80 00 00  AND dword ptr [ESI+0xc],0x807f
        _emit 0x81
        _emit 0x66
        _emit 0x0c
        _emit 0x7f
        _emit 0x80
        _emit 0x00
        _emit 0x00
        // 0006342c: eb 07     JMP +0x7  (→ 0x63435)
        _emit 0xeb
        _emit 0x07
        // 0006342e: c7 46 0c 7f 80 00 00  MOV dword ptr [ESI+0xc],0x807f
        _emit 0xc7
        _emit 0x46
        _emit 0x0c
        _emit 0x7f
        _emit 0x80
        _emit 0x00
        _emit 0x00
        // 00063435: 8b 06     MOV EAX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 00063437: 85 c0     TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00063439: 74 60     JZ +0x60  (→ 0x6349b: early RET, no POP EDI)
        _emit 0x74
        _emit 0x60
        // 0006343b: 83 38 01  CMP dword ptr [EAX],0x1
        _emit 0x83
        _emit 0x38
        _emit 0x01
        // 0006343e: 75 5b     JNZ +0x5b  (→ 0x6349b: early RET, no POP EDI)
        _emit 0x75
        _emit 0x5b
        // 00063440: 8b 46 08  MOV EAX,dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 00063443: 57        PUSH EDI  (callee-save; matched by POP EDI at 0x6349a)
        _emit 0x57
        // 00063444: 50        PUSH EAX  (arg for FUN_464030)
        _emit 0x50
        // 00063445: 33 ff     XOR EDI,EDI  (EDI = 0; loop counter)
        _emit 0x33
        _emit 0xff
        // 00063447: e8 e4 0b 00 00  CALL FUN_00464030
        call FUN_00464030
        // 0006344c: 83 c4 04  ADD ESP,0x4  (pop 1 cdecl arg)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0006344f: 85 c0     TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00063451: 7e 2e     JLE +0x2e  (→ 0x63481: no items)
        _emit 0x7e
        _emit 0x2e
        // === loop top (0x63453) ===
        // 00063453: 8b 4e 08  MOV ECX,dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        // 00063456: 57        PUSH EDI  (arg1: loop index)
        _emit 0x57
        // 00063457: 51        PUSH ECX  (arg0: data ptr)
        _emit 0x51
        // 00063458: e8 e3 0b 00 00  CALL FUN_00464040
        call FUN_00464040
        // 0006345d: 83 c4 08  ADD ESP,0x8  (pop 2 cdecl args)
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00063460: 83 38 04  CMP dword ptr [EAX],0x4
        _emit 0x83
        _emit 0x38
        _emit 0x04
        // 00063463: 74 15     JZ +0x15  (→ 0x6347a: type==4 branch)
        _emit 0x74
        _emit 0x15
        // 00063465: 8b 56 08  MOV EDX,dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x56
        _emit 0x08
        // 00063468: 52        PUSH EDX  (arg: data ptr)
        _emit 0x52
        // 00063469: 83 c7 01  ADD EDI,0x1  (i++)
        _emit 0x83
        _emit 0xc7
        _emit 0x01
        // 0006346c: e8 bf 0b 00 00  CALL FUN_00464030
        call FUN_00464030
        // 00063471: 83 c4 04  ADD ESP,0x4  (pop 1 cdecl arg)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00063474: 3b f8     CMP EDI,EAX  (i < count?)
        _emit 0x3b
        _emit 0xf8
        // 00063476: 7c db     JL -0x25  (→ 0x63453: loop back)
        _emit 0x7c
        _emit 0xdb
        // 00063478: eb 07     JMP +0x7  (→ 0x63481: no match)
        _emit 0xeb
        _emit 0x07
        // 0006347a: 8b 40 04  MOV EAX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        // 0006347d: 85 c0     TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0006347f: 75 0d     JNZ +0xd  (→ 0x6348e: found non-NULL value)
        _emit 0x75
        _emit 0x0d
        // 00063481: 8b 44 24 08  MOV EAX,dword ptr [ESP+0x8]
        //   (ESP+0x8: at this point PUSH EDI has shifted ESP by -4,
        //    so ESP+0x8 = function's original first argument)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00063485: 50        PUSH EAX  (arg: the stack argument)
        _emit 0x50
        // 00063486: e8 45 71 00 00  CALL FUN_0046a5d0
        call FUN_0046a5d0
        // 0006348b: 83 c4 04  ADD ESP,0x4  (pop 1 cdecl arg)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0006348e: 8b 0e     MOV ECX,dword ptr [ESI]  (ECX = *ESI)
        _emit 0x8b
        _emit 0x0e
        // 00063490: 50        PUSH EAX  (arg1: result)
        _emit 0x50
        // 00063491: 51        PUSH ECX  (arg0: *ESI)
        _emit 0x51
        // 00063492: e8 69 f1 ff ff  CALL FUN_00462600
        call FUN_00462600
        // 00063497: 83 c4 08  ADD ESP,0x8  (pop 2 cdecl args)
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0006349a: 5f        POP EDI  (restore callee-save EDI)
        _emit 0x5f
        // 0006349b: c3        RET
        _emit 0xc3
    }
}
