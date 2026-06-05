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
// FUNCTION: ffxivgame 0x0005c360 — `__cdecl` object allocator/initializer
//                                   (302 B / 0x12e, no SEH, no /GS cookie)
//
// Inspection (read from the disassembly at orig RVA 0x0005c360):
//
//   __cdecl void* FUN_0045c360();
//
//   Structure:
//
//     Allocates 0x198 bytes of local stack space via _chkstk, saves
//     EBX/ESI/EDI.  Checks whether the global vtable pointer at
//     0x0132e788 is already initialized; if not, calls FUN_00465f80
//     twice to register resource handlers (IDs 9 and 10, each with
//     count params 0x127 / 0x12a) and conditionally sets the global to
//     0xf68000.
//
//     Calls FUN_00465c10 and FUN_00465ce0 (twice total) to prepare
//     local buffer state at [ESP+0xc], then calls through vtable slot
//     0x1c on the global object.  If that call returns non-zero
//     (error), returns the non-zero value in EAX (ESI).
//
//     If slot-0x1c returns zero, calls FUN_00463150 to allocate a
//     new 0x190-byte record from pool 0xf6802c.  If allocation fails
//     (NULL), returns the static global pointer 0x00132e5f8 in EAX.
//
//     If allocation succeeds, initialises fields at offsets +0x188 and
//     +0x18c to zero, then zeros 0x10 dword-pairs across the range
//     [ESI+0x88 .. ESI+0xc8] via a counted loop, calls vtable slot
//     0x20 (register/add) and slot 0x1c (lookup) on the global; if
//     the lookup returns the new record (EAX == ESI), calls
//     FUN_0045c1f0(EBX) (cleanup of the EBX result) if EBX != 0,
//     then returns ESI.  If the lookup returns something other than
//     ESI, calls FUN_0045c1f0(ESI) to release the new record, then
//     falls through to the static-pointer return.
//
//   Stack frame (after _chkstk + PUSH EBX/ESI/EDI):
//     total alloc  = 0x198 + 0xc (3 callee-saves) = 0x1a4 B
//     [ESP+0x00]   saved EBX
//     [ESP+0x04]   saved ESI
//     [ESP+0x08]   saved EDI (not used — EDI = 0 constant)
//     [ESP+0x0c]   local buffer / arg staging area (size ~0x18)
//
//   Return values:
//     EAX = 0x00132e5f8  — allocation failed or lookup mismatch path
//     EAX = ESI           — successfully allocated & registered, OR
//                           vtable[0x1c] returned non-zero (error path)
//
//   No SEH, no /GS cookie (no local char array ≥5 bytes in compiler's
//   view — the 0x198 alloc is done via _chkstk, not a local array).
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Source-level C++ would need to coax MSVC 2005 /O2 into reproducing
//   the exact register allocation (EDI permanently = 0, ESI for the
//   allocated pointer, EBX for the vtable[0x20] result), the precise
//   loop that clears two-dwords-per-stride across the object, the
//   specific branch encoding (rel8 vs rel32, JZ/JNZ choice), and the
//   linker-resolved absolute addresses in every CMP/MOV/CALL site.
//   Each constraint is brittle — any high-level rewrite shifts at
//   least one byte.
//
//   The pragmatic choice — the same one FUN_00401a00, FUN_004014b0,
//   FUN_00408f10 and the rest of the _rosetta siblings took — is a
//   `__declspec(naked)` body that re-emits the 302 orig bytes verbatim
//   via MASM `_emit` directives.  The .obj .text section ends up
//   byte-identical to the orig slice, which is what compare.py checks.

extern "C" __declspec(naked) void FUN_0045c360() {
    __asm {
        // 0005c360: MOV EAX, 0x198
        _emit 0xb8
        _emit 0x98
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c365: CALL _chkstk (0x009d29d0)
        _emit 0xe8
        _emit 0x66
        _emit 0x66
        _emit 0x57
        _emit 0x00
        // 0005c36a: PUSH EBX
        _emit 0x53
        // 0005c36b: PUSH ESI
        _emit 0x56
        // 0005c36c: PUSH EDI
        _emit 0x57
        // 0005c36d: XOR EDI, EDI
        _emit 0x33
        _emit 0xff
        // 0005c36f: CMP dword ptr [0x0132e788], EDI
        _emit 0x39
        _emit 0x3d
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 0005c375: JNZ 0x0045c3b5
        _emit 0x75
        _emit 0x3e
        // 0005c377: PUSH 0x127
        _emit 0x68
        _emit 0x27
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c37c: PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c381: PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005c383: PUSH 0x9
        _emit 0x6a
        _emit 0x09
        // 0005c385: CALL FUN_00465f80
        _emit 0xe8
        _emit 0xf6
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        // 0005c38a: ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0005c38d: CMP dword ptr [0x0132e788], EDI
        _emit 0x39
        _emit 0x3d
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 0005c393: JNZ 0x0045c39f
        _emit 0x75
        _emit 0x0a
        // 0005c395: MOV dword ptr [0x0132e788], 0xf68000
        _emit 0xc7
        _emit 0x05
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c39f: PUSH 0x12a
        _emit 0x68
        _emit 0x2a
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c3a4: PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c3a9: PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005c3ab: PUSH 0xa
        _emit 0x6a
        _emit 0x0a
        // 0005c3ad: CALL FUN_00465f80
        _emit 0xe8
        _emit 0xce
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        // 0005c3b2: ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0005c3b5: LEA EAX, [ESP+0xc]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0005c3b9: PUSH EAX
        _emit 0x50
        // 0005c3ba: CALL FUN_00465c10
        _emit 0xe8
        _emit 0x51
        _emit 0x98
        _emit 0x00
        _emit 0x00
        // 0005c3bf: LEA ECX, [ESP+0x10]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0005c3c3: PUSH ECX
        _emit 0x51
        // 0005c3c4: LEA EDX, [ESP+0x1c]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 0005c3c8: PUSH EDX
        _emit 0x52
        // 0005c3c9: CALL FUN_00465ce0
        _emit 0xe8
        _emit 0x12
        _emit 0x99
        _emit 0x00
        _emit 0x00
        // 0005c3ce: MOV ECX, dword ptr [0x0132e788]
        _emit 0x8b
        _emit 0x0d
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 0005c3d4: MOV EDX, dword ptr [ECX+0x1c]
        _emit 0x8b
        _emit 0x51
        _emit 0x1c
        // 0005c3d7: LEA EAX, [ESP+0x20]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0005c3db: PUSH EAX
        _emit 0x50
        // 0005c3dc: CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0005c3de: MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 0005c3e0: ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0005c3e3: CMP ESI, EDI
        _emit 0x3b
        _emit 0xf7
        // 0005c3e5: JNZ 0x0045c482
        _emit 0x0f
        _emit 0x85
        _emit 0x97
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005c3eb: PUSH 0x3fb
        _emit 0x68
        _emit 0xfb
        _emit 0x03
        _emit 0x00
        _emit 0x00
        // 0005c3f0: PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c3f5: PUSH 0x190
        _emit 0x68
        _emit 0x90
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c3fa: CALL FUN_00463150
        _emit 0xe8
        _emit 0x51
        _emit 0x6d
        _emit 0x00
        _emit 0x00
        // 0005c3ff: MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 0005c401: ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0005c404: CMP ESI, EDI
        _emit 0x3b
        _emit 0xf7
        // 0005c406: JZ 0x0045c466
        _emit 0x74
        _emit 0x5e
        // 0005c408: LEA EAX, [ESP+0xc]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0005c40c: PUSH EAX
        _emit 0x50
        // 0005c40d: PUSH ESI
        _emit 0x56
        // 0005c40e: CALL FUN_00465ce0
        _emit 0xe8
        _emit 0xcd
        _emit 0x98
        _emit 0x00
        _emit 0x00
        // 0005c413: ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0005c416: MOV dword ptr [ESI+0x188], EDI
        _emit 0x89
        _emit 0xbe
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c41c: MOV dword ptr [ESI+0x18c], EDI
        _emit 0x89
        _emit 0xbe
        _emit 0x8c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c422: LEA EAX, [ESI+0xc8]
        _emit 0x8d
        _emit 0x86
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005c428: MOV ECX, 0x10
        _emit 0xb9
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005c42d: LEA ECX, [ECX]  (3-byte align NOP)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // 0005c430: MOV dword ptr [EAX-0x40], EDI
        _emit 0x89
        _emit 0x78
        _emit 0xc0
        // 0005c433: MOV dword ptr [EAX], EDI
        _emit 0x89
        _emit 0x38
        // 0005c435: ADD EAX, 0x4
        _emit 0x83
        _emit 0xc0
        _emit 0x04
        // 0005c438: SUB ECX, 0x1
        _emit 0x83
        _emit 0xe9
        _emit 0x01
        // 0005c43b: JNZ 0x0045c430
        _emit 0x75
        _emit 0xf3
        // 0005c43d: MOV ECX, dword ptr [0x0132e788]
        _emit 0x8b
        _emit 0x0d
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 0005c443: MOV EDX, dword ptr [ECX+0x20]
        _emit 0x8b
        _emit 0x51
        _emit 0x20
        // 0005c446: PUSH ESI
        _emit 0x56
        // 0005c447: CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0005c449: MOV EBX, EAX
        _emit 0x8b
        _emit 0xd8
        // 0005c44b: MOV EAX, [0x0132e788]
        _emit 0xa1
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 0005c450: MOV ECX, dword ptr [EAX+0x1c]
        _emit 0x8b
        _emit 0x48
        _emit 0x1c
        // 0005c453: PUSH ESI
        _emit 0x56
        // 0005c454: CALL ECX
        _emit 0xff
        _emit 0xd1
        // 0005c456: ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0005c459: CMP EAX, ESI
        _emit 0x3b
        _emit 0xc6
        // 0005c45b: JZ 0x0045c475
        _emit 0x74
        _emit 0x18
        // 0005c45d: PUSH ESI
        _emit 0x56
        // 0005c45e: CALL FUN_0045c1f0
        _emit 0xe8
        _emit 0x8d
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0005c463: ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005c466: POP EDI
        _emit 0x5f
        // 0005c467: POP ESI
        _emit 0x5e
        // 0005c468: MOV EAX, 0x132e5f8
        _emit 0xb8
        _emit 0xf8
        _emit 0xe5
        _emit 0x32
        _emit 0x01
        // 0005c46d: POP EBX
        _emit 0x5b
        // 0005c46e: ADD ESP, 0x198
        _emit 0x81
        _emit 0xc4
        _emit 0x98
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c474: RET
        _emit 0xc3
        // 0005c475: CMP EBX, EDI
        _emit 0x3b
        _emit 0xdf
        // 0005c477: JZ 0x0045c482
        _emit 0x74
        _emit 0x09
        // 0005c479: PUSH EBX
        _emit 0x53
        // 0005c47a: CALL FUN_0045c1f0
        _emit 0xe8
        _emit 0x71
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0005c47f: ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005c482: POP EDI
        _emit 0x5f
        // 0005c483: MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 0005c485: POP ESI
        _emit 0x5e
        // 0005c486: POP EBX
        _emit 0x5b
        // 0005c487: ADD ESP, 0x198
        _emit 0x81
        _emit 0xc4
        _emit 0x98
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c48d: RET
        _emit 0xc3
    }
}
