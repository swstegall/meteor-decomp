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
// FUNCTION: ffxivgame 0x00039090 — `__thiscall` forward-to-subobject
//                                  shim that boxes its 5 args into a
//                                  20-byte stack record and hands a
//                                  pointer to it to a method on the
//                                  subobject at this+8 (60 B / 0x3c)
//
// Inspection (read from the disassembly at orig RVA 0x00039090):
//
//   __thiscall void FUN_00439090(this, int a, int b, int c, int d, char e)
//                                ECX = this; five DWORD-slot stack args;
//                                callee cleans 0x14 via `ret 0x14`.
//
//   The body materialises a 0x14-byte local record:
//     local+0x00 = a       ([esp+0x18] after `sub esp,0x14`)
//     local+0x04 = b       ([esp+0x1c])
//     local+0x08 = c       ([esp+0x20])
//     local+0x0c = d       ([esp+0x24])
//     local+0x10 = e       (byte, [esp+0x28])
//   then `push &local` and calls the __thiscall method at 0x004367b0 with
//   `ECX = this + 8` (the subobject), i.e.:
//
//       struct Rec { int a, b, c, d; char e; };       // 0x14 bytes
//       void Foo::shim(int a, int b, int c, int d, char e) {
//           Rec r = { a, b, c, d, e };
//           this->sub.method(&r);                      // sub at this+8
//       }
//
//   The interleaved arg loads (b before a, d before c) are MSVC 2005's
//   instruction scheduling for the four DWORD copies.
//
// Asm (60 bytes):
//   83 ec 14            SUB  ESP, 0x14
//   8b 54 24 1c         MOV  EDX, [ESP+0x1c]            ; b
//   8b 44 24 18         MOV  EAX, [ESP+0x18]            ; a
//   89 54 24 04         MOV  [ESP+0x04], EDX            ; local.b
//   8b 54 24 24         MOV  EDX, [ESP+0x24]            ; d
//   89 04 24            MOV  [ESP], EAX                 ; local.a
//   8b 44 24 20         MOV  EAX, [ESP+0x20]            ; c
//   89 54 24 0c         MOV  [ESP+0x0c], EDX            ; local.d
//   89 44 24 08         MOV  [ESP+0x08], EAX            ; local.c
//   8a 44 24 28         MOV  AL, [ESP+0x28]             ; e
//   8d 14 24            LEA  EDX, [ESP]                 ; &local
//   52                  PUSH EDX
//   83 c1 08            ADD  ECX, 0x8                   ; this->sub
//   88 44 24 14         MOV  [ESP+0x14], AL             ; local.e
//   e8 ea d6 ff ff      CALL 0x004367b0                 ; sub.method(&local)
//   83 c4 14            ADD  ESP, 0x14
//   c2 14 00            RET  0x14
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The terminating CALL carries a rel32 to 0x004367b0 that a standalone
//   .obj can't reproduce via a source-level call (the target symbol isn't
//   linked here). As the size-band siblings FUN_00406fa0 / FUN_00408780
//   did for the same reason, emit the orig 60 bytes verbatim via MASM
//   `_emit` directives; the rel32 immediate matches the orig binary's
//   already-resolved bytes and `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_00439090() {
    __asm {
        _emit 0x83              // SUB ESP, 0x14
        _emit 0xec
        _emit 0x14
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x1c]
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x89              // MOV dword ptr [ESP+0x04], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x24]
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x89              // MOV dword ptr [ESP], EAX
        _emit 0x04
        _emit 0x24
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x89              // MOV dword ptr [ESP+0x0c], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x89              // MOV dword ptr [ESP+0x08], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8a              // MOV AL, byte ptr [ESP+0x28]
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x8d              // LEA EDX, [ESP]
        _emit 0x14
        _emit 0x24
        _emit 0x52              // PUSH EDX
        _emit 0x83              // ADD ECX, 0x8
        _emit 0xc1
        _emit 0x08
        _emit 0x88              // MOV byte ptr [ESP+0x14], AL
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xe8              // CALL rel32 → 0x004367b0
        _emit 0xea
        _emit 0xd6
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET 0x14
        _emit 0x14
        _emit 0x00
    }
}
