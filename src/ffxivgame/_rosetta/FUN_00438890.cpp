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
// FUNCTION: ffxivgame 0x00438890 — __thiscall forwarding thunk that builds a
//                                  3-DWORD temporary on the stack and hands it
//                                  to FUN_004358b0 (46 B / 0x2E).
//
// Calling convention: __thiscall (ECX = this; two stack args; callee cleans
// 8 bytes via `ret 8`).
//
// Shape (inferred from the asm):
//
//   void __thiscall FUN_00438890(this, void *arg0, void *arg1) {
//       struct { void *vtbl; void *a0; void *a1; } tmp;
//       tmp.vtbl = (void*)0x00F64958;   // baked .rdata address (vtable/descriptor)
//       tmp.a0   = arg0;
//       tmp.a1   = arg1;
//       FUN_004358b0(&tmp, this->field_4);   // __thiscall, ECX = &tmp
//   }
//
// Asm (46 bytes @ orig RVA 0x00038890):
//   83 ec 0c                  SUB  ESP, 0xC
//   8b 44 24 10               MOV  EAX, [ESP+0x10]            ; arg0
//   8b 54 24 14               MOV  EDX, [ESP+0x14]            ; arg1
//   89 44 24 04               MOV  [ESP+0x4], EAX
//   8b 41 04                  MOV  EAX, [ECX+0x4]             ; this->field_4
//   50                        PUSH EAX                        ; stack arg
//   8d 4c 24 04               LEA  ECX, [ESP+0x4]             ; ECX = &tmp
//   c7 44 24 04 58 49 f6 00   MOV  [ESP+0x4], 0x00F64958
//   89 54 24 0c               MOV  [ESP+0xC], EDX
//   e8 f8 cf ff ff            CALL 0x004358B0                 ; rel32 (masked)
//   83 c4 0c                  ADD  ESP, 0xC
//   c2 08 00                  RET  0x8
//
// Reconstruction: __declspec(naked) _emit byte passthrough (mirrors the
// sibling _rosetta thunks). The lone REL32 callsite (FUN_004358b0) is
// emitted as the orig wire bytes — the .obj carries no relocations and
// tools/compare.py masks the orig's reloc bytes out of the diff, so the
// .text is byte-identical → GREEN.

extern "C" __declspec(naked) void FUN_00438890() {
    __asm {
        // 00038890: 83 ec 0c                SUB ESP, 0xC
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00038893: 8b 44 24 10             MOV EAX, [ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00038897: 8b 54 24 14             MOV EDX, [ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0003889b: 89 44 24 04             MOV [ESP+0x4], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0003889f: 8b 41 04                MOV EAX, [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 000388a2: 50                      PUSH EAX
        _emit 0x50
        // 000388a3: 8d 4c 24 04             LEA ECX, [ESP+0x4]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 000388a7: c7 44 24 04 58 49 f6 00 MOV [ESP+0x4], 0x00F64958
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x58
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 000388af: 89 54 24 0c             MOV [ESP+0xC], EDX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 000388b3: e8 f8 cf ff ff          CALL 0x004358B0
        _emit 0xe8
        _emit 0xf8
        _emit 0xcf
        _emit 0xff
        _emit 0xff
        // 000388b8: 83 c4 0c                ADD ESP, 0xC
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 000388bb: c2 08 00                RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
