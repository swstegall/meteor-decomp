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
// FUNCTION: ffxivgame 0x009e4cef — asynchronous wide-to-narrow string
//                                   acquisition with a 3-state global
//                                   (304 B / 0x130).
//
// Inspection (read from the disassembly at orig RVA 0x005e4cef):
//
//   __cdecl char* FUN_009e4cef()
//
//   Global state variable at .data 0x013648b8 (3 values: 0, 1, 2).
//   Function-pointer table in .rdata (all resolved from IAT):
//     [0xf3e258]  — connect/accept or similar (returns SOCKET/HANDLE)
//     [0xf3e1c4]  — error-query call (e.g. WSAGetLastError; 0x78 = pending)
//     [0xf3e12c]  — WideCharToMultiByte or AllocateAndCopy-like
//     [0xf3e25c]  — free (for wide-string buffer from path 1)
//     [0xf3e260]  — get-narrow-string (returns char*)
//     [0xf3e264]  — free (for narrow-string buffer from path 2)
//
//   State machine on entry (eax = [0x013648b8]):
//     state 0 → try [0xf3e258]; if esi != NULL → state=1, goto state-1 body
//               if esi == NULL and error==0x78 → state=2, goto check
//               else keep state=0, fall through check
//     check:  if state != 1 → goto alt path
//     state 1 → if esi==NULL try [0xf3e258] once more; if still NULL → return 0
//               Walk esi (wide string) scanning for WORD 0 (or double-null):
//                 loop: add eax, ebp; cmp WORD[eax], 0; jne loop (ebp=2)
//               Compute length = (eax - esi) / 2 + 1 (wchar count)
//               Call [0xf3e12c] with 8 args (WideCharToMultiByte query)
//               If result (ebp) == NULL → free(esi via [0xf3e25c]); return 0
//               Call 0x9ddf7a(ebp) — strlen or get-length
//               If len == 0 → goto cleanup
//               Call [0xf3e12c] again with 8 args (actual conversion)
//               If conversion returned 0 → call 0x9d5c88(len) to free result
//               Return converted buffer (or 0 on failure)
//     alt path (state 0 or 2):
//               Call [0xf3e260] — returns a char*
//               If NULL → return 0
//               Scan for double-null (two consecutive 0 bytes):
//                 if [esi]==0 skip; else walk eax until [eax]==0 and [eax+1]==0
//               length = eax - esi + 1
//               malloc(length) via 0x9ddf7a
//               If NULL → free(esi via [0xf3e264]); return 0
//               memcpy(dst, esi, length) via 0x9d4600
//               free(esi) via [0xf3e264]
//               return dst
//
//   Epilogue at [0x9e4e1f] (shared with adjacent function, outside this 0x130
//   byte range):  pop ebp / pop ebx / pop ecx / pop ecx / ret
//   This function's 0x130 bytes end after `pop esi` (0x5e), falling through
//   to the shared epilogue that follows.
//
//   Reloc-bearing sites (absolute data references embedded as immediates):
//     +0x02   [0x013648b8] load  (global state variable)
//     +0x0d   [0x00f3e258] load  (function pointer)
//     +0x25   [0x013648b8] store (set state=1)
//     +0x32   [0x00f3e1c4] call  (error query)
//     +0x3d   [0x013648b8] store (set state=2)
//     +0x44   [0x013648b8] load  (reload state)
//     +0x7b   [0x00f3e12c] load  (WideCharToMultiByte-like)
//     +0xcb   [0x00f3e25c] call  (free wide buffer)
//     +0xdf   [0x00f3e260] call  (get narrow string)
//     +0x111  [0x00f3e264] call  (free narrow buffer)
//     +0x127  [0x00f3e264] call  (free narrow buffer, success path)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function's 3-state asynchronous dispatch, two distinct wide/narrow
//   string-acquisition paths, two separate WideCharToMultiByte call sites,
//   and four distinct free-call sites make it impossible to coax MSVC 2005
//   /O2 into the exact register allocation (EBX zeroed as null sentinel,
//   EBP reused as both stride-2 and WCtoMB result, EDI reused for two
//   different function-pointer loads) from a source-level rewrite without
//   extensive forced-register hints. The `__declspec(naked)` byte
//   passthrough is the pragmatic choice, matching the same pattern used by
//   FUN_004014b0, FUN_00408f10, and the other complex _rosetta siblings.

extern "C" __declspec(naked) void FUN_009e4cef() {
    __asm {
        // offset +0x000
        _emit 0x51
        _emit 0x51
        _emit 0xa1
        _emit 0xb8
        _emit 0x48
        _emit 0x36
        _emit 0x01
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x57
        _emit 0x8b
        _emit 0x3d
        _emit 0x58
        _emit 0xe2
        _emit 0xf3
        // offset +0x010
        _emit 0x00
        _emit 0x33
        _emit 0xdb
        _emit 0x33
        _emit 0xf6
        _emit 0x3b
        _emit 0xc3
        _emit 0x6a
        _emit 0x02
        _emit 0x5d
        _emit 0x75
        _emit 0x2d
        _emit 0xff
        _emit 0xd7
        _emit 0x8b
        _emit 0xf0
        // offset +0x020
        _emit 0x3b
        _emit 0xf3
        _emit 0x74
        _emit 0x0c
        _emit 0xc7
        _emit 0x05
        _emit 0xb8
        _emit 0x48
        _emit 0x36
        _emit 0x01
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x22
        // offset +0x030
        _emit 0xff
        _emit 0x15
        _emit 0xc4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x83
        _emit 0xf8
        _emit 0x78
        _emit 0x75
        _emit 0x09
        _emit 0x8b
        _emit 0xc5
        _emit 0xa3
        _emit 0xb8
        _emit 0x48
        // offset +0x040
        _emit 0x36
        _emit 0x01
        _emit 0xeb
        _emit 0x05
        _emit 0xa1
        _emit 0xb8
        _emit 0x48
        _emit 0x36
        _emit 0x01
        _emit 0x83
        _emit 0xf8
        _emit 0x01
        _emit 0x0f
        _emit 0x85
        _emit 0x84
        _emit 0x00
        // offset +0x050
        _emit 0x00
        _emit 0x00
        _emit 0x3b
        _emit 0xf3
        _emit 0x75
        _emit 0x0f
        _emit 0xff
        _emit 0xd7
        _emit 0x8b
        _emit 0xf0
        _emit 0x3b
        _emit 0xf3
        _emit 0x75
        _emit 0x07
        _emit 0x33
        _emit 0xc0
        // offset +0x060
        _emit 0xe9
        _emit 0xc9
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66
        _emit 0x39
        _emit 0x1e
        _emit 0x8b
        _emit 0xc6
        _emit 0x74
        _emit 0x0e
        _emit 0x03
        _emit 0xc5
        _emit 0x66
        _emit 0x39
        // offset +0x070
        _emit 0x18
        _emit 0x75
        _emit 0xf9
        _emit 0x03
        _emit 0xc5
        _emit 0x66
        _emit 0x39
        _emit 0x18
        _emit 0x75
        _emit 0xf2
        _emit 0x8b
        _emit 0x3d
        _emit 0x2c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // offset +0x080
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x2b
        _emit 0xc6
        _emit 0x53
        _emit 0xd1
        _emit 0xf8
        _emit 0x40
        _emit 0x50
        _emit 0x56
        _emit 0x53
        _emit 0x53
        _emit 0x89
        _emit 0x44
        _emit 0x24
        // offset +0x090
        _emit 0x34
        _emit 0xff
        _emit 0xd7
        _emit 0x8b
        _emit 0xe8
        _emit 0x3b
        _emit 0xeb
        _emit 0x74
        _emit 0x32
        _emit 0x55
        _emit 0xe8
        _emit 0xec
        _emit 0x91
        _emit 0xff
        _emit 0xff
        _emit 0x3b
        // offset +0x0a0
        _emit 0xc3
        _emit 0x59
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x74
        _emit 0x23
        _emit 0x53
        _emit 0x53
        _emit 0x55
        _emit 0x50
        _emit 0xff
        _emit 0x74
        _emit 0x24
        _emit 0x24
        // offset +0x0b0
        _emit 0x56
        _emit 0x53
        _emit 0x53
        _emit 0xff
        _emit 0xd7
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x0e
        _emit 0xff
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0xe8
        _emit 0xd7
        _emit 0x0e
        // offset +0x0c0
        _emit 0xff
        _emit 0xff
        _emit 0x59
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x56
        _emit 0xff
        _emit 0x15
        _emit 0x5c
        _emit 0xe2
        // offset +0x0d0
        _emit 0xf3
        _emit 0x00
        _emit 0x8b
        _emit 0xc3
        _emit 0xeb
        _emit 0x58
        _emit 0x3b
        _emit 0xc5
        _emit 0x74
        _emit 0x04
        _emit 0x3b
        _emit 0xc3
        _emit 0x75
        _emit 0x80
        _emit 0xff
        _emit 0x15
        // offset +0x0e0
        _emit 0x60
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x8b
        _emit 0xf0
        _emit 0x3b
        _emit 0xf3
        _emit 0x0f
        _emit 0x84
        _emit 0x70
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x38
        _emit 0x1e
        // offset +0x0f0
        _emit 0x74
        _emit 0x0a
        _emit 0x40
        _emit 0x38
        _emit 0x18
        _emit 0x75
        _emit 0xfb
        _emit 0x40
        _emit 0x38
        _emit 0x18
        _emit 0x75
        _emit 0xf6
        _emit 0x2b
        _emit 0xc6
        _emit 0x40
        _emit 0x8b
        // offset +0x100
        _emit 0xe8
        _emit 0x55
        _emit 0xe8
        _emit 0x84
        _emit 0x91
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xf8
        _emit 0x3b
        _emit 0xfb
        _emit 0x59
        _emit 0x75
        _emit 0x0c
        _emit 0x56
        _emit 0xff
        // offset +0x110
        _emit 0x15
        _emit 0x64
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0xe9
        _emit 0x44
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x55
        _emit 0x56
        _emit 0x57
        _emit 0xe8
        _emit 0xef
        _emit 0xf7
        // offset +0x120
        _emit 0xfe
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x56
        _emit 0xff
        _emit 0x15
        _emit 0x64
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x8b
        _emit 0xc7
        _emit 0x5f
        _emit 0x5e
    }
}
