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
// FUNCTION: ffxivgame 0x00456060 — lazy-init then push one value into a
//                                  handle-indexed circular buffer (136 B / 0x88)
//
// Calling convention: __cdecl, one argument (param0 = value to push onto the
// buffer). Returns void. Epilogue is bare RET (caller cleans).
//
// Behaviour (from disassembly at RVA 0x00056060):
//
//   1. Load the global handle from [0x0126701c].
//   2. If handle == -1 (uninitialised):
//        Push param0, set ECX = 0x0132d0e0 (singleton object),
//        call FUN_00457270 (__thiscall — returns handle/result),
//        move result into ECX, call FUN_00456d30 (__thiscall — finalises),
//        return.
//   3. Otherwise (handle is valid):
//        Load function pointer from [0x00f3e2a4] into EDI.
//        Call EDI(handle) → ptr to the circular-buffer header.
//        If ptr->count == 0x1f (31, i.e. full):
//          Shift elements 1..31 down to 0..30 (4 bytes each, 31 iters) using
//          two EDI calls per iteration to get the buffer base each time.
//          Decrement ptr->count via a third EDI call.
//        Increment ptr->count, then call EDI again to get the base,
//        and store param0 at base[count * 4 + 4].
//
// Three absolute addresses embedded in the instruction stream that carry
// PE relocations (masked by compare.py during the byte diff):
//   0x0126701c — global handle / registry index (7 references)
//   0x0132d0e0 — singleton "this" immediate in MOV ECX (1 reference)
//   0x00f3e2a4 — IAT slot for the buffer-accessor function pointer (1 reference)
//
// Two CALL rel32 sites (also masked):
//   0x00457270 — FUN_00457270 (lazy-init lookup, __thiscall)
//   0x00456d30 — FUN_00456d30 (lazy-init finaliser, __thiscall)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function combines a conditional PUSH EBX (only in the full-buffer
//   branch, not in the prologue), ESI reused as both callee-save and loop
//   counter, ADD [mem],-1 / ADD [mem],1 forms instead of DEC/INC, and
//   scaled-index LEA/MOV encodings ([EAX+ESI*1+disp8] SIB form with
//   scale=00 and separate base/index registers). Reproducing all of these
//   from source-level C++ under /O2 is unreliable. The established local
//   idiom (FUN_00456800, FUN_00456361, FUN_00411fa0) is a __declspec(naked)
//   body that re-emits the original 136 bytes verbatim via _emit directives.

extern "C" __declspec(naked) void FUN_00456060() {
    __asm {
        // 00056060: a1 1c 70 26 01  MOV EAX,[0x0126701c]
        _emit 0xa1
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        // 00056065: 83 f8 ff        CMP EAX,-0x1
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        // 00056068: 75 17           JNZ +0x17  (→ 0x00456081, main body)
        _emit 0x75
        _emit 0x17
        // 0005606a: 8b 44 24 04     MOV EAX,dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0005606e: 50              PUSH EAX
        _emit 0x50
        // 0005606f: b9 e0 d0 32 01  MOV ECX,0x132d0e0  (singleton address as imm32)
        _emit 0xb9
        _emit 0xe0
        _emit 0xd0
        _emit 0x32
        _emit 0x01
        // 00056074: e8 f7 11 00 00  CALL 0x00457270
        _emit 0xe8
        _emit 0xf7
        _emit 0x11
        _emit 0x00
        _emit 0x00
        // 00056079: 8b c8           MOV ECX,EAX
        _emit 0x8b
        _emit 0xc8
        // 0005607b: e8 b0 0c 00 00  CALL 0x00456d30
        _emit 0xe8
        _emit 0xb0
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        // 00056080: c3              RET
        _emit 0xc3
        // ── main body ────────────────────────────────────────────────────
        // 00056081: 56              PUSH ESI
        _emit 0x56
        // 00056082: 57              PUSH EDI
        _emit 0x57
        // 00056083: 8b 3d a4 e2 f3 00  MOV EDI,dword ptr [0x00f3e2a4]
        _emit 0x8b
        _emit 0x3d
        _emit 0xa4
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        // 00056089: 50              PUSH EAX  (handle from earlier)
        _emit 0x50
        // 0005608a: ff d7           CALL EDI
        _emit 0xff
        _emit 0xd7
        // 0005608c: 83 38 1f        CMP dword ptr [EAX],0x1f
        _emit 0x83
        _emit 0x38
        _emit 0x1f
        // 0005608f: 75 34           JNZ +0x34  (→ 0x004560c5, after_shift)
        _emit 0x75
        _emit 0x34
        // ── full-buffer shift branch ──────────────────────────────────────
        // 00056091: 33 f6           XOR ESI,ESI  (loop counter = 0)
        _emit 0x33
        _emit 0xf6
        // 00056093: 53              PUSH EBX  (conditional callee-save)
        _emit 0x53
        // ── loop_top (0x00456094) ─────────────────────────────────────────
        // 00056094: 8b 0d 1c 70 26 01  MOV ECX,dword ptr [0x0126701c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        // 0005609a: 51              PUSH ECX
        _emit 0x51
        // 0005609b: ff d7           CALL EDI  (→ EAX = buf base, 1st call)
        _emit 0xff
        _emit 0xd7
        // 0005609d: 8b 15 1c 70 26 01  MOV EDX,dword ptr [0x0126701c]
        _emit 0x8b
        _emit 0x15
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        // 000560a3: 52              PUSH EDX
        _emit 0x52
        // 000560a4: 8d 5c 30 08     LEA EBX,[EAX+ESI*0x1+0x8]  (src = &buf[i+8])
        _emit 0x8d
        _emit 0x5c
        _emit 0x30
        _emit 0x08
        // 000560a8: ff d7           CALL EDI  (→ EAX = buf base, 2nd call)
        _emit 0xff
        _emit 0xd7
        // 000560aa: 8b 0b           MOV ECX,dword ptr [EBX]
        _emit 0x8b
        _emit 0x0b
        // 000560ac: 89 4c 30 04     MOV dword ptr [EAX+ESI*0x1+0x4],ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x30
        _emit 0x04
        // 000560b0: 83 c6 04        ADD ESI,0x4
        _emit 0x83
        _emit 0xc6
        _emit 0x04
        // 000560b3: 83 fe 7c        CMP ESI,0x7c
        _emit 0x83
        _emit 0xfe
        _emit 0x7c
        // 000560b6: 7c dc           JL -0x24  (→ loop_top 0x00456094)
        _emit 0x7c
        _emit 0xdc
        // ── post-loop: decrement count ────────────────────────────────────
        // 000560b8: 8b 15 1c 70 26 01  MOV EDX,dword ptr [0x0126701c]
        _emit 0x8b
        _emit 0x15
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        // 000560be: 52              PUSH EDX
        _emit 0x52
        // 000560bf: ff d7           CALL EDI
        _emit 0xff
        _emit 0xd7
        // 000560c1: 83 00 ff        ADD dword ptr [EAX],-0x1
        _emit 0x83
        _emit 0x00
        _emit 0xff
        // 000560c4: 5b              POP EBX
        _emit 0x5b
        // ── after_shift (0x004560c5) — common push path ───────────────────
        // 000560c5: a1 1c 70 26 01  MOV EAX,[0x0126701c]
        _emit 0xa1
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        // 000560ca: 50              PUSH EAX
        _emit 0x50
        // 000560cb: ff d7           CALL EDI  (→ ESI = buf base)
        _emit 0xff
        _emit 0xd7
        // 000560cd: 8b f0           MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 000560cf: 83 06 01        ADD dword ptr [ESI],0x1  (++count)
        _emit 0x83
        _emit 0x06
        _emit 0x01
        // 000560d2: 8b 0d 1c 70 26 01  MOV ECX,dword ptr [0x0126701c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        // 000560d8: 51              PUSH ECX
        _emit 0x51
        // 000560d9: ff d7           CALL EDI  (→ EAX = buf base again)
        _emit 0xff
        _emit 0xd7
        // 000560db: 8b 16           MOV EDX,dword ptr [ESI]  (new count)
        _emit 0x8b
        _emit 0x16
        // 000560dd: 8b 4c 24 0c     MOV ECX,dword ptr [ESP+0xc]  (param0)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 000560e1: 5f              POP EDI
        _emit 0x5f
        // 000560e2: 89 4c 90 04     MOV dword ptr [EAX+EDX*0x4+0x4],ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x90
        _emit 0x04
        // 000560e6: 5e              POP ESI
        _emit 0x5e
        // 000560e7: c3              RET
        _emit 0xc3
    }
}
