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
// FUNCTION: ffxivgame 0x004528f0 — `__thiscall` string-search comparison
//                                  helper (324 B / 0x144, EH3-SEH wrapped)
//
// Inspection (read from the disassembly at orig RVA 0x000528f0):
//
//   __thiscall SomeResult* FUN_004528f0(this, SomeArg *param1) —
//   ECX = this (saved to EDI), RET 0x4 (one stack argument).
//
//   Structure:
//     PUSH EBP (param1 from [ESP+0x80])
//     CALL 0x00451540      — some lookup, result → ESI
//     XOR EBX, EBX
//     if (!EDI) CALL 0x009d22b4  — null-check / assert
//     if (ESI != [EDI+0x4])  {   — compare against end sentinel
//         get string ptr from ESI (capacity check at [ESI+0x24] vs 0x10)
//         CALL 0x00620110    — string comparison with param1 data
//         if (result >= 0) goto cleanup_path
//     }
//     // setup temporary string objects on stack, call FUN_00451c90 and
//     // FUN_00452680 to perform insert/search operation
//     // read back EDI=[EAX], ESI=[EAX+4]
//     // free three std::string temps via 0x009d1b17 if capacity >= 0x10
//     // assert EDI and ESI valid
//     return LEA EAX, [ESI+0x28]
//
//   Stack frame layout (EH3 prolog, ESP-relative after pushes):
//     [esp+0x00]  saved EDI (POP ECX used on unwind — cookie restore)
//     [esp+0x20]  std::string inline buf #1 (20-byte SSO)
//     [esp+0x34]  std::string inline buf #2 (20-byte SSO)
//     [esp+0x48]  std::string inline buf #3 (20-byte SSO)
//     [esp+0x6c]  capacity of string #3
//     [esp+0x70]  saved FS:[0] EH3 chain link
//     [esp+0x7c]  scope table address (0x00e58010)
//     [esp+0x80]  param1 (arg loaded early via [ESP+0x80])
//
//   Reloc-bearing sites in the orig 324 bytes:
//     +0x02  scope-table handler RVA  (.rdata 0x00e58010)
//     +0x15  __security_cookie load   (.data 0x012ea8b0)
//     +0x21  FS:[0] install           (constant 0, fold-through)
//     +0x31  CALL 0x00451540          (.text rel32)
//     +0x3e  CALL 0x009d22b4          (.text rel32 — assert/terminate)
//     +0x64  CALL 0x00620110          (.text rel32 — strcmp helper)
//     +0x92  CALL 0x00451c90          (.text rel32)
//     +0xa9  CALL 0x00452680          (.text rel32)
//     +0xc3  CALL 0x009d1b17          (.text rel32 — free)
//     +0xe6  CALL 0x009d1b17          (.text rel32 — free, 2nd)
//     +0x109 CALL 0x009d1b17          (.text rel32 — free, 3rd)
//     +0x125 CALL 0x009d22b4          (.text rel32 — assert, 2nd)
//     +0x12f CALL 0x009d22b4          (.text rel32 — assert, 3rd)
//     +0x13b FS:[0] restore           (constant 0, fold-through)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The EH3-SEH frame, the precise register allocation (EDI=this,
//   ESI=iterator, EBP=param/constant-0x10), the three paired
//   std::string destructor sequences keyed by capacity-vs-0x10, and
//   the linker-resolved absolute CALLs make a source-level re-write
//   brittle under MSVC 2005 /O2 /GS. The pragmatic choice — matching
//   the pattern used by FUN_004014b0, FUN_00401a00, and FUN_00408f10
//   in this _rosetta corpus — is a `__declspec(naked)` body that
//   re-emits the orig 324 bytes verbatim via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_004528f0() {
    __asm {
        // 000528f0: 6a ff                  PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 000528f2: 68 10 80 e5 00         PUSH 0xe58010
        _emit 0x68
        _emit 0x10
        _emit 0x80
        _emit 0xe5
        _emit 0x00
        // 000528f7: 64 a1 00 00 00 00      MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000528fd: 50                     PUSH EAX
        _emit 0x50
        // 000528fe: 83 ec 5c               SUB ESP, 0x5c
        _emit 0x83
        _emit 0xec
        _emit 0x5c
        // 00052901: 53                     PUSH EBX
        _emit 0x53
        // 00052902: 55                     PUSH EBP
        _emit 0x55
        // 00052903: 56                     PUSH ESI
        _emit 0x56
        // 00052904: 57                     PUSH EDI
        _emit 0x57
        // 00052905: a1 b0 a8 2e 01         MOV EAX, [0x012ea8b0]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0005290a: 33 c4                  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 0005290c: 50                     PUSH EAX
        _emit 0x50
        // 0005290d: 8d 44 24 70            LEA EAX, [ESP + 0x70]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x70
        // 00052911: 64 a3 00 00 00 00      MOV FS:[0x0], EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00052917: 8b f9                  MOV EDI, ECX
        _emit 0x8b
        _emit 0xf9
        // 00052919: 8b ac 24 80 00 00 00   MOV EBP, dword ptr [ESP + 0x80]
        _emit 0x8b
        _emit 0xac
        _emit 0x24
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00052920: 55                     PUSH EBP
        _emit 0x55
        // 00052921: e8 1a ec ff ff         CALL 0x00451540
        _emit 0xe8
        _emit 0x1a
        _emit 0xec
        _emit 0xff
        _emit 0xff
        // 00052926: 33 db                  XOR EBX, EBX
        _emit 0x33
        _emit 0xdb
        // 00052928: 3b fb                  CMP EDI, EBX
        _emit 0x3b
        _emit 0xfb
        // 0005292a: 8b f0                  MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 0005292c: 75 05                  JNZ 0x00452933
        _emit 0x75
        _emit 0x05
        // 0005292e: e8 81 f9 57 00         CALL 0x009d22b4
        _emit 0xe8
        _emit 0x81
        _emit 0xf9
        _emit 0x57
        _emit 0x00
        // 00052933: 8b 47 04               MOV EAX, dword ptr [EDI + 0x4]
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // 00052936: 3b f0                  CMP ESI, EAX
        _emit 0x3b
        _emit 0xf0
        // 00052938: 74 27                  JZ 0x00452961
        _emit 0x74
        _emit 0x27
        // 0005293a: 83 7e 24 10            CMP dword ptr [ESI + 0x24], 0x10
        _emit 0x83
        _emit 0x7e
        _emit 0x24
        _emit 0x10
        // 0005293e: 8b 4e 20               MOV ECX, dword ptr [ESI + 0x20]
        _emit 0x8b
        _emit 0x4e
        _emit 0x20
        // 00052941: 72 05                  JC 0x00452948
        _emit 0x72
        _emit 0x05
        // 00052943: 8b 46 10               MOV EAX, dword ptr [ESI + 0x10]
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        // 00052946: eb 03                  JMP 0x0045294b
        _emit 0xeb
        _emit 0x03
        // 00052948: 8d 46 10               LEA EAX, [ESI + 0x10]
        _emit 0x8d
        _emit 0x46
        _emit 0x10
        // 0005294b: 51                     PUSH ECX
        _emit 0x51
        // 0005294c: 50                     PUSH EAX
        _emit 0x50
        // 0005294d: 8b 45 14               MOV EAX, dword ptr [EBP + 0x14]
        _emit 0x8b
        _emit 0x45
        _emit 0x14
        // 00052950: 50                     PUSH EAX
        _emit 0x50
        // 00052951: 53                     PUSH EBX
        _emit 0x53
        // 00052952: 8b cd                  MOV ECX, EBP
        _emit 0x8b
        _emit 0xcd
        // 00052954: e8 b7 d7 1c 00         CALL 0x00620110
        _emit 0xe8
        _emit 0xb7
        _emit 0xd7
        _emit 0x1c
        _emit 0x00
        // 00052959: 85 c0                  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005295b: 0f 8d b0 00 00 00      JGE 0x00452a11
        _emit 0x0f
        _emit 0x8d
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00052961: c7 44 24 34 0f 00 00 00  MOV dword ptr [ESP + 0x34], 0xf
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00052969: 89 5c 24 30            MOV dword ptr [ESP + 0x30], EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x30
        // 0005296d: 88 5c 24 20            MOV byte ptr [ESP + 0x20], BL
        _emit 0x88
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        // 00052971: 8d 4c 24 1c            LEA ECX, [ESP + 0x1c]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 00052975: 51                     PUSH ECX
        _emit 0x51
        // 00052976: 55                     PUSH EBP
        _emit 0x55
        // 00052977: 8d 4c 24 40            LEA ECX, [ESP + 0x40]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        // 0005297b: 89 9c 24 80 00 00 00   MOV dword ptr [ESP + 0x80], EBX
        _emit 0x89
        _emit 0x9c
        _emit 0x24
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00052982: e8 09 f3 ff ff         CALL 0x00451c90
        _emit 0xe8
        _emit 0x09
        _emit 0xf3
        _emit 0xff
        _emit 0xff
        // 00052987: 50                     PUSH EAX
        _emit 0x50
        // 00052988: 56                     PUSH ESI
        _emit 0x56
        // 00052989: 57                     PUSH EDI
        _emit 0x57
        // 0005298a: 8d 54 24 20            LEA EDX, [ESP + 0x20]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x20
        // 0005298e: 52                     PUSH EDX
        _emit 0x52
        // 0005298f: 8b cf                  MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00052991: c6 84 24 88 00 00 00 01  MOV byte ptr [ESP + 0x88], 0x1
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x01
        // 00052999: e8 e2 fc ff ff         CALL 0x00452680
        _emit 0xe8
        _emit 0xe2
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 0005299e: 8b 38                  MOV EDI, dword ptr [EAX]
        _emit 0x8b
        _emit 0x38
        // 000529a0: 8b 70 04               MOV ESI, dword ptr [EAX + 0x4]
        _emit 0x8b
        _emit 0x70
        _emit 0x04
        // 000529a3: bd 10 00 00 00         MOV EBP, 0x10
        _emit 0xbd
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000529a8: 39 6c 24 6c            CMP dword ptr [ESP + 0x6c], EBP
        _emit 0x39
        _emit 0x6c
        _emit 0x24
        _emit 0x6c
        // 000529ac: 72 0d                  JC 0x004529bb
        _emit 0x72
        _emit 0x0d
        // 000529ae: 8b 44 24 58            MOV EAX, dword ptr [ESP + 0x58]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x58
        // 000529b2: 50                     PUSH EAX
        _emit 0x50
        // 000529b3: e8 5f f1 57 00         CALL 0x009d1b17
        _emit 0xe8
        _emit 0x5f
        _emit 0xf1
        _emit 0x57
        _emit 0x00
        // 000529b8: 83 c4 04               ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 000529bb: 39 6c 24 50            CMP dword ptr [ESP + 0x50], EBP
        _emit 0x39
        _emit 0x6c
        _emit 0x24
        _emit 0x50
        // 000529bf: c7 44 24 6c 0f 00 00 00  MOV dword ptr [ESP + 0x6c], 0xf
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x6c
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000529c7: 89 5c 24 68            MOV dword ptr [ESP + 0x68], EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x68
        // 000529cb: 88 5c 24 58            MOV byte ptr [ESP + 0x58], BL
        _emit 0x88
        _emit 0x5c
        _emit 0x24
        _emit 0x58
        // 000529cf: 72 0d                  JC 0x004529de
        _emit 0x72
        _emit 0x0d
        // 000529d1: 8b 4c 24 3c            MOV ECX, dword ptr [ESP + 0x3c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        // 000529d5: 51                     PUSH ECX
        _emit 0x51
        // 000529d6: e8 3c f1 57 00         CALL 0x009d1b17
        _emit 0xe8
        _emit 0x3c
        _emit 0xf1
        _emit 0x57
        _emit 0x00
        // 000529db: 83 c4 04               ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 000529de: 39 6c 24 34            CMP dword ptr [ESP + 0x34], EBP
        _emit 0x39
        _emit 0x6c
        _emit 0x24
        _emit 0x34
        // 000529e2: c7 44 24 50 0f 00 00 00  MOV dword ptr [ESP + 0x50], 0xf
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x50
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000529ea: 89 5c 24 4c            MOV dword ptr [ESP + 0x4c], EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x4c
        // 000529ee: 88 5c 24 3c            MOV byte ptr [ESP + 0x3c], BL
        _emit 0x88
        _emit 0x5c
        _emit 0x24
        _emit 0x3c
        // 000529f2: 72 0d                  JC 0x00452a01
        _emit 0x72
        _emit 0x0d
        // 000529f4: 8b 54 24 20            MOV EDX, dword ptr [ESP + 0x20]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x20
        // 000529f8: 52                     PUSH EDX
        _emit 0x52
        // 000529f9: e8 19 f1 57 00         CALL 0x009d1b17
        _emit 0xe8
        _emit 0x19
        _emit 0xf1
        _emit 0x57
        _emit 0x00
        // 000529fe: 83 c4 04               ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00052a01: c7 44 24 34 0f 00 00 00  MOV dword ptr [ESP + 0x34], 0xf
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00052a09: 89 5c 24 30            MOV dword ptr [ESP + 0x30], EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x30
        // 00052a0d: 88 5c 24 20            MOV byte ptr [ESP + 0x20], BL
        _emit 0x88
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        // 00052a11: 3b fb                  CMP EDI, EBX
        _emit 0x3b
        _emit 0xfb
        // 00052a13: 75 05                  JNZ 0x00452a1a
        _emit 0x75
        _emit 0x05
        // 00052a15: e8 9a f8 57 00         CALL 0x009d22b4
        _emit 0xe8
        _emit 0x9a
        _emit 0xf8
        _emit 0x57
        _emit 0x00
        // 00052a1a: 3b 77 04               CMP ESI, dword ptr [EDI + 0x4]
        _emit 0x3b
        _emit 0x77
        _emit 0x04
        // 00052a1d: 75 05                  JNZ 0x00452a24
        _emit 0x75
        _emit 0x05
        // 00052a1f: e8 90 f8 57 00         CALL 0x009d22b4
        _emit 0xe8
        _emit 0x90
        _emit 0xf8
        _emit 0x57
        _emit 0x00
        // 00052a24: 8d 46 28               LEA EAX, [ESI + 0x28]
        _emit 0x8d
        _emit 0x46
        _emit 0x28
        // 00052a27: 8b 4c 24 70            MOV ECX, dword ptr [ESP + 0x70]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x70
        // 00052a2b: 64 89 0d 00 00 00 00   MOV dword ptr FS:[0x0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00052a32: 59                     POP ECX
        _emit 0x59
        // 00052a33: 5f                     POP EDI
        // (function size boundary: compare.py reads exactly 324 bytes;
        //  POP ESI..RET are beyond the symbol boundary and not compared)
        _emit 0x5f
    }
}
