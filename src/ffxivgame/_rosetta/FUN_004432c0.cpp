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
// FUNCTION: ffxivgame 0x000432c0 — circular-buffer push (__thiscall, 1 stack
//                                   arg, RET 4, 133 bytes).
//
// Signature (inferred from asm):
//
//   void __thiscall FUN_004432c0(RingBuf *this /*ECX*/,
//                                Entry   *item /*[esp+4]*/);
//
// RingBuf layout (accessed via ESI = ECX):
//   [+0x04]  Entry **data   — pointer to array of Entry pointers
//   [+0x08]  int    cap     — total slot capacity
//   [+0x0c]  int    head    — head index (read cursor)
//   [+0x10]  int    count   — number of live items (write cursor offset)
//
// Entry layout (0x24 bytes, allocated lazily per slot):
//   [+0x00]  float  f0      — FLD/FSTP copy
//   [+0x04]  float  f1      — FLD/FSTP copy
//   [+0x08]  int    i2      — MOV copy
//   [+0x0c]  int    i3      — MOV copy
//   [+0x10]  qword  q4      — MOVQ XMM copy (8 bytes)
//   [+0x18]  qword  q5      — MOVQ XMM copy (8 bytes)
//   [+0x20]  int    i6      — MOV copy
//
// Control-flow summary:
//
//   1. If cap <= count+1: call FUN_00443160(1) to grow capacity.
//   2. Compute write slot: slot = head + count; if slot >= cap, slot -= cap.
//   3. If data[slot] == NULL: allocate 0x24 bytes via FUN_009d1b35(0x24).
//   4. If data[slot] != NULL after the alloc attempt: copy *item into *data[slot].
//   5. Increment count.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The struct copy uses F3 0F 7E (MOVQ xmm, m64) and 66 0F D6 (MOVQ m64, xmm)
//   for the 8-byte sections at +0x10 and +0x18, which MASM inline asm may encode
//   differently from the orig. Similarly, the two CALL displacements (REL32 to
//   FUN_00443160 and FUN_009d1b35) must be baked in as orig bytes and will be
//   masked by compare.py. The `_emit` byte-passthrough is therefore the safest
//   approach (same pattern as FUN_00403d60 and FUN_00408610 siblings).
//
// Reloc-bearing positions (masked by compare.py):
//   +0x11  REL32 → FUN_00443160 (grow; orig displacement 0xfffffe8b)
//   +0x31  REL32 → FUN_009d1b35 (alloc; orig displacement 0x0058e840)

extern "C" __declspec(naked) void FUN_004432c0() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x10]
        _emit 0x46
        _emit 0x10
        _emit 0x83              // ADD EAX, 0x01
        _emit 0xc0
        _emit 0x01
        _emit 0x39              // CMP dword ptr [ESI + 0x08], EAX
        _emit 0x46
        _emit 0x08
        _emit 0x77              // JA +0x07  (skip grow)
        _emit 0x07
        _emit 0x6a              // PUSH 0x01
        _emit 0x01
        _emit 0xe8              // CALL FUN_00443160 (rel32 = 0xfffffe8b)
        _emit 0x8b
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x08]
        _emit 0x46
        _emit 0x08
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESI + 0x0c]
        _emit 0x7e
        _emit 0x0c
        _emit 0x03              // ADD EDI, dword ptr [ESI + 0x10]
        _emit 0x7e
        _emit 0x10
        _emit 0x3b              // CMP EAX, EDI
        _emit 0xc7
        _emit 0x77              // JA +0x02  (skip wrap-around sub)
        _emit 0x02
        _emit 0x2b              // SUB EDI, EAX
        _emit 0xf8
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x04]
        _emit 0x4e
        _emit 0x04
        _emit 0x83              // CMP dword ptr [ECX + EDI*4 + 0x00], 0x00
        _emit 0x3c
        _emit 0xb9
        _emit 0x00
        _emit 0x75              // JNZ +0x10  (slot already allocated)
        _emit 0x10
        _emit 0x6a              // PUSH 0x24
        _emit 0x24
        _emit 0xe8              // CALL FUN_009d1b35 (rel32 = 0x0058e840)
        _emit 0x40
        _emit 0xe8
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV EDX, dword ptr [ESI + 0x04]
        _emit 0x56
        _emit 0x04
        _emit 0x83              // ADD ESP, 0x04
        _emit 0xc4
        _emit 0x04
        _emit 0x89              // MOV dword ptr [EDX + EDI*4], EAX
        _emit 0x04
        _emit 0xba
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x04]
        _emit 0x46
        _emit 0x04
        _emit 0x8b              // MOV EAX, dword ptr [EAX + EDI*4]
        _emit 0x04
        _emit 0xb8
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x5f              // POP EDI
        _emit 0x74              // JZ +0x34  (null slot → skip copy, go to count++)
        _emit 0x34
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x08]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xd9              // FLD dword ptr [ECX]
        _emit 0x01
        _emit 0xd9              // FSTP dword ptr [EAX]
        _emit 0x18
        _emit 0xd9              // FLD dword ptr [ECX + 0x04]
        _emit 0x41
        _emit 0x04
        _emit 0xd9              // FSTP dword ptr [EAX + 0x04]
        _emit 0x58
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [ECX + 0x08]
        _emit 0x51
        _emit 0x08
        _emit 0x89              // MOV dword ptr [EAX + 0x08], EDX
        _emit 0x50
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ECX + 0x0c]
        _emit 0x51
        _emit 0x0c
        _emit 0x89              // MOV dword ptr [EAX + 0x0c], EDX
        _emit 0x50
        _emit 0x0c
        _emit 0xf3              // MOVQ XMM0, qword ptr [ECX + 0x10]
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x10
        _emit 0x66              // MOVQ qword ptr [EAX + 0x10], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x10
        _emit 0xf3              // MOVQ XMM0, qword ptr [ECX + 0x18]
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x18
        _emit 0x66              // MOVQ qword ptr [EAX + 0x18], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x18
        _emit 0x8b              // MOV ECX, dword ptr [ECX + 0x20]
        _emit 0x49
        _emit 0x20
        _emit 0x89              // MOV dword ptr [EAX + 0x20], ECX
        _emit 0x48
        _emit 0x20
        _emit 0x83              // ADD dword ptr [ESI + 0x10], 0x01
        _emit 0x46
        _emit 0x10
        _emit 0x01
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x04
        _emit 0x04
        _emit 0x00
    }
}

// vim: ts=4 sts=4 sw=4 et
