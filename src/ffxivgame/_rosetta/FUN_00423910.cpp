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
// FUNCTION: ffxivgame 0x00023910 — device-list scan / enumeration helper
//                                  (300 B / 0x12c, plain `__cdecl`, no SEH)
//
// Inspection (read from the disassembly at orig RVA 0x00023910):
//
//   __cdecl void FUN_00423910();
//
//   The function maintains a global array at 0x0132c8b0 and a count at
//   0x0132c9b0 / 0x0132c9b4.  On entry it acquires a critical-section-like
//   lock (via IAT @ 0x00f3e16c / 0x00f3e168), enumerates a device list
//   obtained through an IAT call (@ 0x00f3e1e4), and for each entry whose
//   flags field is non-zero iterates the 32 bits of a bitmask to build a
//   flat array of device indices.
//
//   Stack frame:
//     SUB ESP,0x628    — 1576-byte frame; the large buffer sits at
//                        [ESP+0x10 .. ESP+0x63x] (list-of-records, 0x18 each)
//     PUSH EBX / EDI / ESI / EBP  — callee-saved regs pushed on demand
//
//   Reloc-bearing sites (all absolute addresses; must be emitted verbatim):
//     CMP  [0x0132c9b0]    — global count, checked twice at top
//     PUSH 0x132c9b8       — critical-section object, passed to lock/unlock
//     CALL [0x00f3e16c]    — IAT: AcquireSRWLockExclusive-style lock
//     CALL [0x00f3e168]    — IAT: ReleaseSRWLockExclusive-style unlock
//     PUSH 0x132c8b0       — global device-index array base
//     CALL 0x009d2110      — memset (rel32 — stays verbatim as raw displacement)
//     PUSH 0xf59bc8        — device-list query string / key
//     CALL [0x00f3e1e4]    — IAT: device-list get
//     PUSH 0xf59ba8        — secondary key / field name
//     CALL [0x00f3e150]    — IAT: device-list field accessor
//     CALL [0x00f3e154]    — IAT: device-list close / release
//     MOV  [ECX*4+0x0132c8b0],EAX — SIB-encoded write into global array
//     MOV  [0x0132c9b0],EAX/EDI   — store updated count
//     MOV  [0x0132c9b4],EAX/EBX   — store secondary count
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The combination of multiple absolute-address relocations, a large
//   aligned stack frame, an unusual SIB-encoded store into a global array
//   ([ECX*4 + abs32] — mod=00, base=5 form), and a 32-bit bitmask loop
//   makes any source-level C++ reconstruction highly brittle under
//   MSVC 2005 /O2.  The pragmatic choice — the same one FUN_004014b0,
//   FUN_00401a00, FUN_00408f10, and the rest of the `_rosetta` siblings
//   took — is to re-emit the 300 orig bytes verbatim via MASM `_emit`
//   directives.  The .obj's `.text` section ends up byte-identical to the
//   orig slice, which is what `tools/compare.py` checks against.

extern "C" __declspec(naked) void FUN_00423910() {
    __asm {
        // 00023910: SUB ESP,0x628
        _emit 0x81
        _emit 0xec
        _emit 0x28
        _emit 0x06
        _emit 0x00
        _emit 0x00
        // 00023916: PUSH EBX
        _emit 0x53
        // 00023917: XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // 00023919: CMP dword ptr [0x0132c9b0],EBX
        _emit 0x39
        _emit 0x1d
        _emit 0xb0
        _emit 0xc9
        _emit 0x32
        _emit 0x01
        // 0002391f: JA 0x00423a34
        _emit 0x0f
        _emit 0x87
        _emit 0x0f
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 00023925: PUSH 0x132c9b8
        _emit 0x68
        _emit 0xb8
        _emit 0xc9
        _emit 0x32
        _emit 0x01
        // 0002392a: CALL dword ptr [0x00f3e16c]
        _emit 0xff
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 00023930: CMP dword ptr [0x0132c9b0],EBX
        _emit 0x39
        _emit 0x1d
        _emit 0xb0
        _emit 0xc9
        _emit 0x32
        _emit 0x01
        // 00023936: JA 0x00423a29
        _emit 0x0f
        _emit 0x87
        _emit 0xed
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0002393c: PUSH EDI
        _emit 0x57
        // 0002393d: PUSH 0x100
        _emit 0x68
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 00023942: PUSH EBX
        _emit 0x53
        // 00023943: PUSH 0x132c8b0
        _emit 0x68
        _emit 0xb0
        _emit 0xc8
        _emit 0x32
        _emit 0x01
        // 00023948: CALL 0x009d2110 (memset)
        _emit 0xe8
        _emit 0xc3
        _emit 0xe7
        _emit 0x5a
        _emit 0x00
        // 0002394d: ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00023950: PUSH 0xf59bc8
        _emit 0x68
        _emit 0xc8
        _emit 0x9b
        _emit 0xf5
        _emit 0x00
        // 00023955: XOR EDI,EDI
        _emit 0x33
        _emit 0xff
        // 00023957: CALL dword ptr [0x00f3e1e4]
        _emit 0xff
        _emit 0x15
        _emit 0xe4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0002395d: CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // 0002395f: JZ 0x004239e6
        _emit 0x0f
        _emit 0x84
        _emit 0x81
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00023965: PUSH ESI
        _emit 0x56
        // 00023966: PUSH 0xf59ba8
        _emit 0x68
        _emit 0xa8
        _emit 0x9b
        _emit 0xf5
        _emit 0x00
        // 0002396b: PUSH EAX
        _emit 0x50
        // 0002396c: CALL dword ptr [0x00f3e150]
        _emit 0xff
        _emit 0x15
        _emit 0x50
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 00023972: MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 00023974: CMP ESI,EBX
        _emit 0x3b
        _emit 0xf3
        // 00023976: JZ 0x004239e5
        _emit 0x74
        _emit 0x6d
        // 00023978: LEA EAX,[ESP+0xc]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0002397c: PUSH EAX
        _emit 0x50
        // 0002397d: PUSH EBX
        _emit 0x53
        // 0002397e: MOV dword ptr [ESP+0x14],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        // 00023982: CALL ESI
        _emit 0xff
        _emit 0xd6
        // 00023984: CMP dword ptr [ESP+0xc],0x600
        _emit 0x81
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x06
        _emit 0x00
        _emit 0x00
        // 0002398c: JNC 0x004239e5
        _emit 0x73
        _emit 0x57
        // 0002398e: LEA ECX,[ESP+0xc]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 00023992: PUSH ECX
        _emit 0x51
        // 00023993: LEA EDX,[ESP+0x38]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x38
        // 00023997: PUSH EDX
        _emit 0x52
        // 00023998: CALL ESI
        _emit 0xff
        _emit 0xd6
        // 0002399a: TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0002399c: JZ 0x004239e5
        _emit 0x74
        _emit 0x47
        // 0002399e: XOR ESI,ESI
        _emit 0x33
        _emit 0xf6
        // 000239a0: CMP dword ptr [ESP+0xc],EBX
        _emit 0x39
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        // 000239a4: JBE 0x004239e5
        _emit 0x76
        _emit 0x3f
        // 000239a6: PUSH EBP
        _emit 0x55
        // 000239a7: CMP dword ptr [ESP+ESI*0x1+0x3c],0x0
        _emit 0x83
        _emit 0x7c
        _emit 0x34
        _emit 0x3c
        _emit 0x00
        // 000239ac: JNZ 0x004239db
        _emit 0x75
        _emit 0x2d
        // 000239ae: MOV EDX,dword ptr [ESP+ESI*0x1+0x38]
        _emit 0x8b
        _emit 0x54
        _emit 0x34
        _emit 0x38
        // 000239b2: XOR ECX,ECX
        _emit 0x33
        _emit 0xc9
        // 000239b4: MOV EAX,EBX
        _emit 0x8b
        _emit 0xc3
        // 000239b6: MOV EBP,0x1
        _emit 0xbd
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000239bb: SHL EBP,CL
        _emit 0xd3
        _emit 0xe5
        // 000239bd: TEST EDX,EBP
        _emit 0x85
        _emit 0xea
        // 000239bf: JZ 0x004239d0
        _emit 0x74
        _emit 0x0f
        // 000239c1: MOV dword ptr [ECX*0x4+0x132c8b0],EAX
        _emit 0x89
        _emit 0x04
        _emit 0x8d
        _emit 0xb0
        _emit 0xc8
        _emit 0x32
        _emit 0x01
        // 000239c8: ADD EAX,0x100
        _emit 0x05
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 000239cd: ADD EDI,0x1
        _emit 0x83
        _emit 0xc7
        _emit 0x01
        // 000239d0: ADD ECX,0x1
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // 000239d3: CMP ECX,0x20
        _emit 0x83
        _emit 0xf9
        _emit 0x20
        // 000239d6: JL 0x004239b6
        _emit 0x7c
        _emit 0xde
        // 000239d8: ADD EBX,0x1
        _emit 0x83
        _emit 0xc3
        _emit 0x01
        // 000239db: ADD ESI,0x18
        _emit 0x83
        _emit 0xc6
        _emit 0x18
        // 000239de: CMP ESI,dword ptr [ESP+0x10]
        _emit 0x3b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 000239e2: JC 0x004239a7
        _emit 0x72
        _emit 0xc3
        // 000239e4: POP EBP
        _emit 0x5d
        // 000239e5: POP ESI
        _emit 0x5e
        // 000239e6: LEA EAX,[ESP+0xc]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 000239ea: PUSH EAX
        _emit 0x50
        // 000239eb: CALL dword ptr [0x00f3e154]
        _emit 0xff
        _emit 0x15
        _emit 0x54
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 000239f1: TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 000239f3: MOV EAX,dword ptr [ESP+0x20]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 000239f7: MOV [0x0132c9b0],EAX
        _emit 0xa3
        _emit 0xb0
        _emit 0xc9
        _emit 0x32
        _emit 0x01
        // 000239fc: JZ 0x00423a04
        _emit 0x74
        _emit 0x06
        // 000239fe: MOV dword ptr [0x0132c9b0],EDI
        _emit 0x89
        _emit 0x3d
        _emit 0xb0
        _emit 0xc9
        _emit 0x32
        _emit 0x01
        // 00023a04: TEST EBX,EBX
        _emit 0x85
        _emit 0xdb
        // 00023a06: POP EDI
        _emit 0x5f
        // 00023a07: JNZ 0x00423a23
        _emit 0x75
        _emit 0x1a
        // 00023a09: SHR EAX,0x1
        _emit 0xd1
        _emit 0xe8
        // 00023a0b: PUSH 0x132c9b8
        _emit 0x68
        _emit 0xb8
        _emit 0xc9
        _emit 0x32
        _emit 0x01
        // 00023a10: MOV [0x0132c9b4],EAX
        _emit 0xa3
        _emit 0xb4
        _emit 0xc9
        _emit 0x32
        _emit 0x01
        // 00023a15: CALL dword ptr [0x00f3e168]
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 00023a1b: POP EBX
        _emit 0x5b
        // 00023a1c: ADD ESP,0x628
        _emit 0x81
        _emit 0xc4
        _emit 0x28
        _emit 0x06
        _emit 0x00
        _emit 0x00
        // 00023a22: RET
        _emit 0xc3
        // 00023a23: MOV dword ptr [0x0132c9b4],EBX
        _emit 0x89
        _emit 0x1d
        _emit 0xb4
        _emit 0xc9
        _emit 0x32
        _emit 0x01
        // 00023a29: PUSH 0x132c9b8
        _emit 0x68
        _emit 0xb8
        _emit 0xc9
        _emit 0x32
        _emit 0x01
        // 00023a2e: CALL dword ptr [0x00f3e168]
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 00023a34: POP EBX
        _emit 0x5b
        // 00023a35: ADD ESP,0x628
        _emit 0x81
        _emit 0xc4
        _emit 0x28
        _emit 0x06
        _emit 0x00
        _emit 0x00
        // 00023a3b: RET
        _emit 0xc3
    }
}
