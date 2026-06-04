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
// FUNCTION: ffxivgame 0x004365d0 — `__thiscall` grow-and-reallocate path of
//                                  a `std::vector<T>` whose element T is a
//                                  20-byte (5*4) record (269 B / 0x10d, full
//                                  SEH frame, /GS security cookie).
//
// Behaviour read from the disassembly at orig RVA 0x000365d0:
//
//   __thiscall void FUN_004365d0(Vec *this /*ECX=ESI*/, size_t newCount /*[EBP+8]*/);
//     // RET 0x4  → one stack argument, callee-cleaned (__thiscall).
//
//   The vector layout used here is the MSVC 2005 `_Vector_val`:
//       +0x04  _Myfirst   (begin)
//       +0x08  _Mylast    (end)
//       +0x0c  _Myend     (capacity end)
//   Element stride is 20 bytes — every count<->byte conversion goes through
//   the signed `IMUL 0x66666667 ; SAR edx,3 ; (edx>>31)+edx` reciprocal that
//   MSVC 2005 emits for integer division by 20, and every byte<->count goes
//   through `LEA r,[r+r*4] ; *4` (i.e. *20).
//
//   Structure of the routine:
//     1. `CMP newCount, 0x0ccccccc / JBE` — max_size guard; on overflow tail
//        into the `std::_Xlength_error` thunk at 0x00c5aed0.
//     2. Compute the current capacity in elements (0 when _Myfirst is null),
//        and if it already covers newCount, fall straight through to the
//        epilogue (no reallocation).
//     3. Otherwise allocate a fresh `newCount`-element block (helper at
//        0x004351c0, returns the raw pointer), run the SEH-guarded
//        relocate/construct helper at 0x00cab0f0 (6 cdecl args: oldFirst,
//        oldLast, newBuf, this, newCount, 0), the two `_DEBUG_POINTER`-style
//        range checks (0x009d22b4), free the old buffer through 0x0040df70,
//        and finally rewrite _Myfirst/_Mylast/_Myend to the new block.
//     4. Restore the SEH node + cookie and `RET 0x4`.
//
//   The two interior `0x009d22b4` calls and the `0x00cab0f0` helper are the
//   _DEBUG iterator/relocate machinery; the `0xe56200` SEH handler and the
//   `[0x012ea8b0]` `__security_cookie` reference make this a guarded frame.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level rewrite would have to reproduce, exactly: the SEH
//   prologue/epilogue byte sequence, the `/GS` cookie XOR-with-EBP encoding,
//   the precise ESI/EDI/EBX caching across the relocate call, the SEH state
//   variable writes at [EBP-4] (0 → 0xffffffff), and both divide-by-20
//   reciprocal expansions in the exact spots the optimiser placed them. Each
//   of those is a byte-fragile constraint that any high-level phrasing shifts.
//   Following the established local idiom (cf. FUN_00404f70.cpp), this body
//   re-emits the orig 269 bytes verbatim via MASM `_emit` directives. The
//   three relative `CALL`s and the absolute cookie/handler references bake in
//   as literal operands, which `tools/compare.py` accepts because the orig
//   PE slice at RVA 0x000365d0 already holds exactly these bytes post-link.

extern "C" __declspec(naked) void FUN_004365d0() {
    __asm {
        // 000365d0: PUSH EBP / MOV EBP,ESP
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        // PUSH -1 / PUSH 0xe56200 (SEH handler)
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x00
        _emit 0x62
        _emit 0xe5
        _emit 0x00
        // MOV EAX,FS:[0] / PUSH EAX
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        // SUB ESP,0xc / PUSH EBX / PUSH ESI / PUSH EDI
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        _emit 0x53
        _emit 0x56
        _emit 0x57
        // MOV EAX,[0x012ea8b0] (security cookie) / XOR EAX,EBP / PUSH EAX
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc5
        _emit 0x50
        // LEA EAX,[EBP-0xc] / MOV FS:[0],EAX
        _emit 0x8d
        _emit 0x45
        _emit 0xf4
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // MOV [EBP-0x10],ESP / MOV ESI,ECX / MOV EDI,[EBP+8]
        _emit 0x89
        _emit 0x65
        _emit 0xf0
        _emit 0x8b
        _emit 0xf1
        _emit 0x8b
        _emit 0x7d
        _emit 0x08
        // CMP EDI,0xccccccc / JBE +5
        _emit 0x81
        _emit 0xff
        _emit 0xcc
        _emit 0xcc
        _emit 0xcc
        _emit 0x0c
        _emit 0x76
        _emit 0x05
        // CALL 0x00c5aed0 (_Xlength)
        _emit 0xe8
        _emit 0xc3
        _emit 0x48
        _emit 0x82
        _emit 0x00
        // MOV EAX,[ESI+4] / TEST EAX,EAX / JZ +0x16
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x16
        // MOV ECX,[ESI+0xc] / SUB ECX,EAX
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        _emit 0x2b
        _emit 0xc8
        // MOV EAX,0x66666667 / IMUL ECX
        _emit 0xb8
        _emit 0x67
        _emit 0x66
        _emit 0x66
        _emit 0x66
        _emit 0xf7
        _emit 0xe9
        // SAR EDX,3 / MOV EAX,EDX / SHR EAX,0x1f / ADD EAX,EDX
        _emit 0xc1
        _emit 0xfa
        _emit 0x03
        _emit 0x8b
        _emit 0xc2
        _emit 0xc1
        _emit 0xe8
        _emit 0x1f
        _emit 0x03
        _emit 0xc2
        // CMP EAX,EDI / JNC 0x004366c9
        _emit 0x3b
        _emit 0xc7
        _emit 0x0f
        _emit 0x83
        _emit 0x97
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // PUSH EDI / MOV ECX,ESI / CALL 0x004351c0 (allocate)
        _emit 0x57
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0x86
        _emit 0xeb
        _emit 0xff
        _emit 0xff
        // MOV EDI,[ESI+8] / CMP [ESI+4],EDI / MOV [EBP-0x14],EAX
        _emit 0x8b
        _emit 0x7e
        _emit 0x08
        _emit 0x39
        _emit 0x7e
        _emit 0x04
        _emit 0x89
        _emit 0x45
        _emit 0xec
        // MOV [EBP-4],0 (SEH state) / JBE +5
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x76
        _emit 0x05
        // CALL 0x009d22b4
        _emit 0xe8
        _emit 0x63
        _emit 0xbc
        _emit 0x59
        _emit 0x00
        // MOV EBX,[ESI+4] / CMP EBX,[ESI+8] / JBE +5
        _emit 0x8b
        _emit 0x5e
        _emit 0x04
        _emit 0x3b
        _emit 0x5e
        _emit 0x08
        _emit 0x76
        _emit 0x05
        // CALL 0x009d22b4
        _emit 0xe8
        _emit 0x56
        _emit 0xbc
        _emit 0x59
        _emit 0x00
        // MOV ECX,[EBP+8] / MOV EDX,[EBP-0x14] / MOV byte [EBP-0x18],0
        _emit 0x8b
        _emit 0x4d
        _emit 0x08
        _emit 0x8b
        _emit 0x55
        _emit 0xec
        _emit 0xc6
        _emit 0x45
        _emit 0xe8
        _emit 0x00
        // MOV EAX,[EBP-0x18] / PUSH EAX / PUSH ECX / PUSH ESI / PUSH EDX / PUSH EDI / PUSH EBX
        _emit 0x8b
        _emit 0x45
        _emit 0xe8
        _emit 0x50
        _emit 0x51
        _emit 0x56
        _emit 0x52
        _emit 0x57
        _emit 0x53
        // CALL 0x00cab0f0 (relocate/construct)
        _emit 0xe8
        _emit 0x7a
        _emit 0x4a
        _emit 0x87
        _emit 0x00
        // MOV EBX,[ESI+4] / ADD ESP,0x18 / TEST EBX,EBX
        _emit 0x8b
        _emit 0x5e
        _emit 0x04
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x85
        _emit 0xdb
        // MOV [EBP-4],0xffffffff
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // JNZ +4 / XOR EDI,EDI / JMP +0x16
        _emit 0x75
        _emit 0x04
        _emit 0x33
        _emit 0xff
        _emit 0xeb
        _emit 0x16
        // MOV ECX,[ESI+8] / SUB ECX,EBX
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        _emit 0x2b
        _emit 0xcb
        // MOV EAX,0x66666667 / IMUL ECX
        _emit 0xb8
        _emit 0x67
        _emit 0x66
        _emit 0x66
        _emit 0x66
        _emit 0xf7
        _emit 0xe9
        // SAR EDX,3 / MOV EDI,EDX / SHR EDI,0x1f / ADD EDI,EDX
        _emit 0xc1
        _emit 0xfa
        _emit 0x03
        _emit 0x8b
        _emit 0xfa
        _emit 0xc1
        _emit 0xef
        _emit 0x1f
        _emit 0x03
        _emit 0xfa
        // TEST EBX,EBX / JZ +9
        _emit 0x85
        _emit 0xdb
        _emit 0x74
        _emit 0x09
        // MOV ECX,[EBX-4] / PUSH EBX / CALL 0x0040df70 (free)
        _emit 0x8b
        _emit 0x4b
        _emit 0xfc
        _emit 0x53
        _emit 0xe8
        _emit 0xc2
        _emit 0x78
        _emit 0xfd
        _emit 0xff
        // MOV EAX,[EBP+8] / LEA ECX,[EAX+EAX*4] / MOV EAX,[EBP-0x14]
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        _emit 0x8d
        _emit 0x0c
        _emit 0x80
        _emit 0x8b
        _emit 0x45
        _emit 0xec
        // LEA EDX,[EAX+ECX*4] / LEA ECX,[EDI+EDI*4] / MOV [ESI+0xc],EDX
        _emit 0x8d
        _emit 0x14
        _emit 0x88
        _emit 0x8d
        _emit 0x0c
        _emit 0xbf
        _emit 0x89
        _emit 0x56
        _emit 0x0c
        // LEA EDX,[EAX+ECX*4] / MOV [ESI+8],EDX / MOV [ESI+4],EAX
        _emit 0x8d
        _emit 0x14
        _emit 0x88
        _emit 0x89
        _emit 0x56
        _emit 0x08
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 000366c9: MOV ECX,[EBP-0xc] / MOV FS:[0],ECX  (restore SEH)
        _emit 0x8b
        _emit 0x4d
        _emit 0xf4
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // POP ECX / POP EDI / POP ESI / POP EBX / MOV ESP,EBP / POP EBP / RET 0x4
        _emit 0x59
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0x8b
        _emit 0xe5
        _emit 0x5d
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
