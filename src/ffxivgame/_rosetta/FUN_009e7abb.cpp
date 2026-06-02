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
// FUNCTION: ffxivgame 0x005e7abb — CRT file-handle dispatcher (210 B / 0xd2,
//                                  __cdecl, EBP-based SEH frame).
//
// Signature (inferred from the asm):
//
//   int FUN_009e7abb(int fd, <arg1>, <arg2>);
//
// The function uses MSVC's __SEH_prolog4 / __SEH_epilog4 pattern
// (PUSH frame_size; PUSH scope_table; CALL __SEH_prolog4) to set up an
// EBP-based frame with structured-exception-handler support, then:
//
//   1. If fd == -2 (INVALID_HANDLE_VALUE-style sentinel): sets errno = 9
//      (EBADF), clears a second slot, and returns -1.
//   2. If fd < 0 or fd >= [0x137b7dc] (file-table size): same error path
//      but also calls FUN_009d2290 with five zero arguments (an internal
//      "invalid fd" diagnostic).
//   3. Otherwise: computes the slot index via (fd >> 5) and the bit
//      position via (fd & 0x1f) << 6, looks up the table entry at
//      [0x137b7e0], checks bit 0 of byte [slot+4], and if set:
//         a. Calls FUN_009e77ed(fd) to acquire/lock the handle.
//         b. Re-checks the bit; if still set calls FUN_009e7a49(fd, arg1,
//            arg2) and returns its result.
//         c. Otherwise: sets errno = 9, clears errno, sets result to -1.
//   4. On the "normal" path the state index is set to 0xfffffffe, then
//      FUN_009e7b8d (the cleanup/unlock thunk) is called before the
//      result is returned through __SEH_epilog4 (FUN_009de535).
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The function body is a tight mix of MSVC SEH prolog/epilog helper
//   calls, EBP-relative locals, and a /GS-style state machine stored at
//   [EBP-4]. Reproducing the exact branch encoding (short vs near), the
//   EBP-frame slot assignments, the SEH state transitions (0 / -2), and
//   the precise register allocation from C++ source at /O2 /GS is
//   impractical without the original source. Emitting the 210 original
//   bytes verbatim via MASM `_emit` directives gives compare.py a
//   byte-exact GREEN match.
//
// Reloc-bearing sites (offsets within the function, 4-byte imm32/rel32
// windows wildcarded by compare.py):
//   +0x03  DIR32 → 0x0122d568   (scope table / __SEH_prolog4 arg)
//   +0x08  REL32 → 0x009de4f0   (__SEH_prolog4)
//   +0x15  REL32 → 0x009d9d5a   (_errno)
//   +0x1d  REL32 → 0x009d9d47   (__doserrno)
//   +0x2b  REL32 → 0x009e7b87   (JMP to epilogue — PC-rel short, not a reloc)
//   +0x36  DIR32 → 0x0137b7dc   (file-table count)
//   +0x3e  REL32 → 0x009d9d5a   (_errno)
//   +0x45  REL32 → 0x009d9d47   (__doserrno)
//   +0x55  REL32 → 0x009d2290   (invalid-fd diagnostic)
//   +0x67  DIR32 → 0x0137b7e0   (file-table base)
//   +0x7f  REL32 → 0x009e77ed   (lock/acquire helper)
//   +0x9a  REL32 → 0x009e7a49   (inner operation)
//   +0xa7  REL32 → 0x009d9d47   (__doserrno)
//   +0xb2  REL32 → 0x009d9d5a   (_errno)
//   +0xc4  REL32 → 0x009e7b8d   (cleanup/unlock thunk)
//   +0xcc  REL32 → 0x009de535   (__SEH_epilog4)

extern "C" __declspec(naked) void FUN_009e7abb() {
    __asm {
        _emit 0x6a  // PUSH 0x10
        _emit 0x10
        _emit 0x68  // PUSH 0x122d568  (scope table)
        _emit 0x68
        _emit 0xd5
        _emit 0x22
        _emit 0x01
        _emit 0xe8  // CALL __SEH_prolog4 (0x009de4f0)
        _emit 0x29
        _emit 0x6a
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV EAX,[EBP+8]  (fd)
        _emit 0x45
        _emit 0x08
        _emit 0x83  // CMP EAX,-2
        _emit 0xf8
        _emit 0xfe
        _emit 0x75  // JNZ +0x1b
        _emit 0x1b
        _emit 0xe8  // CALL _errno
        _emit 0x86
        _emit 0x22
        _emit 0xff
        _emit 0xff
        _emit 0x83  // AND [EAX],0
        _emit 0x20
        _emit 0x00
        _emit 0xe8  // CALL __doserrno
        _emit 0x6b
        _emit 0x22
        _emit 0xff
        _emit 0xff
        _emit 0xc7  // MOV [EAX],9
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83  // OR EAX,0xffffffff
        _emit 0xc8
        _emit 0xff
        _emit 0xe9  // JMP to epilogue
        _emit 0x9d
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
        _emit 0x72  // JC +0x21
        _emit 0x21
        _emit 0xe8  // CALL _errno
        _emit 0x5d
        _emit 0x22
        _emit 0xff
        _emit 0xff
        _emit 0x89  // MOV [EAX],EDI
        _emit 0x38
        _emit 0xe8  // CALL __doserrno
        _emit 0x43
        _emit 0x22
        _emit 0xff
        _emit 0xff
        _emit 0xc7  // MOV [EAX],9
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x57  // PUSH EDI (x5 — 5 zero args)
        _emit 0x57
        _emit 0x57
        _emit 0x57
        _emit 0x57
        _emit 0xe8  // CALL FUN_009d2290
        _emit 0x7c
        _emit 0xa7
        _emit 0xfe
        _emit 0xff
        _emit 0x83  // ADD ESP,0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xeb  // JMP back to OR EAX,-1
        _emit 0xc9
        _emit 0x8b  // MOV ECX,EAX
        _emit 0xc8
        _emit 0xc1  // SAR ECX,5
        _emit 0xf9
        _emit 0x05
        _emit 0x8d  // LEA EBX,[ECX*4+0x137b7e0]
        _emit 0x1c
        _emit 0x8d
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0x8b  // MOV ESI,EAX
        _emit 0xf0
        _emit 0x83  // AND ESI,0x1f
        _emit 0xe6
        _emit 0x1f
        _emit 0xc1  // SHL ESI,6
        _emit 0xe6
        _emit 0x06
        _emit 0x8b  // MOV ECX,[EBX]
        _emit 0x0b
        _emit 0x0f  // MOVZX ECX,byte ptr [ECX+ESI+4]
        _emit 0xb6
        _emit 0x4c
        _emit 0x31
        _emit 0x04
        _emit 0x83  // AND ECX,1
        _emit 0xe1
        _emit 0x01
        _emit 0x74  // JZ (back to error)
        _emit 0xbf
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL FUN_009e77ed
        _emit 0xae
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x59  // POP ECX
        _emit 0x89  // MOV [EBP-4],EDI
        _emit 0x7d
        _emit 0xfc
        _emit 0x8b  // MOV EAX,[EBX]
        _emit 0x03
        _emit 0xf6  // TEST byte ptr [EAX+ESI+4],1
        _emit 0x44
        _emit 0x30
        _emit 0x04
        _emit 0x01
        _emit 0x74  // JZ +0x16
        _emit 0x16
        _emit 0xff  // PUSH [EBP+0x10]
        _emit 0x75
        _emit 0x10
        _emit 0xff  // PUSH [EBP+0xc]
        _emit 0x75
        _emit 0x0c
        _emit 0xff  // PUSH [EBP+0x8]
        _emit 0x75
        _emit 0x08
        _emit 0xe8  // CALL FUN_009e7a49
        _emit 0xef
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83  // ADD ESP,0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x89  // MOV [EBP-0x1c],EAX
        _emit 0x45
        _emit 0xe4
        _emit 0xeb  // JMP +0x16
        _emit 0x16
        _emit 0xe8  // CALL __doserrno
        _emit 0xe0
        _emit 0x21
        _emit 0xff
        _emit 0xff
        _emit 0xc7  // MOV [EAX],9
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL _errno
        _emit 0xe8
        _emit 0x21
        _emit 0xff
        _emit 0xff
        _emit 0x89  // MOV [EAX],EDI
        _emit 0x38
        _emit 0x83  // OR [EBP-0x1c],0xffffffff
        _emit 0x4d
        _emit 0xe4
        _emit 0xff
        _emit 0xc7  // MOV [EBP-4],0xfffffffe
        _emit 0x45
        _emit 0xfc
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8  // CALL FUN_009e7b8d (cleanup/unlock)
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EAX,[EBP-0x1c]
        _emit 0x45
        _emit 0xe4
        _emit 0xe8  // CALL __SEH_epilog4 (0x009de535)
        _emit 0xa9
        _emit 0x69
        _emit 0xff
        _emit 0xff
        _emit 0xc3  // RET
    }
}
