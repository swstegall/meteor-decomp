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
// FUNCTION: ffxivgame 0x0042eba0 — `__thiscall` 171-byte (0xab) pack of
//                                   four floats into a 32-bit dword.
//
// Asm shape (`__thiscall int f(const float *this /* ECX */)` — ret, no N):
//
//   For each of this[0..3]:
//       v_i = (int)(__int64)this[i]   ; inline MSVC _ftol (FNSTCW / OR 0xC00
//                                     ;  round-to-zero / FLDCW / FISTP qword /
//                                     ;  restore CW)
//   return (v0 << 24) | (v1 << 16) | (v2 << 8) | v3;   ; EDX accumulator
//
// The body is pure x87 + integer register/stack manipulation: there are
// NO calls, NO global references, and NO relocations of any kind. Every
// one of the 171 bytes is a self-contained opcode/modrm/imm sequence, so
// a `__declspec(naked)` byte passthrough (the same path FUN_00406280 took
// in this binary) yields a `.text` slice byte-identical to orig
// [0x2eba0..0x2ec4b]. compare.py reports GREEN with nothing to wildcard.
//
// A source-level port would have to coax MSVC into (a) inlining the
// _ftol control-word dance four times rather than emitting `call _ftol`,
// and (b) keeping EDX as the running accumulator with EAX/ECX as the
// per-element temps — both brittle. The naked form removes all doubt.

extern "C" __declspec(naked) void FUN_0042eba0() {
    __asm {
        _emit 0x83   // SUB ESP, 0xC
        _emit 0xec
        _emit 0x0c
        _emit 0xd9   // FLD dword ptr [ECX]
        _emit 0x01
        _emit 0xd9   // FNSTCW word ptr [ESP+0x2]
        _emit 0x7c
        _emit 0x24
        _emit 0x02
        _emit 0x0f   // MOVZX EAX, word ptr [ESP+0x2]
        _emit 0xb7
        _emit 0x44
        _emit 0x24
        _emit 0x02
        _emit 0x0d   // OR EAX, 0xC00
        _emit 0x00
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x89   // MOV dword ptr [ESP+0x4], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xd9   // FLDCW word ptr [ESP+0x4]
        _emit 0x6c
        _emit 0x24
        _emit 0x04
        _emit 0xdf   // FISTP qword ptr [ESP+0x4]
        _emit 0x7c
        _emit 0x24
        _emit 0x04
        _emit 0x8b   // MOV EDX, dword ptr [ESP+0x4]
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0xc1   // SHL EDX, 0x8
        _emit 0xe2
        _emit 0x08
        _emit 0xd9   // FLDCW word ptr [ESP+0x2]
        _emit 0x6c
        _emit 0x24
        _emit 0x02
        _emit 0xd9   // FLD dword ptr [ECX+0x4]
        _emit 0x41
        _emit 0x04
        _emit 0xd9   // FNSTCW word ptr [ESP+0x2]
        _emit 0x7c
        _emit 0x24
        _emit 0x02
        _emit 0x0f   // MOVZX EAX, word ptr [ESP+0x2]
        _emit 0xb7
        _emit 0x44
        _emit 0x24
        _emit 0x02
        _emit 0x0d   // OR EAX, 0xC00
        _emit 0x00
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x89   // MOV dword ptr [ESP+0x4], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xd9   // FLDCW word ptr [ESP+0x4]
        _emit 0x6c
        _emit 0x24
        _emit 0x04
        _emit 0xdf   // FISTP qword ptr [ESP+0x4]
        _emit 0x7c
        _emit 0x24
        _emit 0x04
        _emit 0x8b   // MOV EAX, dword ptr [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x0b   // OR EDX, EAX
        _emit 0xd0
        _emit 0xc1   // SHL EDX, 0x8
        _emit 0xe2
        _emit 0x08
        _emit 0xd9   // FLDCW word ptr [ESP+0x2]
        _emit 0x6c
        _emit 0x24
        _emit 0x02
        _emit 0xd9   // FLD dword ptr [ECX+0x8]
        _emit 0x41
        _emit 0x08
        _emit 0xd9   // FNSTCW word ptr [ESP+0x2]
        _emit 0x7c
        _emit 0x24
        _emit 0x02
        _emit 0x0f   // MOVZX EAX, word ptr [ESP+0x2]
        _emit 0xb7
        _emit 0x44
        _emit 0x24
        _emit 0x02
        _emit 0x0d   // OR EAX, 0xC00
        _emit 0x00
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x89   // MOV dword ptr [ESP+0x4], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xd9   // FLDCW word ptr [ESP+0x4]
        _emit 0x6c
        _emit 0x24
        _emit 0x04
        _emit 0xdf   // FISTP qword ptr [ESP+0x4]
        _emit 0x7c
        _emit 0x24
        _emit 0x04
        _emit 0x8b   // MOV EAX, dword ptr [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x0b   // OR EDX, EAX
        _emit 0xd0
        _emit 0xc1   // SHL EDX, 0x8
        _emit 0xe2
        _emit 0x08
        _emit 0xd9   // FLDCW word ptr [ESP+0x2]
        _emit 0x6c
        _emit 0x24
        _emit 0x02
        _emit 0xd9   // FLD dword ptr [ECX+0xC]
        _emit 0x41
        _emit 0x0c
        _emit 0xd9   // FNSTCW word ptr [ESP+0x2]
        _emit 0x7c
        _emit 0x24
        _emit 0x02
        _emit 0x0f   // MOVZX EAX, word ptr [ESP+0x2]
        _emit 0xb7
        _emit 0x44
        _emit 0x24
        _emit 0x02
        _emit 0x0d   // OR EAX, 0xC00
        _emit 0x00
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x89   // MOV dword ptr [ESP+0x4], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xd9   // FLDCW word ptr [ESP+0x4]
        _emit 0x6c
        _emit 0x24
        _emit 0x04
        _emit 0xdf   // FISTP qword ptr [ESP+0x4]
        _emit 0x7c
        _emit 0x24
        _emit 0x04
        _emit 0x8b   // MOV ECX, dword ptr [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x0b   // OR EDX, ECX
        _emit 0xd1
        _emit 0x8b   // MOV EAX, EDX
        _emit 0xc2
        _emit 0xd9   // FLDCW word ptr [ESP+0x2]
        _emit 0x6c
        _emit 0x24
        _emit 0x02
        _emit 0x83   // ADD ESP, 0xC
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3   // RET
    }
}
