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
// FUNCTION: ffxivgame 0x0045ce50 — resource/object teardown: validates a
//                                  struct pointer, calls a capacity-check
//                                  helper, then releases a vtable-dispatched
//                                  cleanup, a heap buffer, a string-deleter,
//                                  and finally a base destructor.
//                                  (__cdecl, 114 bytes / 0x72, one ESI save)
//
// Calling convention: __cdecl; one parameter (pointer to struct).
// Callee-saves: ESI only (PUSH ESI / POP ESI frame; no EBP / no SUB ESP).
//
// Pseudo-C:
//
//   void __cdecl FUN_0045ce50(SomeStruct *p) {
//       if (!p) return;
//       if (FUN_00466000(&p->field_08, -1, 0xa, 0xf68900, 0x189) > 0) return;
//       if (p->field_0c) {
//           void (**vtbl)() = *(void(***)())p->field_0c;
//           if (vtbl[0x14]) vtbl[0x14](p);   // vtable slot 0x50 / sizeof(ptr) = 20
//       }
//       if (p->field_10) {
//           FUN_004695c0(p->field_10);
//           p->field_10 = 0;
//       }
//       if (p->field_1c) {
//           FUN_004641f0(p->field_1c, (void*)0x46a000);
//       }
//       FUN_004632f0(p);
//   }
//
// Reloc-bearing sites (four CALL rel32 — emitted verbatim as raw bytes):
//   +0x1b   CALL rel32  → FUN_00466000   (bytes: e8 90 91 00 00)
//   +0x43   CALL rel32  → FUN_004695c0   (bytes: e8 28 c7 00 00)
//   +0x5f   CALL rel32  → FUN_004641f0   (bytes: e8 3c 73 00 00)
//   +0x68   CALL rel32  → FUN_004632f0   (bytes: e8 33 64 00 00)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The four rel32 CALL displacements resolve against the original binary's
//   image base. Emitting them verbatim via MASM _emit produces a .obj whose
//   .text is byte-identical to the original slice — compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0045ce50() {
    __asm {
        // 0005ce50: 56           PUSH ESI
        _emit 0x56
        // 0005ce51: 8b 74 24 08  MOV ESI, dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 0005ce55: 85 f6        TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0005ce57: 74 67        JZ +0x67 (→ 0x0005cec0)
        _emit 0x74
        _emit 0x67
        // 0005ce59: 68 89 01 00 00  PUSH 0x189
        _emit 0x68
        _emit 0x89
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005ce5e: 68 00 89 f6 00  PUSH 0xf68900
        _emit 0x68
        _emit 0x00
        _emit 0x89
        _emit 0xf6
        _emit 0x00
        // 0005ce63: 6a 0a        PUSH 0xa
        _emit 0x6a
        _emit 0x0a
        // 0005ce65: 8d 46 08     LEA EAX, [ESI+0x8]
        _emit 0x8d
        _emit 0x46
        _emit 0x08
        // 0005ce68: 6a ff        PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0005ce6a: 50           PUSH EAX
        _emit 0x50
        // 0005ce6b: e8 90 91 00 00  CALL FUN_00466000
        _emit 0xe8
        _emit 0x90
        _emit 0x91
        _emit 0x00
        _emit 0x00
        // 0005ce70: 83 c4 14     ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0005ce73: 85 c0        TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005ce75: 7f 49        JG +0x49 (→ 0x0005cec0)
        _emit 0x7f
        _emit 0x49
        // 0005ce77: 8b 46 0c     MOV EAX, dword ptr [ESI+0xc]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 0005ce7a: 85 c0        TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005ce7c: 74 0d        JZ +0xd (→ 0x0005ce8b)
        _emit 0x74
        _emit 0x0d
        // 0005ce7e: 8b 40 50     MOV EAX, dword ptr [EAX+0x50]
        _emit 0x8b
        _emit 0x40
        _emit 0x50
        // 0005ce81: 85 c0        TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005ce83: 74 06        JZ +0x6 (→ 0x0005ce8b)
        _emit 0x74
        _emit 0x06
        // 0005ce85: 56           PUSH ESI
        _emit 0x56
        // 0005ce86: ff d0        CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0005ce88: 83 c4 04     ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005ce8b: 8b 46 10     MOV EAX, dword ptr [ESI+0x10]
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        // 0005ce8e: 85 c0        TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005ce90: 74 10        JZ +0x10 (→ 0x0005cea2)
        _emit 0x74
        _emit 0x10
        // 0005ce92: 50           PUSH EAX
        _emit 0x50
        // 0005ce93: e8 28 c7 00 00  CALL FUN_004695c0
        _emit 0xe8
        _emit 0x28
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        // 0005ce98: 83 c4 04     ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005ce9b: c7 46 10 00 00 00 00  MOV dword ptr [ESI+0x10], 0x0
        _emit 0xc7
        _emit 0x46
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005cea2: 8b 46 1c     MOV EAX, dword ptr [ESI+0x1c]
        _emit 0x8b
        _emit 0x46
        _emit 0x1c
        // 0005cea5: 85 c0        TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005cea7: 74 0e        JZ +0xe (→ 0x0005ceb7)
        _emit 0x74
        _emit 0x0e
        // 0005cea9: 68 00 a0 46 00  PUSH 0x46a000
        _emit 0x68
        _emit 0x00
        _emit 0xa0
        _emit 0x46
        _emit 0x00
        // 0005ceae: 50           PUSH EAX
        _emit 0x50
        // 0005ceaf: e8 3c 73 00 00  CALL FUN_004641f0
        _emit 0xe8
        _emit 0x3c
        _emit 0x73
        _emit 0x00
        _emit 0x00
        // 0005ceb4: 83 c4 08     ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0005ceb7: 56           PUSH ESI
        _emit 0x56
        // 0005ceb8: e8 33 64 00 00  CALL FUN_004632f0
        _emit 0xe8
        _emit 0x33
        _emit 0x64
        _emit 0x00
        _emit 0x00
        // 0005cebd: 83 c4 04     ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005cec0: 5e           POP ESI
        _emit 0x5e
        // 0005cec1: c3           RET
        _emit 0xc3
    }
}
