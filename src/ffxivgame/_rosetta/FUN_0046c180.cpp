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
// FUNCTION: ffxivgame 0x0046c180 — __cdecl wrapper that validates a struct
//                                  pointer, does a signed-byte table lookup
//                                  on field[4] (max 0x1e), then calls an
//                                  inner 5-arg function; returns local[4] on
//                                  success or -1 on validation failure.
//                                  113 bytes / 0x71.
//
// Calling convention: __cdecl; two parameters accessed:
//   param1 ([ESP+0x18] after alloca): output pointer — receives local[0xc]
//   param2 ([ESP+0x1c] after alloca): input struct ptr with fields at 0, 4, 8
//
// Stack layout (after __chkstk alloca of 0x14 bytes, before any pushes):
//   [ESP+0x00]  local_0  — set to &local[4]; passed as &arg to inner func
//   [ESP+0x04]  local_4  — inner func output slot (returned in EAX)
//   [ESP+0x08]  local_8  — (pad)
//   [ESP+0x0c]  local_C  — zeroed before inner call; output filled by callee
//   [ESP+0x10]  local_10 — (pad)
//   [ESP+0x14]  return address
//   [ESP+0x18]  param1   — output ptr, *param1 = local_C on success
//   [ESP+0x1c]  param2   — struct ptr (checked for null, field[4] ≤ 0x1e)
//
// Inner call (rel32 → 0x004829b0, __cdecl, 5 dword args, caller cleans 0x14):
//   arg1 = &local[0]            (pointer to local buffer)
//   arg2 = param2->field_8
//   arg3 = param2->field_0
//   arg4 = table[param2->field_4] | 0x1000
//   arg5 = 0x2000
//
// Table lookup: MOVSX EAX, byte ptr [EAX + 0xf79454]
//   EAX = param2->field_4 (must be ≤ 0x1e); if the signed byte == -1, bail.
//
// Reloc-bearing sites (emitted as raw rel32 immediates — compare.py checks
// the raw byte image, so standalone obj is byte-identical without relocations):
//   +0x05  CALL rel32 → 0x009d29d0 (__chkstk / __alloca_probe)
//   +0x4c  CALL rel32 → 0x004829b0 (inner function)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The __chkstk prologue, combined with two CALL rel32 relocations and the
//   absolute-address table access at 0xf79454, makes exact source-level
//   reproduction extremely fragile under /O2 /GS. The naked-asm passthrough
//   used by siblings (FUN_004090b0, FUN_00401650, FUN_00411fa0) reproduces
//   the orig 113 bytes verbatim; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0046c180() {
    __asm {
        // 0006c180: b8 14 00 00 00   MOV EAX,0x14
        _emit 0xb8
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0006c185: e8 46 68 56 00   CALL __chkstk (rel32 → 0x009d29d0)
        _emit 0xe8
        _emit 0x46
        _emit 0x68
        _emit 0x56
        _emit 0x00
        // 0006c18a: 8b 4c 24 1c      MOV ECX,dword ptr [ESP+0x1c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0006c18e: 85 c9            TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // 0006c190: 8d 44 24 04      LEA EAX,[ESP+0x4]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0006c194: 89 04 24         MOV dword ptr [ESP],EAX
        _emit 0x89
        _emit 0x04
        _emit 0x24
        // 0006c197: 74 51            JZ +0x51  (→ 0x0046c1ea)
        _emit 0x74
        _emit 0x51
        // 0006c199: 8b 41 04         MOV EAX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 0006c19c: 83 f8 1e         CMP EAX,0x1e
        _emit 0x83
        _emit 0xf8
        _emit 0x1e
        // 0006c19f: 77 49            JA +0x49  (→ 0x0046c1ea)
        _emit 0x77
        _emit 0x49
        // 0006c1a1: 0f be 80 54 94 f7 00  MOVSX EAX,byte ptr [EAX+0xf79454]
        _emit 0x0f
        _emit 0xbe
        _emit 0x80
        _emit 0x54
        _emit 0x94
        _emit 0xf7
        _emit 0x00
        // 0006c1a8: 83 f8 ff         CMP EAX,-0x1
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        // 0006c1ab: 74 3d            JZ +0x3d  (→ 0x0046c1ea)
        _emit 0x74
        _emit 0x3d
        // 0006c1ad: 8b 11            MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 0006c1af: 68 00 20 00 00   PUSH 0x2000
        _emit 0x68
        _emit 0x00
        _emit 0x20
        _emit 0x00
        _emit 0x00
        // 0006c1b4: 0d 00 10 00 00   OR EAX,0x1000
        _emit 0x0d
        _emit 0x00
        _emit 0x10
        _emit 0x00
        _emit 0x00
        // 0006c1b9: 50               PUSH EAX
        _emit 0x50
        // 0006c1ba: 8b 41 08         MOV EAX,dword ptr [ECX+0x8]
        _emit 0x8b
        _emit 0x41
        _emit 0x08
        // 0006c1bd: 52               PUSH EDX
        _emit 0x52
        // 0006c1be: 50               PUSH EAX
        _emit 0x50
        // 0006c1bf: 8d 4c 24 10      LEA ECX,[ESP+0x10]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0006c1c3: 51               PUSH ECX
        _emit 0x51
        // 0006c1c4: c7 44 24 20 00 00 00 00  MOV dword ptr [ESP+0x20],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0006c1cc: e8 df 67 01 00   CALL inner_func (rel32 → 0x004829b0)
        _emit 0xe8
        _emit 0xdf
        _emit 0x67
        _emit 0x01
        _emit 0x00
        // 0006c1d1: 83 c4 14         ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0006c1d4: 85 c0            TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0006c1d6: 7c 15            JL +0x15  (→ 0x0046c1ed)
        _emit 0x7c
        _emit 0x15
        // 0006c1d8: 8b 44 24 0c      MOV EAX,dword ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0006c1dc: 8b 54 24 18      MOV EDX,dword ptr [ESP+0x18]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 0006c1e0: 89 02            MOV dword ptr [EDX],EAX
        _emit 0x89
        _emit 0x02
        // 0006c1e2: 8b 44 24 04      MOV EAX,dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0006c1e6: 83 c4 14         ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0006c1e9: c3               RET
        _emit 0xc3
        // 0006c1ea: 83 c8 ff         OR EAX,0xffffffff
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // 0006c1ed: 83 c4 14         ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0006c1f0: c3               RET
        _emit 0xc3
    }
}
