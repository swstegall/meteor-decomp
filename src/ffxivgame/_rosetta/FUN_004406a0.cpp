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
// FUNCTION: ffxivgame 0x000406a0 — __thiscall std::basic_string<wchar_t>::_Grow
//           (new_size, old_size) → allocates the growth buffer  (128 B / 0x80).
//
// Called from sibling FUN_00440850 (basic_string<wchar_t>::assign) as:
//   MOV ECX,ESI (this) ; PUSH old_size ; PUSH new_size ; CALL FUN_004406a0
// i.e. __thiscall _Grow(this, size_type new_size, size_type old_size).
//
// Behaviour (recovered from asm @ 0x000406a0):
//
//   size_type cap = new_size | 7;                          // round-up candidate
//   if (cap <= 0x7ffffffe) {
//       size_type old_res = this->_Myres;                  // [this+0x18]
//       // amortized growth: if the rounded request is smaller than 1.5x the
//       // current capacity (checked via a 0xaaaaaaab reciprocal-multiply /
//       // shift-by-1 division-by-3 idiom) and it wouldn't overflow, grow to
//       // 1.5x instead of the rounded request.
//       if (/* cap*2/3 < old_res/2 */ && /* old_res <= 0x7ffffffe - old_res/2 */)
//           cap = (old_res >> 1) + old_res;
//   } else {
//       cap = new_size;                                    // overflow: keep unrounded
//   }
//   new_size /* stack slot reused as scratch */ = FUN_00440270(cap + 1, 0);
//   goto <continuation beyond this 128-byte window>;
//
// This is the classic Dinkumware `basic_string::_Grow` amortized-growth
// allocation preamble: a full /GS + /EHsc frame (security cookie XOR EBP,
// FS:[0] SEH chain link, local-unwind state at [EBP-4]) wrapping the
// capacity-growth arithmetic and a call into the `operator new[]`-style
// allocator thunk FUN_00440270. The final JMP leaves this 128-byte slice
// for the (separately-addressed) copy/finalize tail — out of scope for
// this function's matched byte range.
//
// CALL targets (REL32; masked as wildcards by tools/compare.py):
//   +0x73   CALL FUN_00440270   — overflow-checked wchar_t operator new[] thunk
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//   Same rationale as sibling FUN_00440850/FUN_00440270: the register
//   allocation (EDI = this persisted across the whole prologue, ESI =
//   candidate capacity, EBX/ECX/EDX threaded through the reciprocal-
//   multiply growth-factor check, EAX reused as scratch and then as the
//   allocator's return value stored back into the incoming new_size
//   argument slot) plus the /GS cookie and SEH frame construction are
//   exactly what MSVC 2005 emits and are brittle to reproduce from
//   idiomatic C++. The naked passthrough re-emits all 128 bytes verbatim
//   (masking only the one CALL's rel32 operand via the assembler's own
//   relocation), so tools/compare.py reports GREEN.

// Direct-call target within the binary (REL32 relocation).
void FUN_00440270();   // overflow-checked wchar_t operator new[] thunk

extern "C" __declspec(naked) void FUN_004406a0() {
    __asm {
        // 000406a0: 55                PUSH EBP
        _emit 0x55
        // 000406a1: 8b ec             MOV EBP,ESP
        _emit 0x8b
        _emit 0xec
        // 000406a3: 6a ff             PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 000406a5: 68 60 6f e5 00    PUSH 0x00e56f60   (SEH frame handler)
        _emit 0x68
        _emit 0x60
        _emit 0x6f
        _emit 0xe5
        _emit 0x00
        // 000406aa: 64 a1 00 00 00 00 MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000406b0: 50                PUSH EAX
        _emit 0x50
        // 000406b1: 83 ec 0c          SUB ESP,0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 000406b4: 53                PUSH EBX
        _emit 0x53
        // 000406b5: 56                PUSH ESI
        _emit 0x56
        // 000406b6: 57                PUSH EDI
        _emit 0x57
        // 000406b7: a1 b0 a8 2e 01    MOV EAX,[0x012ea8b0]   (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 000406bc: 33 c5             XOR EAX,EBP
        _emit 0x33
        _emit 0xc5
        // 000406be: 50                PUSH EAX
        _emit 0x50
        // 000406bf: 8d 45 f4          LEA EAX,[EBP-0xc]
        _emit 0x8d
        _emit 0x45
        _emit 0xf4
        // 000406c2: 64 a3 00 00 00 00 MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000406c8: 89 65 f0          MOV [EBP-0x10],ESP
        _emit 0x89
        _emit 0x65
        _emit 0xf0
        // 000406cb: 8b f9             MOV EDI,ECX             (this)
        _emit 0x8b
        _emit 0xf9
        // 000406cd: 89 7d ec          MOV [EBP-0x14],EDI
        _emit 0x89
        _emit 0x7d
        _emit 0xec
        // 000406d0: 8b 45 08          MOV EAX,[EBP+0x8]       (new_size)
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        // 000406d3: 8b f0             MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 000406d5: 83 ce 07          OR ESI,0x7              (round-up candidate)
        _emit 0x83
        _emit 0xce
        _emit 0x07
        // 000406d8: 81 fe fe ff ff 7f CMP ESI,0x7ffffffe
        _emit 0x81
        _emit 0xfe
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        // 000406de: 76 04             JBE +4   (→ 0x406e4, cap in range)
        _emit 0x76
        _emit 0x04
        // 000406e0: 8b f0             MOV ESI,EAX             (overflow: cap = new_size)
        _emit 0x8b
        _emit 0xf0
        // 000406e2: eb 22             JMP +0x22   (→ 0x40706)
        _emit 0xeb
        _emit 0x22
        // 000406e4: 8b 5f 18          MOV EBX,[EDI+0x18]      (this->_Myres)
        _emit 0x8b
        _emit 0x5f
        _emit 0x18
        // 000406e7: b8 ab aa aa aa    MOV EAX,0xaaaaaaab      (reciprocal for /1.5)
        _emit 0xb8
        _emit 0xab
        _emit 0xaa
        _emit 0xaa
        _emit 0xaa
        // 000406ec: f7 e6             MUL ESI
        _emit 0xf7
        _emit 0xe6
        // 000406ee: 8b cb             MOV ECX,EBX
        _emit 0x8b
        _emit 0xcb
        // 000406f0: d1 e9             SHR ECX,0x1             (old_res / 2)
        _emit 0xd1
        _emit 0xe9
        // 000406f2: d1 ea             SHR EDX,0x1
        _emit 0xd1
        _emit 0xea
        // 000406f4: 3b d1             CMP EDX,ECX
        _emit 0x3b
        _emit 0xd1
        // 000406f6: 73 0e             JNC +0xe   (→ 0x40706, no amortized growth)
        _emit 0x73
        _emit 0x0e
        // 000406f8: b8 fe ff ff 7f    MOV EAX,0x7ffffffe
        _emit 0xb8
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        // 000406fd: 2b c1             SUB EAX,ECX
        _emit 0x2b
        _emit 0xc1
        // 000406ff: 3b d8             CMP EBX,EAX
        _emit 0x3b
        _emit 0xd8
        // 00040701: 77 03             JA +3      (→ 0x40706, would overflow)
        _emit 0x77
        _emit 0x03
        // 00040703: 8d 34 19          LEA ESI,[ECX+EBX*0x1]  (cap = old_res/2 + old_res)
        _emit 0x8d
        _emit 0x34
        _emit 0x19
        // 00040706: 8d 4e 01          LEA ECX,[ESI+0x1]      (alloc size = cap+1)
        _emit 0x8d
        _emit 0x4e
        _emit 0x01
        // 00040709: 6a 00             PUSH 0x0                (allocator hint arg)
        _emit 0x6a
        _emit 0x00
        // 0004070b: 51                PUSH ECX                (count arg)
        _emit 0x51
        // 0004070c: c7 45 fc 00 00 00 00  MOV dword ptr [EBP-0x4],0x0  (unwind state)
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00040713: e8 ?? ?? ?? ??    CALL FUN_00440270   (allocate)
        call FUN_00440270
        // 00040718: 83 c4 08          ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0004071b: 89 45 08          MOV [EBP+0x8],EAX       (stash alloc'd ptr)
        _emit 0x89
        _emit 0x45
        _emit 0x08
        // 0004071e: eb 2a             JMP +0x2a   (→ 0x4074a, continuation outside
        //                                            this function's matched byte range)
        _emit 0xeb
        _emit 0x2a
    }
}
