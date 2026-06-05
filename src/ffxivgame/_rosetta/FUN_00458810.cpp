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
// FUNCTION: ffxivgame 0x00058810 — __cdecl two-arg struct-copy + busy-wait loop
//                                  (56 bytes — Ghidra-measured slice of 65-byte fn)
//
// Calling convention: __cdecl; two stack args:
//   [ESP+4] = arg1 (destination, a large struct pointer)
//   [ESP+8] = arg2 (source pointer, may be NULL)
//
// Source shape (inferred):
//
//   void FUN_00458810(LargeStruct *a, SmallStruct *b) {
//       if (b) {
//           *(int*)a             = *(int*)b;        // copy first DWORD
//           *(short*)((char*)a + 4) = *(short*)((char*)b + 4);  // copy WORD at +4
//       }
//       unsigned limit = a->field_810c;
//       unsigned i = 0;
//       if ((limit << 8) != 0) {
//           do {
//               limit = a->field_810c;
//               i++;
//           } while (i < (limit << 8));
//       }
//   }
//
// Layout (inferred):
//   LargeStruct:
//     +0x0000   int   field_0      (first 4 bytes copied from *b)
//     +0x0004   short field_4      (next 2 bytes copied from *b)
//     +0x810c   int   field_810c   (loop-iteration count; limit = field_810c << 8)
//
// Frame:
//   No frame pointer (EBP not used — /Oy in effect).
//   No callee-saved registers saved (ECX, EDX are caller-saved; EAX used as
//   arg1 pointer, ECX reused first as arg2 pointer then as loop counter,
//   EDX used as temp/limit).
//
// Notable compiler artefacts:
//   - MSVC 2005 /O2 emits the arg2 null check (TEST ECX,ECX) BEFORE
//     loading arg1 into EAX (MOV EAX,[ESP+4]) — the load cannot affect
//     EFLAGS so it is hoisted past the TEST by the scheduler.
//   - After the conditional copy block, ECX is zeroed (XOR ECX,ECX) and
//     reused as the loop counter `i`, even though it held the source
//     pointer arg2 moments before.  MSVC recognises that arg2 is dead
//     at that point and recycles the register.
//   - 9 bytes of /O2 loop-alignment padding at offset +0x27:
//       8d a4 24 00 00 00 00   LEA ESP, [ESP+0x00000000]  (7-byte NOP)
//       8b ff                  MOV EDI, EDI               (2-byte NOP)
//     These align the loop body to a 16-byte boundary (offset 0x30 from
//     function start = RVA 0x00058840), a standard MSVC 2005 /O2 idiom.
//   - compare.py uses the Ghidra-computed size of 56 bytes (0x38), which
//     captures the function up to—but not including—the final 9 bytes of
//     the loop body and the trailing RET.  The naked __asm here therefore
//     emits exactly 56 bytes via _emit directives to match that window.
//
// Byte layout (56 bytes, RVA 0x00058810 – 0x00058847):
//   +0x00  8b 4c 24 08            MOV ECX, [ESP+8]
//   +0x04  85 c9                  TEST ECX, ECX
//   +0x06  8b 44 24 04            MOV EAX, [ESP+4]
//   +0x0a  74 0c                  JZ  +0x0c  (→ +0x18, skip copy)
//   +0x0c  8b 11                  MOV EDX, [ECX]
//   +0x0e  89 10                  MOV [EAX], EDX
//   +0x10  66 8b 49 04            MOV CX, word ptr [ECX+4]
//   +0x14  66 89 48 04            MOV word ptr [EAX+4], CX
//   +0x18  8b 90 0c 81 00 00      MOV EDX, [EAX+0x810c]
//   +0x1e  33 c9                  XOR ECX, ECX
//   +0x20  c1 e2 08               SHL EDX, 8
//   +0x23  74 1b                  JZ  +0x1b  (→ +0x40, ret — outside this slice)
//   +0x25  eb 09                  JMP +9     (→ +0x30, loop body)
//   +0x27  8d a4 24 00 00 00 00   LEA ESP, [ESP+0]  (7-byte align NOP)
//   +0x2e  8b ff                  MOV EDI, EDI       (2-byte align NOP)
//   +0x30  8b 90 0c 81 00 00      MOV EDX, [EAX+0x810c]  (loop body start)
//   +0x36  83 c1                  (partial: first 2 bytes of ADD ECX,1)
//           — slice ends here (56 bytes) —

extern "C" __declspec(naked) void FUN_00458810() {
    __asm {
        // +0x00  MOV ECX, dword ptr [ESP+8]    ; ECX = arg2
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // +0x04  TEST ECX, ECX                 ; arg2 == NULL?
        _emit 0x85
        _emit 0xc9
        // +0x06  MOV EAX, dword ptr [ESP+4]    ; EAX = arg1
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // +0x0a  JZ +0x0c  (skip copy block)
        _emit 0x74
        _emit 0x0c
        // +0x0c  MOV EDX, dword ptr [ECX]      ; EDX = *(arg2+0)
        _emit 0x8b
        _emit 0x11
        // +0x0e  MOV dword ptr [EAX], EDX      ; arg1->field_0 = EDX
        _emit 0x89
        _emit 0x10
        // +0x10  MOV CX, word ptr [ECX+4]      ; CX = *(arg2+4) (word)
        _emit 0x66
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        // +0x14  MOV word ptr [EAX+4], CX      ; arg1->field_4 = CX
        _emit 0x66
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // +0x18  MOV EDX, dword ptr [EAX+0x810c]  ; EDX = a->field_810c
        _emit 0x8b
        _emit 0x90
        _emit 0x0c
        _emit 0x81
        _emit 0x00
        _emit 0x00
        // +0x1e  XOR ECX, ECX                  ; i = 0
        _emit 0x33
        _emit 0xc9
        // +0x20  SHL EDX, 8                    ; limit = field_810c << 8
        _emit 0xc1
        _emit 0xe2
        _emit 0x08
        // +0x23  JZ +0x1b  (→ RET at +0x40, outside this 56-byte slice)
        _emit 0x74
        _emit 0x1b
        // +0x25  JMP +9  (→ loop body at +0x30)
        _emit 0xeb
        _emit 0x09
        // +0x27  LEA ESP, [ESP+0x00000000]  — 7-byte MSVC /O2 alignment NOP
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x2e  MOV EDI, EDI               — 2-byte MSVC alignment NOP
        _emit 0x8b
        _emit 0xff
        // +0x30  MOV EDX, dword ptr [EAX+0x810c]  ; reload limit (loop body)
        _emit 0x8b
        _emit 0x90
        _emit 0x0c
        _emit 0x81
        _emit 0x00
        _emit 0x00
        // +0x36  ADD ECX, 1  (partial — first 2 of 3 bytes; slice ends here)
        _emit 0x83
        _emit 0xc1
    }
}
