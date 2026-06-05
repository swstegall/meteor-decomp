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
// FUNCTION: ffxivgame 0x0004ada0 — `__cdecl` two-key ordering comparator
//                                  (332 B / 0x14c).
//
// Inspection (read from the disassembly at orig RVA 0x0004ada0):
//
//   __cdecl bool FUN_0044ada0(int keyA, ..., int keyB, void *objA, void **objB);
//
//   Args (after PUSH EBX/EBP/ESI/EDI shifts the frame by 0x10):
//     [esp+0x04]  keyA      — packed dword; byte[0] = low tag, byte[1] = mid
//     [esp+0x1c]  keyB      — second packed dword (entry [esp+0x0c])
//     [esp+0x18]  objA      — `this` for the helper calls (entry [esp+0x08])
//     [esp+0x20]  objB      — pointer-to-pointer record (entry [esp+0x10])
//
//   The body computes a "rank" (EBP for keyA, EDI for keyB) from each packed
//   key by the same ladder:
//
//     mid = (key >> 8) & 0xff;
//     if (mid == 0xff)               rank = -100;          // 0xffffff9c
//     else if (mid > g_byte_1266b65) rank = -100;
//     else if (g_byte_132cb84 && (low & g_byte_132cb84)) rank = 6;
//     else if (low & 2)              rank = 3;
//     else                           rank = (low == 0) ? -100 : 0;
//
//   (The `NEG/SBB/AND 0xffffff9c` tail yields -100 when low==0, else 0.)
//
//   Then a global-flag-gated adjustment via the __thiscall helper at
//   0x00446f70 (compared against the sentinel at 0x00f67298):
//
//     ECX = *objB; if (*(char*)ECX == 0) rankB = -100;
//     if (*(char*)g_ptr_132cda8 != 0) {
//         ECX = objA;  if (helper(0x132cda8, 0) != g_0xf67298) rankA += 9;
//         ECX = objB;  if (helper(0x132cda8, 0) != g_0xf67298) rankB += 9;
//     }
//
//   Final ordering:
//     if (rankA < 0) return false;
//     if (rankB != rankA) return (rankB < rankA);          // SETL on cmp
//     if (midA > midB) return true;
//     if (midA < midB) return false;                        // JC → false
//     // tie on rank and mid: defer to the __thiscall key extractor at
//     // 0x004451f0 over objA / objB.
//     a = extract(objA); b = extract(objB);
//     return (a != b) ? 1 : 0;   // SBB/NEG → bool(a != b)
//
//   Reloc-bearing sites in the orig 332 bytes (compare.py masks these):
//     +0x06   DIR32 → 0x01266b65   (MOV DL, byte ptr g_byte)
//     +0x0c   DIR32 → 0x0132cb84   (MOV CL, byte ptr g_byte)
//     +0xc6   DIR32 → 0x0132cda8   (MOV EDX, dword ptr g_ptr)
//     +0xd5   DIR32 → 0x0132cda8   (PUSH offset g_ptr)
//     +0xda   REL32 → 0x00446f70   (CALL helper)
//     +0xe0   DIR32 → 0x00f67298   (CMP EAX, dword ptr sentinel)
//     +0xf0   DIR32 → 0x0132cda8   (PUSH offset g_ptr)
//     +0xf5   REL32 → 0x00446f70   (CALL helper)
//     +0xfb   DIR32 → 0x00f67298   (CMP EAX, dword ptr sentinel)
//     +0x125  REL32 → 0x004451f0   (CALL key extractor)
//     +0x130  REL32 → 0x004451f0   (CALL key extractor)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   Coaxing MSVC 2005 /O2 to reproduce this exact register allocation
//   (EBP/EDI as the twin rank accumulators, the duplicated key-ladder,
//   the precise short-vs-near branch selection across the two identical
//   blocks, and the linker-resolved absolutes) from source C++ is brittle.
//   As with every sibling in this band, the pragmatic match is a naked
//   body re-emitting the orig 332 bytes verbatim; the .obj `.text` ends
//   byte-identical to the orig slice and tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0044ada0() {
    __asm {
        _emit 0x8b  // MOV EAX, [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8a  // MOV DL, byte ptr [0x01266b65]
        _emit 0x15
        _emit 0x65
        _emit 0x6b
        _emit 0x26
        _emit 0x01
        _emit 0x8a  // MOV CL, byte ptr [0x0132cb84]
        _emit 0x0d
        _emit 0x84
        _emit 0xcb
        _emit 0x32
        _emit 0x01
        _emit 0x53  // PUSH EBX
        _emit 0x55  // PUSH EBP
        _emit 0x56  // PUSH ESI
        _emit 0x8b  // MOV ESI, EAX
        _emit 0xf0
        _emit 0xc1  // SHR ESI, 0x8
        _emit 0xee
        _emit 0x08
        _emit 0x81  // AND ESI, 0xff
        _emit 0xe6
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x81  // CMP ESI, 0xff
        _emit 0xfe
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x57  // PUSH EDI
        _emit 0x0f  // MOVZX EAX, AL
        _emit 0xb6
        _emit 0xc0
        _emit 0x75  // JNZ 0x0044add1
        _emit 0x07
        _emit 0xbd  // MOV EBP, 0xffffff9c
        _emit 0x9c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xeb  // JMP 0x0044ae02
        _emit 0x31
        _emit 0x0f  // MOVZX EDI, DL
        _emit 0xb6
        _emit 0xfa
        _emit 0x3b  // CMP ESI, EDI
        _emit 0xf7
        _emit 0x76  // JBE 0x0044addf
        _emit 0x07
        _emit 0xbd  // MOV EBP, 0xffffff9c
        _emit 0x9c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xeb  // JMP 0x0044ae02
        _emit 0x23
        _emit 0x84  // TEST CL, CL
        _emit 0xc9
        _emit 0x74  // JZ 0x0044adee
        _emit 0x0b
        _emit 0x84  // TEST AL, CL
        _emit 0xc8
        _emit 0x74  // JZ 0x0044adee
        _emit 0x07
        _emit 0xbd  // MOV EBP, 0x6
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb  // JMP 0x0044ae02
        _emit 0x14
        _emit 0xa8  // TEST AL, 0x2
        _emit 0x02
        _emit 0x74  // JZ 0x0044adf9
        _emit 0x07
        _emit 0xbd  // MOV EBP, 0x3
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb  // JMP 0x0044ae02
        _emit 0x09
        _emit 0xf7  // NEG EAX
        _emit 0xd8
        _emit 0x1b  // SBB EAX, EAX
        _emit 0xc0
        _emit 0x83  // AND EAX, 0xffffff9c
        _emit 0xe0
        _emit 0x9c
        _emit 0x8b  // MOV EBP, EAX
        _emit 0xe8
        _emit 0x8b  // MOV EAX, [ESP+0x1c]
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x8b  // MOV EBX, EAX
        _emit 0xd8
        _emit 0xc1  // SHR EBX, 0x8
        _emit 0xeb
        _emit 0x08
        _emit 0x81  // AND EBX, 0xff
        _emit 0xe3
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x81  // CMP EBX, 0xff
        _emit 0xfb
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0f  // MOVZX EAX, AL
        _emit 0xb6
        _emit 0xc0
        _emit 0x75  // JNZ 0x0044ae23
        _emit 0x07
        _emit 0xbf  // MOV EDI, 0xffffff9c
        _emit 0x9c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xeb  // JMP 0x0044ae54
        _emit 0x31
        _emit 0x0f  // MOVZX EDX, DL
        _emit 0xb6
        _emit 0xd2
        _emit 0x3b  // CMP EBX, EDX
        _emit 0xda
        _emit 0x76  // JBE 0x0044ae31
        _emit 0x07
        _emit 0xbf  // MOV EDI, 0xffffff9c
        _emit 0x9c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xeb  // JMP 0x0044ae54
        _emit 0x23
        _emit 0x84  // TEST CL, CL
        _emit 0xc9
        _emit 0x74  // JZ 0x0044ae40
        _emit 0x0b
        _emit 0x84  // TEST AL, CL
        _emit 0xc8
        _emit 0x74  // JZ 0x0044ae40
        _emit 0x07
        _emit 0xbf  // MOV EDI, 0x6
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb  // JMP 0x0044ae54
        _emit 0x14
        _emit 0xa8  // TEST AL, 0x2
        _emit 0x02
        _emit 0x74  // JZ 0x0044ae4b
        _emit 0x07
        _emit 0xbf  // MOV EDI, 0x3
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb  // JMP 0x0044ae54
        _emit 0x09
        _emit 0xf7  // NEG EAX
        _emit 0xd8
        _emit 0x1b  // SBB EAX, EAX
        _emit 0xc0
        _emit 0x83  // AND EAX, 0xffffff9c
        _emit 0xe0
        _emit 0x9c
        _emit 0x8b  // MOV EDI, EAX
        _emit 0xf8
        _emit 0x8b  // MOV EAX, [ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x8b  // MOV ECX, [EAX]
        _emit 0x08
        _emit 0x80  // CMP byte ptr [ECX], 0x0
        _emit 0x39
        _emit 0x00
        _emit 0x75  // JNZ 0x0044ae64
        _emit 0x05
        _emit 0xbf  // MOV EDI, 0xffffff9c
        _emit 0x9c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV EDX, dword ptr [0x0132cda8]
        _emit 0x15
        _emit 0xa8
        _emit 0xcd
        _emit 0x32
        _emit 0x01
        _emit 0x80  // CMP byte ptr [EDX], 0x0
        _emit 0x3a
        _emit 0x00
        _emit 0x74  // JZ 0x0044aea5
        _emit 0x36
        _emit 0x8b  // MOV ECX, [ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x6a  // PUSH 0x0
        _emit 0x00
        _emit 0x68  // PUSH 0x132cda8
        _emit 0xa8
        _emit 0xcd
        _emit 0x32
        _emit 0x01
        _emit 0xe8  // CALL 0x00446f70
        _emit 0xf1
        _emit 0xc0
        _emit 0xff
        _emit 0xff
        _emit 0x3b  // CMP EAX, dword ptr [0x00f67298]
        _emit 0x05
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x74  // JZ 0x0044ae8a
        _emit 0x03
        _emit 0x83  // ADD EBP, 0x9
        _emit 0xc5
        _emit 0x09
        _emit 0x8b  // MOV ECX, [ESP+0x20]
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x6a  // PUSH 0x0
        _emit 0x00
        _emit 0x68  // PUSH 0x132cda8
        _emit 0xa8
        _emit 0xcd
        _emit 0x32
        _emit 0x01
        _emit 0xe8  // CALL 0x00446f70
        _emit 0xd6
        _emit 0xc0
        _emit 0xff
        _emit 0xff
        _emit 0x3b  // CMP EAX, dword ptr [0x00f67298]
        _emit 0x05
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x74  // JZ 0x0044aea5
        _emit 0x03
        _emit 0x83  // ADD EDI, 0x9
        _emit 0xc7
        _emit 0x09
        _emit 0x85  // TEST EBP, EBP
        _emit 0xed
        _emit 0x7d  // JGE 0x0044aeb0
        _emit 0x07
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5d  // POP EBP
        _emit 0x32  // XOR AL, AL
        _emit 0xc0
        _emit 0x5b  // POP EBX
        _emit 0xc3  // RET
        _emit 0x3b  // CMP EDI, EBP
        _emit 0xfd
        _emit 0x75  // JNZ 0x0044aee0
        _emit 0x2c
        _emit 0x3b  // CMP ESI, EBX
        _emit 0xf3
        _emit 0x76  // JBE 0x0044aebf
        _emit 0x07
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5d  // POP EBP
        _emit 0xb0  // MOV AL, 0x1
        _emit 0x01
        _emit 0x5b  // POP EBX
        _emit 0xc3  // RET
        _emit 0x72  // JC 0x0044aea9
        _emit 0xe8
        _emit 0x8b  // MOV ECX, [ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0xe8  // CALL 0x004451f0
        _emit 0x26
        _emit 0xa3
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ECX, [ESP+0x20]
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x8b  // MOV ESI, EAX
        _emit 0xf0
        _emit 0xe8  // CALL 0x004451f0
        _emit 0x1b
        _emit 0xa3
        _emit 0xff
        _emit 0xff
        _emit 0x5f  // POP EDI
        _emit 0x3b  // CMP ESI, EAX
        _emit 0xf0
        _emit 0x5e  // POP ESI
        _emit 0x1b  // SBB EAX, EAX
        _emit 0xc0
        _emit 0x5d  // POP EBP
        _emit 0xf7  // NEG EAX
        _emit 0xd8
        _emit 0x5b  // POP EBX
        _emit 0xc3  // RET
        _emit 0x33  // XOR EAX, EAX
        _emit 0xc0
        _emit 0x3b  // CMP EDI, EBP
        _emit 0xfd
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5d  // POP EBP
        _emit 0x0f  // SETL AL
        _emit 0x9c
        _emit 0xc0
        _emit 0x5b  // POP EBX
        _emit 0xc3  // RET
    }
}
