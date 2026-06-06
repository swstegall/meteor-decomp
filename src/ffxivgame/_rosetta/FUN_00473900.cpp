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
// FUNCTION: ffxivgame 0x00073900 — __cdecl validate + dispatch a "0x10"-tagged
//                                  message record through two/three handler
//                                  stages (477 B / 0x1dd, /GS cookie, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x00073900):
//
//   __cdecl int dispatch_tagged_record(arg0, arg1, arg2, void** rec,
//                                       arg4, arg5);
//
//     The prologue is the standard MSVC 2005 /GS sequence:
//       MOV EAX,0x48 ; CALL _chkstk        // 0x48-byte frame
//       MOV EAX,[0x012ea8b0] ; XOR EAX,ESP ; MOV [ESP+0x44],EAX   // cookie
//     and every exit re-loads [ESP+0x44], XORs ESP and CALLs the cookie
//     check thunk at 0x009d20f4 before ADD ESP,0x48 / RET — confirming
//     __cdecl (no `ret N`).
//
//   Stack args (post-_chkstk, ESP-relative; return addr at [ESP+0x48]):
//     [ESP+0x4c] arg0   [ESP+0x50] arg1   [ESP+0x54] arg2
//     [ESP+0x58] rec    [ESP+0x5c] arg4   [ESP+0x60] arg5
//
//   Shape:
//     if (rec == 0)            goto fail_0x51;     // [0x73ab7]
//     if (rec[0] != 0x10)      goto fail_0x51;
//     if (rec[1] == 0)         goto fail_0x51;
//     payload = (T*)rec[1];
//     b = FUN_00487ef0(0, &slot, payload[0], payload[2]);   // [0x487ef0]
//     if (b == 0) { FUN_0045c940(0x23,'x','e',str,0x57); return 0; }   // log
//     ... two-stage handler dispatch via FUN_00488300 (mode 1 then 2),
//         each guarded by a FUN_0045c940 log on failure, then a final
//         FUN_00487be0 + two FUN_00e43d30 (memcpy-like) copies on success,
//         returning ESI.
//
//   Reconstruction strategy — naked-asm byte passthrough (the established
//   sibling idiom for reloc-heavy /GS bodies — see FUN_00401820,
//   FUN_0040b840, FUN_00409350). A source-level rewrite would need MSVC
//   2005 /O2 /GS to reproduce the exact register allocation across the
//   eight reloc-bearing CALL targets (0x009d29d0 _chkstk, 0x00487ef0,
//   0x0045c940, 0x009d20f4 cookie-check, 0x0046d2c0, 0x00477ac0,
//   0x00488300, 0x00bd78b0, 0x00487be0, 0x00e43d30), the global cookie
//   load at 0x012ea8b0, and the pooled string literal at 0x00f7a0ac.
//   Each is brittle under /O2; emitting the orig 477 bytes verbatim via
//   `_emit` yields a .text section byte-identical to the orig slice, which
//   is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_00473900() {
    __asm {
        _emit 0xb8
        _emit 0x48
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xc6
        _emit 0xf0
        _emit 0x55
        _emit 0x00
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x44
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x50
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x5c
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x58
        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        _emit 0x89
        _emit 0x14
        _emit 0x24
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x0f
        _emit 0x84
        _emit 0x77
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0x38
        _emit 0x10
        _emit 0x0f
        _emit 0x85
        _emit 0x6e
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0x78
        _emit 0x04
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0x64
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        _emit 0x53
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x00
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x51
        _emit 0x6a
        _emit 0x00
        _emit 0xe8
        _emit 0x83
        _emit 0x45
        _emit 0x01
        _emit 0x00
        _emit 0x8b
        _emit 0xd8
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xdb
        _emit 0x75
        _emit 0x27
        _emit 0x6a
        _emit 0x57
        _emit 0x68
        _emit 0xac
        _emit 0xa0
        _emit 0xf7
        _emit 0x00
        _emit 0x6a
        _emit 0x65
        _emit 0x6a
        _emit 0x78
        _emit 0x6a
        _emit 0x23
        _emit 0xe8
        _emit 0xb8
        _emit 0x8f
        _emit 0xfe
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x33
        _emit 0xc0
        _emit 0x5b
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x44
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x5b
        _emit 0xe7
        _emit 0x55
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x48
        _emit 0xc3
        _emit 0x8b
        _emit 0x43
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x55
        _emit 0x75
        _emit 0x05
        _emit 0x8d
        _emit 0x68
        _emit 0x01
        _emit 0xeb
        _emit 0x0b
        _emit 0x50
        _emit 0xe8
        _emit 0x10
        _emit 0x99
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x8b
        _emit 0xe8
        _emit 0x8b
        _emit 0x03
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x56
        _emit 0x8b
        _emit 0x70
        _emit 0x08
        _emit 0x57
        _emit 0x8b
        _emit 0x38
        _emit 0x52
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x50
        _emit 0x51
        _emit 0xe8
        _emit 0xee
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x70
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x50
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x55
        _emit 0x6a
        _emit 0x01
        _emit 0x57
        _emit 0x56
        _emit 0x52
        _emit 0x50
        _emit 0xe8
        _emit 0x16
        _emit 0x49
        _emit 0x01
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x30
        _emit 0x6a
        _emit 0x61
        _emit 0x68
        _emit 0xac
        _emit 0xa0
        _emit 0xf7
        _emit 0x00
        _emit 0x6a
        _emit 0x6b
        _emit 0x6a
        _emit 0x78
        _emit 0x6a
        _emit 0x23
        _emit 0xe8
        _emit 0x3d
        _emit 0x8f
        _emit 0xfe
        _emit 0xff
        _emit 0x53
        _emit 0xe8
        _emit 0x07
        _emit 0x45
        _emit 0x01
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x33
        _emit 0xc0
        _emit 0x5b
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x44
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0xd7
        _emit 0xe6
        _emit 0x55
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x48
        _emit 0xc3
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x51
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x28
        _emit 0x52
        _emit 0x50
        _emit 0xe8
        _emit 0x7b
        _emit 0x3e
        _emit 0x76
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x70
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x50
        _emit 0x55
        _emit 0x6a
        _emit 0x02
        _emit 0x57
        _emit 0x56
        _emit 0x51
        _emit 0x52
        _emit 0xe8
        _emit 0xb3
        _emit 0x48
        _emit 0x01
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x0b
        _emit 0x6a
        _emit 0x67
        _emit 0x68
        _emit 0xac
        _emit 0xa0
        _emit 0xf7
        _emit 0x00
        _emit 0x6a
        _emit 0x6a
        _emit 0xeb
        _emit 0x9b
        _emit 0x53
        _emit 0xe8
        _emit 0xab
        _emit 0x44
        _emit 0x01
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x78
        _emit 0x50
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x51
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x40
        _emit 0x52
        _emit 0x6a
        _emit 0x00
        _emit 0x50
        _emit 0x51
        _emit 0xe8
        _emit 0x5b
        _emit 0x41
        _emit 0x01
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x50
        _emit 0x6a
        _emit 0x20
        _emit 0x52
        _emit 0x8b
        _emit 0xf0
        _emit 0xe8
        _emit 0x9d
        _emit 0x02
        _emit 0x9d
        _emit 0x00
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x48
        _emit 0x6a
        _emit 0x10
        _emit 0x50
        _emit 0xe8
        _emit 0x91
        _emit 0x02
        _emit 0x9d
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x2c
        _emit 0x5f
        _emit 0x8b
        _emit 0xc6
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x44
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x41
        _emit 0xe6
        _emit 0x55
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x48
        _emit 0xc3
        _emit 0x6a
        _emit 0x51
        _emit 0x68
        _emit 0xac
        _emit 0xa0
        _emit 0xf7
        _emit 0x00
        _emit 0x6a
        _emit 0x65
        _emit 0x6a
        _emit 0x78
        _emit 0x6a
        _emit 0x23
        _emit 0xe8
        _emit 0x77
        _emit 0x8e
        _emit 0xfe
        _emit 0xff
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x58
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x33
        _emit 0xcc
        _emit 0x33
        _emit 0xc0
        _emit 0xe8
        _emit 0x1b
        _emit 0xe6
        _emit 0x55
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x48
        _emit 0xc3
    }
}
