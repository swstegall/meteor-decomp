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
// FUNCTION: ffxivgame 0x00452150 — recursive tree-node destructor/free walk;
//           108 bytes at RVA 0x00052150.
//
// __thiscall void SomeClass::freeNode(SomeNode *node)
//
// Semantics (recovered from asm):
//
//   if (node->flag != 0) return;  // [node+0x45] byte flag
//   freeNode(node->child);        // recurse: child ptr at [node+0x8]
//   // clear std::string at node+0x2c (cap@0x40, len@0x3c, buf@0x2c)
//   if (node->str2_cap >= 16) free(node->str2_buf);
//   node->str2_cap = 15; node->str2_len = 0; node->str2_buf[0] = 0;
//   // clear std::string at node+0x10 (cap@0x24, len@0x20, buf@0x10)
//   if (node->str1_cap >= 16) free(node->str1_buf);
//   node->str1_cap = 15; node->str1_len = 0; node->str1_buf[0] = 0;
//   free(node);
//
// The shared epilogue (POP EDI/ESI/EBX/ECX; RET 4) lives at RVA 0x004521c6
// outside this function's 108-byte region; the JNZ at byte +0x13 jumps there
// directly, and the last byte of the function (POP EBP at +0x6b) falls
// through to it via the 10 bytes of padding at 0x521bc-0x521c5.
//
// Reloc-bearing call sites within the 108 bytes:
//   +0x28  CALL rel32 → self (FUN_00452150, RVA 0x00052150; disp = 0xFFFFFFD3)
//   +0x39  CALL rel32 → 0x009d1b17 (free / operator delete; disp = 0x0057F989)
//   +0x54  CALL rel32 → 0x009d1b17 (free / operator delete; disp = 0x0057F96E)
//   +0x66  CALL rel32 → 0x009d1b17 (free / operator delete; disp = 0x0057F95C)
//
// Reconstruction strategy — naked-asm byte passthrough (identical to
// siblings FUN_004090b0 and FUN_004091f0 in this directory).

extern "C" __declspec(naked) void FUN_00452150() {
    __asm {
        // --- prologue (21 bytes) -------------------------------------------
        _emit 0x51              // PUSH ECX
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x10]
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0x38              // CMP byte ptr [ESI+0x45], BL
        _emit 0x5e
        _emit 0x45
        _emit 0x57              // PUSH EDI
        _emit 0x89              // MOV dword ptr [ESP+0xc], ECX
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x8b              // MOV EDI, ESI
        _emit 0xfe
        _emit 0x75              // JNZ +0x61 → shared epilogue at 0x4521C6
        _emit 0x61
        // --- loop-body setup (11 bytes) ------------------------------------
        _emit 0x55              // PUSH EBP
        _emit 0x8d              // LEA EBP, [EBX+0xF]   (EBP = 15 = small-string capacity)
        _emit 0x6b
        _emit 0x0f
        _emit 0x8d              // LEA ESP, [ESP+0]  (7-byte loop-alignment NOP)
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // --- recursive self-call (13 bytes) --------------------------------
        _emit 0x8b              // MOV EAX, dword ptr [EDI+8]   (child ptr)
        _emit 0x47
        _emit 0x08
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x10]  (reload `this`)
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_00452150  (rel32 = 0xFFFFFFD3)
        _emit 0xd3
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // --- str2 buffer free (17 bytes) -----------------------------------
        _emit 0x83              // CMP dword ptr [ESI+0x40], 0x10
        _emit 0x7e
        _emit 0x40
        _emit 0x10
        _emit 0x8b              // MOV EDI, dword ptr [EDI]
        _emit 0x3f
        _emit 0x72              // JC +0x0C  (skip if cap < 16)
        _emit 0x0c
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x2C]
        _emit 0x4e
        _emit 0x2c
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL 0x9D1B17  (rel32 = 0x0057F989)
        _emit 0x89
        _emit 0xf9
        _emit 0x57
        _emit 0x00
        // --- ADD ESP, 4  (3 bytes) -----------------------------------------
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // --- reset str2 fields + conditional str1 free (15 bytes) ----------
        _emit 0x89              // MOV dword ptr [ESI+0x40], EBP  (cap = 15)
        _emit 0x6e
        _emit 0x40
        _emit 0x89              // MOV dword ptr [ESI+0x3C], EBX  (len = 0)
        _emit 0x5e
        _emit 0x3c
        _emit 0x88              // MOV byte ptr [ESI+0x2C], BL    (buf[0] = 0)
        _emit 0x5e
        _emit 0x2c
        _emit 0x83              // CMP dword ptr [ESI+0x24], 0x10
        _emit 0x7e
        _emit 0x24
        _emit 0x10
        _emit 0x72              // JC +0x0C  (skip if cap < 16)
        _emit 0x0c
        // --- str1 buffer free (9 bytes) ------------------------------------
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x10]
        _emit 0x56
        _emit 0x10
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL 0x9D1B17  (rel32 = 0x0057F96E)
        _emit 0x6e
        _emit 0xf9
        _emit 0x57
        _emit 0x00
        // --- ADD ESP, 4  (3 bytes) -----------------------------------------
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // --- reset str1 fields + free node (15 bytes) ----------------------
        _emit 0x89              // MOV dword ptr [ESI+0x24], EBP  (cap = 15)
        _emit 0x6e
        _emit 0x24
        _emit 0x89              // MOV dword ptr [ESI+0x20], EBX  (len = 0)
        _emit 0x5e
        _emit 0x20
        _emit 0x56              // PUSH ESI  (node ptr as arg to free)
        _emit 0x88              // MOV byte ptr [ESI+0x10], BL    (buf[0] = 0)
        _emit 0x5e
        _emit 0x10
        _emit 0xe8              // CALL 0x9D1B17  (rel32 = 0x0057F95C)
        _emit 0x5c
        _emit 0xf9
        _emit 0x57
        _emit 0x00
        // --- POP EBP (1 byte) — last byte of function; falls through to
        //     shared epilogue at 0x4521C6 via 10-byte padding gap ----------
        _emit 0x5d              // POP EBP
    }
}
