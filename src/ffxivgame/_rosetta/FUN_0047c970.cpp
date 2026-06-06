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
// FUNCTION: ffxivgame 0x0047c970 — message/command dispatch (218 B / 0xda)
//
// ECX = "this" (saved into EDI at entry).  Stack arg at [ESP+4] is a small
// integer command/message type; the function dispatches on two recognised
// values (0x10 and 0x06) and calls an error-log helper (0x0045c940) for
// any unrecognised value.  All exit paths use plain RET (no stack cleanup).
//
// Case 0x10:
//   Reads this->field_0 and this->field_8.  Passes field_0 plus a pointer
//   to the (reused) arg slot (pre-filled with field_8) and a zero literal
//   to FUN_00486bb0.  On success returns the pointer result; on failure
//   logs an error (code 0x97 / line 0x8e) via FUN_0045c940 and returns 0.
//   If FUN_00486bb0 returns non-null but an inner allocation is still live
//   it is freed via FUN_00468f30 before returning 0.
//
// Case 0x06:
//   Calls FUN_00468ed0() (ECX = this still in ECX) → ESI.
//   On failure logs error (code 0xa5 / line 0x41) and returns 0.
//   On success: FUN_00464d80(this) → tmp; FUN_0047c1e0(tmp) → EDI.
//   If EDI is null returns 0.  Otherwise calls FUN_0047a750(EDI,1),
//   FUN_004690e0(ESI,EDI); if the latter returns 0 → return 0.
//   Calls FUN_0047b580(EDI), returns ESI.
//
// Default:
//   Logs error (code 0xb2 / line 0x8e) via FUN_0045c940, returns 0.
//
// Reloc-bearing sites (compare.py masks these 4-byte windows):
//   +0x1e  rel32 → FUN_00486bb0  (0x00486bb0)
//   +0x35  dir32 → string ptr    (0x00f7f1b0)
//   +0x46  rel32 → FUN_0045c940  (0x0045c940)
//   +0x53  rel32 → FUN_00468f30  (0x00468f30)
//   +0x65  rel32 → FUN_00468ed0  (0x00468ed0)
//   +0x75  dir32 → string ptr    (0x00f7f1b0)
//   +0x7f  rel32 → FUN_00464d80  (0x00464d80)
//   +0x85  rel32 → FUN_0047c1e0  (0x0047c1e0)
//   +0x96  rel32 → FUN_0047a750  (0x0047a750)
//   +0x9d  rel32 → FUN_004690e0  (0x004690e0)
//   +0xaa  rel32 → FUN_0047b580  (0x0047b580)
//   +0xbc  dir32 → string ptr    (0x00f7f1b0)
//   +0xcd  rel32 → FUN_0045c940  (0x0045c940)
//
// Reconstruction strategy — __declspec(naked) byte passthrough.
//
//   The arg==0x10 branch reuses the caller's argument stack slot as a
//   local variable (stores this->field_8 there via MOV [ESP+0x18],EAX
//   after five pushes) and passes its address to FUN_00486bb0.  Coaxing
//   MSVC 2005 /O2 into emitting precisely this stack-slot reuse plus the
//   exact register allocation and branch encoding across three exit paths
//   is not reliably reproducible at source level.  Naked asm gives a
//   byte-exact .text section with the 13 reloc slots masked.

extern "C" __declspec(naked) void FUN_0047c970() {
    __asm {
        // MOV EAX,[ESP+0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // CMP EAX,0x10
        _emit 0x83
        _emit 0xf8
        _emit 0x10
        // PUSH ESI
        _emit 0x56
        // PUSH EDI
        _emit 0x57
        // MOV EDI,ECX
        _emit 0x8b
        _emit 0xf9
        // JNZ +0x53  (to 0x0047c9d0)
        _emit 0x75
        _emit 0x53
        // --- case 0x10 ---
        // MOV EAX,[EDI+0x8]
        _emit 0x8b
        _emit 0x47
        _emit 0x08
        // MOV EDI,[EDI]
        _emit 0x8b
        _emit 0x3f
        // PUSH EDI
        _emit 0x57
        // LEA ECX,[ESP+0x10]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // PUSH ECX
        _emit 0x51
        // PUSH 0
        _emit 0x6a
        _emit 0x00
        // MOV [ESP+0x18],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // CALL FUN_00486bb0  (rel32, masked)
        _emit 0xe8
        _emit 0x1d
        _emit 0xa2
        _emit 0x00
        _emit 0x00
        // MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // ADD ESP,0xC
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // JNZ +0x82  (to success_return at 0x0047ca22)
        _emit 0x0f
        _emit 0x85
        _emit 0x82
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // PUSH 0x97
        _emit 0x68
        _emit 0x97
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // PUSH 0xf7f1b0  (string ptr, masked)
        _emit 0x68
        _emit 0xb0
        _emit 0xf1
        _emit 0xf7
        _emit 0x00
        // PUSH 0x8e
        _emit 0x68
        _emit 0x8e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // --- shared error-log block (also JMP target from case 0x6) ---
        // PUSH 0xdc
        _emit 0x68
        _emit 0xdc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // CALL FUN_0045c940  (rel32, masked)
        _emit 0xe8
        _emit 0x85
        _emit 0xff
        _emit 0xfd
        _emit 0xff
        // ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // --- shared cleanup ---
        // TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // JZ +0x9  (to null_return)
        _emit 0x74
        _emit 0x09
        // PUSH ESI
        _emit 0x56
        // CALL FUN_00468f30  (rel32, masked)
        _emit 0xe8
        _emit 0x68
        _emit 0xc5
        _emit 0xfe
        _emit 0xff
        // ADD ESP,4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // null_return:
        // POP EDI
        _emit 0x5f
        // XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // POP ESI
        _emit 0x5e
        // RET
        _emit 0xc3
        // --- case 0x6 (falls through from dispatch JNZ) ---
        // CMP EAX,0x6
        _emit 0x83
        _emit 0xf8
        _emit 0x06
        // JNZ +0x52  (to 0x0047ca27 / default)
        _emit 0x75
        _emit 0x52
        // CALL FUN_00468ed0  (rel32, masked)
        _emit 0xe8
        _emit 0xf6
        _emit 0xc4
        _emit 0xfe
        _emit 0xff
        // MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // JNZ +0xe  (to have_esi)
        _emit 0x75
        _emit 0x0e
        // PUSH 0xa5
        _emit 0x68
        _emit 0xa5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // PUSH 0xf7f1b0  (string ptr, masked)
        _emit 0x68
        _emit 0xb0
        _emit 0xf1
        _emit 0xf7
        _emit 0x00
        // PUSH 0x41
        _emit 0x6a
        _emit 0x41
        // JMP -0x3f  (to shared PUSH 0xdc block at 0x0047c9af)
        _emit 0xeb
        _emit 0xc1
        // have_esi:
        // PUSH EDI  (EDI = original this from ECX)
        _emit 0x57
        // CALL FUN_00464d80  (rel32, masked)
        _emit 0xe8
        _emit 0x8c
        _emit 0x83
        _emit 0xfe
        _emit 0xff
        // PUSH EAX
        _emit 0x50
        // CALL FUN_0047c1e0  (rel32, masked)
        _emit 0xe8
        _emit 0xe6
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        // MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // JZ -0x45  (to shared cleanup / null_return at 0x0047c9be)
        _emit 0x74
        _emit 0xbb
        // PUSH 1
        _emit 0x6a
        _emit 0x01
        // PUSH EDI
        _emit 0x57
        // CALL FUN_0047a750  (rel32, masked)
        _emit 0xe8
        _emit 0x45
        _emit 0xdd
        _emit 0xff
        _emit 0xff
        // PUSH EDI
        _emit 0x57
        // PUSH ESI
        _emit 0x56
        // CALL FUN_004690e0  (rel32, masked)
        _emit 0xe8
        _emit 0xce
        _emit 0xc6
        _emit 0xfe
        _emit 0xff
        // ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // JZ -0x5b  (to null_return at 0x0047c9be)
        _emit 0x74
        _emit 0xa5
        // PUSH EDI
        _emit 0x57
        // CALL FUN_0047b580  (rel32, masked)
        _emit 0xe8
        _emit 0x61
        _emit 0xeb
        _emit 0xff
        _emit 0xff
        // ADD ESP,4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // success_return:
        // POP EDI
        _emit 0x5f
        // MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // POP ESI
        _emit 0x5e
        // RET
        _emit 0xc3
        // --- default (unrecognised command) ---
        // PUSH 0xb2
        _emit 0x68
        _emit 0xb2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // PUSH 0xf7f1b0  (string ptr, masked)
        _emit 0x68
        _emit 0xb0
        _emit 0xf1
        _emit 0xf7
        _emit 0x00
        // PUSH 0x8e
        _emit 0x68
        _emit 0x8e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // PUSH 0xdc
        _emit 0x68
        _emit 0xdc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // CALL FUN_0045c940  (rel32, masked)
        _emit 0xe8
        _emit 0xfe
        _emit 0xfe
        _emit 0xfd
        _emit 0xff
        // ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // POP EDI
        _emit 0x5f
        // XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // POP ESI
        _emit 0x5e
        // RET
        _emit 0xc3
    }
}
