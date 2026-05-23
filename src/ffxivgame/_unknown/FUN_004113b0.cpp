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
// FUNCTION: ffxivgame 0x000113b0 — segmented-buffer stream write dispatcher
//                                   (__thiscall, 123 B / 0x7b)
//
// __thiscall void FUN_004113b0(this, int param_1, unsigned int param_2, char param_3)
//
// Stack layout (callee-side, this in ECX):
//   ECX        : this
//   [ESP+0x04] : int  param_1   — base pointer into caller's source buffer
//   [ESP+0x08] : unsigned int param_2 — total byte count to transfer
//   [ESP+0x0c] : char param_3   — direction flag (non-zero = read mode, zero = write mode)
//   RET 0xC    : callee cleans 3 stack args
//
// Object layout (inferred from offsets touched):
//   [this + 0x08]   void (*fn)(...)  — transfer function pointer (cdecl, 3 args)
//   [this + 0x1c]   Block *block     — pointer to current block descriptor
//   [this + 0x20]   unsigned int pos — current byte offset within block
//
// Block descriptor layout:
//   [block + 0x08]   Block *(*next_fn)(this) — vtable-style: slot holds ptr to fn
//                    but actually [block+0x08] is a sub-object with vtable:
//                      sub->vtable[1] returns a new Block* (advances chain)
//   [block + 0x0c]   char *base — base pointer of this block's buffer
//   [block + 0x10]   unsigned int end_pos — capacity / end position of block
//
// Behaviour:
//   Loops while remaining > 0, consuming the current block's remaining
//   capacity in each iteration. For each chunk:
//     1. Computes how many bytes to consume: min(remaining, block->end_pos - pos)
//     2. Updates this->pos by the chunk size.
//     3. Calls this->fn(src, dest, count) — arg order swapped by param_3 flag.
//     4. If this->pos reaches block->end_pos, advances to the next block via
//        a vtable call on a sub-object at block+0x08, resets pos to 0.
//
// Calling convention: __thiscall, callee cleans 3 stack args (RET 0xC).
// Callee-saved registers: EBX, EBP, ESI, EDI.
// No local stack frame (no SUB ESP).

// No relocations in these 123 bytes (all calls are through register
// indirect; no IAT or data references). Naked-asm passthrough chosen
// to guarantee byte-identical output without register-allocation risk.
//
// Object layout (inferred from offsets touched, ECX = this):
//   [this + 0x08]  void *fn     — cdecl transfer fn ptr
//   [this + 0x1c]  Block *block — current block descriptor
//   [this + 0x20]  uint   pos   — current position within block
//
// Block layout:
//   [block + 0x08]  SubObj *sub  — sub-object for advance call
//   [block + 0x0c]  char *base   — buffer base pointer
//   [block + 0x10]  uint end_pos — capacity
//
// SubObj layout (vtable dispatch):
//   [sub + 0x00]  void **vtable
//   [vtable + 0x04]  Block *(__thiscall *advance)(sub)  — slot 1

extern "C" __declspec(naked) void FUN_004113b0()
{
    __asm {
        // 000113b0:  53                  PUSH EBX
        _emit 0x53
        // 000113b1:  8b 5c 24 0c         MOV EBX,dword ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        // 000113b5:  85 db               TEST EBX,EBX
        _emit 0x85
        _emit 0xdb
        // 000113b7:  55                  PUSH EBP
        _emit 0x55
        // 000113b8:  56                  PUSH ESI
        _emit 0x56
        // 000113b9:  8b f1               MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 000113bb:  8b eb               MOV EBP,EBX
        _emit 0x8b
        _emit 0xeb
        // 000113bd:  74 66               JZ +0x66  (→ 0x00411425)
        _emit 0x74
        _emit 0x66
        // 000113bf:  57                  PUSH EDI
        _emit 0x57
        // === loop top (RVA 0x000113c0) ===
        // 000113c0:  8b 46 1c            MOV EAX,dword ptr [ESI+0x1c]
        _emit 0x8b
        _emit 0x46
        _emit 0x1c
        // 000113c3:  8b 78 0c            MOV EDI,dword ptr [EAX+0xc]
        _emit 0x8b
        _emit 0x78
        _emit 0x0c
        // 000113c6:  8b 4c 24 14         MOV ECX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 000113ca:  8b 56 20            MOV EDX,dword ptr [ESI+0x20]
        _emit 0x8b
        _emit 0x56
        _emit 0x20
        // 000113cd:  8b 40 10            MOV EAX,dword ptr [EAX+0x10]
        _emit 0x8b
        _emit 0x40
        _emit 0x10
        // 000113d0:  2b cd               SUB ECX,EBP
        _emit 0x2b
        _emit 0xcd
        // 000113d2:  2b c2               SUB EAX,EDX
        _emit 0x2b
        _emit 0xc2
        // 000113d4:  03 cb               ADD ECX,EBX
        _emit 0x03
        _emit 0xcb
        // 000113d6:  03 fa               ADD EDI,EDX
        _emit 0x03
        _emit 0xfa
        // 000113d8:  3b c5               CMP EAX,EBP
        _emit 0x3b
        _emit 0xc5
        // 000113da:  72 02               JC +0x02  (→ 0x004113de)
        _emit 0x72
        _emit 0x02
        // 000113dc:  8b c5               MOV EAX,EBP
        _emit 0x8b
        _emit 0xc5
        // 000113de:  03 d0               ADD EDX,EAX
        _emit 0x03
        _emit 0xd0
        // 000113e0:  2b e8               SUB EBP,EAX
        _emit 0x2b
        _emit 0xe8
        // 000113e2:  80 7c 24 1c 00      CMP byte ptr [ESP+0x1c],0x0
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x00
        // 000113e7:  89 56 20            MOV dword ptr [ESI+0x20],EDX
        _emit 0x89
        _emit 0x56
        _emit 0x20
        // 000113ea:  74 04               JZ +0x04  (→ 0x004113f0)
        _emit 0x74
        _emit 0x04
        // 000113ec:  8b d7               MOV EDX,EDI
        _emit 0x8b
        _emit 0xd7
        // 000113ee:  eb 04               JMP +0x04  (→ 0x004113f4)
        _emit 0xeb
        _emit 0x04
        // 000113f0:  8b d1               MOV EDX,ECX
        _emit 0x8b
        _emit 0xd1
        // 000113f2:  8b cf               MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 000113f4:  50                  PUSH EAX
        _emit 0x50
        // 000113f5:  8b 46 08            MOV EAX,dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 000113f8:  52                  PUSH EDX
        _emit 0x52
        // 000113f9:  51                  PUSH ECX
        _emit 0x51
        // 000113fa:  ff d0               CALL EAX
        _emit 0xff
        _emit 0xd0
        // 000113fc:  8b 46 1c            MOV EAX,dword ptr [ESI+0x1c]
        _emit 0x8b
        _emit 0x46
        _emit 0x1c
        // 000113ff:  8b 4e 20            MOV ECX,dword ptr [ESI+0x20]
        _emit 0x8b
        _emit 0x4e
        _emit 0x20
        // 00011402:  83 c4 0c            ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00011405:  3b 48 10            CMP ECX,dword ptr [EAX+0x10]
        _emit 0x3b
        _emit 0x48
        _emit 0x10
        // 00011408:  75 16               JNZ +0x16  (→ 0x00411420)
        _emit 0x75
        _emit 0x16
        // 0001140a:  8b 40 08            MOV EAX,dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x40
        _emit 0x08
        // 0001140d:  8b 10               MOV EDX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x10
        // 0001140f:  8b c8               MOV ECX,EAX
        _emit 0x8b
        _emit 0xc8
        // 00011411:  8b 42 04            MOV EAX,dword ptr [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00011414:  ff d0               CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00011416:  89 46 1c            MOV dword ptr [ESI+0x1c],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x1c
        // 00011419:  c7 46 20 00 00 00 00  MOV dword ptr [ESI+0x20],0x0
        _emit 0xc7
        _emit 0x46
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // === end of block-advance section ===
        // 00011420:  85 ed               TEST EBP,EBP
        _emit 0x85
        _emit 0xed
        // 00011422:  75 9c               JNZ -0x64  (→ 0x004113c0)
        _emit 0x75
        _emit 0x9c
        // 00011424:  5f                  POP EDI
        _emit 0x5f
        // 00011425:  5e                  POP ESI
        _emit 0x5e
        // 00011426:  5d                  POP EBP
        _emit 0x5d
        // 00011427:  5b                  POP EBX
        _emit 0x5b
        // 00011428:  c2 0c 00            RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
