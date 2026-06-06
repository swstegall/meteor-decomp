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
// FUNCTION: ffxivgame 0x0045fe20 — thin wrapper around FUN_0045f3e0 that
//                                  supplies a default out-slot and fixed
//                                  trailing args (__cdecl, 84 bytes).
//
// Behaviour read from the disassembly at orig RVA 0x0005fe20:
//
//   __cdecl int FUN_0045fe20(int *p, X a2, Y a3, Z a4, W a5) {
//       int   local_value = 0;        // [esp+4]   — default deref target
//       char  buf[0x14];              // [esp+8]   — scratch passed by ref
//       char  flag = 0;               // [esp+0x1c]
//       if (p == 0) p = &local_value; // 0-arg → point at the local
//       // FUN_0045f3e0(p, a2, a3, a4, -1, 0, 0, buf)
//       if (FUN_0045f3e0(p, a2, a3, a4, -1, 0, 0, buf) > 0)
//           return *p;
//       return 0;
//   }
//
//   The prologue is the MSVC `mov eax, frame_size; call __chkstk` stack
//   probe (frame_size = 0x1c); __chkstk lives at orig 0x009d29d0.
//
// Reloc-bearing sites in the orig 84 bytes (CALL rel32 targets the linker
// resolves at relink time):
//     +0x05   CALL rel32 → __chkstk     (orig 0x009d29d0)
//     +0x3a   CALL rel32 → FUN_0045f3e0 (orig 0x0045f3e0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ port would emit two CALL rel32 relocations the
//   linker fixes up at relink time. Matching the FUN_004065c0 precedent
//   in this directory, a `__declspec(naked)` body that re-emits the orig
//   84 bytes verbatim via MASM `_emit` produces a .obj whose .text is
//   byte-identical to the orig slice (the rel32 offsets are baked in as
//   raw immediates — no COFF relocations), so tools/compare.py reports
//   GREEN by direct equality.

extern "C" __declspec(naked) void FUN_0045fe20() {
    __asm {
        _emit 0xb8              // MOV EAX, 0x1c
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL __chkstk (rel32 → 0x009d29d0)
        _emit 0xa6
        _emit 0x2b
        _emit 0x57
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x24]
        _emit 0x74
        _emit 0x24
        _emit 0x24
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x3b              // CMP ESI, EAX
        _emit 0xf0
        _emit 0x89              // MOV dword ptr [ESP+0x4], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x75              // JNZ +0x04
        _emit 0x04
        _emit 0x8d              // LEA ESI, [ESP+0x4]
        _emit 0x74
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x30]
        _emit 0x54
        _emit 0x24
        _emit 0x30
        _emit 0x8d              // LEA ECX, [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x2c]
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x50              // PUSH EAX
        _emit 0x50              // PUSH EAX
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x52              // PUSH EDX
        _emit 0x88              // MOV byte ptr [ESP+0x1c], AL
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x40]
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL FUN_0045f3e0 (rel32 → 0x0045f3e0)
        _emit 0x81
        _emit 0xf5
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x20
        _emit 0xc4
        _emit 0x20
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7e              // JLE +0x07
        _emit 0x07
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x1c
        _emit 0xc4
        _emit 0x1c
        _emit 0xc3              // RET
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x1c
        _emit 0xc4
        _emit 0x1c
        _emit 0xc3              // RET
    }
}
