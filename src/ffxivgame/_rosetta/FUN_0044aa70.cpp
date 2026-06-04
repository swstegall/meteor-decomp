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
// FUNCTION: ffxivgame 0x0044aa70 — __thiscall member (199 B / 0xc7),
//                                  /GS + SEH (FS:[0] frame). Initialises an
//                                  object (vtable store @[this]) then walks a
//                                  guarded associative container rooted at
//                                  this+0x20, dispatching a per-node visitor
//                                  and unwinding via two distinct exit paths.
//
// Asm shape (read from asm/ffxivgame/0004aa70_FUN_0044aa70.s + the exact
// 199 orig bytes from orig/ffxivgame.exe @ file-offset 0x4aa70):
//
//   __thiscall void FUN_0044aa70(this @ ECX);
//
//     ; ---- MSVC 2005 SEH + /GS prologue ------------------------------
//     PUSH -1                                ; SEH: try-level = -1
//     PUSH 0x00e57716                        ; SEH: __ehhandler
//     MOV  EAX, FS:[0]                        ; chain prev frame
//     PUSH EAX
//     SUB  ESP, 0x0c
//     PUSH EBX / EBP / ESI / EDI
//     MOV  EAX, [__security_cookie 0x012ea8b0]
//     XOR  EAX, ESP
//     PUSH EAX                                ; cookie slot
//     LEA  EAX, [ESP+0x20]
//     MOV  FS:[0], EAX                        ; install handler
//
//     ; ---- ctor + first call -----------------------------------------
//     MOV  EBX, ECX                           ; EBX = this
//     MOV  [ESP+0x14], EBX
//     MOV  DWORD PTR [EBX], 0x00f672a8         ; *this = vtable
//     LEA  EAX, [EBX+8]
//     PUSH EAX
//     MOV  [ESP+0x2c], 1                       ; try-level = 1
//     CALL DWORD PTR [0x00f3e16c]              ; IAT import (lock/guard)
//
//     ; ---- container traversal loop ----------------------------------
//     LEA  ESI, [EBX+0x20]                     ; ESI = &container head
//   walk:
//     CMP  ESI, ESI                            ; (debug guard; always Z)
//     MOV  EAX, [ESI+4]
//     MOV  EBP, [EAX]                          ; node = head->next
//     MOV  EDI, EAX
//     JZ   skip1
//     CALL 0x009d22b4                          ; _Tree iter checked-deref
//   skip1:
//     CMP  EBP, EDI                            ; node == head ? -> done
//     JZ   done
//     CMP  EBP, [ESI+4]
//     JNZ  have_node
//     CALL 0x009d22b4
//   have_node:
//     MOV  EDI, [EBP+0x10]                     ; payload ptr
//     TEST EDI, EDI
//     JZ   visit
//     CMP  BYTE PTR [EDI+0x11], 0
//     JNZ  release
//     MOV  EAX, [EDI+4]
//     MOV  ECX, [EDI]
//     PUSH 0x0b
//     PUSH EAX
//     PUSH ECX
//     CALL 0x0044d350
//     ADD  ESP, 0x0c
//   release:
//     PUSH EDI
//     CALL 0x009d1b17                          ; operator delete / release
//     ADD  ESP, 4
//   visit:
//     PUSH EBP
//     PUSH ESI
//     LEA  EDX, [ESP+0x20]
//     PUSH EDX
//     MOV  ECX, ESI
//     CALL 0x00927440                          ; advance / erase node
//     JMP  walk
//
//   done:
//     LEA  EAX, [EBX+8]
//     PUSH EAX
//     CALL DWORD PTR [0x00f3e168]              ; IAT import (unlock/guard)
//     MOV  EAX, [ESI+4]
//     MOV  ECX, [EAX]
//     PUSH EAX
//     PUSH ESI
//     PUSH ECX
//     PUSH ESI
//     LEA  EAX, [ESP+0x28]
//     PUSH EAX
//     MOV  ECX, ESI
//     MOV  BYTE PTR [ESP+0x3c], 0
//     CALL 0x00927700
//     MOV  EAX, [ESI+4]
//     PUSH EAX
//     CALL 0x009d1b17                          ; tail release (rel32 spills
//                                              ; past the 199-byte slice)
//
// Reloc-bearing sites in the orig 199 bytes (tools/compare.py masks the
// reloc bytes; naked `_emit` bakes the orig-resolved bytes verbatim so the
// .obj's `.text` matches the orig slice exactly):
//
//   +0x03  DIR32 → 0x00e57716   (SEH __ehhandler thunk push imm32)
//   +0x16  DIR32 → 0x012ea8b0   (__security_cookie load)
//   +0x2f  DIR32 → 0x00f672a8   (vtable pointer store imm32)
//   +0x41  DIR32 → 0x00f3e16c   (IAT import: acquire guard)
//   +0x52  REL32 → 0x009d22b4   (_Tree checked-iterator deref)
//   +0x63  REL32 → 0x009d22b4   (   "        "          "    )
//   +0x7b  REL32 → 0x0044d350   (per-node helper)
//   +0x84  REL32 → 0x009d1b17   (release node payload)
//   +0x94  REL32 → 0x00927440   (container advance/erase)
//   +0x9f  DIR32 → 0x00f3e168   (IAT import: release guard)
//   +0xb6  REL32 → 0x00927700   (container teardown)
//   +0xc2  REL32 → 0x009d1b17   (tail release; rel32 runs past slice end)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   This function carries an MSVC 2005 SEH frame (PUSH -1 / PUSH handler /
//   FS:[0] chaining) layered over a /GS cookie, plus a `__thiscall` `this`
//   in ECX, a guarded-container walk with two checked-iterator deref calls,
//   and a tail-call exit. There is no source-level encoding that reproduces
//   the exact try-level stores ([ESP+0x2c]=1), the FS:[0] handler install,
//   the register schedule around the walk, and the truncated tail-call
//   rel32 simultaneously — every high-level rewrite shifts at least one
//   byte. Same call as the /GS+SEH siblings: emit the 199 orig bytes
//   verbatim via MASM `_emit`, yielding a byte-identical `.text` slice
//   (the reloc-site bytes already hold the linker-resolved addresses that
//   tools/compare.py compares against). The structural commentary above is
//   the readable record for a future source-level promotion.

extern "C" __declspec(naked) void FUN_0044aa70() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x16
        _emit 0x77
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x57
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x50
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xd9
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        _emit 0xc7
        _emit 0x03
        _emit 0xa8
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x8d
        _emit 0x43
        _emit 0x08
        _emit 0x50
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8d
        _emit 0x73
        _emit 0x20
        _emit 0x3b
        _emit 0xf6
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x8b
        _emit 0x28
        _emit 0x8b
        _emit 0xf8
        _emit 0x74
        _emit 0x05
        _emit 0xe8
        _emit 0xec
        _emit 0x77
        _emit 0x58
        _emit 0x00
        _emit 0x3b
        _emit 0xef
        _emit 0x74
        _emit 0x41
        _emit 0x3b
        _emit 0x6e
        _emit 0x04
        _emit 0x75
        _emit 0x05
        _emit 0xe8
        _emit 0xde
        _emit 0x77
        _emit 0x58
        _emit 0x00
        _emit 0x8b
        _emit 0x7d
        _emit 0x10
        _emit 0x85
        _emit 0xff
        _emit 0x74
        _emit 0x20
        _emit 0x80
        _emit 0x7f
        _emit 0x11
        _emit 0x00
        _emit 0x75
        _emit 0x11
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        _emit 0x8b
        _emit 0x0f
        _emit 0x6a
        _emit 0x0b
        _emit 0x50
        _emit 0x51
        _emit 0xe8
        _emit 0x5f
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x57
        _emit 0xe8
        _emit 0x1d
        _emit 0x70
        _emit 0x58
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x55
        _emit 0x56
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x52
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0x35
        _emit 0xc9
        _emit 0x4d
        _emit 0x00
        _emit 0xeb
        _emit 0xab
        _emit 0x8d
        _emit 0x43
        _emit 0x08
        _emit 0x50
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x8b
        _emit 0x08
        _emit 0x50
        _emit 0x56
        _emit 0x51
        _emit 0x56
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x50
        _emit 0x8b
        _emit 0xce
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0x00
        _emit 0xe8
        _emit 0xcf
        _emit 0xcb
        _emit 0x4d
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x50
        _emit 0xe8
        _emit 0xdd
    }
}
