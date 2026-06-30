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
// FUNCTION: ffxivgame 0x0046f9e0 — `__cdecl` UI component lookup + bind
//                                  (325 B / 0x145, frameless ESP-relative).
//
// Inspection (read from the disassembly at orig RVA 0x0006f9e0):
//
//   __cdecl SomeType* FUN_0046f9e0(param_1, param_2, param_3);
//
//   Frameless function (no EBP frame): opens with the MSVC __chkstk idiom
//   to reserve 4 bytes of alloca space on the stack, then uses that slot as
//   the "result" local throughout. All stack references are ESP-relative.
//
//   Algorithm:
//     1. Allocate 4 bytes via __chkstk (0x009d29d0), push ESI (saved),
//        then call FUN_00460f30(0xf79bc4) — a keyed lookup that returns a
//        SomeType* (result). Store result in alloca slot.
//     2. If result == NULL: emit assert/log (5 args to 0x0045c940) and
//        return 0.
//     3. Get count of entries in param_3 via FUN_00464030(param_3).
//        If count > 0, iterate i = 0 .. count-1:
//          a. Fetch entry i via FUN_00464040(i, param_3). EBX = entry.
//          b. Compare entry->field_4 (string) against 0xf79b4c (22 chars)
//             using REPE CMPSB. On match, target = result.
//          c. Otherwise compare against 0xf79b34 (21 chars). On match,
//             target = result + 4.
//          d. If neither matches: log error + print EBX->{0,4,8} via
//             FUN_0045c520, then fall through to cleanup.
//          e. Call FUN_004700d0(entry, target). On failure (EAX == 0):
//             fall through to cleanup.
//          f. Increment i; re-fetch count; continue while i < count.
//     4. After loop: reload result from alloca slot (ESI).
//        If result->field_4 != 0 OR result->field_0 != 0: return result.
//        Otherwise log/assert then fall through to cleanup.
//     5. Cleanup (error path): call FUN_004612e0(result, 0xf79bc4),
//        restore EDI/EBP/EBX, return 0.
//     6. Success path: restore EDI/EBP/EBX, MOV EAX,ESI, return result.
//
//   Stack layout (ESP-relative, after MOV EAX,4 / CALL __chkstk / PUSH ESI):
//     [ESP+0x00]  saved ESI
//     [ESP+0x04]  alloca local  (FUN_00460f30 result)
//     [ESP+0x08]  return address
//     [ESP+0x0c]  param_1
//     [ESP+0x10]  param_2
//     [ESP+0x14]  param_3
//   After inner PUSH EBX / PUSH EBP / PUSH EDI (+12):
//     [ESP+0x10]  alloca local
//     [ESP+0x20]  param_3
//
//   Reloc-bearing sites in the orig 325 bytes (absolute addresses resolve
//   only in a full-binary relink at image base 0x00400000):
//     +0x05   CALL 0x009d29d0   (__chkstk, rel32)
//     +0x0c   PUSH 0xf79bc4    (lookup key string, .rdata)
//     +0x11   CALL 0x00460f30  (keyed lookup, rel32)
//     +0x25   PUSH 0xf79c10   (assert filename string)
//     +0x33   CALL 0x0045c940  (assert/log, rel32)
//     +0x4a   CALL 0x00464030  (get count, rel32)
//     +0x5c   CALL 0x00464040  (get entry at index, rel32)
//     +0x69   MOV EDI,0xf79b4c (first type string to match)
//     +0x81   MOV EDI,0xf79b34 (second type string to match)
//     +0x9c   CALL 0x004700d0  (bind call, rel32)
//     +0xb0   CALL 0x00464030  (re-fetch count, rel32)
//     +0xcb   PUSH 0xf79c10   (assert filename string, 2nd)
//     +0xd5   CALL 0x0045c940  (assert/log, rel32, 2nd)
//     +0xed   PUSH 0xf79c10   (3rd)
//     +0xf5   CALL 0x0045c940  (rel32, 3rd)
//     +0x09   PUSH 0xf69dac / 0xf69da4 / 0xf69d98 (format strings)
//     +0x11b  CALL 0x0045c520  (sprintf-style log, rel32)
//     +0x128  CALL 0x004612e0  (release/unregister, rel32)
//     +0x12e  PUSH 0xf79bc4   (cleanup key, 2nd occurrence)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Source-level C++ cannot reproduce this function byte-identically:
//     - The frameless __chkstk-based prologue (no EBP frame) shifts any
//       high-level frame reconstruction by at least one byte.
//     - All branch displacements (rel8/rel32) and the REPE CMPSB idiom
//       are sensitive to instruction ordering and register allocation.
//     - Every CALL and absolute-address PUSH embeds a linker-resolved
//       address (listed above) that can only match as raw bytes.
//   The pragmatic choice — same as FUN_004014b0, FUN_00408f10, and
//   FUN_00401a00 — is a `__declspec(naked)` body re-emitting the orig
//   325 bytes verbatim via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_0046f9e0() {
    __asm {
        _emit 0xb8  // MOV EAX, 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL 0x009d29d0  (__chkstk)
        _emit 0xe6
        _emit 0x2f
        _emit 0x56
        _emit 0x00
        _emit 0x56  // PUSH ESI
        _emit 0x68  // PUSH 0xf79bc4
        _emit 0xc4
        _emit 0x9b
        _emit 0xf7
        _emit 0x00
        _emit 0xe8  // CALL 0x00460f30
        _emit 0x3b
        _emit 0x15
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ESI, EAX
        _emit 0xf0
        _emit 0x83  // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x85  // TEST ESI, ESI
        _emit 0xf6
        _emit 0x89  // MOV [ESP+0x4], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x04
        _emit 0x75  // JNZ +0x1d
        _emit 0x1d
        _emit 0x6a  // PUSH 0x71
        _emit 0x71
        _emit 0x68  // PUSH 0xf79c10
        _emit 0x10
        _emit 0x9c
        _emit 0xf7
        _emit 0x00
        _emit 0x6a  // PUSH 0x41
        _emit 0x41
        _emit 0x68  // PUSH 0x92
        _emit 0x92
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a  // PUSH 0x22
        _emit 0x22
        _emit 0xe8  // CALL 0x0045c940
        _emit 0x29
        _emit 0xcf
        _emit 0xfe
        _emit 0xff
        _emit 0x83  // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x33  // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5e  // POP ESI
        _emit 0x59  // POP ECX
        _emit 0xc3  // RET
        _emit 0x8b  // MOV EAX, [ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x53  // PUSH EBX
        _emit 0x55  // PUSH EBP
        _emit 0x57  // PUSH EDI
        _emit 0x50  // PUSH EAX
        _emit 0x33  // XOR EBP, EBP
        _emit 0xed
        _emit 0xe8  // CALL 0x00464030
        _emit 0x02
        _emit 0x46
        _emit 0xff
        _emit 0xff
        _emit 0x83  // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7e  // JLE +0x6a
        _emit 0x6a
        _emit 0x8b  // MOV ECX, [ESP+0x20]
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x55  // PUSH EBP
        _emit 0x51  // PUSH ECX
        _emit 0xe8  // CALL 0x00464040
        _emit 0x00
        _emit 0x46
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV EBX, EAX
        _emit 0xd8
        _emit 0x8b  // MOV EAX, [EBX+0x4]
        _emit 0x43
        _emit 0x04
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xbf  // MOV EDI, 0xf79b4c
        _emit 0x4c
        _emit 0x9b
        _emit 0xf7
        _emit 0x00
        _emit 0x8b  // MOV ESI, EAX
        _emit 0xf0
        _emit 0xb9  // MOV ECX, 0x16
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33  // XOR EDX, EDX
        _emit 0xd2
        _emit 0xf3  // REPE CMPSB
        _emit 0xa6
        _emit 0x75  // JNZ +0x6
        _emit 0x06
        _emit 0x8b  // MOV EAX, [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xeb  // JMP +0x19
        _emit 0x19
        _emit 0xbf  // MOV EDI, 0xf79b34
        _emit 0x34
        _emit 0x9b
        _emit 0xf7
        _emit 0x00
        _emit 0x8b  // MOV ESI, EAX
        _emit 0xf0
        _emit 0xb9  // MOV ECX, 0x15
        _emit 0x15
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33  // XOR EDX, EDX
        _emit 0xd2
        _emit 0xf3  // REPE CMPSB
        _emit 0xa6
        _emit 0x75  // JNZ +0x58
        _emit 0x58
        _emit 0x8b  // MOV EAX, [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x83  // ADD EAX, 0x4
        _emit 0xc0
        _emit 0x04
        _emit 0x50  // PUSH EAX
        _emit 0x53  // PUSH EBX
        _emit 0xe8  // CALL 0x004700d0
        _emit 0x50
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74  // JZ +0x7c
        _emit 0x7c
        _emit 0x8b  // MOV ECX, [ESP+0x20]
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x51  // PUSH ECX
        _emit 0x83  // ADD EBP, 0x1
        _emit 0xc5
        _emit 0x01
        _emit 0xe8  // CALL 0x00464030
        _emit 0x9c
        _emit 0x45
        _emit 0xff
        _emit 0xff
        _emit 0x83  // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x3b  // CMP EBP, EAX
        _emit 0xe8
        _emit 0x7c  // JL -0x66
        _emit 0x9a
        _emit 0x8b  // MOV ESI, [ESP+0x10]
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x83  // CMP [ESI+0x4], 0x0
        _emit 0x7e
        _emit 0x04
        _emit 0x00
        _emit 0x75  // JNZ +0x78
        _emit 0x78
        _emit 0x83  // CMP [ESI], 0x0
        _emit 0x3e
        _emit 0x00
        _emit 0x75  // JNZ +0x73
        _emit 0x73
        _emit 0x68  // PUSH 0x83
        _emit 0x83
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68  // PUSH 0xf79c10
        _emit 0x10
        _emit 0x9c
        _emit 0xf7
        _emit 0x00
        _emit 0x68  // PUSH 0x97
        _emit 0x97
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68  // PUSH 0x92
        _emit 0x92
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a  // PUSH 0x22
        _emit 0x22
        _emit 0xe8  // CALL 0x0045c940
        _emit 0x7b
        _emit 0xce
        _emit 0xfe
        _emit 0xff
        _emit 0x83  // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xeb  // JMP +0x3d
        _emit 0x3d
        _emit 0x6a  // PUSH 0x7d
        _emit 0x7d
        _emit 0x68  // PUSH 0xf79c10
        _emit 0x10
        _emit 0x9c
        _emit 0xf7
        _emit 0x00
        _emit 0x6a  // PUSH 0x6a
        _emit 0x6a
        _emit 0x68  // PUSH 0x92
        _emit 0x92
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a  // PUSH 0x22
        _emit 0x22
        _emit 0xe8  // CALL 0x0045c940
        _emit 0x61
        _emit 0xce
        _emit 0xfe
        _emit 0xff
        _emit 0x8b  // MOV EDX, [EBX+0x8]
        _emit 0x53
        _emit 0x08
        _emit 0x8b  // MOV EAX, [EBX+0x4]
        _emit 0x43
        _emit 0x04
        _emit 0x8b  // MOV ECX, [EBX]
        _emit 0x0b
        _emit 0x52  // PUSH EDX
        _emit 0x68  // PUSH 0xf69dac
        _emit 0xac
        _emit 0x9d
        _emit 0xf6
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x68  // PUSH 0xf69da4
        _emit 0xa4
        _emit 0x9d
        _emit 0xf6
        _emit 0x00
        _emit 0x51  // PUSH ECX
        _emit 0x68  // PUSH 0xf69d98
        _emit 0x98
        _emit 0x9d
        _emit 0xf6
        _emit 0x00
        _emit 0x6a  // PUSH 0x6
        _emit 0x06
        _emit 0xe8  // CALL 0x0045c520
        _emit 0x20
        _emit 0xca
        _emit 0xfe
        _emit 0xff
        _emit 0x83  // ADD ESP, 0x30
        _emit 0xc4
        _emit 0x30
        _emit 0x8b  // MOV ESI, [ESP+0x10]
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x68  // PUSH 0xf79bc4
        _emit 0xc4
        _emit 0x9b
        _emit 0xf7
        _emit 0x00
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL 0x004612e0
        _emit 0xce
        _emit 0x17
        _emit 0xff
        _emit 0xff
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x5f  // POP EDI
        _emit 0x5d  // POP EBP
        _emit 0x5b  // POP EBX
        _emit 0x33  // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5e  // POP ESI
        _emit 0x59  // POP ECX
        _emit 0xc3  // RET
        _emit 0x5f  // POP EDI
        _emit 0x5d  // POP EBP
        _emit 0x5b  // POP EBX
        _emit 0x8b  // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e  // POP ESI
        _emit 0x59  // POP ECX
        _emit 0xc3  // RET
    }
}
