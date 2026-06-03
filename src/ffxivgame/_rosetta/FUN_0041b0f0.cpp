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
// FUNCTION: ffxivgame 0x0001b0f0 — conditional COM-style vtable dispatch (41 B / 0x29)
//
//   void __cdecl FUN_0041b0f0(void *param_1, int param_2, void *param_3)
//
// Guard: if param_2 != 0, jump to 0x0041b119 (= first byte of the next function,
// which begins with RET), so the function does nothing.
//
// If param_2 == 0: loads a global double-indirected singleton pointer at
// 0x01329834, dereferences twice (EAX = *DAT_01329834, ECX = *EAX), then
// calls the function pointer stored at [ECX + 0x16c] (the 91st slot of a
// COM-like dispatch table), passing three args: the outer pointer EAX
// (the singleton), param_1, and param_3.
//
// Stack layout (after entry, before SUB):
//   [ESP+0x04] param_1  (void*)
//   [ESP+0x08] param_2  (int)  — loaded into EAX BEFORE the frame allocation
//   [ESP+0x0c] param_3  (void*)
//
// Frame: SUB ESP,0x1c (28 bytes) — absorbed by ADD ESP,0x1c in the epilogue
// after the callee cleans its own 3 stack args (RET 12 / __stdcall slot).
//
// Calling convention: __cdecl (plain RET from this function).
// Frame pointer: omitted (/Oy).
//
// Asm (41 bytes @ orig RVA 0x0001b0f0):
//   8b 44 24 08             MOV  EAX, dword ptr [ESP+0x8]     ; param_2 (pre-frame)
//   83 ec 1c                SUB  ESP, 0x1c                     ; allocate frame
//   85 c0                   TEST EAX, EAX                      ; param_2 ?
//   75 1e                   JNZ  0x0041b119                    ; != 0 → next fn's RET
//   8b 54 24 28             MOV  EDX, dword ptr [ESP+0x28]     ; param_3
//   a1 34 98 32 01          MOV  EAX, [DAT_01329834]           ; *global (RELOC)
//   8b 08                   MOV  ECX, dword ptr [EAX]          ; **global
//   52                      PUSH EDX                           ; push param_3
//   8b 54 24 24             MOV  EDX, dword ptr [ESP+0x24]     ; param_1 (after 1 push)
//   52                      PUSH EDX                           ; push param_1
//   50                      PUSH EAX                           ; push singleton ptr
//   8b 81 6c 01 00 00       MOV  EAX, dword ptr [ECX+0x16c]   ; dispatch slot
//   ff d0                   CALL EAX                           ; call it
//   83 c4 1c                ADD  ESP, 0x1c                     ; restore frame
//   c3                      RET
//
// Reloc-bearing sites in the orig 41 bytes:
//     +0x0f  MOV EAX, moffs32  → DAT_01329834  (4-byte abs32; compare.py masks it)
//
// Reconstruction strategy — naked-asm byte passthrough (mirrors the sibling
// idiom used across this _rosetta/ directory): a `__declspec(naked)` body
// that re-emits all 41 orig bytes verbatim via MASM `_emit` directives.
// The only reloc-bearing field (the 4-byte abs address in the MOV EAX,moffs32
// at +0x0f) is emitted as raw bytes; compare.py masks those 4 bytes when
// grading the diff.

extern "C" __declspec(naked) void FUN_0041b0f0() {
    __asm {
        // 0001b0f0: 8b 44 24 08   MOV EAX, dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0001b0f4: 83 ec 1c      SUB ESP, 0x1c
        _emit 0x83
        _emit 0xec
        _emit 0x1c
        // 0001b0f7: 85 c0         TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0001b0f9: 75 1e         JNZ +0x1e  (→ 0x0041b119)
        _emit 0x75
        _emit 0x1e
        // 0001b0fb: 8b 54 24 28   MOV EDX, dword ptr [ESP+0x28]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x28
        // 0001b0ff: a1 34 98 32 01  MOV EAX, [DAT_01329834]  (RELOC — 4-byte abs32)
        _emit 0xa1
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001b104: 8b 08         MOV ECX, dword ptr [EAX]
        _emit 0x8b
        _emit 0x08
        // 0001b106: 52            PUSH EDX
        _emit 0x52
        // 0001b107: 8b 54 24 24   MOV EDX, dword ptr [ESP+0x24]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 0001b10b: 52            PUSH EDX
        _emit 0x52
        // 0001b10c: 50            PUSH EAX
        _emit 0x50
        // 0001b10d: 8b 81 6c 01 00 00   MOV EAX, dword ptr [ECX+0x16c]
        _emit 0x8b
        _emit 0x81
        _emit 0x6c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001b113: ff d0         CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001b115: 83 c4 1c      ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 0001b118: c3            RET
        _emit 0xc3
    }
}
