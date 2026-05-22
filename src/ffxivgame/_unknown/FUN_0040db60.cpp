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
// FUNCTION: ffxivgame 0x0000db60 — FUN_0040db60 (77 B / 0x4d)
//                                   __thiscall pop-and-compute on a word-indexed
//                                   table; returns base + stride*val, or 0 if empty.
//
// Calling convention: __thiscall (ECX = this); plain RET (no stack args).
// Callee-saves pushed: ESI only (late-push after early-return guard).
// No local stack frame. No /GS cookie. No relocations.
//
// this->m_state layout (struct at [ECX+0x4]):
//   +0x00  int    base_val      — added to the return value
//   +0x04  short* word_array    — pointer to word value table
//   +0x08  short  total_count   — total number of entries
//   +0x0a  short  rem_count     — remaining (countdown) index; decremented each call
//   +0x0c  short  elem_size     — element stride multiplier
//   +0x10  int    pop_count     — cumulative pop counter (DWORD)
//   +0x18  DWORD  max_depth     — high-water mark of (total_count - rem_count)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The CMP word [EAX+0a], 0 / ADD word [EAX+0a], 0xffff memory-operand pattern
//   requires MSVC to not cache the field in a register across the comparison and
//   decrement.  In our source TU, MSVC 2005 /O2 instead emits MOVZX EDX + TEST
//   for the comparison and register-based ADD before storing back.  Since the 77
//   bytes carry no relocations (pure arithmetic, no CALL, no ABS address), a
//   naked-asm byte passthrough reproduces the original verbatim.

extern "C" __declspec(naked) void FUN_0040db60() {
    __asm {
        // 0000db60
        _emit 0x8b              // MOV EAX, dword ptr [ECX+0x4]
        _emit 0x41
        _emit 0x04
        // 0000db63
        _emit 0x66              // CMP word ptr [EAX+0xa], 0x0
        _emit 0x83
        _emit 0x78
        _emit 0x0a
        _emit 0x00
        // 0000db68
        _emit 0x75              // JNZ +3 (→ 0x0040db6d)
        _emit 0x03
        // 0000db6a
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        // 0000db6c
        _emit 0xc3              // RET
        // 0000db6d
        _emit 0x66              // ADD word ptr [EAX+0xa], 0xffff  (rem_count--)
        _emit 0x81
        _emit 0x40
        _emit 0x0a
        _emit 0xff
        _emit 0xff
        // 0000db73
        _emit 0x8b              // MOV EAX, dword ptr [ECX+0x4]
        _emit 0x41
        _emit 0x04
        // 0000db76
        _emit 0x0f              // MOVZX EDX, word ptr [EAX+0xa]  (idx = rem_count)
        _emit 0xb7
        _emit 0x50
        _emit 0x0a
        // 0000db7a
        _emit 0x56              // PUSH ESI
        // 0000db7b
        _emit 0x8b              // MOV ESI, dword ptr [EAX+0x4]   (word_array)
        _emit 0x70
        _emit 0x04
        // 0000db7e
        _emit 0x0f              // MOVZX ESI, word ptr [ESI+EDX*2] (val = word_array[idx])
        _emit 0xb7
        _emit 0x34
        _emit 0x56
        // 0000db82
        _emit 0x83              // ADD dword ptr [EAX+0x10], 0x1  (pop_count++)
        _emit 0x40
        _emit 0x10
        _emit 0x01
        // 0000db86
        _emit 0x8b              // MOV EAX, dword ptr [ECX+0x4]
        _emit 0x41
        _emit 0x04
        // 0000db89
        _emit 0x66              // MOV DX, word ptr [EAX+0x8]     (DX = total_count)
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 0000db8d
        _emit 0x66              // SUB DX, word ptr [EAX+0xa]     (DX = total_count - rem_count)
        _emit 0x2b
        _emit 0x50
        _emit 0x0a
        // 0000db91
        _emit 0x0f              // MOVZX EDX, DX                  (depth = (ushort)DX)
        _emit 0xb7
        _emit 0xd2
        // 0000db94
        _emit 0x39              // CMP dword ptr [EAX+0x18], EDX
        _emit 0x50
        _emit 0x18
        // 0000db97
        _emit 0x73              // JNC +3 (→ 0x0040db9c, skip update)
        _emit 0x03
        // 0000db99
        _emit 0x89              // MOV dword ptr [EAX+0x18], EDX  (max_depth = depth)
        _emit 0x50
        _emit 0x18
        // 0000db9c
        _emit 0x8b              // MOV ECX, dword ptr [ECX+0x4]
        _emit 0x49
        _emit 0x04
        // 0000db9f
        _emit 0x0f              // MOVZX EAX, word ptr [ECX+0xc]  (stride)
        _emit 0xb7
        _emit 0x41
        _emit 0x0c
        // 0000dba3
        _emit 0x0f              // MOVZX EDX, SI                  (val)
        _emit 0xb7
        _emit 0xd6
        // 0000dba6
        _emit 0x0f              // IMUL EAX, EDX                  (stride * val)
        _emit 0xaf
        _emit 0xc2
        // 0000dba9
        _emit 0x03              // ADD EAX, dword ptr [ECX]       (+ base_val)
        _emit 0x01
        // 0000dbab
        _emit 0x5e              // POP ESI
        // 0000dbac
        _emit 0xc3              // RET
    }
}
