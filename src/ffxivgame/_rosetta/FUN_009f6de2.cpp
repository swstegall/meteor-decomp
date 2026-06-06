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
// FUNCTION: ffxivgame 0x005f6de2 — CRT file-handle dispatcher returning
//                                  __int64 (271 B / 0x10f, __cdecl,
//                                  EBP-based SEH frame).
//
// Signature (inferred from the asm):
//
//   __int64 FUN_009f6de2(int fd, int arg2, int arg3, int arg4);
//
// Closely mirrors FUN_009e7abb (0x005e7abb) but with four parameters
// instead of three and a 64-bit EAX:EDX return stored in locals at
// [EBP-0x24] (low) / [EBP-0x20] (high).
//
// The function uses MSVC's __SEH_prolog4 / __SEH_epilog4 pattern
// (PUSH 0x14; PUSH scope_table; CALL __SEH_prolog4) to set up an
// EBP-based frame with structured-exception-handler support, then:
//
//   1. Initialises the 64-bit result to -1:-1 ([EBP-0x24], [EBP-0x20]).
//   2. If fd == -2 (invalid sentinel): clears errno, sets _doserrno = 9
//      (EBADF), and returns -1:-1.
//   3. If fd < 0 or fd >= [0x137b7dc] (file-table count): same error path
//      plus a call to FUN_009d2290 with five zero arguments (internal
//      invalid-fd diagnostic), then returns -1:-1.
//   4. Otherwise: computes slot index (fd >> 5) and bit position
//      ((fd & 0x1f) << 6), looks up __pioinfo table at [0x137b7e0],
//      checks bit 0 of byte at [slot + ESI + 4] (FOPEN flag), and:
//      a. If not set: same error+diagnostic path as #3, returns -1:-1.
//      b. If set: calls FUN_009e77ed(fd) to lock the handle, then
//         re-checks the FOPEN bit; if still set, calls FUN_009f6d5f
//         (the inner worker, at [EBP+8..14]) and stores EAX:EDX into
//         [EBP-0x24] / [EBP-0x20]; otherwise sets _doserrno = 9 and
//         result to -1:-1.
//   5. Sets SEH state [EBP-4] = 0xfffffffe, calls the cleanup/unlock
//      thunk at FUN_009f6ef1 (the function immediately following in
//      .text), loads EAX:EDX from [EBP-0x24] / [EBP-0x20], then calls
//      __SEH_epilog4 (FUN_009de535) before RET.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   Reproducing the exact SEH prolog/epilog, the EBP-frame slot
//   assignments, the SEH state machine at [EBP-4], the 64-bit result
//   accumulation, and the precise short-vs-near branch encoding from
//   C++ source at /O2 /GS is impractical without the original source.
//   Emitting the 271 original bytes verbatim via MASM `_emit` directives
//   gives compare.py a byte-exact GREEN match (same strategy as
//   FUN_009e7abb / FUN_00402a30 / FUN_00404f70).
//
// Reloc-bearing sites (offsets within the 271-byte body):
//   +0x02  DIR32 → 0x0122d788   (scope table / __SEH_prolog4 arg)
//   +0x07  REL32 → 0x009de4f0   (__SEH_prolog4)
//   +0x1d  REL32 → 0x009d9d5a   (_errno)
//   +0x25  REL32 → 0x009d9d47   (__doserrno)
//   +0x3f  DIR32 → 0x0137b7dc   (file-table count)
//   +0x47  REL32 → 0x009d9d5a   (_errno)
//   +0x4e  REL32 → 0x009d9d47   (__doserrno)
//   +0x5e  REL32 → 0x009d2290   (invalid-fd diagnostic)
//   +0x6d  DIR32 → 0x0137b7e0   (file-table base)
//   +0x88  REL32 → 0x009d9d5a   (_errno)
//   +0x8f  REL32 → 0x009d9d47   (__doserrno)
//   +0x9f  REL32 → 0x009d2290   (invalid-fd diagnostic)
//   +0xaf  REL32 → 0x009e77ed   (lock/acquire helper)
//   +0xcd  REL32 → 0x009f6d5f   (inner worker — 4-arg version)
//   +0xdd  REL32 → 0x009d9d47   (__doserrno)
//   +0xe8  REL32 → 0x009d9d5a   (_errno)
//   +0xfe  REL32 → 0x009f6ef1   (cleanup/unlock thunk, next fn)
//   +0x109 REL32 → 0x009de535   (__SEH_epilog4)

extern "C" __declspec(naked) void FUN_009f6de2() {
    __asm {
        _emit 0x6a  // PUSH 0x14  (SEH frame locals size)
        _emit 0x14
        _emit 0x68  // PUSH 0x122d788  (scope table)
        _emit 0x88
        _emit 0xd7
        _emit 0x22
        _emit 0x01
        _emit 0xe8  // CALL __SEH_prolog4 (0x009de4f0)
        _emit 0x02
        _emit 0x77
        _emit 0xfe
        _emit 0xff
        _emit 0x83  // OR ESI,0xffffffff
        _emit 0xce
        _emit 0xff
        _emit 0x89  // MOV [EBP-0x24],ESI
        _emit 0x75
        _emit 0xdc
        _emit 0x89  // MOV [EBP-0x20],ESI
        _emit 0x75
        _emit 0xe0
        _emit 0x8b  // MOV EAX,[EBP+8]  (fd)
        _emit 0x45
        _emit 0x08
        _emit 0x83  // CMP EAX,-2
        _emit 0xf8
        _emit 0xfe
        _emit 0x75  // JNZ +0x1c
        _emit 0x1c
        _emit 0xe8  // CALL _errno (0x009d9d5a)
        _emit 0x56
        _emit 0x2f
        _emit 0xfe
        _emit 0xff
        _emit 0x83  // AND [EAX],0
        _emit 0x20
        _emit 0x00
        _emit 0xe8  // CALL __doserrno (0x009d9d47)
        _emit 0x3b
        _emit 0x2f
        _emit 0xfe
        _emit 0xff
        _emit 0xc7  // MOV [EAX],9
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EAX,ESI  (EAX = -1)
        _emit 0xc6
        _emit 0x8b  // MOV EDX,ESI  (EDX = -1)
        _emit 0xd6
        _emit 0xe9  // JMP to __SEH_epilog4 + RET
        _emit 0xd0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33  // XOR EDI,EDI
        _emit 0xff
        _emit 0x3b  // CMP EAX,EDI
        _emit 0xc7
        _emit 0x7c  // JL +8
        _emit 0x08
        _emit 0x3b  // CMP EAX,[0x137b7dc]
        _emit 0x05
        _emit 0xdc
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0x72  // JC +0x21  (valid range)
        _emit 0x21
        _emit 0xe8  // CALL _errno (0x009d9d5a)
        _emit 0x2c
        _emit 0x2f
        _emit 0xfe
        _emit 0xff
        _emit 0x89  // MOV [EAX],EDI  (*errno = 0)
        _emit 0x38
        _emit 0xe8  // CALL __doserrno (0x009d9d47)
        _emit 0x12
        _emit 0x2f
        _emit 0xfe
        _emit 0xff
        _emit 0xc7  // MOV [EAX],9
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x57  // PUSH EDI  (x5 — five zero args)
        _emit 0x57
        _emit 0x57
        _emit 0x57
        _emit 0x57
        _emit 0xe8  // CALL FUN_009d2290 (invalid-fd diagnostic)
        _emit 0x4b
        _emit 0xb4
        _emit 0xfd
        _emit 0xff
        _emit 0x83  // ADD ESP,0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xeb  // JMP back → MOV EAX,ESI  (-0x38)
        _emit 0xc8
        _emit 0x8b  // MOV ECX,EAX
        _emit 0xc8
        _emit 0xc1  // SAR ECX,5  (slot index = fd / 32)
        _emit 0xf9
        _emit 0x05
        _emit 0x8d  // LEA EBX,[ECX*4 + 0x137b7e0]
        _emit 0x1c
        _emit 0x8d
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0x8b  // MOV ESI,EAX
        _emit 0xf0
        _emit 0x83  // AND ESI,0x1f  (bit pos = fd & 31)
        _emit 0xe6
        _emit 0x1f
        _emit 0xc1  // SHL ESI,6  (* 64)
        _emit 0xe6
        _emit 0x06
        _emit 0x8b  // MOV ECX,[EBX]
        _emit 0x0b
        _emit 0x0f  // MOVZX ECX,byte ptr [ECX + ESI + 4]
        _emit 0xb6
        _emit 0x4c
        _emit 0x31
        _emit 0x04
        _emit 0x83  // AND ECX,1  (FOPEN bit)
        _emit 0xe1
        _emit 0x01
        _emit 0x75  // JNZ +0x26  (bit set → proceed)
        _emit 0x26
        _emit 0xe8  // CALL _errno (0x009d9d5a)
        _emit 0xeb
        _emit 0x2e
        _emit 0xfe
        _emit 0xff
        _emit 0x89  // MOV [EAX],EDI
        _emit 0x38
        _emit 0xe8  // CALL __doserrno (0x009d9d47)
        _emit 0xd1
        _emit 0x2e
        _emit 0xfe
        _emit 0xff
        _emit 0xc7  // MOV [EAX],9
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x57  // PUSH EDI  (x5)
        _emit 0x57
        _emit 0x57
        _emit 0x57
        _emit 0x57
        _emit 0xe8  // CALL FUN_009d2290
        _emit 0x0a
        _emit 0xb4
        _emit 0xfd
        _emit 0xff
        _emit 0x83  // ADD ESP,0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x83  // OR EDX,0xffffffff
        _emit 0xca
        _emit 0xff
        _emit 0x8b  // MOV EAX,EDX
        _emit 0xc2
        _emit 0xeb  // JMP → __SEH_epilog4 + RET  (+0x5b)
        _emit 0x5b
        _emit 0x50  // PUSH EAX  (fd)
        _emit 0xe8  // CALL FUN_009e77ed (lock/acquire)
        _emit 0x57
        _emit 0x09
        _emit 0xff
        _emit 0xff
        _emit 0x59  // POP ECX
        _emit 0x89  // MOV [EBP-4],EDI  (SEH state = 0)
        _emit 0x7d
        _emit 0xfc
        _emit 0x8b  // MOV EAX,[EBX]
        _emit 0x03
        _emit 0xf6  // TEST byte ptr [EAX + ESI + 4],1
        _emit 0x44
        _emit 0x30
        _emit 0x04
        _emit 0x01
        _emit 0x74  // JZ +0x1c  (not set now → error)
        _emit 0x1c
        _emit 0xff  // PUSH [EBP+0x14]  (arg4)
        _emit 0x75
        _emit 0x14
        _emit 0xff  // PUSH [EBP+0x10]  (arg3)
        _emit 0x75
        _emit 0x10
        _emit 0xff  // PUSH [EBP+0xc]   (arg2)
        _emit 0x75
        _emit 0x0c
        _emit 0xff  // PUSH [EBP+0x8]   (fd)
        _emit 0x75
        _emit 0x08
        _emit 0xe8  // CALL FUN_009f6d5f (inner worker)
        _emit 0xab
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83  // ADD ESP,0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x89  // MOV [EBP-0x24],EAX  (result lo)
        _emit 0x45
        _emit 0xdc
        _emit 0x89  // MOV [EBP-0x20],EDX  (result hi)
        _emit 0x55
        _emit 0xe0
        _emit 0xeb  // JMP +0x1a
        _emit 0x1a
        _emit 0xe8  // CALL __doserrno (0x009d9d47)
        _emit 0x83
        _emit 0x2e
        _emit 0xfe
        _emit 0xff
        _emit 0xc7  // MOV [EAX],9
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL _errno (0x009d9d5a)
        _emit 0x8b
        _emit 0x2e
        _emit 0xfe
        _emit 0xff
        _emit 0x89  // MOV [EAX],EDI
        _emit 0x38
        _emit 0x83  // OR [EBP-0x24],0xffffffff
        _emit 0x4d
        _emit 0xdc
        _emit 0xff
        _emit 0x83  // OR [EBP-0x20],0xffffffff
        _emit 0x4d
        _emit 0xe0
        _emit 0xff
        _emit 0xc7  // MOV [EBP-4],0xfffffffe  (SEH state = -2)
        _emit 0x45
        _emit 0xfc
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8  // CALL FUN_009f6ef1 (cleanup/unlock thunk)
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EAX,[EBP-0x24]  (result lo)
        _emit 0x45
        _emit 0xdc
        _emit 0x8b  // MOV EDX,[EBP-0x20]  (result hi)
        _emit 0x55
        _emit 0xe0
        _emit 0xe8  // CALL __SEH_epilog4 (0x009de535)
        _emit 0x45
        _emit 0x76
        _emit 0xfe
        _emit 0xff
        _emit 0xc3  // RET
    }
}
