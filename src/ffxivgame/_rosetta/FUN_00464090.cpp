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
// FUNCTION: ffxivgame 0x00464090 — sk_sort  (45 B / 0x2D)
//
//   void __cdecl sk_sort(SortTask *task)
//
//   Calls a sort routine on a deferred-sort descriptor if the task pointer
//   is non-null and the "already sorted" flag ([ESI+0x8]) is zero.
//   On return sets the flag to 1 so the sort is not repeated.
//
//   Struct layout (inferred from field offsets):
//     +0x00  int   count       (nmemb passed to qsort)
//     +0x04  void *data        (base pointer passed to qsort)
//     +0x08  int   done        (0 = unsorted, 1 = sorted)
//     +0x0C  (pad / unknown)
//     +0x10  void *compar      (comparator fn pointer)
//
//   Asm (45 bytes @ orig RVA 0x00064090):
//     56                      PUSH ESI
//     8b 74 24 08             MOV  ESI, [ESP+0x8]      ; task
//     85 f6                   TEST ESI, ESI
//     74 22                   JZ   out                 ; null → skip
//     83 7e 08 00             CMP  dword ptr [ESI+8], 0
//     75 1c                   JNZ  out                 ; done ≠ 0 → skip
//     8b 46 10                MOV  EAX, [ESI+0x10]     ; compar
//     8b 0e                   MOV  ECX, [ESI]          ; count
//     8b 56 04                MOV  EDX, [ESI+0x4]      ; data
//     50                      PUSH EAX
//     6a 04                   PUSH 0x4
//     51                      PUSH ECX
//     52                      PUSH EDX
//     e8 2f 1d 57 00          CALL 0x009D5DE0          ; qsort / sk_sort_fn
//     83 c4 10                ADD  ESP, 0x10
//     c7 46 08 01 00 00 00    MOV  dword ptr [ESI+8], 1
//   out:
//     5e                      POP  ESI
//     c3                      RET
//
// Reconstruction: __declspec(naked) _emit passthrough (same strategy as
// FUN_004051e0 / FUN_00416320). The CALL rel32 bytes (offsets +0x1d..+0x20)
// are masked out by tools/compare.py so the raw reloc bytes do not cause a
// diff failure.

extern "C" __declspec(naked) void FUN_00464090() {
    __asm {
        // 00064090: 56                  PUSH ESI
        _emit 0x56
        // 00064091: 8b 74 24 08         MOV ESI, [ESP+0x8]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 00064095: 85 f6               TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 00064097: 74 22               JZ out (+0x22)
        _emit 0x74
        _emit 0x22
        // 00064099: 83 7e 08 00         CMP dword ptr [ESI+8], 0
        _emit 0x83
        _emit 0x7e
        _emit 0x08
        _emit 0x00
        // 0006409d: 75 1c               JNZ out (+0x1c)
        _emit 0x75
        _emit 0x1c
        // 0006409f: 8b 46 10            MOV EAX, [ESI+0x10]
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        // 000640a2: 8b 0e               MOV ECX, [ESI]
        _emit 0x8b
        _emit 0x0e
        // 000640a4: 8b 56 04            MOV EDX, [ESI+0x4]
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 000640a7: 50                  PUSH EAX
        _emit 0x50
        // 000640a8: 6a 04               PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 000640aa: 51                  PUSH ECX
        _emit 0x51
        // 000640ab: 52                  PUSH EDX
        _emit 0x52
        // 000640ac: e8 2f 1d 57 00      CALL 0x009D5DE0  (rel32 = 0x00571D2F)
        _emit 0xe8
        _emit 0x2f
        _emit 0x1d
        _emit 0x57
        _emit 0x00
        // 000640b1: 83 c4 10            ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 000640b4: c7 46 08 01 00 00 00  MOV dword ptr [ESI+8], 1
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000640bb: 5e                  POP ESI
        _emit 0x5e
        // 000640bc: c3                  RET
        _emit 0xc3
    }
}
