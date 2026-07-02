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
// FUNCTION: ffxivgame 0x00442150 — __thiscall member (242 B / 0xf2),
//                                  /GS + SEH (FS:[0] frame). Walks a
//                                  guarded tree/set container rooted at
//                                  this+0x18, releasing every node's
//                                  payload and vtable-dispatched
//                                  destructor, then resets the sentinel
//                                  node and tail-dispatches into a
//                                  container-teardown helper.
//
// Asm shape (read from asm/ffxivgame/00042150_FUN_00442150.s, cross-
// checked against the exact 242 orig bytes from orig/ffxivgame.exe
// @ rva 0x00042150 via tools/emit_passthrough_cpp.py):
//
//   __thiscall void FUN_00442150(this @ ECX);
//
//     ; ---- MSVC 2005 SEH + /GS prologue ------------------------------
//     PUSH -1                                ; SEH: try-level = -1
//     PUSH 0x00e57143                        ; SEH: __ehhandler
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
//     ; ---- ctor-ish setup ----------------------------------------------
//     MOV  [ESP+0x14], ECX                    ; save this
//     MOV  DWORD PTR [ECX], 0x00f67038         ; *this = vtable
//     MOV  EAX, [ECX+0x1c]
//     MOV  EBP, [EAX]                          ; node = *(this+0x1c)
//     LEA  ESI, [ECX+0x18]                     ; ESI = &container head
//     MOV  EDI, ESI
//     MOV  DWORD PTR [ESP+0x28], 2             ; try-level = 2
//     MOV  [ESP+0x1c], EBP
//     MOV  [ESP+0x18], EDI
//     JMP  walk                                ; short jump over 3-byte
//                                              ; LEA ECX,[ECX+0] alignment
//                                              ; filler (8d 49 00)
//
//     ; ---- tree traversal / erase loop --------------------------------
//   walk:
//     TEST EDI, EDI
//     MOV  EBX, [ESI+4]
//     JZ   chk1
//     CMP  EDI, ESI
//     JZ   have_node
//   chk1:
//     CALL 0x009d22b4                          ; checked-iterator deref
//   have_node:
//     CMP  EBP, EBX
//     JZ   done_walk
//     TEST EDI, EDI
//     JNZ  chk2
//     CALL 0x009d22b4
//   chk2:
//     CMP  EBP, [EDI+4]
//     JNZ  have_node2
//     CALL 0x009d22b4
//   have_node2:
//     MOV  ECX, [EBP+0x10]                     ; payload ptr
//     TEST ECX, ECX
//     JZ   advance
//     MOV  EAX, [ECX]
//     MOV  EDX, [EAX]
//     PUSH 1
//     CALL EDX                                 ; vtable[0](1) — dtor+free
//   advance:
//     LEA  ECX, [ESP+0x18]
//     CALL 0x009172c0                          ; iterator advance/erase
//     MOV  EBP, [ESP+0x1c]
//     MOV  EDI, [ESP+0x18]
//     JMP  walk
//
//   done_walk:
//     MOV  EAX, [ESI+4]
//     MOV  EBP, [EAX+4]                        ; EBP = leftmost node
//     CMP  BYTE PTR [EBP+0x15], 0
//     MOV  EDI, EBP
//     JNZ  reset_sentinel
//   release_leaves:
//     MOV  ECX, [EDI+8]
//     PUSH ECX
//     MOV  ECX, ESI
//     CALL 0x008a8780                          ; per-node release helper
//     MOV  EDI, [EDI]
//     PUSH EBP
//     CALL 0x009d1b17                          ; node free
//     ADD  ESP, 4
//     CMP  BYTE PTR [EDI+0x15], 0
//     MOV  EBP, EDI
//     JZ   release_leaves
//
//   reset_sentinel:
//     MOV  EAX, [ESI+4]
//     MOV  [EAX+4], EAX                        ; head->left = head
//     MOV  EAX, [ESI+4]
//     MOV  DWORD PTR [ESI+8], 0                 ; size = 0
//     MOV  [EAX], EAX                          ; head->parent = head
//     MOV  EAX, [ESI+4]
//     MOV  [EAX+8], EAX                        ; head->right = head
//     MOV  EAX, [ESI+4]
//     MOV  ECX, [EAX]
//     PUSH EAX
//     PUSH ESI
//     PUSH ECX
//     PUSH ESI
//     LEA  EDX, [ESP+0x28]
//     PUSH EDX
//     MOV  ECX, ESI
//     MOV  BYTE PTR [ESP+0x3c], 1
//     CALL 0x00927700                          ; container teardown
//                                              ; (tail dispatch — falls
//                                              ; outside this 242-byte
//                                              ; slice entirely)
//
// Reloc-bearing sites in the orig 242 bytes (tools/compare.py masks the
// reloc bytes; naked `_emit` bakes the orig-resolved bytes verbatim so
// the .obj's `.text` matches the orig slice exactly):
//
//   +0x03  DIR32 → 0x00e57143   (SEH __ehhandler thunk push imm32)
//   +0x16  DIR32 → 0x012ea8b0   (__security_cookie load)
//   +0x2b  DIR32 → 0x00f67038   (vtable pointer store imm32)
//   +0x5b  REL32 → 0x009d22b4   (_Tree checked-iterator deref)
//   +0x68  REL32 → 0x009d22b4   (   "        "          "    )
//   +0x72  REL32 → 0x009d22b4   (   "        "          "    )
//   +0x8a  REL32 → 0x009172c0   (iterator advance/erase)
//   +0xad  REL32 → 0x008a8780   (per-node release helper)
//   +0xb5  REL32 → 0x009d1b17   (node free)
//   +0xed  REL32 → 0x00927700   (container teardown, tail dispatch)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   Same call as the /GS+SEH siblings (e.g. FUN_0044aa70): this function
//   carries an MSVC 2005 SEH frame (PUSH -1 / PUSH handler / FS:[0]
//   chaining) layered over a /GS cookie, plus a `__thiscall` `this` in
//   ECX, explicit try-level stores ([ESP+0x28]=2, later [ESP+0x3c]=1),
//   a 3-byte alignment-filler LEA jumped over by the initial JMP, and a
//   guarded-tree walk with three checked-iterator deref calls. There is
//   no source-level encoding that reproduces the exact try-level stores,
//   the FS:[0] handler install, the register schedule around the walk,
//   and the alignment padding simultaneously — every high-level rewrite
//   shifts at least one byte. Emit the 242 orig bytes verbatim via MASM
//   `_emit`, yielding a byte-identical `.text` slice (the reloc-site
//   bytes already hold the linker-resolved addresses that
//   tools/compare.py compares against). The structural commentary above
//   is the readable record for a future source-level promotion.

extern "C" __declspec(naked) void FUN_00442150() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x43
        _emit 0x71
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
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xc7
        _emit 0x01
        _emit 0x38
        _emit 0x70
        _emit 0xf6
        _emit 0x00
        _emit 0x8b
        _emit 0x41
        _emit 0x1c
        _emit 0x8b
        _emit 0x28
        _emit 0x8d
        _emit 0x71
        _emit 0x18
        _emit 0x8b
        _emit 0xfe
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0xeb
        _emit 0x03
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        _emit 0x85
        _emit 0xff
        _emit 0x8b
        _emit 0x5e
        _emit 0x04
        _emit 0x74
        _emit 0x04
        _emit 0x3b
        _emit 0xfe
        _emit 0x74
        _emit 0x05
        _emit 0xe8
        _emit 0x04
        _emit 0x01
        _emit 0x59
        _emit 0x00
        _emit 0x3b
        _emit 0xeb
        _emit 0x74
        _emit 0x35
        _emit 0x85
        _emit 0xff
        _emit 0x75
        _emit 0x05
        _emit 0xe8
        _emit 0xf7
        _emit 0x00
        _emit 0x59
        _emit 0x00
        _emit 0x3b
        _emit 0x6f
        _emit 0x04
        _emit 0x75
        _emit 0x05
        _emit 0xe8
        _emit 0xed
        _emit 0x00
        _emit 0x59
        _emit 0x00
        _emit 0x8b
        _emit 0x4d
        _emit 0x10
        _emit 0x85
        _emit 0xc9
        _emit 0x74
        _emit 0x08
        _emit 0x8b
        _emit 0x01
        _emit 0x8b
        _emit 0x10
        _emit 0x6a
        _emit 0x01
        _emit 0xff
        _emit 0xd2
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0xe8
        _emit 0xe1
        _emit 0x50
        _emit 0x4d
        _emit 0x00
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0xeb
        _emit 0xb7
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x8b
        _emit 0x68
        _emit 0x04
        _emit 0x80
        _emit 0x7d
        _emit 0x15
        _emit 0x00
        _emit 0x8b
        _emit 0xfd
        _emit 0x75
        _emit 0x1e
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        _emit 0x51
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0x7e
        _emit 0x65
        _emit 0x46
        _emit 0x00
        _emit 0x8b
        _emit 0x3f
        _emit 0x55
        _emit 0xe8
        _emit 0x0d
        _emit 0xf9
        _emit 0x58
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x80
        _emit 0x7f
        _emit 0x15
        _emit 0x00
        _emit 0x8b
        _emit 0xef
        _emit 0x74
        _emit 0xe2
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x89
        _emit 0x40
        _emit 0x04
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x89
        _emit 0x40
        _emit 0x08
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
        _emit 0x54
        _emit 0x24
        _emit 0x28
        _emit 0x52
        _emit 0x8b
        _emit 0xce
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0x01
    }
}
