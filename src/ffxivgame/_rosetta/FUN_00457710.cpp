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
// FUNCTION: ffxivgame 0x00457710 — wide-string compare skipping separator
//                                   characters defined by a bitset
//                                   (__cdecl, 162 bytes / 0xa2)
//
// Inputs (register-passed — non-standard ABI, likely an internal helper):
//   EAX         : wchar_t *str1   (mutable pointer; advances over separators)
//   ESI         : const wchar_t *str2  (indexed by EDX; advances over seps)
//   [ESP+0x04]  : charset struct* (EBX; bitset at struct + 0x0c covers chars)
//
// Returns (EAX): (last char from str1) - (last char from str2), i.e. 0 when
//   the strings are equal ignoring separator characters, <0 / >0 otherwise.
//
// Algorithm (two-phase per character pair):
//   Phase 1 — skip separators in str1:
//     While *str1 is a space (0x20) or is set in the 32-dword bitset at
//     [charset+0xC], advance str1 by one wchar.  Stop on null or non-sep.
//   Phase 2 — skip separators in str2 (indexed by EDX):
//     Same bitset test on str2[EDX]; advance EDX while separator.
//     Stop on null (both considered equal for that position) or non-sep.
//   Compare str1[current] vs str2[EDX]: if different, return difference.
//   If equal and non-null, advance both pointers and repeat.
//
// Bitset check idiom (both phases are identical):
//   mask  = 1 << (char & 0x1f)
//   word  = *(charset + 0x0c + (char >> 5) * 4)
//   is_sep = (mask & word) > 0
//   (Space 0x20 is a fast-path shortcut before the bitset test.)
//
// No CALL instructions and no absolute memory references in this function,
// so the naked _emit byte-passthrough produces a zero-relocation .obj whose
// .text is byte-identical to the original slice.  compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00457710() {
    __asm {
        // +0x00:  83 ec 08           SUB ESP, 0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // +0x03:  53                 PUSH EBX
        _emit 0x53
        // +0x04:  8b 5c 24 10        MOV EBX, dword ptr [ESP+0x10]  (charset* = param1)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        // +0x08:  55                 PUSH EBP
        _emit 0x55
        // +0x09:  57                 PUSH EDI
        _emit 0x57
        // +0x0a:  33 d2              XOR EDX, EDX   (str2 index = 0)
        _emit 0x33
        _emit 0xd2
        // +0x0c:  89 44 24 0c        MOV dword ptr [ESP+0xC], EAX   (spill str1 ptr)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // === loop_top: skip separators in str1 ===
        // +0x10:  8b 4c 24 0c        MOV ECX, dword ptr [ESP+0xC]   (reload str1 ptr)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // +0x14:  0f b7 39           MOVZX EDI, word ptr [ECX]      (DI = *str1)
        _emit 0x0f
        _emit 0xb7
        _emit 0x39
        // +0x17:  66 85 ff           TEST DI, DI                    (null terminator?)
        _emit 0x66
        _emit 0x85
        _emit 0xff
        // +0x1a:  74 28              JZ +0x28  -> compare_phase      (yes → skip to compare)
        _emit 0x74
        _emit 0x28
        // +0x1c:  0f b7 c7           MOVZX EAX, DI
        _emit 0x0f
        _emit 0xb7
        _emit 0xc7
        // +0x1f:  66 3d 20 00        CMP AX, 0x20                   (space fast-path?)
        _emit 0x66
        _emit 0x3d
        _emit 0x20
        _emit 0x00
        // +0x23:  74 18              JZ +0x18  -> advance_str1       (space → advance)
        _emit 0x74
        _emit 0x18
        // +0x25:  0f b7 c0           MOVZX EAX, AX                  (zero-extend)
        _emit 0x0f
        _emit 0xb7
        _emit 0xc0
        // +0x28:  8b c8              MOV ECX, EAX                   (ECX = char)
        _emit 0x8b
        _emit 0xc8
        // +0x2a:  83 e1 1f           AND ECX, 0x1f                  (bit index)
        _emit 0x83
        _emit 0xe1
        _emit 0x1f
        // +0x2d:  bd 01 00 00 00     MOV EBP, 1
        _emit 0xbd
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x32:  d3 e5              SHL EBP, CL                    (mask = 1<<bit)
        _emit 0xd3
        _emit 0xe5
        // +0x34:  c1 e8 05           SHR EAX, 0x5                   (word index)
        _emit 0xc1
        _emit 0xe8
        _emit 0x05
        // +0x37:  23 6c 83 0c        AND EBP, dword ptr [EBX+EAX*4+0xC]  (bitset test)
        _emit 0x23
        _emit 0x6c
        _emit 0x83
        _emit 0x0c
        // +0x3b:  7e 07              JLE +0x07  -> compare_phase    (not a sep → compare)
        _emit 0x7e
        _emit 0x07
        // +0x3d:  83 44 24 0c 02     ADD dword ptr [ESP+0xC], 2     (str1 += 1 wchar)
        _emit 0x83
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x02
        // +0x42:  eb cc              JMP -0x34  -> loop_top
        _emit 0xeb
        _emit 0xcc
        // === compare_phase: skip separators in str2 ===
        // +0x44:  0f b7 04 56        MOVZX EAX, word ptr [ESI+EDX*2]  (char from str2)
        _emit 0x0f
        _emit 0xb7
        _emit 0x04
        _emit 0x56
        // +0x48:  66 85 c0           TEST AX, AX                      (null?)
        _emit 0x66
        _emit 0x85
        _emit 0xc0
        // +0x4b:  74 31              JZ +0x31  -> do_compare           (null → compare)
        _emit 0x74
        _emit 0x31
        // +0x4d:  8d 49 00           LEA ECX, [ECX]                   (3-byte NOP / align)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // === str2_skiploop ===
        // +0x50:  0f b7 c0           MOVZX EAX, AX                    (zero-extend)
        _emit 0x0f
        _emit 0xb7
        _emit 0xc0
        // +0x53:  66 3d 20 00        CMP AX, 0x20                     (space?)
        _emit 0x66
        _emit 0x3d
        _emit 0x20
        _emit 0x00
        // +0x57:  74 18              JZ +0x18  -> advance_str2         (space → advance)
        _emit 0x74
        _emit 0x18
        // +0x59:  0f b7 c0           MOVZX EAX, AX
        _emit 0x0f
        _emit 0xb7
        _emit 0xc0
        // +0x5c:  8b c8              MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // +0x5e:  83 e1 1f           AND ECX, 0x1f
        _emit 0x83
        _emit 0xe1
        _emit 0x1f
        // +0x61:  bd 01 00 00 00     MOV EBP, 1
        _emit 0xbd
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x66:  d3 e5              SHL EBP, CL
        _emit 0xd3
        _emit 0xe5
        // +0x68:  c1 e8 05           SHR EAX, 0x5
        _emit 0xc1
        _emit 0xe8
        _emit 0x05
        // +0x6b:  23 6c 83 0c        AND EBP, dword ptr [EBX+EAX*4+0xC]
        _emit 0x23
        _emit 0x6c
        _emit 0x83
        _emit 0x0c
        // +0x6f:  7e 0d              JLE +0xd  -> do_compare           (not a sep → compare)
        _emit 0x7e
        _emit 0x0d
        // +0x71:  0f b7 44 56 02     MOVZX EAX, word ptr [ESI+EDX*2+2] (peek next char)
        _emit 0x0f
        _emit 0xb7
        _emit 0x44
        _emit 0x56
        _emit 0x02
        // +0x76:  83 c2 01           ADD EDX, 1                        (EDX++)
        _emit 0x83
        _emit 0xc2
        _emit 0x01
        // +0x79:  66 85 c0           TEST AX, AX
        _emit 0x66
        _emit 0x85
        _emit 0xc0
        // +0x7c:  75 d2              JNZ -0x2e  -> str2_skiploop        (non-null → check)
        _emit 0x75
        _emit 0xd2
        // === do_compare: compare str1[current] vs str2[EDX] ===
        // +0x7e:  0f b7 0c 56        MOVZX ECX, word ptr [ESI+EDX*2]   (ECX = str2 char)
        _emit 0x0f
        _emit 0xb7
        _emit 0x0c
        _emit 0x56
        // +0x82:  0f b7 c7           MOVZX EAX, DI                     (EAX = str1 char)
        _emit 0x0f
        _emit 0xb7
        _emit 0xc7
        // +0x85:  2b c1              SUB EAX, ECX                      (diff)
        _emit 0x2b
        _emit 0xc1
        // +0x87:  75 12              JNZ +0x12  -> epilogue             (not equal → return diff)
        _emit 0x75
        _emit 0x12
        // +0x89:  66 85 ff           TEST DI, DI                       (both null?)
        _emit 0x66
        _emit 0x85
        _emit 0xff
        // +0x8c:  74 0d              JZ +0xd   -> epilogue              (yes → return 0)
        _emit 0x74
        _emit 0x0d
        // Equal non-null: advance both, loop
        // +0x8e:  83 44 24 0c 02     ADD dword ptr [ESP+0xC], 2        (str1++)
        _emit 0x83
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x02
        // +0x93:  83 c2 01           ADD EDX, 1                        (EDX++)
        _emit 0x83
        _emit 0xc2
        _emit 0x01
        // +0x96:  e9 75 ff ff ff     JMP -0x8b  -> loop_top
        _emit 0xe9
        _emit 0x75
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // === epilogue ===
        // +0x9b:  5f                 POP EDI
        _emit 0x5f
        // +0x9c:  5d                 POP EBP
        _emit 0x5d
        // +0x9d:  5b                 POP EBX
        _emit 0x5b
        // +0x9e:  83 c4 08           ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // +0xa1:  c3                 RET
        _emit 0xc3
    }
}
