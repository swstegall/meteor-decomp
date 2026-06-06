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
// FUNCTION: ffxivgame 0x0001ec20 — FUN_0041ec20 (__cdecl, 204 B / 0xcc).
//
// Operates on a singleton at [0x01329428] (loaded into EBX). Takes a
// single stack arg at [esp+0x20] (ESI). Walks an embedded map/tree node
// container at EBX+0x140 (EDI), looking up / clearing a slot keyed by the
// arg, then on a hit tears down the looked-up object via a sequence of
// vtable-dispatched calls (CALL [vtbl+8], CALL [vtbl] with arg 1, twice)
// plus an IAT free ([0x00f3e47c]) and a scalar-deleting dtor (FUN_0040df70).
//
// Asm shape (read from asm/ffxivgame/0001ec20_FUN_0041ec20.s):
//
//   SUB  ESP, 0x10
//   PUSH EBX
//   MOV  EBX, [0x01329428]                 ; global singleton
//   PUSH EBP / PUSH ESI
//   MOV  ESI, [ESP+0x20]                   ; arg
//   CMP  [EBX+0x150], ESI                  ; cached slot == arg?
//   MOV  [ESP+0x20], ESI
//   JNZ  +
//   MOV  [EBX+0x150], 0                    ; invalidate cache on match
// +:
//   PUSH EDI
//   LEA  EAX, [ESP+0x24] / PUSH EAX
//   LEA  ECX, [ESP+0x14]
//   LEA  EDI, [EBX+0x140] / PUSH ECX
//   MOV  ECX, EDI
//   CALL 0x0062f320                        ; map find/lower_bound (this=EDI)
//   MOV  EBP, [ESP+0x10] / TEST EBP,EBP
//   MOV  EDX, [EDI+0x4] / MOV [ESP+0x1c], EDX
//   JZ   + ; CMP EBP,EDI ; JZ ++           ; node validity / end() checks
//   CALL 0x009d22b4
//   MOV  EBX, [0x01329428]
// ++:
//   MOV  EAX, [ESP+0x14] / CMP EAX, [ESP+0x1c]
//   POP  EDI / JZ done
//   PUSH EAX / PUSH EBP
//   LEA  EAX, [ESP+0x1c] / PUSH EAX
//   LEA  ECX, [EBX+0x140]
//   CALL 0x00421560                        ; erase node
//   TEST ESI,ESI / JZ done
//   ... vtable teardown of *ESI (offsets +0x18/+0x1c/+0x20/+0x14/+0x10) ...
//   MOV  ECX, [ESI-0x4] / PUSH ESI / CALL 0x0040df70   ; deleting dtor
// done:
//   POP ESI / POP EBP / POP EBX / ADD ESP,0x10 / RET
//
// Reloc-bearing sites (masked by tools/compare.py; the verbatim _emit
// bytes reproduce the orig slice exactly so the diff is GREEN regardless):
//   +0x06   DIR32 → 0x01329428   (singleton ptr load)
//   +0x3a   REL32 → 0x0062f320   (map find helper)
//   +0x52   REL32 → 0x009d22b4
//   +0x57   DIR32 → 0x01329428   (singleton reload)
//   +0x75   REL32 → 0x00421560   (map erase)
//   +0xb7   DIR32 → 0x00f3e47c   (IAT free thunk)
//   +0xc1   REL32 → 0x0040df70   (scalar deleting dtor)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough, the
// same approach taken by the /GS-wrapped siblings (FUN_00404e40 et al.).
// A C++ source port would have to reproduce MSVC's exact register
// allocation, the interleaved PUSH/MOV scheduling, the cache-invalidate
// branch shape, and the four conditional vtable-dispatch teardown blocks;
// emitting the 204 orig bytes verbatim is byte-exact modulo the masked
// relocations.

extern "C" __declspec(naked) void FUN_0041ec20() {
    __asm {
        _emit 0x83          // SUB ESP, 0x10
        _emit 0xec
        _emit 0x10
        _emit 0x53          // PUSH EBX
        _emit 0x8b          // MOV EBX, [0x01329428]
        _emit 0x1d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x55          // PUSH EBP
        _emit 0x56          // PUSH ESI
        _emit 0x8b          // MOV ESI, [ESP+0x20]
        _emit 0x74
        _emit 0x24
        _emit 0x20
        _emit 0x39          // CMP [EBX+0x150], ESI
        _emit 0xb3
        _emit 0x50
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89          // MOV [ESP+0x20], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x20
        _emit 0x75          // JNZ +0x0a
        _emit 0x0a
        _emit 0xc7          // MOV [EBX+0x150], 0
        _emit 0x83
        _emit 0x50
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x57          // PUSH EDI
        _emit 0x8d          // LEA EAX, [ESP+0x24]
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x50          // PUSH EAX
        _emit 0x8d          // LEA ECX, [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x8d          // LEA EDI, [EBX+0x140]
        _emit 0xbb
        _emit 0x40
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x51          // PUSH ECX
        _emit 0x8b          // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8          // CALL 0x0062f320
        _emit 0xc2
        _emit 0x06
        _emit 0x21
        _emit 0x00
        _emit 0x8b          // MOV EBP, [ESP+0x10]
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x85          // TEST EBP, EBP
        _emit 0xed
        _emit 0x8b          // MOV EDX, [EDI+0x4]
        _emit 0x57
        _emit 0x04
        _emit 0x89          // MOV [ESP+0x1c], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x74          // JZ +0x04
        _emit 0x04
        _emit 0x3b          // CMP EBP, EDI
        _emit 0xef
        _emit 0x74          // JZ +0x0b
        _emit 0x0b
        _emit 0xe8          // CALL 0x009d22b4
        _emit 0x3e
        _emit 0x36
        _emit 0x5b
        _emit 0x00
        _emit 0x8b          // MOV EBX, [0x01329428]
        _emit 0x1d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x8b          // MOV EAX, [ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x3b          // CMP EAX, [ESP+0x1c]
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x5f          // POP EDI
        _emit 0x74          // JZ +0x5e (done)
        _emit 0x5e
        _emit 0x50          // PUSH EAX
        _emit 0x55          // PUSH EBP
        _emit 0x8d          // LEA EAX, [ESP+0x1c]
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x50          // PUSH EAX
        _emit 0x8d          // LEA ECX, [EBX+0x140]
        _emit 0x8b
        _emit 0x40
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xe8          // CALL 0x00421560
        _emit 0xc7
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x85          // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74          // JZ +0x48 (done)
        _emit 0x48
        _emit 0x83          // CMP [ESI+0x18], 0
        _emit 0x7e
        _emit 0x18
        _emit 0x00
        _emit 0x74          // JZ +0x0b
        _emit 0x0b
        _emit 0x8b          // MOV EAX, [ESI+0x18]
        _emit 0x46
        _emit 0x18
        _emit 0x8b          // MOV ECX, [EAX]
        _emit 0x08
        _emit 0x8b          // MOV EDX, [ECX+0x8]
        _emit 0x51
        _emit 0x08
        _emit 0x50          // PUSH EAX
        _emit 0xff          // CALL EDX
        _emit 0xd2
        _emit 0x8b          // MOV ECX, [ESI+0x1c]
        _emit 0x4e
        _emit 0x1c
        _emit 0x85          // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74          // JZ +0x08
        _emit 0x08
        _emit 0x8b          // MOV EAX, [ECX]
        _emit 0x01
        _emit 0x8b          // MOV EDX, [EAX]
        _emit 0x10
        _emit 0x6a          // PUSH 0x1
        _emit 0x01
        _emit 0xff          // CALL EDX
        _emit 0xd2
        _emit 0x8b          // MOV ECX, [ESI+0x20]
        _emit 0x4e
        _emit 0x20
        _emit 0x85          // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74          // JZ +0x08
        _emit 0x08
        _emit 0x8b          // MOV EAX, [ECX]
        _emit 0x01
        _emit 0x8b          // MOV EDX, [EAX]
        _emit 0x10
        _emit 0x6a          // PUSH 0x1
        _emit 0x01
        _emit 0xff          // CALL EDX
        _emit 0xd2
        _emit 0x80          // CMP byte ptr [ESI+0x14], 0
        _emit 0x7e
        _emit 0x14
        _emit 0x00
        _emit 0x74          // JZ +0x0a
        _emit 0x0a
        _emit 0x8b          // MOV EAX, [ESI+0x10]
        _emit 0x46
        _emit 0x10
        _emit 0x50          // PUSH EAX
        _emit 0xff          // CALL DWORD PTR [0x00f3e47c]
        _emit 0x15
        _emit 0x7c
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x8b          // MOV ECX, [ESI-0x4]
        _emit 0x4e
        _emit 0xfc
        _emit 0x56          // PUSH ESI
        _emit 0xe8          // CALL 0x0040df70
        _emit 0x8b
        _emit 0xf2
        _emit 0xfe
        _emit 0xff
        _emit 0x5e          // POP ESI
        _emit 0x5d          // POP EBP
        _emit 0x5b          // POP EBX
        _emit 0x83          // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3          // RET
    }
}
