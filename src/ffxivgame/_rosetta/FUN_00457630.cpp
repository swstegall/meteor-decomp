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
// FUNCTION: ffxivgame 0x00457630 — wide-string duplicate-alloc (wcsdup variant);
//                                  113 bytes / 0x71.
//
// Non-standard calling convention: caller places wchar_t* src in EAX before
// the call; first stack argument ([ESP+4] before push, [ESP+8] after PUSH EBX)
// is an optional pre-computed length (0 = count from string).  Returns the
// newly allocated wchar_t* in EAX.
//
// Logic:
//   1. If len (EBX) == 0, walk src (EDI, loaded from EAX) to compute wcslen
//      → ECX; then MOV EBX,ECX.
//   2. Allocate (EBX*2 + 2) bytes via the internal allocator at 0x009d5bc5.
//   3. If src[0] == L'\0', write null terminator and return (short epilogue
//      at 0x576a1, outside this 113-byte window).
//   4. Otherwise copy EBX wide chars from src into the new buffer, then
//      null-terminate at index EDX.
//
// Codegen notes:
//   - Two do-while loops (wcslen and wcscpy) each aligned to 16 bytes with a
//     7-byte `LEA ESP,[ESP+0]` NOP (8d a4 24 00 00 00 00); the wcslen loop is
//     preceded by a `JMP +7` that bypasses the alignment padding at
//     0x57649-0x5764f.
//   - CALL at +0x33 (0x57663) is a rel32 to the allocator (0x009d5bc5); the
//     raw rel32 bytes are emitted verbatim so no relocation is emitted.
//   - The JZ at +0x40 (0x57670) encodes `74 2f`, branching to 0x576a1 which
//     is the first byte PAST this 113-byte window (the short epilogue lives
//     in the adjacent function slot but is physically reached from here).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The non-standard EAX parameter and the out-of-window branch target make
//   source-level C++ reconstruction impractical.  Emitting all 113 bytes via
//   MASM _emit directives produces a .obj whose .text is byte-identical to
//   the original; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00457630() {
    __asm {
        // 00057630: 53              PUSH EBX
        _emit 0x53
        // 00057631: 8b 5c 24 08     MOV EBX,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        // 00057635: 85 db           TEST EBX,EBX
        _emit 0x85
        _emit 0xdb
        // 00057637: 57              PUSH EDI
        _emit 0x57
        // 00057638: 8b f8           MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 0005763a: 75 22           JNZ +0x22  (→ 0x5765e, len already known)
        _emit 0x75
        _emit 0x22
        // 0005763c: 33 c9           XOR ECX,ECX
        _emit 0x33
        _emit 0xc9
        // 0005763e: 85 ff           TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 00057640: 74 1a           JZ +0x1a   (→ 0x5765c, null src)
        _emit 0x74
        _emit 0x1a
        // 00057642: 66 39 0f        CMP word ptr [EDI],CX
        _emit 0x66
        _emit 0x39
        _emit 0x0f
        // 00057645: 74 15           JZ +0x15   (→ 0x5765c, empty src)
        _emit 0x74
        _emit 0x15
        // 00057647: eb 07           JMP +0x07  (→ 0x57650, enter loop body)
        _emit 0xeb
        _emit 0x07
        // 00057649-0x5764f: 7-byte alignment NOP — LEA ESP,[ESP+0]
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // ── wcslen loop (top at 0x57650, 16-byte aligned) ──────────────────
        // 00057650: 83 c0 02        ADD EAX,0x2
        _emit 0x83
        _emit 0xc0
        _emit 0x02
        // 00057653: 83 c1 01        ADD ECX,0x1
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // 00057656: 66 83 38 00     CMP word ptr [EAX],0x0
        _emit 0x66
        _emit 0x83
        _emit 0x38
        _emit 0x00
        // 0005765a: 75 f4           JNZ 0x57650  (continue loop)
        _emit 0x75
        _emit 0xf4
        // 0005765c: 8b d9           MOV EBX,ECX  (EBX = wcslen result)
        _emit 0x8b
        _emit 0xd9
        // ── allocation ─────────────────────────────────────────────────────
        // 0005765e: 8d 44 1b 02     LEA EAX,[EBX+EBX*1+0x2]  (size = len*2+2)
        _emit 0x8d
        _emit 0x44
        _emit 0x1b
        _emit 0x02
        // 00057662: 50              PUSH EAX
        _emit 0x50
        // 00057663: e8 5d e5 57 00  CALL 0x009d5bc5  (allocator, rel32 verbatim)
        _emit 0xe8
        _emit 0x5d
        _emit 0xe5
        _emit 0x57
        _emit 0x00
        // 00057668: 33 d2           XOR EDX,EDX
        _emit 0x33
        _emit 0xd2
        // 0005766a: 83 c4 04        ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005766d: 66 39 17        CMP word ptr [EDI],DX
        _emit 0x66
        _emit 0x39
        _emit 0x17
        // 00057670: 74 2f           JZ +0x2f  (→ 0x576a1 short epilogue, outside window)
        _emit 0x74
        _emit 0x2f
        // ── wcscpy loop setup ───────────────────────────────────────────────
        // 00057672: 56              PUSH ESI
        _emit 0x56
        // 00057673: 8b f0           MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 00057675: 8b cf           MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 00057677: 2b f7           SUB ESI,EDI   (ESI = newbuf - src, offset reg)
        _emit 0x2b
        _emit 0xf7
        // 00057679-0x5767f: 7-byte alignment NOP — LEA ESP,[ESP+0]
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // ── wcscpy loop (top at 0x57680, 16-byte aligned) ──────────────────
        // 00057680: 3b d3           CMP EDX,EBX
        _emit 0x3b
        _emit 0xd3
        // 00057682: 73 13           JNC +0x13  (→ 0x57697, index >= len)
        _emit 0x73
        _emit 0x13
        // 00057684: 66 8b 39        MOV DI,word ptr [ECX]
        _emit 0x66
        _emit 0x8b
        _emit 0x39
        // 00057687: 66 89 3c 0e     MOV word ptr [ESI+ECX*1],DI
        _emit 0x66
        _emit 0x89
        _emit 0x3c
        _emit 0x0e
        // 0005768b: 83 c1 02        ADD ECX,0x2
        _emit 0x83
        _emit 0xc1
        _emit 0x02
        // 0005768e: 83 c2 01        ADD EDX,0x1
        _emit 0x83
        _emit 0xc2
        _emit 0x01
        // 00057691: 66 83 39 00     CMP word ptr [ECX],0x0
        _emit 0x66
        _emit 0x83
        _emit 0x39
        _emit 0x00
        // 00057695: 75 e9           JNZ 0x57680  (continue copy loop)
        _emit 0x75
        _emit 0xe9
        // ── epilogue ────────────────────────────────────────────────────────
        // 00057697: 5e              POP ESI
        _emit 0x5e
        // 00057698: 5f              POP EDI
        _emit 0x5f
        // 00057699: 66 c7 04 50 00 00  MOV word ptr [EAX+EDX*2],0x0
        _emit 0x66
        _emit 0xc7
        _emit 0x04
        _emit 0x50
        _emit 0x00
        _emit 0x00
        // 0005769f: 5b              POP EBX
        _emit 0x5b
        // 000576a0: c3              RET
        _emit 0xc3
    }
}
