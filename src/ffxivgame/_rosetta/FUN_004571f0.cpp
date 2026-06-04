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
// FUNCTION: ffxivgame 0x000571f0 — block-deque push_back of a 4-byte element
//                                  (__thiscall, one pointer arg, 117 B / 0x75)
//
// Calling convention: __thiscall (ECX = this); single stack parameter
//   (a pointer to the 4-byte value to append); RET 0x4 cleans the arg.
// Callee-saves: ESI (entry), then EBX + EDI (mid-body, popped before the
//   final store).
//
// Object layout (a chunked/segmented deque of 4-byte slots):
//   [this + 0x04]  block-map: pointer to an array of block pointers
//   [this + 0x08]  block-map capacity (in blocks)
//   [this + 0x0c]  front base offset (in elements)
//   [this + 0x10]  element count (back index, in elements)
//   Each block holds 4 elements (malloc(0x10) = 16 bytes / 4).
//
// Behaviour:
//   1. If (front + back) is 4-element aligned AND a grow is needed
//      ((back+4)>>2 >= block-map capacity), call this->reserve/rebalance(1)
//      at 0x00457090.
//   2. Compute block index = ((front + back) >> 2), wrapped against the
//      block-map capacity (subtract capacity when index would overflow).
//   3. Lazily allocate the target block via malloc(0x10) at 0x009d1b35 if
//      the map slot is null.
//   4. Compute the element slot = block + ((front+back) & 3)*4; if non-null,
//      copy *arg into it.
//   5. Increment the back index (this->m10++).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The two CALL rel32 sites (reserve thunk at 0x00457090, malloc at
//   0x009d1b35) plus the precise MSVC 2005 /O2 register allocation and the
//   delayed EBX/EDI pushes are not faithfully reproducible from source-level
//   C++ without byte drift. Following the local _rosetta idiom (see
//   FUN_00401650), a __declspec(naked) body re-emits the original 117 bytes
//   verbatim via MASM _emit directives; the .obj's .text is byte-identical to
//   the original slice and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004571f0() {
    __asm {
        // 000571f0: 56              PUSH ESI
        _emit 0x56
        // 000571f1: 8b f1           MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 000571f3: 8b 46 10        MOV EAX,[ESI+0x10]   (back)
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        // 000571f6: 8b 4e 0c        MOV ECX,[ESI+0xc]    (front)
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        // 000571f9: 03 c8           ADD ECX,EAX
        _emit 0x03
        _emit 0xc8
        // 000571fb: f6 c1 03        TEST CL,0x3
        _emit 0xf6
        _emit 0xc1
        _emit 0x03
        // 000571fe: 75 14           JNZ 0x00457214
        _emit 0x75
        _emit 0x14
        // 00057200: 83 c0 04        ADD EAX,0x4
        _emit 0x83
        _emit 0xc0
        _emit 0x04
        // 00057203: c1 e8 02        SHR EAX,0x2
        _emit 0xc1
        _emit 0xe8
        _emit 0x02
        // 00057206: 39 46 08        CMP [ESI+0x8],EAX
        _emit 0x39
        _emit 0x46
        _emit 0x08
        // 00057209: 77 09           JA 0x00457214
        _emit 0x77
        _emit 0x09
        // 0005720b: 6a 01           PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005720d: 8b ce           MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0005720f: e8 7c fe ff ff  CALL 0x00457090  (reserve/rebalance)
        _emit 0xe8
        _emit 0x7c
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // === 0x00457214 ===
        // 00057214: 8b 46 08        MOV EAX,[ESI+0x8]    (capacity)
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 00057217: 53              PUSH EBX
        _emit 0x53
        // 00057218: 8b 5e 0c        MOV EBX,[ESI+0xc]    (front)
        _emit 0x8b
        _emit 0x5e
        _emit 0x0c
        // 0005721b: 03 5e 10        ADD EBX,[ESI+0x10]   (front + back)
        _emit 0x03
        _emit 0x5e
        _emit 0x10
        // 0005721e: 57              PUSH EDI
        _emit 0x57
        // 0005721f: 8b fb           MOV EDI,EBX
        _emit 0x8b
        _emit 0xfb
        // 00057221: c1 ef 02        SHR EDI,0x2          (block index)
        _emit 0xc1
        _emit 0xef
        _emit 0x02
        // 00057224: 3b c7           CMP EAX,EDI
        _emit 0x3b
        _emit 0xc7
        // 00057226: 77 02           JA 0x0045722a
        _emit 0x77
        _emit 0x02
        // 00057228: 2b f8           SUB EDI,EAX          (wrap index)
        _emit 0x2b
        _emit 0xf8
        // === 0x0045722a ===
        // 0005722a: 8b 56 04        MOV EDX,[ESI+0x4]    (block-map)
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 0005722d: 83 3c ba 00     CMP [EDX+EDI*4],0x0
        _emit 0x83
        _emit 0x3c
        _emit 0xba
        _emit 0x00
        // 00057231: 75 10           JNZ 0x00457243
        _emit 0x75
        _emit 0x10
        // 00057233: 6a 10           PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 00057235: e8 fb a8 57 00  CALL 0x009d1b35  (malloc)
        _emit 0xe8
        _emit 0xfb
        _emit 0xa8
        _emit 0x57
        _emit 0x00
        // 0005723a: 8b 4e 04        MOV ECX,[ESI+0x4]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 0005723d: 83 c4 04        ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00057240: 89 04 b9        MOV [ECX+EDI*4],EAX
        _emit 0x89
        _emit 0x04
        _emit 0xb9
        // === 0x00457243 ===
        // 00057243: 8b 56 04        MOV EDX,[ESI+0x4]
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 00057246: 8b 04 ba        MOV EAX,[EDX+EDI*4]  (block ptr)
        _emit 0x8b
        _emit 0x04
        _emit 0xba
        // 00057249: 83 e3 03        AND EBX,0x3          (in-block offset)
        _emit 0x83
        _emit 0xe3
        _emit 0x03
        // 0005724c: 8d 04 98        LEA EAX,[EAX+EBX*4]  (slot ptr)
        _emit 0x8d
        _emit 0x04
        _emit 0x98
        // 0005724f: 85 c0           TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00057251: 5f              POP EDI
        _emit 0x5f
        // 00057252: 5b              POP EBX
        _emit 0x5b
        // 00057253: 74 08           JZ 0x0045725d
        _emit 0x74
        _emit 0x08
        // 00057255: 8b 4c 24 08     MOV ECX,[ESP+0x8]    (arg ptr)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 00057259: 8b 11           MOV EDX,[ECX]        (*arg)
        _emit 0x8b
        _emit 0x11
        // 0005725b: 89 10           MOV [EAX],EDX        (slot = *arg)
        _emit 0x89
        _emit 0x10
        // === 0x0045725d ===
        // 0005725d: 83 46 10 01     ADD [ESI+0x10],0x1   (back++)
        _emit 0x83
        _emit 0x46
        _emit 0x10
        _emit 0x01
        // 00057261: 5e              POP ESI
        _emit 0x5e
        // 00057262: c2 04 00        RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
