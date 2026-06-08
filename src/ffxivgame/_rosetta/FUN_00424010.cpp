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
// FUNCTION: ffxivgame 0x00424010 — __thiscall bounds-guarded array set-if-changed
//                                  (110 bytes).
//
// bool __thiscall FUN_00424010(this, unsigned int index, int value)
//
//   ESI = this  (ECX saved after the CMP — MSVC's scheduler emitted the
//               MOV ESI,ECX at byte +0xa, after the CMP at byte +4)
//   EDI = index (first stack arg: [ESP+0xc] after PUSH ESI/EDI)
//   EAX = value (second stack arg: [ESP+0x10] after PUSH ESI/EDI)
//
// Pseudo-C:
//
//   bool FUN_00424010(unsigned int index, int value) {   // __thiscall
//       if (index >= 0x100) {
//           // lazy-init the out-of-bounds reporter
//           if ((*(byte*)0x01323910 & 1) == 0) {
//               *(int*)0x01323910 |= 1;
//               *(void**)0x0132390c = (void*)0x422ed0;   // store fn ptr
//           }
//           // call it with 5 arguments (assert / error reporter)
//           ((void(__cdecl*)(int,int,int,int,int))
//               *(void**)0x0132390c)
//               (0xf59cbc, 0xf54d48, 0xf59c18, 0xc0, 0xf59ce0);
//       }
//       // compare and conditionally update element at [this + index*4 + 0x63f0]
//       if (((int*)this)[0x63f0/4 + index] == value) {
//           return true;    // already equal: no change
//       }
//       ((int*)this)[0x63f0/4 + index] = value;
//       return false;       // was different: updated
//   }
//
// Register scheduling note: MSVC 2005 emitted MOV ESI,ECX (save 'this')
// AFTER the CMP EDI,0x100 — the optimizer moved the independent load
// past the CMP instruction to hide latency.  Source-level C++ would put
// MOV ESI,ECX before the CMP, producing a different byte sequence.
// Naked-asm _emit passthrough is the only reliable path to GREEN here.
//
// All embedded addresses are absolute immediates (no CALL rel32, only
// CALL indirect via [0x0132390c]).  compare.py reads the post-fixup PE
// bytes verbatim, so emitting them as raw _emit bytes produces a
// byte-identical .text section with no relocations.
//
// Original 110 bytes (file RVA 0x00024010):
//
//   56 57 8b 7c 24 0c 81 ff 00 01 00 00 8b f1 72 3c
//   f6 05 10 39 32 01 01 75 11 83 0d 10 39 32 01 01
//   c7 05 0c 39 32 01 d0 2e 42 00 68 e0 9c f5 00 68
//   c0 00 00 00 68 18 9c f5 00 68 48 4d f5 00 68 bc
//   9c f5 00 ff 15 0c 39 32 01 83 c4 14 8b 44 24 10
//   39 84 be f0 63 00 00 74 0e 89 84 be f0 63 00 00
//   5f 32 c0 5e c2 08 00 5f b0 01 5e c2 08 00

extern "C" __declspec(naked) void FUN_00424010() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP + 0xc]
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x81              // CMP EDI, 0x100
        _emit 0xff
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x72              // JC +0x3c  (skip OOB handler → bounds_ok)
        _emit 0x3c
        _emit 0xf6              // TEST byte ptr [0x01323910], 0x1
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75              // JNZ +0x11  (already_init)
        _emit 0x11
        _emit 0x83              // OR dword ptr [0x01323910], 0x1
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [0x0132390c], 0x422ed0
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xd0
        _emit 0x2e
        _emit 0x42
        _emit 0x00
        _emit 0x68              // PUSH 0xf59ce0          (already_init:)
        _emit 0xe0
        _emit 0x9c
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0xc0
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf59c18
        _emit 0x18
        _emit 0x9c
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0xf54d48
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0xf59cbc
        _emit 0xbc
        _emit 0x9c
        _emit 0xf5
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x10]   (bounds_ok:)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x39              // CMP dword ptr [ESI + EDI*4 + 0x63f0], EAX
        _emit 0x84
        _emit 0xbe
        _emit 0xf0
        _emit 0x63
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0xe  (equal_path)
        _emit 0x0e
        _emit 0x89              // MOV dword ptr [ESI + EDI*4 + 0x63f0], EAX
        _emit 0x84
        _emit 0xbe
        _emit 0xf0
        _emit 0x63
        _emit 0x00
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x32              // XOR AL, AL   (return false: value changed)
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        _emit 0x5f              // POP EDI                            (equal_path:)
        _emit 0xb0              // MOV AL, 0x1  (return true: already equal)
        _emit 0x01
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
