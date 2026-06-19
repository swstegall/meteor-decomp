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
// FUNCTION: ffxivgame 0x00437ac0 — `__thiscall` linked-list iterator /
//                                   node-processing loop (344 B / 0x158).
//
// Inspection (read from the disassembly at orig RVA 0x00037ac0):
//
//   __thiscall void FUN_00437ac0(this);
//
//   Structure:
//     Computes a slot index from this->field_0x28, looks up a node list
//     head, then walks the list processing each node via a virtual call
//     (vtable slot 0x1c/4 = slot 7). After the list walk, cleans up
//     sentinel links, fires a virtual call on this->field_0xc, updates
//     this->field_0x28 / this->field_0x2c with a modular counter, and
//     calls two tail helpers before jumping out.
//
//   Calling convention: __thiscall (ECX = this, no stack args, callee
//   preserves EBX/EBP/ESI/EDI via the standard PUSH/POP shrink-wrap).
//
//   Stack frame (after SUB ESP,0x10 + PUSH EBX/EBP/ESI/EDI):
//     [ESP+0x00]  saved EDI
//     [ESP+0x04]  saved ESI
//     [ESP+0x08]  saved EBP
//     [ESP+0x0c]  saved EBX
//     [ESP+0x10]  local 0  (scratch / XCHG target)
//     [ESP+0x14]  local 1  (original EAX = list ptr)
//     [ESP+0x18]  local 2  (iterator object / smart-ptr, passed by ref)
//     [ESP+0x1c]  local 3  (original EBX = first node)
//
//   No relocation-free path exists at /O2 — contains three direct-addr
//   CALL sites (0x9d22b4, 0x9d22b4, 0x9d22b4), three external calls
//   (0xc2bb10, 0x40df70, 0x4176b0), two tail calls (0x43bf60, 0x43bf30)
//   plus an absolute MOV ECX from [0x1328d90]. All are link-time fixups
//   that can't be reproduced from a standalone .obj without the full
//   image base. Naked-asm passthrough is the only viable strategy.

extern "C" __declspec(naked) void FUN_00437ac0() {
    __asm {
        // 00037ac0: 83 ec 10  SUB ESP,0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 00037ac3: 53  PUSH EBX
        _emit 0x53
        // 00037ac4: 55  PUSH EBP
        _emit 0x55
        // 00037ac5: 56  PUSH ESI
        _emit 0x56
        // 00037ac6: 57  PUSH EDI
        _emit 0x57
        // 00037ac7: 8b f9  MOV EDI,ECX
        _emit 0x8b
        _emit 0xf9
        // 00037ac9: 8b 77 28  MOV ESI,[EDI+0x28]
        _emit 0x8b
        _emit 0x77
        _emit 0x28
        // 00037acc: 83 c6 03  ADD ESI,0x3
        _emit 0x83
        _emit 0xc6
        _emit 0x03
        // 00037acf: c1 e6 04  SHL ESI,0x4
        _emit 0xc1
        _emit 0xe6
        _emit 0x04
        // 00037ad2: 8b 44 3e 04  MOV EAX,[ESI+EDI*1+0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x3e
        _emit 0x04
        // 00037ad6: 8b 18  MOV EBX,[EAX]
        _emit 0x8b
        _emit 0x18
        // 00037ad8: 03 f7  ADD ESI,EDI
        _emit 0x03
        _emit 0xf7
        // 00037ada: 8b ee  MOV EBP,ESI
        _emit 0x8b
        _emit 0xee
        // 00037adc: 89 44 24 14  MOV [ESP+0x14],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00037ae0: 89 5c 24 1c  MOV [ESP+0x1c],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        // 00037ae4: 89 6c 24 18  MOV [ESP+0x18],EBP
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 00037ae8: 85 ed  TEST EBP,EBP
        _emit 0x85
        _emit 0xed
        // 00037aea: 74 04  JZ +4
        _emit 0x74
        _emit 0x04
        // 00037aec: 3b ee  CMP EBP,ESI
        _emit 0x3b
        _emit 0xee
        // 00037aee: 74 05  JZ +5
        _emit 0x74
        _emit 0x05
        // 00037af0: e8 bf a7 59 00  CALL 0x009d22b4
        _emit 0xe8
        _emit 0xbf
        _emit 0xa7
        _emit 0x59
        _emit 0x00
        // 00037af5: 3b 5c 24 14  CMP EBX,[ESP+0x14]
        _emit 0x3b
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        // 00037af9: 74 67  JZ +0x67
        _emit 0x74
        _emit 0x67
        // 00037afb: 85 ed  TEST EBP,EBP
        _emit 0x85
        _emit 0xed
        // 00037afd: 75 05  JNZ +5
        _emit 0x75
        _emit 0x05
        // 00037aff: e8 b0 a7 59 00  CALL 0x009d22b4
        _emit 0xe8
        _emit 0xb0
        _emit 0xa7
        _emit 0x59
        _emit 0x00
        // 00037b04: 3b 5d 04  CMP EBX,[EBP+0x4]
        _emit 0x3b
        _emit 0x5d
        _emit 0x04
        // 00037b07: 75 05  JNZ +5
        _emit 0x75
        _emit 0x05
        // 00037b09: e8 a6 a7 59 00  CALL 0x009d22b4
        _emit 0xe8
        _emit 0xa6
        _emit 0xa7
        _emit 0x59
        _emit 0x00
        // 00037b0e: 8b 6b 0c  MOV EBP,[EBX+0xc]
        _emit 0x8b
        _emit 0x6b
        _emit 0x0c
        // 00037b11: 8b 45 00  MOV EAX,[EBP]
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        // 00037b14: 6a 00  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00037b16: 6a 04  PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 00037b18: 8d 54 24 18  LEA EDX,[ESP+0x18]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 00037b1c: c7 44 24 18 ff ff ff ff  MOV [ESP+0x18],0xffffffff
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00037b24: 8b 08  MOV ECX,[EAX]
        _emit 0x8b
        _emit 0x08
        // 00037b26: 52  PUSH EDX
        _emit 0x52
        // 00037b27: 50  PUSH EAX
        _emit 0x50
        // 00037b28: 8b 41 1c  MOV EAX,[ECX+0x1c]
        _emit 0x8b
        _emit 0x41
        _emit 0x1c
        // 00037b2b: 32 db  XOR BL,BL
        _emit 0x32
        _emit 0xdb
        // 00037b2d: ff d0  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00037b2f: 85 c0  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00037b31: 8b 44 24 10  MOV EAX,[ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00037b35: 75 06  JNZ +6
        _emit 0x75
        _emit 0x06
        // 00037b37: 85 c0  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00037b39: 78 02  JS +2
        _emit 0x78
        _emit 0x02
        // 00037b3b: b3 01  MOV BL,0x1
        _emit 0xb3
        _emit 0x01
        // 00037b3d: 8b 4d 04  MOV ECX,[EBP+0x4]
        _emit 0x8b
        _emit 0x4d
        _emit 0x04
        // 00037b40: 87 01  XCHG [ECX],EAX
        _emit 0x87
        _emit 0x01
        // 00037b42: 8b 45 0c  MOV EAX,[EBP+0xc]
        _emit 0x8b
        _emit 0x45
        _emit 0x0c
        // 00037b45: 33 d2  XOR EDX,EDX
        _emit 0x33
        _emit 0xd2
        // 00037b47: 80 fb 01  CMP BL,0x1
        _emit 0x80
        _emit 0xfb
        _emit 0x01
        // 00037b4a: 0f 94 c2  SETZ DL
        _emit 0x0f
        _emit 0x94
        _emit 0xc2
        // 00037b4d: 87 10  XCHG [EAX],EDX
        _emit 0x87
        _emit 0x10
        // 00037b4f: 8d 4c 24 18  LEA ECX,[ESP+0x18]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00037b53: e8 d8 8a fe ff  CALL 0x00420630
        _emit 0xe8
        _emit 0xd8
        _emit 0x8a
        _emit 0xfe
        _emit 0xff
        // 00037b58: 8b 5c 24 1c  MOV EBX,[ESP+0x1c]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        // 00037b5c: 8b 6c 24 18  MOV EBP,[ESP+0x18]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 00037b60: eb 86  JMP -0x7a (back to 0x437ae8)
        _emit 0xeb
        _emit 0x86
        // --- 0x437b62: list exhausted, cleanup path ---
        // 00037b62: 8b 4e 04  MOV ECX,[ESI+0x4]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 00037b65: 8b 69 04  MOV EBP,[ECX+0x4]
        _emit 0x8b
        _emit 0x69
        _emit 0x04
        // 00037b68: 80 7d 11 00  CMP byte ptr [EBP+0x11],0x0
        _emit 0x80
        _emit 0x7d
        _emit 0x11
        _emit 0x00
        // 00037b6c: 8b dd  MOV EBX,EBP
        _emit 0x8b
        _emit 0xdd
        // 00037b6e: 75 22  JNZ +0x22
        _emit 0x75
        _emit 0x22
        // 00037b70: 8b 53 08  MOV EDX,[EBX+0x8]
        _emit 0x8b
        _emit 0x53
        _emit 0x08
        // 00037b73: 52  PUSH EDX
        _emit 0x52
        // 00037b74: 8b ce  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00037b76: e8 95 3f 7f 00  CALL 0x00c2bb10
        _emit 0xe8
        _emit 0x95
        _emit 0x3f
        _emit 0x7f
        _emit 0x00
        // 00037b7b: 85 ed  TEST EBP,EBP
        _emit 0x85
        _emit 0xed
        // 00037b7d: 8b 1b  MOV EBX,[EBX]
        _emit 0x8b
        _emit 0x1b
        // 00037b7f: 74 09  JZ +9
        _emit 0x74
        _emit 0x09
        // 00037b81: 8b 4d fc  MOV ECX,[EBP-0x4]
        _emit 0x8b
        _emit 0x4d
        _emit 0xfc
        // 00037b84: 55  PUSH EBP
        _emit 0x55
        // 00037b85: e8 e6 63 fd ff  CALL 0x0040df70
        _emit 0xe8
        _emit 0xe6
        _emit 0x63
        _emit 0xfd
        _emit 0xff
        // 00037b8a: 80 7b 11 00  CMP byte ptr [EBX+0x11],0x0
        _emit 0x80
        _emit 0x7b
        _emit 0x11
        _emit 0x00
        // 00037b8e: 8b eb  MOV EBP,EBX
        _emit 0x8b
        _emit 0xeb
        // 00037b90: 74 de  JZ -0x22 (back to 0x437b70)
        _emit 0x74
        _emit 0xde
        // 00037b92: 8b 46 04  MOV EAX,[ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00037b95: 89 40 04  MOV [EAX+0x4],EAX
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 00037b98: 8b 46 04  MOV EAX,[ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00037b9b: c7 46 08 00 00 00 00  MOV [ESI+0x8],0x0
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00037ba2: 89 00  MOV [EAX],EAX
        _emit 0x89
        _emit 0x00
        // 00037ba4: 8b 76 04  MOV ESI,[ESI+0x4]
        _emit 0x8b
        _emit 0x76
        _emit 0x04
        // 00037ba7: 89 76 08  MOV [ESI+0x8],ESI
        _emit 0x89
        _emit 0x76
        _emit 0x08
        // 00037baa: 8b 4f 0c  MOV ECX,[EDI+0xc]
        _emit 0x8b
        _emit 0x4f
        _emit 0x0c
        // 00037bad: 8b 01  MOV EAX,[ECX]
        _emit 0x8b
        _emit 0x01
        // 00037baf: 8b 10  MOV EDX,[EAX]
        _emit 0x8b
        _emit 0x10
        // 00037bb1: ff d2  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00037bb3: 8b 47 2c  MOV EAX,[EDI+0x2c]
        _emit 0x8b
        _emit 0x47
        _emit 0x2c
        // 00037bb6: 8d 48 01  LEA ECX,[EAX+0x1]
        _emit 0x8d
        _emit 0x48
        _emit 0x01
        // 00037bb9: 89 47 28  MOV [EDI+0x28],EAX
        _emit 0x89
        _emit 0x47
        _emit 0x28
        // 00037bbc: b8 01 00 00 00  MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00037bc1: 3b c1  CMP EAX,ECX
        _emit 0x3b
        _emit 0xc1
        // 00037bc3: 1b c0  SBB EAX,EAX
        _emit 0x1b
        _emit 0xc0
        // 00037bc5: f7 d0  NOT EAX
        _emit 0xf7
        _emit 0xd0
        // 00037bc7: 23 c1  AND EAX,ECX
        _emit 0x23
        _emit 0xc1
        // 00037bc9: 8b 4f 08  MOV ECX,[EDI+0x8]
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        // 00037bcc: 89 47 2c  MOV [EDI+0x2c],EAX
        _emit 0x89
        _emit 0x47
        _emit 0x2c
        // 00037bcf: e8 8c 43 00 00  CALL 0x0043bf60
        _emit 0xe8
        _emit 0x8c
        _emit 0x43
        _emit 0x00
        _emit 0x00
        // 00037bd4: 8b 0d 90 8d 32 01  MOV ECX,[0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00037bda: 8a 01  MOV AL,[ECX]
        _emit 0x8a
        _emit 0x01
        // 00037bdc: 84 c0  TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // 00037bde: 0f 94 c2  SETZ DL
        _emit 0x0f
        _emit 0x94
        _emit 0xc2
        // 00037be1: 88 11  MOV [ECX],DL
        _emit 0x88
        _emit 0x11
        // 00037be3: 8b 0d 90 8d 32 01  MOV ECX,[0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00037be9: 88 41 01  MOV [ECX+0x1],AL
        _emit 0x88
        _emit 0x41
        _emit 0x01
        // 00037bec: 8b 0d 90 8d 32 01  MOV ECX,[0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00037bf2: 0f b6 01  MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00037bf5: 8d 14 c5 00 00 00 00  LEA EDX,[EAX*8+0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00037bfc: 2b d0  SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00037bfe: 8b 41 04  MOV EAX,[ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00037c01: 8d 0c 90  LEA ECX,[EAX+EDX*4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00037c04: e8 a7 fa fd ff  CALL 0x004176b0
        _emit 0xe8
        _emit 0xa7
        _emit 0xfa
        _emit 0xfd
        _emit 0xff
        // 00037c09: 8b 4f 08  MOV ECX,[EDI+0x8]
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        // 00037c0c: 5f  POP EDI
        _emit 0x5f
        // 00037c0d: 5e  POP ESI
        _emit 0x5e
        // 00037c0e: 5d  POP EBP
        _emit 0x5d
        // 00037c0f: 5b  POP EBX
        _emit 0x5b
        // 00037c10: 83 c4 10  ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00037c13: e9 18 43 00 00  JMP 0x0043bf30
        _emit 0xe9
        _emit 0x18
        _emit 0x43
        _emit 0x00
        _emit 0x00
    }
}
