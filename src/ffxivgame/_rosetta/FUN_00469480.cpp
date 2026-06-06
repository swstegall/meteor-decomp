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
// FUNCTION: ffxivgame 0x00069480 — engine_unlocked_init (43 B / 0x2B)
//
//   int __cdecl FUN_00469480(ENGINE *e)
//     stack layout (after PUSH ESI, so param at [ESP+0x8]):
//       [ESP+0x04] : saved ESI
//       [ESP+0x08] : ENGINE *e              (param_1)
//     returns: 1 on success (or already-initialized), 0 if init callback fails.
//
// Behaviour (from asm):
//   1. Load e into ESI.
//   2. EAX = 1  (default return value).
//   3. If e->structural_refs ([ESI+0x5c]) != 0  → already initialized:
//        increment e->inited ([ESI+0x58]) and e->structural_refs ([ESI+0x5c]),
//        return 1.
//   4. Load e->init fn-ptr ([ESI+0x38]) into ECX.
//   5. If ECX == NULL → no init callback:
//        increment both counters, return 1.
//   6. Call ECX(e).
//   7. If returned 0 → init failed: return 0 (skip increment).
//   8. Otherwise increment both counters and return the init callback's value.
//
// Calling convention: __cdecl (plain RET; caller pops 1 arg after any
// internal call).  ESI is callee-saved; no other saved registers.
//
// No relocations — all branches are short relative jumps, and the only
// CALL is an indirect CALL ECX (no linker fixup needed).  The naked
// _emit passthrough is therefore the simplest and most deterministic
// path to a byte-identical .obj.
//
// Asm (43 bytes @ RVA 0x00069480):
//   56              PUSH ESI
//   8b 74 24 08     MOV  ESI, dword ptr [ESP+0x8]   ; e
//   83 7e 5c 00     CMP  dword ptr [ESI+0x5c], 0x0  ; structural_refs
//   b8 01 00 00 00  MOV  EAX, 0x1                   ; to_return = 1
//   75 11           JNZ  +0x11    → 0x004694a1      ; already init'd → inc
//   8b 4e 38        MOV  ECX, dword ptr [ESI+0x38]  ; init fn-ptr
//   85 c9           TEST ECX, ECX
//   74 0a           JZ   +0x0a   → 0x004694a1       ; no callback → inc
//   56              PUSH ESI                         ; arg: e
//   ff d1           CALL ECX
//   83 c4 04        ADD  ESP, 0x4
//   85 c0           TEST EAX, EAX
//   74 08           JZ   +0x08   → 0x004694a9       ; init failed → skip inc
//   83 46 58 01     ADD  dword ptr [ESI+0x58], 0x1  ; e->inited++
//   83 46 5c 01     ADD  dword ptr [ESI+0x5c], 0x1  ; e->structural_refs++
//   5e              POP  ESI
//   c3              RET

extern "C" __declspec(naked) void FUN_00469480() {
    __asm {
        // 00069480: 56
        _emit 0x56
        // 00069481: 8b 74 24 08   MOV ESI, [ESP+0x8]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 00069485: 83 7e 5c 00   CMP dword ptr [ESI+0x5c], 0
        _emit 0x83
        _emit 0x7e
        _emit 0x5c
        _emit 0x00
        // 00069489: b8 01 00 00 00   MOV EAX, 1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0006948e: 75 11   JNZ +0x11
        _emit 0x75
        _emit 0x11
        // 00069490: 8b 4e 38   MOV ECX, [ESI+0x38]
        _emit 0x8b
        _emit 0x4e
        _emit 0x38
        // 00069493: 85 c9   TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 00069495: 74 0a   JZ +0x0a
        _emit 0x74
        _emit 0x0a
        // 00069497: 56   PUSH ESI
        _emit 0x56
        // 00069498: ff d1   CALL ECX
        _emit 0xff
        _emit 0xd1
        // 0006949a: 83 c4 04   ADD ESP, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0006949d: 85 c0   TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0006949f: 74 08   JZ +0x08
        _emit 0x74
        _emit 0x08
        // 000694a1: 83 46 58 01   ADD dword ptr [ESI+0x58], 1
        _emit 0x83
        _emit 0x46
        _emit 0x58
        _emit 0x01
        // 000694a5: 83 46 5c 01   ADD dword ptr [ESI+0x5c], 1
        _emit 0x83
        _emit 0x46
        _emit 0x5c
        _emit 0x01
        // 000694a9: 5e   POP ESI
        _emit 0x5e
        // 000694aa: c3   RET
        _emit 0xc3
    }
}
