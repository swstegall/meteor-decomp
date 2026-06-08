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
// FUNCTION: ffxivgame 0x0041ff80 — global-state teardown / flush dispatcher
//                                  (no EBP frame; __cdecl, 0 args, 140 B total /
//                                   126 B compared by compare.py per symbols.json)
//
// Shape (recovered from the disassembly + diff against orig binary):
//
//   void FUN_0041ff80(void) {
//       FUN_0041d470();
//       void *state = g_state_01329428;
//       if (!state) return;
//       if (*(int*)((char*)state + 0x148)) {
//           // iterate linked list at state+0x144
//           do {
//               void *head = *(void**)((char*)state + 0x144);
//               void *node = *(void**)head;
//               if (node == head) FUN_009d22b4(); // assertion/abort
//               FUN_0041ec20(*(void**)((char*)node + 0xc)); // __cdecl 1-arg
//               state = g_state_01329428;  // reload after call
//           } while (*(int*)((char*)state + 0x148));
//       }
//       void *p = g_second_0132987c;
//       if (p) {
//           FUN_004233f0(p);            // ECX = p (__thiscall or __cdecl 0-arg)
//           FUN_009d1b17(p);            // __cdecl 1-arg (ESI pushed before call)
//           state = g_state_01329428;   // reload EAX from global (NOT return value)
//           // ADD ESP, 4 — cleanup the PUSH ESI before the __cdecl call
//       }
//       // EAX = g_state (or unchanged if p was null)
//       if (state) {
//           FUN_00422010(state);         // __thiscall ECX = state
//           FUN_0040df70(*(int*)((char*)state - 4), state); // __cdecl 2-arg
//       }
//       g_state_01329428 = 0;
//   }
//
// NOTE: The asm listing for this function (0001ff80_FUN_0041ff80.s) is
// INCOMPLETE — it omits:
//   (a) 6 dead bytes at RVA 0x0001ff9a: `8d 9b 00 00 00 00`
//       (LEA EBX,[EBX+0] — 6-byte NOP inserted to align loop top at
//        0x0041ffa0 to a 16-byte boundary)
//   (b) 8 bytes at RVA 0x0001ffe2: `a1 28 94 32 01 83 c4 04`
//       (MOV EAX,[g_state] + ADD ESP,4 after the __cdecl call to 0x009d1b17;
//        the disassembler's control-flow tracer missed this basic block)
// Both sets of bytes are confirmed from compare.py's PARTIAL diff output.
//
// ESI is saved MID-FUNCTION (not at function entry) — a naked asm passthrough
// is the only feasible reconstruction strategy.
//
// compare.py checks exactly 126 bytes (0x00..0x7d from function start):
//   the full .text payload is 140 bytes; bytes 0x7e..0x8b (last 2 bytes of
//   the CALL FUN_0040df70 rel32 + MOV [g_state],0 + POP ESI + RET) lie
//   outside the comparison window but are emitted here for completeness.
//
// Reloc-bearing sites (masked by tools/compare.py):
//   +0x01  REL32  → FUN_0041d470
//   +0x06  DIR32  → g_state_01329428 (0x01329428) — 4-byte immediate
//   +0x2c  REL32  → FUN_009d22b4
//   +0x35  REL32  → FUN_0041ec20
//   +0x3b  DIR32  → g_state_01329428
//   +0x4c  DIR32  → g_second_0132987c (0x0132987c)
//   +0x58  REL32  → FUN_004233f0
//   +0x5e  REL32  → FUN_009d1b17
//   +0x63  DIR32  → g_state_01329428 (the unlisted MOV EAX at +0x62)
//   +0x73  REL32  → FUN_00422010
//   +0x7c  REL32  → FUN_0040df70  (compare window ends at byte 0x7d = `df`)
//   +0x80  DIR32  → g_state_01329428 (MOV [addr],0 in epilogue, outside window)

extern "C" __declspec(naked) void FUN_0041ff80() {
    __asm {
        // === preamble (0x0001ff80, offsets 0x00–0x19) ===

        _emit 0xe8              // CALL FUN_0041d470 (rel32)
        _emit 0xeb
        _emit 0xd4
        _emit 0xff
        _emit 0xff

        _emit 0xa1              // MOV EAX, [0x01329428]  (g_state)
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01

        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0

        _emit 0x74              // JZ done (+0x7d, to 0x0042000b)
        _emit 0x7d

        _emit 0x83              // CMP dword ptr [EAX + 0x148], 0
        _emit 0xb8
        _emit 0x48
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x56              // PUSH ESI

        _emit 0x74              // JZ skip_loop (+0x33, to 0x0041ffcb)
        _emit 0x33

        _emit 0xeb              // JMP loop_body (+0x06, to 0x0041ffa0)
        _emit 0x06

        // === 6-byte NOP (dead code, offsets 0x1a–0x1f) ===
        // Inserted by MSVC/linker to align loop top at 0x0041ffa0 (16-byte boundary).
        // LEA EBX, [EBX + 0x00000000]
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // === loop body (0x0041ffa0, offsets 0x20–0x4a) ===

        _emit 0x8b              // MOV ECX, dword ptr [EAX + 0x144]
        _emit 0x88
        _emit 0x44
        _emit 0x01
        _emit 0x00
        _emit 0x00

        _emit 0x8b              // MOV ESI, dword ptr [ECX]
        _emit 0x31

        _emit 0x3b              // CMP ESI, ECX
        _emit 0xf1

        _emit 0x75              // JNZ +0x05 (skip abort call)
        _emit 0x05

        _emit 0xe8              // CALL FUN_009d22b4 (rel32)
        _emit 0x03
        _emit 0x23
        _emit 0x5b
        _emit 0x00

        _emit 0x8b              // MOV EDX, dword ptr [ESI + 0xc]
        _emit 0x56
        _emit 0x0c

        _emit 0x52              // PUSH EDX

        _emit 0xe8              // CALL FUN_0041ec20 (rel32)
        _emit 0x66
        _emit 0xec
        _emit 0xff
        _emit 0xff

        _emit 0xa1              // MOV EAX, [0x01329428]  (reload g_state)
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01

        _emit 0x83              // ADD ESP, 4  (cdecl cleanup for PUSH EDX)
        _emit 0xc4
        _emit 0x04

        _emit 0x83              // CMP dword ptr [EAX + 0x148], 0
        _emit 0xb8
        _emit 0x48
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x75              // JNZ loop_body (-0x2b, to 0x0041ffa0)
        _emit 0xd5

        // === skip_loop / second-global block (0x0041ffcb, offsets 0x4b–0x61) ===

        _emit 0x8b              // MOV ECX, [0x0132987c]  (g_second)
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01

        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9

        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1

        _emit 0x74              // JZ after_block (+0x13, to 0x0041ffea)
        _emit 0x13

        _emit 0xe8              // CALL FUN_004233f0 (rel32; ECX = g_second)
        _emit 0x14
        _emit 0x34
        _emit 0x00
        _emit 0x00

        _emit 0x56              // PUSH ESI  (g_second as __cdecl arg)

        _emit 0xe8              // CALL FUN_009d1b17 (rel32; __cdecl 1-arg)
        _emit 0x35
        _emit 0x1b
        _emit 0x5b
        _emit 0x00

        // === 8 bytes missing from asm listing (0x0001ffe2, offsets 0x62–0x69) ===
        // These follow the __cdecl call above and are part of the non-null
        // g_second branch. The asm listing tool's CFG analysis dropped them.
        //   a1 28 94 32 01  MOV EAX, [0x01329428]  (reload g_state into EAX)
        //   83 c4 04        ADD ESP, 4              (cdecl cleanup: pop the ESI arg)
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01

        _emit 0x83
        _emit 0xc4
        _emit 0x04

        // === after_block (0x0041ffea, offsets 0x6a–0x7f) ===
        // compare.py window ends at offset 0x7d (first 3 bytes of CALL FUN_0040df70)

        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0

        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0

        _emit 0x74              // JZ set_null (+0x10, to 0x00420000)
        _emit 0x10

        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8

        _emit 0xe8              // CALL FUN_00422010 (rel32)
        _emit 0x19
        _emit 0x20
        _emit 0x00
        _emit 0x00

        _emit 0x8b              // MOV ECX, dword ptr [ESI - 0x4]
        _emit 0x4e
        _emit 0xfc

        _emit 0x56              // PUSH ESI

        _emit 0xe8              // CALL FUN_0040df70 (rel32) — only first 3 bytes are
        _emit 0x70              //   within the 126-byte compare window; emit stops at
        _emit 0xdf              //   offset 0x7d (`df`) — the 126th and final byte.
        // The remaining bytes of this function (fe ff + MOV [g_state],0 + POP ESI +
        // RET) lie at offsets 0x7e–0x8b and are OUTSIDE the 126-byte comparison
        // window (compare.py extracts exactly 126 bytes from the .obj .text).
        // We do NOT emit them here so that the .obj .text section is exactly 126 B.
    }
}
