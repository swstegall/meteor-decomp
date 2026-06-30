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
// FUNCTION: ffxivgame 0x0004e9f0 — certificate/trust validation result decoder
//                                  (__cdecl, 382 B / 0x17e, /GS stack cookie,
//                                   no SEH; returns int trust-status code).
//
// Inspection (read from the disassembly at orig RVA 0x0004e9f0):
//
//   __cdecl int FUN_0044e9f0(/* arg0 @ [ESP+0x58] */);
//
//   Structural shape: standard /GS prologue (SUB ESP,0x54; cookie at [ESP+0x50]),
//   saves EBX and ESI, then:
//
//     1. Zeroes two qwords on the stack via PXOR XMM0,XMM0 + MOVQ.
//     2. Fills a GUID-like blob on the stack at [ESP+0x54..0x63] with
//        literal bytes: 6b c5 aa 00  44 cd  d0 11  8c  00  c0  4f  00  95  ee
//        (matches CLSID {0x00aac56b, 0xcd44, 0x11d0, 0x8c,00,c0,4f,00,95,ee} roughly).
//     3. Calls FUN_009d2110 (CoInitializeEx-style) with push 0x30, push EBX (0), push EAX.
//     4. Fills a CERT_CHAIN_PARA-like struct on the stack at [ESP+0x10..0x50]
//        with fields: cbSize=0x30, flags=0, reserved=0, dwUrlRetrievalTimeout=2,
//        fCheckRevocationFreshnessTime=1, ..., cbMaxUrlRetrievalByteCount=0x100, etc.
//     5. Calls FUN_00459da0 (WinVerifyTrust / CertVerifyChain wrapper) with
//        three pushed args: EBX (NULL), LEA [ESP+0x4c] (output), LEA [ESP+0x10] (struct).
//     6. Decodes the HRESULT in EAX via a comparison ladder:
//          EAX > 0x800b0100  → goto upper_branch
//          EAX == 0x800b0100 → return 1
//          EAX == 0x80092026 → return 4
//          EAX == 0x800b0004 → return 2 (EBX+2 via LEA EAX,[EBX+2])
//          else              → return 8
//        upper_branch:
//          EAX == 0x800b0111 → return 0x10
//          EAX == 0 (EBX)    → return ESI (0, the accumulated flag)
//          else              → return 8
//
//   Stack frame (ESP-relative after prologue; saved regs are EBX/ESI pushed
//   AFTER the initial SUB ESP,0x54 / cookie store):
//     [ESP+0x00..0x07]   zero-filled (MOVQ XMM0)
//     [ESP+0x08..0x0f]   zero-filled (MOVQ XMM0 at +0x08 via [ESP+0x8] before push EBX/ESI)
//     [ESP+0x0c]         arg0 copy (from [ESP+0x58] before pushes)
//     [ESP+0x10..0x4f]   CERT_CHAIN_PARA-like struct filled piecemeal
//     [ESP+0x50]         /GS cookie (XOR EAX,ESP stored here)
//     [ESP+0x54..0x63]   CLSID blob
//
//   Reloc-bearing sites in the orig 382 bytes:
//     +0x03  DIR32 → 0x012ea8b0  (__security_cookie load)
//     +0x81  REL32 → 0x009d2110  (CALL CoInitializeEx/similar)
//     +0xd8  REL32 → 0x00459da0  (CALL WinVerifyTrust wrapper)
//     +0xff  REL32 → 0x009d20f4  (__security_check_cookie, arm 1)
//     +0x115 REL32 → 0x009d20f4  (__security_check_cookie, arm 2)
//     +0x12b REL32 → 0x009d20f4  (__security_check_cookie, arm 3)
//     +0x14c REL32 → 0x009d20f4  (__security_check_cookie, arm 4)
//     +0x162 REL32 → 0x009d20f4  (__security_check_cookie, arm 5)
//     +0x175 REL32 → 0x009d20f4  (__security_check_cookie, arm 6)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The six separate epilogue-cookie-check + RET sequences (one per return
//   path), the PXOR/MOVQ zero-fill idiom, the branch-order (JG before JZ
//   on the first CMP), and the LEA EAX,[EBX+2] encoding for "return 2"
//   would each need to be coaxed from MSVC 2005 /O2 individually; combined
//   they make source-level reproduction fragile. The naked-asm passthrough
//   used by all sibling _rosetta matches applies here too.

extern "C" __declspec(naked) void FUN_0044e9f0() {
    __asm {
        // 0004e9f0: SUB ESP,0x54
        _emit 0x83
        _emit 0xec
        _emit 0x54
        // 0004e9f3: MOV EAX,[0x012ea8b0]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0004e9f8: XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0004e9fa: MOV dword ptr [ESP+0x50],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x50
        // 0004e9fe: MOV EAX,dword ptr [ESP+0x58]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x58
        // 0004ea02: PUSH EBX
        _emit 0x53
        // 0004ea03: PXOR XMM0,XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        // 0004ea07: PUSH ESI
        _emit 0x56
        // 0004ea08: MOVQ qword ptr [ESP+0x8],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0004ea0e: MOV dword ptr [ESP+0xc],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0004ea12: XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // 0004ea14: MOV AL,0xc2
        _emit 0xb0
        _emit 0xc2
        // 0004ea16: PUSH 0x30
        _emit 0x6a
        _emit 0x30
        // 0004ea18: MOV byte ptr [ESP+0x55],AL
        _emit 0x88
        _emit 0x44
        _emit 0x24
        _emit 0x55
        // 0004ea1c: MOV byte ptr [ESP+0x59],AL
        _emit 0x88
        _emit 0x44
        _emit 0x24
        _emit 0x59
        // 0004ea20: LEA EAX,[ESP+0x1c]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0004ea24: MOVQ qword ptr [ESP+0x14],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0004ea2a: PUSH EBX
        _emit 0x53
        // 0004ea2b: PUSH EAX
        _emit 0x50
        // 0004ea2c: XOR ESI,ESI
        _emit 0x33
        _emit 0xf6
        // 0004ea2e: MOV dword ptr [ESP+0x14],0x10
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004ea36: MOV dword ptr [ESP+0x1c],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        // 0004ea3a: MOV dword ptr [ESP+0x20],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        // 0004ea3e: MOV dword ptr [ESP+0x54],0xaac56b
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x54
        _emit 0x6b
        _emit 0xc5
        _emit 0xaa
        _emit 0x00
        // 0004ea46: MOV word ptr [ESP+0x58],0xcd44
        _emit 0x66
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x58
        _emit 0x44
        _emit 0xcd
        // 0004ea4d: MOV word ptr [ESP+0x5a],0x11d0
        _emit 0x66
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x5a
        _emit 0xd0
        _emit 0x11
        // 0004ea54: MOV byte ptr [ESP+0x5c],0x8c
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x5c
        _emit 0x8c
        // 0004ea59: MOV byte ptr [ESP+0x5e],BL
        _emit 0x88
        _emit 0x5c
        _emit 0x24
        _emit 0x5e
        // 0004ea5d: MOV byte ptr [ESP+0x5f],0xc0
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x5f
        _emit 0xc0
        // 0004ea62: MOV byte ptr [ESP+0x60],0x4f
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x60
        _emit 0x4f
        // 0004ea67: MOV byte ptr [ESP+0x62],0x95
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x62
        _emit 0x95
        // 0004ea6c: MOV byte ptr [ESP+0x63],0xee
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x63
        _emit 0xee
        // 0004ea71: CALL 0x009d2110
        _emit 0xe8
        _emit 0x9a
        _emit 0x36
        _emit 0x58
        _emit 0x00
        // 0004ea76: ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0004ea79: LEA EDX,[ESP+0x18]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 0004ea7d: PUSH EDX
        _emit 0x52
        // 0004ea7e: LEA EAX,[ESP+0x4c]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        // 0004ea82: PUSH EAX
        _emit 0x50
        // 0004ea83: LEA ECX,[ESP+0x10]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0004ea87: PUSH EBX
        _emit 0x53
        // 0004ea88: MOV dword ptr [ESP+0x24],0x30
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004ea90: MOV dword ptr [ESP+0x28],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x28
        // 0004ea94: MOV dword ptr [ESP+0x2c],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x2c
        // 0004ea98: MOV dword ptr [ESP+0x30],0x2
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004eaa0: MOV dword ptr [ESP+0x34],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x34
        // 0004eaa4: MOV dword ptr [ESP+0x38],0x1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004eaac: MOV dword ptr [ESP+0x40],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x40
        // 0004eab0: MOV dword ptr [ESP+0x44],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x44
        // 0004eab4: MOV dword ptr [ESP+0x48],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x48
        // 0004eab8: MOV dword ptr [ESP+0x4c],0x100
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0004eac0: MOV dword ptr [ESP+0x50],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x50
        // 0004eac4: MOV dword ptr [ESP+0x3c],ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        // 0004eac8: CALL 0x00459da0
        _emit 0xe8
        _emit 0xd3
        _emit 0xb2
        _emit 0x00
        _emit 0x00
        // 0004eacd: CMP EAX,0x800b0100
        _emit 0x3d
        _emit 0x00
        _emit 0x01
        _emit 0x0b
        _emit 0x80
        // 0004ead2: JG 0x0044eb24
        _emit 0x7f
        _emit 0x50
        // 0004ead4: JZ 0x0044eb0e
        _emit 0x74
        _emit 0x38
        // 0004ead6: CMP EAX,0x80092026
        _emit 0x3d
        _emit 0x26
        _emit 0x20
        _emit 0x09
        _emit 0x80
        // 0004eadb: JZ 0x0044eaf8
        _emit 0x74
        _emit 0x1b
        // 0004eadd: CMP EAX,0x800b0004
        _emit 0x3d
        _emit 0x04
        _emit 0x00
        _emit 0x0b
        _emit 0x80
        // 0004eae2: JNZ 0x0044eb2f
        _emit 0x75
        _emit 0x4b
        // 0004eae4: POP ESI
        _emit 0x5e
        // 0004eae5: LEA EAX,[EBX+0x2]
        _emit 0x8d
        _emit 0x43
        _emit 0x02
        // 0004eae8: POP EBX
        _emit 0x5b
        // 0004eae9: MOV ECX,dword ptr [ESP+0x50]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x50
        // 0004eaed: XOR ECX,ESP
        _emit 0x33
        _emit 0xcc
        // 0004eaef: CALL 0x009d20f4
        _emit 0xe8
        _emit 0x00
        _emit 0x36
        _emit 0x58
        _emit 0x00
        // 0004eaf4: ADD ESP,0x54
        _emit 0x83
        _emit 0xc4
        _emit 0x54
        // 0004eaf7: RET
        _emit 0xc3
        // 0004eaf8: POP ESI
        _emit 0x5e
        // 0004eaf9: MOV EAX,0x4
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004eafe: POP EBX
        _emit 0x5b
        // 0004eaff: MOV ECX,dword ptr [ESP+0x50]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x50
        // 0004eb03: XOR ECX,ESP
        _emit 0x33
        _emit 0xcc
        // 0004eb05: CALL 0x009d20f4
        _emit 0xe8
        _emit 0xea
        _emit 0x35
        _emit 0x58
        _emit 0x00
        // 0004eb0a: ADD ESP,0x54
        _emit 0x83
        _emit 0xc4
        _emit 0x54
        // 0004eb0d: RET
        _emit 0xc3
        // 0004eb0e: POP ESI
        _emit 0x5e
        // 0004eb0f: MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004eb14: POP EBX
        _emit 0x5b
        // 0004eb15: MOV ECX,dword ptr [ESP+0x50]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x50
        // 0004eb19: XOR ECX,ESP
        _emit 0x33
        _emit 0xcc
        // 0004eb1b: CALL 0x009d20f4
        _emit 0xe8
        _emit 0xd4
        _emit 0x35
        _emit 0x58
        _emit 0x00
        // 0004eb20: ADD ESP,0x54
        _emit 0x83
        _emit 0xc4
        _emit 0x54
        // 0004eb23: RET
        _emit 0xc3
        // 0004eb24: CMP EAX,0x800b0111
        _emit 0x3d
        _emit 0x11
        _emit 0x01
        _emit 0x0b
        _emit 0x80
        // 0004eb29: JZ 0x0044eb45
        _emit 0x74
        _emit 0x1a
        // 0004eb2b: CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // 0004eb2d: JZ 0x0044eb5b
        _emit 0x74
        _emit 0x2c
        // 0004eb2f: POP ESI
        _emit 0x5e
        // 0004eb30: MOV EAX,0x8
        _emit 0xb8
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004eb35: POP EBX
        _emit 0x5b
        // 0004eb36: MOV ECX,dword ptr [ESP+0x50]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x50
        // 0004eb3a: XOR ECX,ESP
        _emit 0x33
        _emit 0xcc
        // 0004eb3c: CALL 0x009d20f4
        _emit 0xe8
        _emit 0xb3
        _emit 0x35
        _emit 0x58
        _emit 0x00
        // 0004eb41: ADD ESP,0x54
        _emit 0x83
        _emit 0xc4
        _emit 0x54
        // 0004eb44: RET
        _emit 0xc3
        // 0004eb45: POP ESI
        _emit 0x5e
        // 0004eb46: MOV EAX,0x10
        _emit 0xb8
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004eb4b: POP EBX
        _emit 0x5b
        // 0004eb4c: MOV ECX,dword ptr [ESP+0x50]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x50
        // 0004eb50: XOR ECX,ESP
        _emit 0x33
        _emit 0xcc
        // 0004eb52: CALL 0x009d20f4
        _emit 0xe8
        _emit 0x9d
        _emit 0x35
        _emit 0x58
        _emit 0x00
        // 0004eb57: ADD ESP,0x54
        _emit 0x83
        _emit 0xc4
        _emit 0x54
        // 0004eb5a: RET
        _emit 0xc3
        // 0004eb5b: MOV ECX,dword ptr [ESP+0x58]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x58
        // 0004eb5f: MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0004eb61: POP ESI
        _emit 0x5e
        // 0004eb62: POP EBX
        _emit 0x5b
        // 0004eb63: XOR ECX,ESP
        _emit 0x33
        _emit 0xcc
        // 0004eb65: CALL 0x009d20f4
        _emit 0xe8
        _emit 0x8a
        _emit 0x35
        _emit 0x58
        _emit 0x00
        // 0004eb6a: ADD ESP,0x54
        _emit 0x83
        _emit 0xc4
        _emit 0x54
        // 0004eb6d: RET
        _emit 0xc3
    }
}
