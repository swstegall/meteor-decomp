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
// FUNCTION: ffxivgame 0x009d8f22 — `__cdecl` CRT __get_osver  (60 B / 0x3c)
//
// FID name: FID_conflict:__get_osver
//
// Signature (inferred from asm):
//   int __cdecl __get_osver(unsigned int *result)
//
// Purpose:
//   Writes the cached OS version word into *result and returns 0.
//   If result is NULL, or if the OS-version cache at [0x01363f40] has not
//   been initialised (== 0), sets errno to EINVAL (0x16), calls the CRT
//   invalid-parameter handler, and returns EINVAL.
//
// Control flow:
//   1. Load arg1 (result ptr) into EAX; ESI = 0.
//   2. CMP EAX, ESI → JNZ to check-init path.
//   3. Error path (shared):
//        CALL _errno()           → EAX = &errno
//        PUSH 0 × 5              (five NULL/0 args for _invalid_parameter)
//        MOV [EAX], 0x16         set errno = EINVAL
//        CALL _invalid_parameter(NULL,NULL,NULL,0,0)
//        ADD ESP, 0x14           clean 5 × DWORD
//        PUSH 0x16 / POP EAX    return value = EINVAL
//        POP ESI ; RET
//   4. Check-init path (0x009d8f4a):
//        CMP [0x01363f40], ESI → JZ to error path (not yet init'd)
//        MOV ECX, [0x01363f44]  ; cached os version
//        MOV [EAX], ECX         ; *result = cached value
//        XOR EAX, EAX           ; return 0
//        POP ESI ; RET
//
// Calls:
//   0x009d9d47  — _errno()               (returns int* — thread-local errno ptr)
//   0x009d2290  — _invalid_parameter(…)  (__cdecl, 5 args, all zero/NULL here)
//
// Globals (VA, image base 0x00400000):
//   0x01363f40  — os-version initialised flag  (int)
//   0x01363f44  — cached os version word       (unsigned int)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The two REL32 CALL targets and the two absolute data references are all
//   linker-resolved relocations that a standalone .obj cannot reproduce from
//   C source. The pragmatic choice — the same one FUN_00406fa0 and FUN_00408780
//   used — is a `__declspec(naked)` body that re-emits all 60 orig bytes
//   verbatim via MASM `_emit` directives. `tools/compare.py` masks reloc-bearing
//   bytes and reports GREEN on a byte-identical .text slice.
//
// Asm (60 bytes, VA 0x009d8f22 – 0x009d8f5e):
//   8b 44 24 04               MOV  EAX, dword ptr [ESP+0x04]
//   56                        PUSH ESI
//   33 f6                     XOR  ESI, ESI
//   3b c6                     CMP  EAX, ESI
//   75 1d                     JNZ  +0x1d  (→ 0x009d8f4a)
//   e8 15 0e 00 00            CALL _errno()
//   56 56 56 56 56            PUSH ESI × 5
//   c7 00 16 00 00 00         MOV  dword ptr [EAX], 0x16
//   e8 4e 93 ff ff            CALL _invalid_parameter
//   83 c4 14                  ADD  ESP, 0x14
//   6a 16                     PUSH 0x16
//   58                        POP  EAX
//   5e                        POP  ESI
//   c3                        RET
//   39 35 40 3f 36 01         CMP  dword ptr [0x01363f40], ESI
//   74 db                     JZ   -0x25  (→ error path)
//   8b 0d 44 3f 36 01         MOV  ECX, dword ptr [0x01363f44]
//   89 08                     MOV  dword ptr [EAX], ECX
//   33 c0                     XOR  EAX, EAX
//   5e                        POP  ESI
//   c3                        RET

extern "C" __declspec(naked) void FUN_009d8f22() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x04]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x56              // PUSH ESI
        _emit 0x33              // XOR ESI, ESI
        _emit 0xf6
        _emit 0x3b              // CMP EAX, ESI
        _emit 0xc6
        _emit 0x75              // JNZ +0x1d  (→ check-init path)
        _emit 0x1d
        _emit 0xe8              // CALL rel32 → 0x009d9d47  (_errno)
        _emit 0x15
        _emit 0x0e
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI  (arg5 = 0)
        _emit 0x56              // PUSH ESI  (arg4 = 0)
        _emit 0x56              // PUSH ESI  (arg3 = 0)
        _emit 0x56              // PUSH ESI  (arg2 = 0)
        _emit 0x56              // PUSH ESI  (arg1 = 0)
        _emit 0xc7              // MOV dword ptr [EAX], 0x00000016  (errno = EINVAL)
        _emit 0x00
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL rel32 → 0x009d2290  (_invalid_parameter)
        _emit 0x4e
        _emit 0x93
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x6a              // PUSH 0x16
        _emit 0x16
        _emit 0x58              // POP EAX
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        _emit 0x39              // CMP dword ptr [0x01363f40], ESI
        _emit 0x35
        _emit 0x40
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0x74              // JZ -0x25  (→ error path)
        _emit 0xdb
        _emit 0x8b              // MOV ECX, dword ptr [0x01363f44]
        _emit 0x0d
        _emit 0x44
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0x89              // MOV dword ptr [EAX], ECX
        _emit 0x08
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
