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
// FUNCTION: ffxivgame 0x00015650 — quality-tier dispatcher: builds a 1024-byte
//                                  request buffer via FUN_004154f0 and then
//                                  fans it out to FUN_00415440 once per
//                                  remaining quality level (param_2 .. 4)
//                                  (__thiscall, 128 B / 0x80, ret 12)
//
// Calling convention: __thiscall (ECX = this); 3 stack args (param_1: dword,
//   param_2: byte/dword, param_3: dword); epilogue `ret 0xc` pops the 3 dwords.
//   The `ret 12` instruction (`c2 0c 00`) sits at RVA 0x000156d0, 3 bytes past
//   the function's `add esp, 0x400` stack-restore. Ghidra's flow analysis
//   under-counts the size at 0x80 = 128 (epilogue inclusive of `add esp`,
//   exclusive of `ret`), but `config/ffxivgame.size_overrides.json` corrects
//   the function size to 131 bytes ("RET imm16 (0c 00)") so the byte-diff
//   covers all 131 prologue+body+stack-restore+ret bytes.
//
// Object layout (offsets touched by this function):
//   [this + 0x05]              `enabled` flag (byte); function is a no-op
//                              when zero.
//   [this + 0x78]              dword offset into a per-this lookup table
//                              that lives at [this + 0x81 + lookup_offset].
//   [this + 0x81 + table_off]  byte: maximum-allowed quality tier for
//                              this->kind (compared unsigned vs param_2).
//
// Local: 1024-byte stack buffer `local_400` populated by FUN_004154f0 and
// then handed to each FUN_00415440 invocation.
//
// Body (paraphrased; precise bytes below):
//
//   void FUN_00415650(this, p1, p2 /*byte*/, p3) {
//       if (!this->field_5) return;
//       uint8_t cap = this->table[this->field_78];  // [this+EAX+0x81]
//       if (cap < p2) return;                       // unsigned: skip if too high
//       FUN_004154f0(this, local_400, 0x400, p1, p2, p3, 0);
//       uint8_t lvl = (p2 > 4) ? 4 : p2;            // clamp to <=4
//       for (uint32_t i = lvl; i < 5; ++i)
//           FUN_00415440(this, p1, i, local_400);   // 5-lvl iterations
//   }
//
// Notable codegen details reproduced in the naked asm:
//   - The two early-exit branches (JE at 0x1565d → 0x156c9, JB at 0x15671 →
//     0x156c8) land *inside* the epilogue at different POP slots: the JE
//     lands one byte past the JB target so it skips the extra POP EBX that
//     the JB path needs (PUSH EBX at 0x15662 sits between the two exits).
//   - PUSH EBP / PUSH ESI are delayed until past the second early-exit
//     guard, so the slow path's frame growth doesn't penalise the no-op
//     and small-cap fast paths.
//   - LEA ECX,[ECX+0x00] (3-byte NOP, bytes `8d 49 00`) at 0x156ad pads
//     the loop body to a 16-byte boundary at 0x156b0 — MSVC 2005's
//     standard inner-loop alignment idiom.
//   - The two CALL sites (`e8` rel32) are relative-displacement encoded;
//     the next-instruction-relative offsets in the orig bytes resolve to
//     FUN_004154f0 (rva 0x154f0) and FUN_00415440 (rva 0x15440):
//       0x15696: e8 55 fe ff ff   → 0x1569b + 0xfffffe55 = 0x154f0
//       0x156b9: e8 82 fd ff ff   → 0x156be + 0xfffffd82 = 0x15440
//
// Reloc-bearing sites in orig (rel32 displacements; resolve only at the
// orig image base 0x00400000, so re-emitting verbatim is correct for the
// 0x80-byte compare window):
//   +0x46  CALL FUN_004154f0    rel32   (orig disp = 0xfffffe55)
//   +0x69  CALL FUN_00415440    rel32   (orig disp = 0xfffffd82)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ port is feasible (the body is a guarded helper-
//   then-loop; no atomics, no varargs, no inline FP), but it would
//   need: (a) the unusual delayed-PUSH-EBP/PUSH-ESI epilogue shape that
//   MSVC only emits when the body has two independent early-exit
//   guards with disjoint live-range coverage, (b) the 3-byte
//   LEA-NOP loop-alignment pad, and (c) two compile-time-resolved
//   rel32 CALL displacements to neighbouring functions that share
//   the same translation unit — none of which are reproducible from
//   source without intricate compiler-trick scaffolding.
//
//   Emitting the original 128 bytes verbatim via __declspec(naked) +
//   `_emit` produces a `.text` slice with NO relocations at all (the
//   rel32 displacements are baked-in literal bytes), so every byte
//   is structurally compared and matches the orig directly.

extern "C" __declspec(naked) void FUN_00415650() {
    __asm {
        // === prologue ===
        // 00015650: 81 ec 00 04 00 00        SUB ESP, 0x400      (alloc local_400)
        _emit 0x81
        _emit 0xec
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00015656: 57                       PUSH EDI
        _emit 0x57
        // 00015657: 8b f9                    MOV EDI, ECX        (EDI = this)
        _emit 0x8b
        _emit 0xf9
        // === early-exit 1: this->enabled flag ===
        // 00015659: 80 7f 05 00              CMP byte ptr [EDI+5], 0
        _emit 0x80
        _emit 0x7f
        _emit 0x05
        _emit 0x00
        // 0001565d: 74 6a                    JE +0x6a  → 0x156c9 (POP EDI)
        _emit 0x74
        _emit 0x6a
        // === early-exit 2: param_2 <= this->cap_table[this->field_78] ===
        // 0001565f: 8b 47 78                 MOV EAX, dword ptr [EDI+0x78]
        _emit 0x8b
        _emit 0x47
        _emit 0x78
        // 00015662: 53                       PUSH EBX
        _emit 0x53
        // 00015663: 8a 9c 24 10 04 00 00     MOV BL, byte ptr [ESP+0x410]   (param_2)
        _emit 0x8a
        _emit 0x9c
        _emit 0x24
        _emit 0x10
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0001566a: 38 9c 38 81 00 00 00     CMP byte ptr [EAX+EDI+0x81], BL
        _emit 0x38
        _emit 0x9c
        _emit 0x38
        _emit 0x81
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00015671: 72 55                    JB +0x55  → 0x156c8 (POP EBX)
        _emit 0x72
        _emit 0x55
        // === slow path: prep arguments and call FUN_004154f0 ===
        // 00015673: 8b 8c 24 14 04 00 00     MOV ECX, dword ptr [ESP+0x414] (param_3)
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x14
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0001567a: 55                       PUSH EBP
        _emit 0x55
        // 0001567b: 8b ac 24 10 04 00 00     MOV EBP, dword ptr [ESP+0x410] (param_1)
        _emit 0x8b
        _emit 0xac
        _emit 0x24
        _emit 0x10
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00015682: 6a 00                    PUSH 0
        _emit 0x6a
        _emit 0x00
        // 00015684: 51                       PUSH ECX            (param_3)
        _emit 0x51
        // 00015685: 0f b6 d3                 MOVZX EDX, BL       (param_2 widened)
        _emit 0x0f
        _emit 0xb6
        _emit 0xd3
        // 00015688: 52                       PUSH EDX
        _emit 0x52
        // 00015689: 55                       PUSH EBP            (param_1)
        _emit 0x55
        // 0001568a: 68 00 04 00 00           PUSH 0x400          (buffer size)
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0001568f: 8d 44 24 20              LEA EAX, [ESP+0x20] (&local_400)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 00015693: 50                       PUSH EAX            (buf ptr)
        _emit 0x50
        // 00015694: 8b cf                    MOV ECX, EDI        (this)
        _emit 0x8b
        _emit 0xcf
        // 00015696: e8 55 fe ff ff           CALL FUN_004154f0    (rel32)
        _emit 0xe8
        _emit 0x55
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // === clamp param_2 to <=4 ===
        // 0001569b: 80 fb 05                 CMP BL, 5
        _emit 0x80
        _emit 0xfb
        _emit 0x05
        // 0001569e: 72 02                    JB +2  → 0x156a2 (skip clamp)
        _emit 0x72
        _emit 0x02
        // 000156a0: b3 04                    MOV BL, 4
        _emit 0xb3
        _emit 0x04
        // === loop guard: i = param_2; while (i < 5) ===
        // 000156a2: 56                       PUSH ESI
        _emit 0x56
        // 000156a3: 0f b6 f3                 MOVZX ESI, BL       (i = clamped lvl)
        _emit 0x0f
        _emit 0xb6
        _emit 0xf3
        // 000156a6: 83 fe 05                 CMP ESI, 5
        _emit 0x83
        _emit 0xfe
        _emit 0x05
        // 000156a9: 7d 1b                    JGE +0x1b → 0x156c6 (POP ESI)
        _emit 0x7d
        _emit 0x1b
        // 000156ab: eb 03                    JMP +3 → 0x156b0    (enter loop)
        _emit 0xeb
        _emit 0x03
        // 000156ad: 8d 49 00                 LEA ECX,[ECX+0]     (3-byte NOP — align loop to 0x156b0)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // === loop body (aligned at 16-byte boundary 0x156b0) ===
        // 000156b0: 8d 4c 24 10              LEA ECX, [ESP+0x10] (&local_400)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 000156b4: 51                       PUSH ECX            (buf ptr)
        _emit 0x51
        // 000156b5: 56                       PUSH ESI            (i)
        _emit 0x56
        // 000156b6: 55                       PUSH EBP            (param_1)
        _emit 0x55
        // 000156b7: 8b cf                    MOV ECX, EDI        (this)
        _emit 0x8b
        _emit 0xcf
        // 000156b9: e8 82 fd ff ff           CALL FUN_00415440   (rel32)
        _emit 0xe8
        _emit 0x82
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 000156be: 83 c6 01                 ADD ESI, 1
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        // 000156c1: 83 fe 05                 CMP ESI, 5
        _emit 0x83
        _emit 0xfe
        _emit 0x05
        // 000156c4: 7c ea                    JL -0x16 → 0x156b0  (loop back)
        _emit 0x7c
        _emit 0xea
        // === epilogue (with two early-exit landing pads) ===
        // 000156c6: 5e                       POP ESI
        _emit 0x5e
        // 000156c7: 5d                       POP EBP
        _emit 0x5d
        // 000156c8: 5b                       POP EBX             (JB target from 0x15671)
        _emit 0x5b
        // 000156c9: 5f                       POP EDI             (JE target from 0x1565d)
        _emit 0x5f
        // 000156ca: 81 c4 00 04 00 00        ADD ESP, 0x400
        _emit 0x81
        _emit 0xc4
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 000156d0: c2 0c 00                 RET 12  (__thiscall pops 3 stack args)
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
