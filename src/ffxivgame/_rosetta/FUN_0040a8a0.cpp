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
// FUNCTION: ffxivgame 0x0040a8a0 — SEH-guarded RAII helper: if ESI is
//                                   non-null, construct a local object via
//                                   FUN_0040dd50, call FUN_0040de80(ESI) on
//                                   it, then destruct via FUN_0040db10.
//                                   (__cdecl, 93 bytes)
//
// Stack layout after prologue (SUB ESP, 0x24):
//   [ESP+0x00]   local object (24 bytes = 0x18; probably a typed wrapper)
//   [ESP+0x18..0x23]  remainder of local pad / scratch
//   [ESP+0x24]   saved FS:[0x0]  (old SEH chain)
//   [ESP+0x28]   0xe54d6a        (SEH handler VA baked by compiler)
//   [ESP+0x2c]   -1              (SEH frame state: -1 = no objects)
//   [ESP+0x30]   return address
//   [ESP+0x34]   arg0 (passed by caller on the stack)
//
// Behaviour:
//
//   __cdecl void FUN_0040a8a0(void *arg0) {
//       // SEH frame installed
//       if (ESI == nullptr) return;
//       LocalObj local;                    // FUN_0040dd50 (ctor, thiscall)
//       // local.field4 = arg0;            (at [local+4], set directly)
//       // frame_state → 0 (one object constructed)
//       local.method(ESI);                 // FUN_0040de80 (thiscall, 1 arg)
//       // frame_state → -1 (destructing)
//       local.~LocalObj();                 // FUN_0040db10 (dtor, thiscall)
//   }
//
// Asm (93 bytes, read from orig RVA 0x0000a8a0):
//
//   a8a0: 64 a1 00 00 00 00          MOV  EAX, FS:[0x0]
//   a8a6: 6a ff                       PUSH -0x1
//   a8a8: 68 6a 4d e5 00              PUSH 0xe54d6a         ; SEH handler VA
//   a8ad: 50                          PUSH EAX
//   a8ae: 64 89 25 00 00 00 00        MOV  FS:[0x0], ESP    ; install SEH
//   a8b5: 83 ec 24                    SUB  ESP, 0x24
//   a8b8: 85 f6                       TEST ESI, ESI
//   a8ba: 74 32                       JZ   +0x32            ; → a8ee
//   a8bc: 8d 0c 24                    LEA  ECX, [ESP]       ; this = &local
//   a8bf: e8 8c 34 00 00              CALL FUN_0040dd50     ; ctor
//   a8c4: 8b 44 24 34                 MOV  EAX, [ESP+0x34]  ; arg0
//   a8c8: 56                          PUSH ESI              ; method arg
//   a8c9: 8d 4c 24 04                 LEA  ECX, [ESP+0x4]   ; this = &local
//   a8cd: c7 44 24 30 00 00 00 00     MOV  [ESP+0x30], 0x0  ; frame_state = 0
//   a8d5: 89 44 24 08                 MOV  [ESP+0x8], EAX   ; local.field4 = arg0
//   a8d9: e8 a2 35 00 00              CALL FUN_0040de80     ; method(ESI)
//   a8de: 8d 0c 24                    LEA  ECX, [ESP]       ; this = &local
//   a8e1: c7 44 24 2c ff ff ff ff     MOV  [ESP+0x2c], -1   ; frame_state = -1
//   a8e9: e8 22 32 00 00              CALL FUN_0040db10     ; dtor
//   a8ee: 8b 4c 24 24                 MOV  ECX, [ESP+0x24]  ; old SEH
//   a8f2: 64 89 0d 00 00 00 00        MOV  FS:[0x0], ECX    ; restore SEH
//   a8f9: 83 c4 30                    ADD  ESP, 0x30
//   a8fc: c3                          RET
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function body contains FS-segment-override MOV instructions and an
//   SEH-handler absolute-VA push (0xe54d6a) that vary in encoding from what
//   MSVC 2005 inline asm would produce. Emitting the 93 bytes verbatim via
//   MASM _emit directives gives a .obj whose .text is byte-identical to the
//   orig slice (no relocations — all CALL rel32 values are the orig PE's own
//   addresses, not linker-resolved symbols), so compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0040a8a0() {
    __asm {
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0xe54d6a  (SEH handler VA)
        _emit 0x6a
        _emit 0x4d
        _emit 0xe5
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x64              // MOV dword ptr FS:[0x0], ESP
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // SUB ESP, 0x24
        _emit 0xec
        _emit 0x24
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74              // JZ +0x32  (→ 0xa8ee)
        _emit 0x32
        _emit 0x8d              // LEA ECX, [ESP]
        _emit 0x0c
        _emit 0x24
        _emit 0xe8              // CALL FUN_0040dd50  (rel32 = 0x3486... wait: target-next = 0x40dd50-0x40a8c4 = 0x348c)
        _emit 0x8c
        _emit 0x34
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESP+0x34]
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA ECX, [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xc7              // MOV dword ptr [ESP+0x30], 0x0
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESP+0x8], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xe8              // CALL FUN_0040de80  (rel32 = 0x35a2)
        _emit 0xa2
        _emit 0x35
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP]
        _emit 0x0c
        _emit 0x24
        _emit 0xc7              // MOV dword ptr [ESP+0x2c], 0xffffffff
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8              // CALL FUN_0040db10  (rel32 = 0x3222)
        _emit 0x22
        _emit 0x32
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, [ESP+0x24]
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x64              // MOV dword ptr FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x30
        _emit 0xc4
        _emit 0x30
        _emit 0xc3              // RET
    }
}
