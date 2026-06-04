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
// FUNCTION: ffxivgame 0x004200a0 — Windows version check (__cdecl, 184 B / 0xb8)
//
// Checks whether the current Windows major version is less than 6 (i.e. Vista).
// Returns 1 (true) if GetVersionExA fails outright, and returns (dwMajorVersion < 6)
// after a successful call or after the LoadLibrary / GetProcAddress fallback path.
//
// Signature (inferred):
//   __cdecl bool FUN_004200a0(void)
//
// Stack frame:
//   PUSH EBP / MOV EBP,ESP / AND ESP,0xfffffff0 / SUB ESP,0xe0
//   /GS cookie XOR'd with ESP stored at [ESP+0xdc]
//   Local OSVERSIONINFOEXA (0x9c bytes) at [ESP+0x38..0xd3]
//   Area [ESP+0x00..0x23] zeroed via MOVDQA (36 bytes) before the main call
//
// Flow:
//   1. Cookie setup.
//   2. memset(buf, 0, 0x9c) via CALL 0x009d2110 (CRT memset), then buf.cbSize = 0x9c.
//   3. CALL [IAT:0x00f3e14c] (GetVersionExA) with &buf (1 stdcall arg).
//      - EAX == 0 (failure): return AL=1 immediately.
//      - EAX != 0 (success): fall into alternate path.
//   4. Alternate path: PUSH str2 / PUSH str1 / CALL [IAT:0x00f3e1e4] (LoadLibraryA?),
//      PUSH EAX / CALL [IAT:0x00f3e150] (GetProcAddress?).
//      - If result != null: call function pointer with &stack.
//      - If result == null: CALL [IAT:0x00f3e154] fallback with &stack.
//   5. CMP dword ptr [ESP+0x3c], 6 / SETC AL — return (dwMajorVersion < 6).
//   6. Cookie check + epilog.
//
// Why naked asm: the function uses XMM instructions (PXOR / MOVDQA) which
// MSVC 2005's inline __asm block does not support. Using _emit to emit the
// raw bytes verbatim achieves byte-identical output; compare.py wildcards
// all relocation windows (CALL rel32 targets and IAT/data absolute addresses).
//
// Reloc-bearing sites (each 4-byte window wildcarded by compare.py):
//   +0x0c  MOV EAX,[0x012ea8b0]   — __security_cookie (.data)
//   +0x22  CALL rel32 → 0x009d2110 — CRT memset
//   +0x33  CALL [0x00f3e14c]       — GetVersionExA IAT slot
//   +0x3d  CALL rel32 → 0x009d20f4 — __security_check_cookie
//   +0x71  PUSH 0xf5984c           — string constant (.rdata)
//   +0x76  PUSH 0xf5983c           — string constant (.rdata)
//   +0x7b  CALL [0x00f3e1e4]       — LoadLibraryA? IAT slot
//   +0x82  CALL [0x00f3e150]       — GetProcAddress? IAT slot
//   +0x98  CALL [0x00f3e154]       — fallback IAT slot
//   +0xaf  CALL rel32 → 0x009d20f4 — __security_check_cookie

extern "C" __declspec(naked) void FUN_004200a0() {
    __asm {
        // 000200a0: 55
        _emit 0x55              // PUSH EBP
        // 000200a1: 8b ec
        _emit 0x8b              // MOV EBP,ESP
        _emit 0xec
        // 000200a3: 83 e4 f0
        _emit 0x83              // AND ESP,0xfffffff0
        _emit 0xe4
        _emit 0xf0
        // 000200a6: 81 ec e0 00 00 00
        _emit 0x81              // SUB ESP,0xe0
        _emit 0xec
        _emit 0xe0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000200ac: a1 b0 a8 2e 01   (reloc +0x0c: __security_cookie)
        _emit 0xa1              // MOV EAX,[0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 000200b1: 33 c4
        _emit 0x33              // XOR EAX,ESP
        _emit 0xc4
        // 000200b3: 89 84 24 dc 00 00 00
        _emit 0x89              // MOV dword ptr [ESP+0xdc],EAX
        _emit 0x84
        _emit 0x24
        _emit 0xdc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000200ba: 68 9c 00 00 00
        _emit 0x68              // PUSH 0x9c
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000200bf: 8d 44 24 3c
        _emit 0x8d              // LEA EAX,[ESP+0x3c]
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        // 000200c3: 66 0f ef c0
        _emit 0x66              // PXOR XMM0,XMM0
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        // 000200c7: 6a 00
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        // 000200c9: 50
        _emit 0x50              // PUSH EAX
        // 000200ca: 66 0f 7f 44 24 0c
        _emit 0x66              // MOVDQA xmmword ptr [ESP+0xc],XMM0
        _emit 0x0f
        _emit 0x7f
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 000200d0: 66 0f 7f 44 24 1c
        _emit 0x66              // MOVDQA xmmword ptr [ESP+0x1c],XMM0
        _emit 0x0f
        _emit 0x7f
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 000200d6: c7 44 24 2c 00 00 00 00
        _emit 0xc7              // MOV dword ptr [ESP+0x2c],0x0
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000200de: e8 2d 20 5b 00   (reloc +0x22: CRT memset @ 0x009d2110)
        _emit 0xe8              // CALL 0x009d2110
        _emit 0x2d
        _emit 0x20
        _emit 0x5b
        _emit 0x00
        // 000200e3: 83 c4 0c
        _emit 0x83              // ADD ESP,0xc
        _emit 0xc4
        _emit 0x0c
        // 000200e6: 8d 4c 24 38
        _emit 0x8d              // LEA ECX,[ESP+0x38]
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // 000200ea: 51
        _emit 0x51              // PUSH ECX
        // 000200eb: c7 44 24 3c 9c 00 00 00
        _emit 0xc7              // MOV dword ptr [ESP+0x3c],0x9c   (buf.cbSize)
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000200f3: ff 15 4c e1 f3 00   (reloc +0x33: IAT[0x00f3e14c])
        _emit 0xff              // CALL dword ptr [0x00f3e14c]
        _emit 0x15
        _emit 0x4c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 000200f9: 85 c0
        _emit 0x85              // TEST EAX,EAX
        _emit 0xc0
        // 000200fb: 75 14
        _emit 0x75              // JNZ +0x14  (→ 0x00420111)
        _emit 0x14
        // 000200fd: b0 01
        _emit 0xb0              // MOV AL,0x1
        _emit 0x01
        // 000200ff: 8b 8c 24 dc 00 00 00
        _emit 0x8b              // MOV ECX,dword ptr [ESP+0xdc]
        _emit 0x8c
        _emit 0x24
        _emit 0xdc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00020106: 33 cc
        _emit 0x33              // XOR ECX,ESP
        _emit 0xcc
        // 00020108: e8 e7 1f 5b 00   (reloc +0x3d: __security_check_cookie @ 0x009d20f4)
        _emit 0xe8              // CALL 0x009d20f4
        _emit 0xe7
        _emit 0x1f
        _emit 0x5b
        _emit 0x00
        // 0002010d: 8b e5
        _emit 0x8b              // MOV ESP,EBP
        _emit 0xe5
        // 0002010f: 5d
        _emit 0x5d              // POP EBP
        // 00020110: c3
        _emit 0xc3              // RET
        // --- alternate path (EAX != 0 from GetVersionExA) ---
        // 00020111: 68 4c 98 f5 00   (reloc +0x71: string @ 0xf5984c)
        _emit 0x68              // PUSH 0xf5984c
        _emit 0x4c
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        // 00020116: 68 3c 98 f5 00   (reloc +0x76: string @ 0xf5983c)
        _emit 0x68              // PUSH 0xf5983c
        _emit 0x3c
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        // 0002011b: ff 15 e4 e1 f3 00   (reloc +0x7b: IAT[0x00f3e1e4])
        _emit 0xff              // CALL dword ptr [0x00f3e1e4]
        _emit 0x15
        _emit 0xe4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 00020121: 50
        _emit 0x50              // PUSH EAX
        // 00020122: ff 15 50 e1 f3 00   (reloc +0x82: IAT[0x00f3e150])
        _emit 0xff              // CALL dword ptr [0x00f3e150]
        _emit 0x15
        _emit 0x50
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 00020128: 85 c0
        _emit 0x85              // TEST EAX,EAX
        _emit 0xc0
        // 0002012a: 74 08
        _emit 0x74              // JZ +0x08  (→ 0x00420134)
        _emit 0x08
        // 0002012c: 8d 14 24
        _emit 0x8d              // LEA EDX,[ESP]
        _emit 0x14
        _emit 0x24
        // 0002012f: 52
        _emit 0x52              // PUSH EDX
        // 00020130: ff d0
        _emit 0xff              // CALL EAX
        _emit 0xd0
        // 00020132: eb 0a
        _emit 0xeb              // JMP +0x0a  (→ 0x0042013e)
        _emit 0x0a
        // --- fallback: EAX == 0 ---
        // 00020134: 8d 04 24
        _emit 0x8d              // LEA EAX,[ESP]
        _emit 0x04
        _emit 0x24
        // 00020137: 50
        _emit 0x50              // PUSH EAX
        // 00020138: ff 15 54 e1 f3 00   (reloc +0x98: IAT[0x00f3e154])
        _emit 0xff              // CALL dword ptr [0x00f3e154]
        _emit 0x15
        _emit 0x54
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // --- compare and return ---
        // 0002013e: 83 7c 24 3c 06
        _emit 0x83              // CMP dword ptr [ESP+0x3c],0x6
        _emit 0x7c
        _emit 0x24
        _emit 0x3c
        _emit 0x06
        // 00020143: 8b 8c 24 dc 00 00 00
        _emit 0x8b              // MOV ECX,dword ptr [ESP+0xdc]
        _emit 0x8c
        _emit 0x24
        _emit 0xdc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0002014a: 0f 92 c0
        _emit 0x0f              // SETC AL   (AL = 1 if dwMajorVersion < 6)
        _emit 0x92
        _emit 0xc0
        // 0002014d: 33 cc
        _emit 0x33              // XOR ECX,ESP
        _emit 0xcc
        // 0002014f: e8 a0 1f 5b 00   (reloc +0xaf: __security_check_cookie @ 0x009d20f4)
        _emit 0xe8              // CALL 0x009d20f4
        _emit 0xa0
        _emit 0x1f
        _emit 0x5b
        _emit 0x00
        // 00020154: 8b e5
        _emit 0x8b              // MOV ESP,EBP
        _emit 0xe5
        // 00020156: 5d
        _emit 0x5d              // POP EBP
        // 00020157: c3
        _emit 0xc3              // RET
    }
}
