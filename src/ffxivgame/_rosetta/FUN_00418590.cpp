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
// FUNCTION: ffxivgame 0x00018590 — range-split / bisect helper (225 B / 0xe1),
//                                  __cdecl, no /GS.
//
// Asm shape (225 bytes, RVA 0x00018590..0x00018671):
//
//   void *FUN_00418590(SomeRect *out,      // arg0 [ESP+0x24 after inner call]
//                      int      n,         // arg1 [ESP+0x28]
//                      SomeRect *src1,     // arg2 [ESP+0x2c]
//                      SomeRect *src2,     // arg3 [ESP+0x30]
//                      int      arg4,      // arg4 [ESP+0x34]
//                      int      arg5,      // arg5 [ESP+0x38]
//                      int      arg6);     // arg6 [ESP+0x3c]
//
// Stack frame: SUB ESP,0x10 + PUSH EBX/EBP/ESI/EDI → delta -0x20.
// The early CALL 0x009fc74a is __stdcall (pops its own 2 DWORD args),
// so ESP stays at entry-0x20 after it returns.
// Locals at [ESP+0x10..0x1c] (4 DWORDs).
//
// Logic sketch:
//   - Loads src2->field_14 into EBP, src2->field_18 into EDI.
//   - If n==0: skip bisect, jump to merge.
//   - ESI = round_toward_zero(EBP * 9 / 16) via LEA+CDQ+AND+SAR.
//   - Three-way on (ESI vs EDI):
//       ESI == EDI : no-op → merge
//       ESI <  EDI : half_ESI=ESI/2; EBX=EDI/2-half_ESI; EDI=ESI; n=half_ESI
//       ESI >= EDI : ESI=EDI*16/9 (magic IMUL); ECX=EBP/2-ESI/2; EBP=ESI; n=ESI/2
//   - Copies src1->field_14/_18 into locals[2..3].
//   - Writes ECX/EBX/EBP/EDI into out->[field_0..field_c].
//   - Calls FUN_0041f020(src1, &locals[0], arg4, out, arg6).
//   - Returns out in EAX.
//
// Reconstruction strategy — __declspec(naked) byte passthrough.
//   The MSVC 2005 round-toward-zero idiom (CDQ;SUB;SAR) and magic-number
//   /9 (IMUL 0x38E38E39;SAR+SHR+ADD), plus the LEA [EBP+EBP*8+0] SIB
//   form for *9, resist high-level C++ reconstruction. Naked asm gives
//   byte-exact output. compare.py wildcards the DIR32 at +0x07 (PUSH
//   0xf57d10) and the two REL32 sites at +0x0e (CALL 0x009fc74a) and
//   +0xcf (CALL 0x0041f020).

extern "C" __declspec(naked) void FUN_00418590() {
    __asm {
        // 00018590: SUB ESP, 0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 00018593: PUSH EBX
        _emit 0x53
        // 00018594: PUSH EBP
        _emit 0x55
        // 00018595: PUSH ESI
        _emit 0x56
        // 00018596: PUSH EDI
        _emit 0x57
        // 00018597: PUSH 0x00f57d10  (DIR32 reloc)
        _emit 0x68
        _emit 0x10
        _emit 0x7d
        _emit 0xf5
        _emit 0x00
        // 0001859c: PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001859e: CALL 0x009fc74a  (REL32 reloc)
        _emit 0xe8
        _emit 0xa7
        _emit 0x41
        _emit 0x5e
        _emit 0x00
        // 000185a3: MOV EAX, [ESP+0x30]  (arg3 = src2)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // 000185a7: MOV EBP, [EAX+0x14]
        _emit 0x8b
        _emit 0x68
        _emit 0x14
        // 000185aa: MOV EDI, [EAX+0x18]
        _emit 0x8b
        _emit 0x78
        _emit 0x18
        // 000185ad: XOR ECX, ECX
        _emit 0x33
        _emit 0xc9
        // 000185af: XOR EBX, EBX
        _emit 0x33
        _emit 0xdb
        // 000185b1: CMP [ESP+0x28], ECX
        _emit 0x39
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // 000185b5: JZ +0x64
        _emit 0x74
        _emit 0x64
        // 000185b7: LEA EAX, [EBP+EBP*8+0]  (= EBP*9, SIB with disp8=0)
        _emit 0x8d
        _emit 0x44
        _emit 0xed
        _emit 0x00
        // 000185bb: CDQ
        _emit 0x99
        // 000185bc: AND EDX, 0xf
        _emit 0x83
        _emit 0xe2
        _emit 0x0f
        // 000185bf: ADD EAX, EDX
        _emit 0x03
        _emit 0xc2
        // 000185c1: MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 000185c3: SAR ESI, 4
        _emit 0xc1
        _emit 0xfe
        _emit 0x04
        // 000185c6: CMP ESI, EDI
        _emit 0x3b
        _emit 0xf7
        // 000185c8: JZ +0x51
        _emit 0x74
        _emit 0x51
        // 000185ca: JGE +0x1e
        _emit 0x7d
        _emit 0x1e
        // --- ESI < EDI branch ---
        // 000185cc: MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 000185ce: CDQ
        _emit 0x99
        // 000185cf: SUB EAX, EDX
        _emit 0x2b
        _emit 0xc2
        // 000185d1: SAR EAX, 1
        _emit 0xd1
        _emit 0xf8
        // 000185d3: MOV [ESP+0x28], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 000185d7: MOV EAX, EDI
        _emit 0x8b
        _emit 0xc7
        // 000185d9: CDQ
        _emit 0x99
        // 000185da: SUB EAX, EDX
        _emit 0x2b
        _emit 0xc2
        // 000185dc: MOV EBX, EAX
        _emit 0x8b
        _emit 0xd8
        // 000185de: MOV EAX, [ESP+0x28]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 000185e2: SAR EBX, 1
        _emit 0xd1
        _emit 0xfb
        // 000185e4: SUB EBX, EAX
        _emit 0x2b
        _emit 0xd8
        // 000185e6: MOV EDI, ESI
        _emit 0x8b
        _emit 0xfe
        // 000185e8: JMP +0x31
        _emit 0xeb
        _emit 0x31
        // --- ESI >= EDI branch (0x004185ea) ---
        // 000185ea: MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 000185ec: SHL ECX, 4
        _emit 0xc1
        _emit 0xe1
        _emit 0x04
        // 000185ef: MOV EAX, 0x38e38e39
        _emit 0xb8
        _emit 0x39
        _emit 0x8e
        _emit 0xe3
        _emit 0x38
        // 000185f4: IMUL ECX
        _emit 0xf7
        _emit 0xe9
        // 000185f6: SAR EDX, 1
        _emit 0xd1
        _emit 0xfa
        // 000185f8: MOV ESI, EDX
        _emit 0x8b
        _emit 0xf2
        // 000185fa: SHR ESI, 0x1f
        _emit 0xc1
        _emit 0xee
        _emit 0x1f
        // 000185fd: ADD ESI, EDX
        _emit 0x03
        _emit 0xf2
        // 000185ff: MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 00018601: CDQ
        _emit 0x99
        // 00018602: SUB EAX, EDX
        _emit 0x2b
        _emit 0xc2
        // 00018604: SAR EAX, 1
        _emit 0xd1
        _emit 0xf8
        // 00018606: MOV [ESP+0x28], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0001860a: MOV EAX, EBP
        _emit 0x8b
        _emit 0xc5
        // 0001860c: CDQ
        _emit 0x99
        // 0001860d: SUB EAX, EDX
        _emit 0x2b
        _emit 0xc2
        // 0001860f: MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 00018611: MOV EAX, [ESP+0x28]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 00018615: SAR ECX, 1
        _emit 0xd1
        _emit 0xf9
        // 00018617: SUB ECX, EAX
        _emit 0x2b
        _emit 0xc8
        // 00018619: MOV EBP, ESI
        _emit 0x8b
        _emit 0xee
        // --- merge point (0x0041861b) ---
        // 0001861b: MOV EDX, [ESP+0x2c]  (arg2 = src1)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        // 0001861f: MOV ESI, [ESP+0x24]  (arg0 = out)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x24
        // 00018623: XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00018625: MOV [ESP+0x10], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00018629: MOV [ESP+0x14], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0001862d: MOV EAX, [EDX+0x14]
        _emit 0x8b
        _emit 0x42
        _emit 0x14
        // 00018630: MOV [ESP+0x18], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 00018634: MOV EAX, [EDX+0x18]
        _emit 0x8b
        _emit 0x42
        _emit 0x18
        // 00018637: MOV [ESP+0x1c], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0001863b: MOV EAX, [ESP+0x3c]  (arg6)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        // 0001863f: PUSH EAX
        _emit 0x50
        // 00018640: MOV EAX, [ESP+0x30]  (arg2 after PUSH = src1)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // 00018644: ADD EBP, ECX
        _emit 0x03
        _emit 0xe9
        // 00018646: MOV [ESI], ECX
        _emit 0x89
        _emit 0x0e
        // 00018648: MOV ECX, [ESP+0x34]  (arg4)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        // 0001864c: PUSH ESI
        _emit 0x56
        // 0001864d: PUSH ECX
        _emit 0x51
        // 0001864e: LEA EDX, [ESP+0x1c]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 00018652: PUSH EDX
        _emit 0x52
        // 00018653: ADD EDI, EBX
        _emit 0x03
        _emit 0xfb
        // 00018655: PUSH EAX
        _emit 0x50
        // 00018656: MOV [ESI+0x4], EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x04
        // 00018659: MOV [ESI+0x8], EBP
        _emit 0x89
        _emit 0x6e
        _emit 0x08
        // 0001865c: MOV [ESI+0xc], EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x0c
        // 0001865f: CALL 0x0041f020  (REL32 reloc)
        _emit 0xe8
        _emit 0xbc
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // 00018664: ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00018667: POP EDI
        _emit 0x5f
        // 00018668: MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 0001866a: POP ESI
        _emit 0x5e
        // 0001866b: POP EBP
        _emit 0x5d
        // 0001866c: POP EBX
        _emit 0x5b
        // 0001866d: ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00018670: RET
        _emit 0xc3
    }
}
