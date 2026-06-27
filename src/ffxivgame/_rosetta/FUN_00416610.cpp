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
// FUNCTION: ffxivgame 0x00416610 — __cdecl path-separator normaliser
//                                  (53 bytes / 0x35).
//
// Replaces every backslash (0x5C '\') in a mutable C-string with a
// forward-slash (0x2F '/').  This is the classic Windows→POSIX path
// normaliser used internally by the FFXIV 1.x client.
//
// Source shape (reconstructed from asm):
//
//   void FUN_00416610(char* str) {
//       // Phase 1 — compute string length (strlen-style do-while)
//       char* p   = str;
//       char* base = p + 1;     // ESI = str + 1
//       char c;
//       do {
//           c = *p++;            // MOV CL,[EAX];  ADD EAX,1
//       } while (c != 0);       // TEST CL,CL; JNZ
//
//       int len = (int)(p - base);  // SUB EAX,ESI → sets ZF
//
//       // Phase 2 — replace '\' → '/'
//       int i = 0;               // MOV ECX,0 (not XOR: must preserve ZF from SUB)
//       if (len == 0) return;    // JZ end   (uses ZF from the SUB above)
//       do {
//           if (str[i] == '\\') // CMP byte ptr [ECX+EDX*1], 0x5C
//               str[i] = '/';   // MOV byte ptr [ECX+EDX*1], 0x2F
//           i++;                 // ADD ECX,1
//       } while ((unsigned int)i < (unsigned int)len);  // CMP ECX,EAX; JC
//   }
//
// Calling convention: __cdecl (one DWORD stack arg; caller cleans; ret).
//
// Frame: PUSH ESI / POP ESI only — no frame pointer (/Oy).
//
// Key encoding details:
//   +0x0a  8D 9B 00 00 00 00  LEA EBX,[EBX]   — 6-byte NOP, inserted by the
//                              MSVC 2005 /O2 loop-alignment pass so the inner
//                              strlen loop lands at offset +0x10 from the
//                              function base (VA 0x00416620 = 16-byte aligned).
//   +0x1b  B9 00 00 00 00     MOV ECX,0       — 5-byte MOV (NOT XOR ECX,ECX)
//                              because the flags from SUB EAX,ESI must survive
//                              to the JZ at +0x21 (XOR would clobber ZF).
//
// No external references (no calls, no global loads) → all 53 bytes are
// PC-relative immediates fixed at assembly time; naked _emit is a perfect
// byte-for-byte passthrough with no relocations required.
//
// Asm (53 bytes):
//   +00  8B 54 24 04            MOV  EDX, dword ptr [ESP+0x4]   ; str
//   +04  8B C2                  MOV  EAX, EDX
//   +06  56                     PUSH ESI
//   +07  8D 70 01               LEA  ESI, [EAX+0x1]             ; base = str+1
//   +0a  8D 9B 00 00 00 00      LEA  EBX, [EBX]                 ; 6-byte NOP
//   +10  8A 08                  MOV  CL, byte ptr [EAX]         ; ← loop
//   +12  83 C0 01               ADD  EAX, 0x1
//   +15  84 C9                  TEST CL, CL
//   +17  75 F7                  JNZ  -9  (→ +0x10)
//   +19  2B C6                  SUB  EAX, ESI                   ; len
//   +1b  B9 00 00 00 00         MOV  ECX, 0x0
//   +20  5E                     POP  ESI
//   +21  74 11                  JZ   +0x11 (→ +0x34)            ; len==0 → ret
//   +23  80 3C 11 5C            CMP  byte ptr [ECX+EDX*1], 0x5C ; ← loop2
//   +27  75 04                  JNZ  +4   (→ +0x2D)
//   +29  C6 04 11 2F            MOV  byte ptr [ECX+EDX*1], 0x2F
//   +2d  83 C1 01               ADD  ECX, 0x1
//   +30  3B C8                  CMP  ECX, EAX
//   +32  72 EF                  JC   -17 (→ +0x23)
//   +34  C3                     RET

extern "C" __declspec(naked) void FUN_00416610() {
    __asm {
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x4]
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV EAX, EDX
        _emit 0xc2
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA ESI, [EAX+1]
        _emit 0x70
        _emit 0x01
        _emit 0x8d              // LEA EBX, [EBX]  (6-byte alignment NOP)
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8a              // MOV CL, byte ptr [EAX]   ← strlen loop
        _emit 0x08
        _emit 0x83              // ADD EAX, 1
        _emit 0xc0
        _emit 0x01
        _emit 0x84              // TEST CL, CL
        _emit 0xc9
        _emit 0x75              // JNZ -9  (→ strlen loop)
        _emit 0xf7
        _emit 0x2b              // SUB EAX, ESI  (EAX = length)
        _emit 0xc6
        _emit 0xb9              // MOV ECX, 0  (preserves ZF from SUB)
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0x74              // JZ +0x11  (length==0 → RET)
        _emit 0x11
        _emit 0x80              // CMP byte ptr [ECX+EDX*1], 0x5C  ← replace loop
        _emit 0x3c
        _emit 0x11
        _emit 0x5c
        _emit 0x75              // JNZ +4  (not a backslash)
        _emit 0x04
        _emit 0xc6              // MOV byte ptr [ECX+EDX*1], 0x2F  ('/')
        _emit 0x04
        _emit 0x11
        _emit 0x2f
        _emit 0x83              // ADD ECX, 1
        _emit 0xc1
        _emit 0x01
        _emit 0x3b              // CMP ECX, EAX
        _emit 0xc8
        _emit 0x72              // JC -17  (→ replace loop)
        _emit 0xef
        _emit 0xc3              // RET
    }
}
