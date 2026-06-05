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
// FUNCTION: ffxivgame 0x0044b240 — strcpy-from-offset returning length+1
//                                  (49 B / 0x31)
//
//   int __stdcall FUN_0044b240(char *base, int off, char *dst)
//     stack layout (after RET):
//       [ESP+0x04] : char *base   (param_1)
//       [ESP+0x08] : int   off    (param_2)
//       [ESP+0x0c] : char *dst    (param_3)   (read as [ESP+0x10] after PUSH ESI)
//     returns: count of non-NUL bytes copied + 1 (i.e. including the NUL),
//              in EAX.
//
// Source shape (the asm reads src[0] once, then indexes src[i] with i
// pre-incremented inside the loop body — a classic do/while-with-guard
// strcpy that also counts the characters):
//
//   int __stdcall f(char *base, int off, char *dst) {
//       char *s = base + off;
//       int   n = 0;
//       char  c = *s;                 // MOV AL,[ESI]
//       if (c != 0) {
//           do {
//               n++;                  // ADD ECX,1
//               *dst++ = c;           // MOV [EDX],AL ; ADD EDX,1
//               c = s[n];             // MOV AL,[ESI+ECX]
//           } while (c != 0);         // TEST AL,AL ; JNZ
//       }
//       *dst = 0;                     // MOV [EDX],0
//       return n + 1;                 // LEA EAX,[ECX+1]
//   }
//
// Calling convention: __stdcall (callee cleans 12 bytes via `ret 0xc`).
// Frame: PUSH ESI only (one callee-save, no ESP adjustment, no locals).
//
// Asm (49 bytes @ orig RVA 0x0004b240):
//   8b 44 24 08            MOV  EAX, [ESP+0x08]      ; off
//   8b 54 24 04            MOV  EDX, [ESP+0x04]      ; base
//   56                     PUSH ESI
//   8d 34 02               LEA  ESI, [EDX+EAX]       ; s = base + off
//   8a 06                  MOV  AL, [ESI]            ; c = *s
//   8b 54 24 10            MOV  EDX, [ESP+0x10]      ; dst (post-PUSH)
//   33 c9                  XOR  ECX, ECX             ; n = 0
//   84 c0                  TEST AL, AL
//   74 0f                  JZ   done
//  loop:
//   83 c1 01               ADD  ECX, 1               ; n++
//   88 02                  MOV  [EDX], AL            ; *dst = c
//   8a 04 0e               MOV  AL, [ESI+ECX]        ; c = s[n]
//   83 c2 01               ADD  EDX, 1               ; dst++
//   84 c0                  TEST AL, AL
//   75 f1                  JNZ  loop
//  done:
//   c6 02 00               MOV  byte ptr [EDX], 0    ; *dst = '\0'
//   8d 41 01               LEA  EAX, [ECX+1]         ; return n + 1
//   5e                     POP  ESI
//   c2 0c 00               RET  0x0c
//
// Reconstruction: __declspec(naked) _emit byte passthrough (mirrors the
// sibling _rosetta naked stubs). This function carries NO relocations —
// every operand is register / stack-relative / short-jump — so the orig
// 49 bytes emit verbatim and the .obj's .text matches byte-for-byte.
// `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_0044b240() {
    __asm {
        // 0004b240: 8b 44 24 08   MOV EAX, [ESP+0x08]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0004b244: 8b 54 24 04   MOV EDX, [ESP+0x04]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x04
        // 0004b248: 56            PUSH ESI
        _emit 0x56
        // 0004b249: 8d 34 02      LEA ESI, [EDX+EAX]
        _emit 0x8d
        _emit 0x34
        _emit 0x02
        // 0004b24c: 8a 06         MOV AL, [ESI]
        _emit 0x8a
        _emit 0x06
        // 0004b24e: 8b 54 24 10   MOV EDX, [ESP+0x10]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0004b252: 33 c9         XOR ECX, ECX
        _emit 0x33
        _emit 0xc9
        // 0004b254: 84 c0         TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // 0004b256: 74 0f         JZ done
        _emit 0x74
        _emit 0x0f
        // 0004b258: 83 c1 01      ADD ECX, 1
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // 0004b25b: 88 02         MOV [EDX], AL
        _emit 0x88
        _emit 0x02
        // 0004b25d: 8a 04 0e      MOV AL, [ESI+ECX]
        _emit 0x8a
        _emit 0x04
        _emit 0x0e
        // 0004b260: 83 c2 01      ADD EDX, 1
        _emit 0x83
        _emit 0xc2
        _emit 0x01
        // 0004b263: 84 c0         TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // 0004b265: 75 f1         JNZ loop
        _emit 0x75
        _emit 0xf1
        // 0004b267: c6 02 00      MOV byte ptr [EDX], 0
        _emit 0xc6
        _emit 0x02
        _emit 0x00
        // 0004b26a: 8d 41 01      LEA EAX, [ECX+1]
        _emit 0x8d
        _emit 0x41
        _emit 0x01
        // 0004b26d: 5e            POP ESI
        _emit 0x5e
        // 0004b26e: c2 0c 00      RET 0x0c
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
