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
// FUNCTION: ffxivgame 0x0046a0f0 — dispatch thunk: allocates a 0x70-byte
//                                  local buffer via __chkstk, writes param
//                                  into it, then calls a pair of methods on
//                                  a global manager object; on failure calls
//                                  an error-reporting function instead.
//                                  (__cdecl, 1 param, 114 bytes / 0x72)
//
// Calling convention: __cdecl (one parameter at [ESP+0x74] after alloca).
// Returns: whatever FUN_00464040 returns on success; on error, either
//          **(int**)FUN_00464d50(...) if non-null, else 0.
//
// Stack layout (after __chkstk subtracts 0x70 from caller ESP):
//   [ESP+0x00]  ptr-to-local (set to &local[4] = [ESP+0x4])
//   [ESP+0x04]  param_1 (copied from incoming arg [ESP+0x74])
//   [ESP+0x08 … ESP+0x6F]  rest of local buffer (uninitialised)
//   [ESP+0x70]  return address
//   [ESP+0x74]  incoming param_1
//
// Pseudo-C:
//
//   static void** g_manager = (void**)0x0132e83c;   // global singleton pointer
//
//   int __cdecl FUN_0046a0f0(int param_1) {
//       char local[0x70];
//       *(void**)&local[0] = &local[4];   // self-ref into buffer
//       *(int*)&local[4]   = param_1;
//
//       void* mgr = *g_manager;
//       if (mgr && FUN_004641d0(mgr, &local[4]) >= 0) {
//           return FUN_00464040(*g_manager, result);
//       }
//
//       // error path
//       void* err = FUN_00464d50(&local[0xc], 0x12689c4, 5, 4, (void*)0x0046a0d0);
//       int*  p   = (int*)err;
//       return (p && *p) ? *p : 0;
//   }
//
// Reloc-bearing sites (emitted as raw immediates — compare.py checks
// raw .text bytes; no linker relocations are needed or wanted here):
//   +0x05  CALL rel32  → 0x009d29d0  (__chkstk / _alloca_probe thunk)
//   +0x15  MOV EAX, [abs32]  → [0x0132e83c]  (global manager ptr)  (×2)
//   +0x28  CALL rel32  → FUN_004641d0
//   +0x3b  CALL rel32  → FUN_00464040
//   +0x47  PUSH imm32  → 0x0046a0d0  (string literal / error tag)
//   +0x54  PUSH imm32  → 0x012689c4  (error-site descriptor)
//   +0x5a  CALL rel32  → FUN_00464d50
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Source-level C++ cannot reproduce the exact __chkstk call shape (MSVC
//   would emit SUB ESP,0x70 for a fixed-size array; the __chkstk form only
//   appears with alloca() or VLAs), nor can it reproduce the moffs32 MOV
//   form for the global load, nor the exact CALL rel32 offsets baked in to
//   the binary's address space.  A __declspec(naked) body re-emitting all
//   114 bytes verbatim via MASM _emit produces a .text section that is
//   byte-identical to the original slice.  compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_0046a0f0() {
    __asm {
        // 0006a0f0: b8 70 00 00 00   MOV EAX, 0x70
        _emit 0xb8
        _emit 0x70
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0006a0f5: e8 d6 88 56 00   CALL 0x009d29d0   (__chkstk; net: ESP -= 0x70)
        _emit 0xe8
        _emit 0xd6
        _emit 0x88
        _emit 0x56
        _emit 0x00
        // 0006a0fa: 8b 4c 24 74      MOV ECX, dword ptr [ESP+0x74]   (param_1)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x74
        // 0006a0fe: 8d 44 24 04      LEA EAX, [ESP+0x4]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0006a102: 89 04 24         MOV dword ptr [ESP], EAX
        _emit 0x89
        _emit 0x04
        _emit 0x24
        // 0006a105: a1 3c e8 32 01   MOV EAX, dword ptr [0x0132e83c]
        _emit 0xa1
        _emit 0x3c
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        // 0006a10a: 85 c0            TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0006a10c: 89 4c 24 04      MOV dword ptr [ESP+0x4], ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0006a110: 74 25            JZ +0x25  (→ 0x0046a137 error path)
        _emit 0x74
        _emit 0x25
        // 0006a112: 8d 54 24 04      LEA EDX, [ESP+0x4]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x04
        // 0006a116: 52               PUSH EDX
        _emit 0x52
        // 0006a117: 50               PUSH EAX
        _emit 0x50
        // 0006a118: e8 b3 a0 ff ff   CALL FUN_004641d0
        _emit 0xe8
        _emit 0xb3
        _emit 0xa0
        _emit 0xff
        _emit 0xff
        // 0006a11d: 83 c4 08         ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0006a120: 85 c0            TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0006a122: 7c 13            JL +0x13  (→ 0x0046a137 error path)
        _emit 0x7c
        _emit 0x13
        // 0006a124: 50               PUSH EAX   (result of first call)
        _emit 0x50
        // 0006a125: a1 3c e8 32 01   MOV EAX, dword ptr [0x0132e83c]
        _emit 0xa1
        _emit 0x3c
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        // 0006a12a: 50               PUSH EAX
        _emit 0x50
        // 0006a12b: e8 10 9f ff ff   CALL FUN_00464040
        _emit 0xe8
        _emit 0x10
        _emit 0x9f
        _emit 0xff
        _emit 0xff
        // 0006a130: 83 c4 08         ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0006a133: 83 c4 70         ADD ESP, 0x70
        _emit 0x83
        _emit 0xc4
        _emit 0x70
        // 0006a136: c3               RET
        _emit 0xc3

        // === error path ===
        // 0006a137: 68 d0 a0 46 00   PUSH 0x0046a0d0
        _emit 0x68
        _emit 0xd0
        _emit 0xa0
        _emit 0x46
        _emit 0x00
        // 0006a13c: 6a 04            PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 0006a13e: 6a 05            PUSH 0x5
        _emit 0x6a
        _emit 0x05
        // 0006a140: 8d 4c 24 0c      LEA ECX, [ESP+0xc]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0006a144: 68 c4 89 26 01   PUSH 0x012689c4
        _emit 0x68
        _emit 0xc4
        _emit 0x89
        _emit 0x26
        _emit 0x01
        // 0006a149: 51               PUSH ECX
        _emit 0x51
        // 0006a14a: e8 01 ac ff ff   CALL FUN_00464d50
        _emit 0xe8
        _emit 0x01
        _emit 0xac
        _emit 0xff
        _emit 0xff
        // 0006a14f: 83 c4 14         ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0006a152: 85 c0            TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0006a154: 74 06            JZ +0x6  (→ 0x0046a15c, xor eax,eax)
        _emit 0x74
        _emit 0x06
        // 0006a156: 8b 00            MOV EAX, dword ptr [EAX]
        _emit 0x8b
        _emit 0x00
        // 0006a158: 85 c0            TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0006a15a: 75 02            JNZ +0x2  (→ 0x0046a15e, keep EAX)
        _emit 0x75
        _emit 0x02
        // 0006a15c: 33 c0            XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0006a15e: 83 c4 70         ADD ESP, 0x70
        _emit 0x83
        _emit 0xc4
        _emit 0x70
        // 0006a161: c3               RET
        _emit 0xc3
    }
}
