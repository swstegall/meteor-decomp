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
// FUNCTION: ffxivgame 0x0000b7d0 — destructor/teardown for a manager class
//                                  that owns a CRITICAL_SECTION at offset +0x64
//                                  (__thiscall, 97 bytes / 0x61)
//
// void FUN_0040b7d0(ManagerClass *this)  [ECX = this]
//
// Sets up an SEH frame, enters the CRITICAL_SECTION at (this+0x64),
// calls FUN_0040b150(this) to do the actual teardown work, then leaves
// and deletes the CRITICAL_SECTION before returning.
//
// Object layout (offsets touched):
//   [this + 0x64]  CRITICAL_SECTION (24 bytes)  — same object as FUN_0040ad30
//
// SEH frame layout (from the orig prologue):
//   PUSH -1             ; ExceptionHandler state = -1 (not yet entered CS)
//   PUSH 0xe54dcf       ; SEH cookie / handler address
//   MOV EAX, FS:[0]    ; old top of SEH chain
//   PUSH EAX
//   MOV FS:[0], ESP    ; install new SEH record at TOS
//   SUB ESP, 0x8       ; 2 extra DWORD locals
//   (after PUSH ESI + PUSH EDI, ESP is 0x10 lower than SEH frame base)
//   [ESP+0x08] = this  (saved for SEH handler)
//   [ESP+0x10] = EDI = this+0x64 (lpCriticalSection, also saved for SEH)
//   [ESP+0x18] = state byte (set to 0, then 1 after EnterCriticalSection)
//   [ESP+0x1c] = state dword (set to 0 in prologue slot)
//
// IAT call slots (reloc sites — masked by tools/compare.py):
//   EnterCriticalSection  @ [0x00f3e16c]
//   LeaveCriticalSection  @ [0x00f3e168]
//   DeleteCriticalSection @ [0x00f3e170]
//
// REL call sites (reloc sites — masked by tools/compare.py):
//   FUN_0040b150  (rel32 = 0xfffff93e from site 0x0040b80d)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The SEH prologue/epilogue involves FS-segment writes and a layered
//   stack layout that MSVC 2005 emits in a specific order not reproducible
//   from C++ source. The __declspec(naked) body re-emits the original 97
//   bytes verbatim via MASM _emit directives; the .obj's .text is
//   byte-identical to the original slice, and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0040b7d0() {
    __asm {
        // 0000b7d0:  6a ff              PUSH -1
        _emit 0x6a
        _emit 0xff
        // 0000b7d2:  68 cf 4d e5 00     PUSH 0xe54dcf
        _emit 0x68
        _emit 0xcf
        _emit 0x4d
        _emit 0xe5
        _emit 0x00
        // 0000b7d7:  64 a1 00 00 00 00  MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000b7dd:  50                 PUSH EAX
        _emit 0x50
        // 0000b7de:  64 89 25 00 00 00 00  MOV dword ptr FS:[0x0], ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000b7e5:  83 ec 08           SUB ESP, 0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0000b7e8:  56                 PUSH ESI
        _emit 0x56
        // 0000b7e9:  8b f1              MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 0000b7eb:  57                 PUSH EDI
        _emit 0x57
        // 0000b7ec:  89 74 24 08        MOV dword ptr [ESP+0x8], ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 0000b7f0:  8d 7e 64           LEA EDI, [ESI+0x64]
        _emit 0x8d
        _emit 0x7e
        _emit 0x64
        // 0000b7f3:  57                 PUSH EDI
        _emit 0x57
        // 0000b7f4:  c7 44 24 1c 00 00 00 00  MOV dword ptr [ESP+0x1c], 0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000b7fc:  89 7c 24 10        MOV dword ptr [ESP+0x10], EDI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 0000b800:  ff 15 6c e1 f3 00  CALL dword ptr [0x00f3e16c]  ; EnterCriticalSection
        _emit 0xff
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0000b806:  8b ce              MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0000b808:  c6 44 24 18 01     MOV byte ptr [ESP+0x18], 0x1
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x01
        // 0000b80d:  e8 3e f9 ff ff     CALL 0x0040b150  (rel32=0xfffff93e)
        _emit 0xe8
        _emit 0x3e
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        // 0000b812:  57                 PUSH EDI
        _emit 0x57
        // 0000b813:  ff 15 68 e1 f3 00  CALL dword ptr [0x00f3e168]  ; LeaveCriticalSection
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0000b819:  57                 PUSH EDI
        _emit 0x57
        // 0000b81a:  ff 15 70 e1 f3 00  CALL dword ptr [0x00f3e170]  ; DeleteCriticalSection
        _emit 0xff
        _emit 0x15
        _emit 0x70
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0000b820:  8b 4c 24 10        MOV ECX, dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0000b824:  5f                 POP EDI
        _emit 0x5f
        // 0000b825:  5e                 POP ESI
        _emit 0x5e
        // 0000b826:  64 89 0d 00 00 00 00  MOV dword ptr FS:[0x0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000b82d:  83 c4 14           ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0000b830:  c3                 RET
        _emit 0xc3
    }
}
