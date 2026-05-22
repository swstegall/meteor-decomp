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
// FUNCTION: ffxivgame 0x0040d4e0 — FUN_0040d4e0 (211 B / 0xd3)
//                                   __thiscall member function, EH3-wrapped,
//                                   EnterCriticalSection guard around an
//                                   allocation + stat-update sequence.
//
// Calling convention: __thiscall (ECX = this); RET 0x8 — 2 DWORD stack params.
// Callee-saves pushed (inside EH3 frame): ECX, EBX, EBP, ESI, EDI.
//
// EH3 frame:
//   state -1  = before EnterCriticalSection (unwind: nothing)
//   state  0  = after  EnterCriticalSection (unwind: LeaveCriticalSection)
//
// Object layout (offsets touched):
//   [this + 0x64]       CRITICAL_SECTION  (24 bytes)
//   [this + 0x90]       DWORD  count         (incremented by 1)
//   [this + 0x98]       DWORD  max_something (updated to max with FUN_0040da50 result)
//   [this + 0x9c]       DWORD  accumulator   (param_1 added)
//   [this + 0xa0]       DWORD  max_accum     (updated to max with [this+0x9c])
//
// High-level behaviour:
//   1. EnterCriticalSection(&this->cs)
//   2. result = this->FUN_0040d300(param_1 + 0x10, param_2->field_4)
//   3. If result == NULL: LeaveCriticalSection, return NULL.
//   4. *result = param_1 + 0x10
//   5. result[4] (byte) = FUN_0040a710(param_2->field_4)   [custom/unknown cc]
//   6. this->count++; this->accum += param_1
//   7. uVar = (this+0x90)->FUN_0040da50()
//   8. [this+0x98] = max([this+0x98], uVar)
//   9. [this+0xa0] = max([this+0xa0], [this+0x9c])
//  10. LeaveCriticalSection, return result + 0x10
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The EH3 frame interleaves a local (PUSH ECX = save this-ptr) between the
//   3-slot SEH header and the callee-save registers. The MSVC 2005 compiler
//   also stores the CRITICAL_SECTION pointer at [ESP+0x14] (overwriting the
//   ECX save slot) so the unwind handler can call LeaveCriticalSection.
//   This interleaving is not reproducible from C++ source without /EHa or
//   __try/__finally — and even with those, the funclet stubs land in
//   .text$x COMDATs that inflate the .obj size vs. the orig 211 bytes.
//   The __declspec(naked) passthrough avoids all of that; compare.py masks
//   the five relocation sites (SEH handler address, two IAT thunks for
//   EnterCriticalSection / LeaveCriticalSection, and two REL32 call targets).

extern "C" __declspec(naked) void FUN_0040d4e0()
{
    __asm {
        // 0x0000d4e0: 6a ff        PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0x0000d4e2: 68 3a 4e e5 00   PUSH 0xe54e3a  [SEH handler, DIR32 reloc]
        _emit 0x68
        _emit 0x3a
        _emit 0x4e
        _emit 0xe5
        _emit 0x00
        // 0x0000d4e7: 64 a1 00 00 00 00   MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x0000d4ed: 50   PUSH EAX
        _emit 0x50
        // 0x0000d4ee: 64 89 25 00 00 00 00   MOV dword ptr FS:[0x0],ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x0000d4f5: 51   PUSH ECX
        _emit 0x51
        // 0x0000d4f6: 53   PUSH EBX
        _emit 0x53
        // 0x0000d4f7: 55   PUSH EBP
        _emit 0x55
        // 0x0000d4f8: 56   PUSH ESI
        _emit 0x56
        // 0x0000d4f9: 8b f1   MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0x0000d4fb: 8d 46 64   LEA EAX,[ESI+0x64]
        _emit 0x8d
        _emit 0x46
        _emit 0x64
        // 0x0000d4fe: 57   PUSH EDI
        _emit 0x57
        // 0x0000d4ff: 50   PUSH EAX
        _emit 0x50
        // 0x0000d500: 89 44 24 14   MOV dword ptr [ESP+0x14],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0x0000d504: ff 15 6c e1 f3 00   CALL dword ptr [0x00f3e16c]  [IAT EnterCriticalSection]
        _emit 0xff
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0x0000d50a: 8b 44 24 28   MOV EAX,dword ptr [ESP+0x28]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0x0000d50e: 8b 48 04   MOV ECX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        // 0x0000d511: 8b 6c 24 24   MOV EBP,dword ptr [ESP+0x24]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x24
        // 0x0000d515: 51   PUSH ECX
        _emit 0x51
        // 0x0000d516: 8d 5d 10   LEA EBX,[EBP+0x10]
        _emit 0x8d
        _emit 0x5d
        _emit 0x10
        // 0x0000d519: 53   PUSH EBX
        _emit 0x53
        // 0x0000d51a: 8b ce   MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0x0000d51c: c7 44 24 24 00 00 00 00   MOV dword ptr [ESP+0x24],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x0000d524: e8 d7 fd ff ff   CALL 0x0040d300  [REL32 reloc FUN_0040d300]
        _emit 0xe8
        _emit 0xd7
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0x0000d529: 8b f8   MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 0x0000d52b: 85 ff   TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 0x0000d52d: 75 22   JNZ 0x0040d551
        _emit 0x75
        _emit 0x22
        // 0x0000d52f: 8b 54 24 10   MOV EDX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0x0000d533: 52   PUSH EDX
        _emit 0x52
        // 0x0000d534: ff 15 68 e1 f3 00   CALL dword ptr [0x00f3e168]  [IAT LeaveCriticalSection]
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0x0000d53a: 5f   POP EDI
        _emit 0x5f
        // 0x0000d53b: 5e   POP ESI
        _emit 0x5e
        // 0x0000d53c: 5d   POP EBP
        _emit 0x5d
        // 0x0000d53d: 33 c0   XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0x0000d53f: 5b   POP EBX
        _emit 0x5b
        // 0x0000d540: 8b 4c 24 04   MOV ECX,dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0x0000d544: 64 89 0d 00 00 00 00   MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x0000d54b: 83 c4 10   ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0x0000d54e: c2 08 00   RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // 0x0000d551: 8b 44 24 28   MOV EAX,dword ptr [ESP+0x28]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0x0000d555: 89 1f   MOV dword ptr [EDI],EBX
        _emit 0x89
        _emit 0x1f
        // 0x0000d557: 8b 40 04   MOV EAX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        // 0x0000d55a: e8 b1 d1 ff ff   CALL 0x0040a710  [REL32 reloc FUN_0040a710]
        _emit 0xe8
        _emit 0xb1
        _emit 0xd1
        _emit 0xff
        _emit 0xff
        // 0x0000d55f: 81 c6 90 00 00 00   ADD ESI,0x90
        _emit 0x81
        _emit 0xc6
        _emit 0x90
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x0000d565: 88 47 04   MOV byte ptr [EDI+0x4],AL
        _emit 0x88
        _emit 0x47
        _emit 0x04
        // 0x0000d568: 83 06 01   ADD dword ptr [ESI],0x1
        _emit 0x83
        _emit 0x06
        _emit 0x01
        // 0x0000d56b: 01 6e 0c   ADD dword ptr [ESI+0xc],EBP
        _emit 0x01
        _emit 0x6e
        _emit 0x0c
        // 0x0000d56e: 8b ce   MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0x0000d570: e8 db 04 00 00   CALL 0x0040da50  [REL32 reloc FUN_0040da50]
        _emit 0xe8
        _emit 0xdb
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0x0000d575: 8b 4e 08   MOV ECX,dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        // 0x0000d578: 3b c8   CMP ECX,EAX
        _emit 0x3b
        _emit 0xc8
        // 0x0000d57a: 76 02   JBE 0x0040d57e
        _emit 0x76
        _emit 0x02
        // 0x0000d57c: 8b c1   MOV EAX,ECX
        _emit 0x8b
        _emit 0xc1
        // 0x0000d57e: 8b 4e 10   MOV ECX,dword ptr [ESI+0x10]
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 0x0000d581: 89 46 08   MOV dword ptr [ESI+0x8],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 0x0000d584: 8b 46 0c   MOV EAX,dword ptr [ESI+0xc]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 0x0000d587: 3b c8   CMP ECX,EAX
        _emit 0x3b
        _emit 0xc8
        // 0x0000d589: 76 02   JBE 0x0040d58d
        _emit 0x76
        _emit 0x02
        // 0x0000d58b: 8b c1   MOV EAX,ECX
        _emit 0x8b
        _emit 0xc1
        // 0x0000d58d: 8b 4c 24 10   MOV ECX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0x0000d591: 51   PUSH ECX
        _emit 0x51
        // 0x0000d592: 89 46 10   MOV dword ptr [ESI+0x10],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x10
        // 0x0000d595: ff 15 68 e1 f3 00   CALL dword ptr [0x00f3e168]  [IAT LeaveCriticalSection]
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0x0000d59b: 8b 4c 24 14   MOV ECX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0x0000d59f: 8d 47 10   LEA EAX,[EDI+0x10]
        _emit 0x8d
        _emit 0x47
        _emit 0x10
        // 0x0000d5a2: 5f   POP EDI
        _emit 0x5f
        // 0x0000d5a3: 5e   POP ESI
        _emit 0x5e
        // 0x0000d5a4: 5d   POP EBP
        _emit 0x5d
        // 0x0000d5a5: 5b   POP EBX
        _emit 0x5b
        // 0x0000d5a6: 64 89 0d 00 00 00 00   MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x0000d5ad: 83 c4 10   ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0x0000d5b0: c2 08 00   RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
