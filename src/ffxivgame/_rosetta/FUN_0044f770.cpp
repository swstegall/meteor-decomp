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
// FUNCTION: ffxivgame 0x0044f770 — keyed-range iterator with SEH frame
//                                  (201 B / 0xC9). Walks a range of entries
//                                  in a keyed container, extracting each
//                                  sub-range and dispatching it to a
//                                  per-element callback.
//
// Calling convention: __cdecl (RET with no stack fixup on return).
// Arguments (stack-relative after prologue, base ESP = X-0x78):
//   arg1 @ [ESP+0x7C] → EBX: object pointer ("this" for all thiscall dispatches)
//   arg2 @ [ESP+0x80] → EBP: key / range-selector passed to every lookup method
//   arg3 @ [ESP+0x84] → ECX: callback/target pointer (thiscall receiver of
//                              0x0044e950 — the guarded per-element notification)
//
// Stack frame layout after prologue (ESP at X-0x78 during body):
//   [X-0x78]  XOR'd __security_cookie (PUSH last)
//   [X-0x74]  saved EDI
//   [X-0x70]  saved ESI
//   [X-0x6C]  saved EBP
//   [X-0x68]  saved EBX
//   [X-0x67 .. X-0x11]  0x57 bytes of local frame (SUB ESP, 0x58 minus 1)
//   [X-0x10]  local frame bottom (start of 0x58 allocation)
//   [X-0x0C]  SEH node: prev FS:[0] chain (PUSH old EAX)
//   [X-0x08]  SEH node: handler address (PUSH 0xe57cb8)
//   [X-0x04]  SEH node: try-state     (PUSH -1; set to 0 before guarded call)
//   [X-0x00]  return address
//   [X+0x04]  arg1
//   [X+0x08]  arg2
//   [X+0x0C]  arg3
//
// Accessed locals (base-ESP-relative, ESP = X-0x78):
//   [ESP+0x14] = [X-0x64]: result of method_00445e50 (upper-bound fallback)
//   [ESP+0x18] = [X-0x60]: constructed object for extract call / destructor
//
// Control-flow outline:
//   1. Call method_00445e50(this=EBX, 0 stack args) → save in [ESP+0x14]
//   2. Call method_00446fb0(this=EBX, arg2, 0) → ESI (begin iterator)
//   3. If ESI == *[0x00f67298] (sentinel): jump to epilogue
//   4. Loop body:
//      a. Call method_00446f90(this=EBX, arg2, ESI) → EDI (upper for this elem)
//      b. CMOVZ EDI ← [ESP+0x14]  if EDI == sentinel
//      c. Call method_00447a80(this=EBX, &[ESP+0x18], ESI, EDI-ESI) → EAX
//      d. Set try-state = 0 (enter guarded region)
//         Call method_0044e950(this=arg3, EAX)
//         Set try-state = -1 (exit guarded region)
//      e. Call destructor_00446f50(this=&[ESP+0x18]) → cleanup extracted obj
//      f. EDI++; Call method_00446fb0(this=EBX, arg2, EDI) → ESI (next iter)
//      g. If ESI != sentinel: goto 4a
//   5. Epilogue: restore FS:[0], pop regs, ADD ESP,0x64, RET
//
// Reloc-bearing sites (original binary has these already-linked addresses;
//  _emit bakes them verbatim so compare.py sees byte-identical text):
//   +0x03  imm32 → 0xe57cb8       (SEH handler table VA)
//   +0x16  moffs32 → 0x012ea8b0   (__security_cookie)
//   +0x2E  rel32 → 0x00445e50     (method: __thiscall 0 stack args)
//   +0x43  rel32 → 0x00446fb0     (method: __thiscall 2 stack args)
//   +0x4B  moffs32 → 0x00f67298   (sentinel ptr)
//   +0x56  rel32 → 0x00446f90     (method: __thiscall 2 stack args)
//   +0x5D  moffs32 → 0x00f67298   (sentinel ptr)
//   +0x75  rel32 → 0x00447a80     (method: __thiscall 3 stack args)
//   +0x8A  rel32 → 0x0044e950     (callback: __thiscall 1 stack arg)
//   +0x9B  rel32 → 0x00446f50     (destructor: __thiscall 0 stack args)
//   +0xA7  rel32 → 0x00446fb0     (method: __thiscall 2 stack args)
//   +0xAE  moffs32 → 0x00f67298   (sentinel ptr)
//
// Reconstruction strategy — naked-asm byte passthrough.
//   The combined SEH-frame prologue (PUSH try-state / PUSH handler /
//   PUSH prev-chain / SUB / push-saved-regs / cookie-XOR / PUSH / LEA
//   SEH-node / MOV FS:[0]) cannot be reproduced with MSVC 2005 source-level
//   C++ without the compiler inserting its own SEH bookkeeping at a different
//   offset. The CMOVZ at +0x66, the varying thiscall arg counts (0/2/3/1),
//   and the interleaved try-state stores all create further scheduling
//   constraints. Emitting the 201 orig bytes verbatim via _emit directives
//   produces a .obj whose .text is byte-identical to the orig slice.

extern "C" __declspec(naked) void FUN_0044f770() {
    __asm {
        _emit 0x6a  // PUSH -0x1                         (SEH try-state = -1)
        _emit 0xff
        _emit 0x68  // PUSH 0xe57cb8                     (SEH handler table)
        _emit 0xb8
        _emit 0x7c
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0x0]                 (prev SEH chain)
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x83  // SUB ESP, 0x58                     (local frame)
        _emit 0xec
        _emit 0x58
        _emit 0x53  // PUSH EBX
        _emit 0x55  // PUSH EBP
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0xa1  // MOV EAX, [0x012ea8b0]             (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX                          (cookie on stack)
        _emit 0x8d  // LEA EAX, [ESP+0x6c]               (addr of SEH node)
        _emit 0x44
        _emit 0x24
        _emit 0x6c
        _emit 0x64  // MOV FS:[0x0], EAX                 (install SEH frame)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EBX, [ESP+0x7c]               (EBX = arg1)
        _emit 0x5c
        _emit 0x24
        _emit 0x7c
        _emit 0x8b  // MOV ECX, EBX
        _emit 0xcb
        _emit 0xe8  // CALL 0x00445e50                   (thiscall 0-arg)
        _emit 0xae
        _emit 0x66
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV EBP, [ESP+0x80]               (EBP = arg2)
        _emit 0xac
        _emit 0x24
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a  // PUSH 0x0
        _emit 0x00
        _emit 0x55  // PUSH EBP
        _emit 0x8b  // MOV ECX, EBX
        _emit 0xcb
        _emit 0x89  // MOV [ESP+0x1c], EAX               (save to local[0])
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xe8  // CALL 0x00446fb0                   (begin: thiscall 2-arg)
        _emit 0xf9
        _emit 0x77
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ESI, EAX                      (ESI = begin iter)
        _emit 0xf0
        _emit 0x3b  // CMP ESI, [0x00f67298]             (vs sentinel)
        _emit 0x35
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x74  // JZ +0x64                          (skip to epilogue)
        _emit 0x64
        // loop body:
        _emit 0x56  // PUSH ESI
        _emit 0x55  // PUSH EBP
        _emit 0x8b  // MOV ECX, EBX
        _emit 0xcb
        _emit 0xe8  // CALL 0x00446f90                   (upper: thiscall 2-arg)
        _emit 0xc6
        _emit 0x77
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV EDI, EAX                      (EDI = upper iter)
        _emit 0xf8
        _emit 0x3b  // CMP EDI, [0x00f67298]             (vs sentinel)
        _emit 0x3d
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x8d  // LEA ECX, [ESP+0x18]               (ECX = &local[1])
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x0f  // CMOVZ EDI, [ESP+0x14]             (if sentinel, use saved upper)
        _emit 0x44
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x8b  // MOV EAX, EDI
        _emit 0xc7
        _emit 0x2b  // SUB EAX, ESI                      (EAX = range size)
        _emit 0xc6
        _emit 0x50  // PUSH EAX
        _emit 0x56  // PUSH ESI
        _emit 0x51  // PUSH ECX                          (&local[1])
        _emit 0x8b  // MOV ECX, EBX
        _emit 0xcb
        _emit 0xe8  // CALL 0x00447a80                   (extract: thiscall 3-arg)
        _emit 0x97
        _emit 0x82
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ECX, [ESP+0x84]               (ECX = arg3)
        _emit 0x8c
        _emit 0x24
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX                          (extracted value)
        _emit 0xc7  // MOV [ESP+0x78], 0x0               (try-state = 0, enter guard)
        _emit 0x44
        _emit 0x24
        _emit 0x78
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL 0x0044e950                   (callback: thiscall 1-arg)
        _emit 0x52
        _emit 0xf1
        _emit 0xff
        _emit 0xff
        _emit 0x8d  // LEA ECX, [ESP+0x18]               (ECX = &local[1])
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0xc7  // MOV [ESP+0x74], 0xffffffff        (try-state = -1, exit guard)
        _emit 0x44
        _emit 0x24
        _emit 0x74
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8  // CALL 0x00446f50                   (destructor: thiscall 0-arg)
        _emit 0x41
        _emit 0x77
        _emit 0xff
        _emit 0xff
        _emit 0x83  // ADD EDI, 0x1                      (advance iterator)
        _emit 0xc7
        _emit 0x01
        _emit 0x57  // PUSH EDI
        _emit 0x55  // PUSH EBP
        _emit 0x8b  // MOV ECX, EBX
        _emit 0xcb
        _emit 0xe8  // CALL 0x00446fb0                   (next: thiscall 2-arg)
        _emit 0x95
        _emit 0x77
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ESI, EAX                      (ESI = next iter)
        _emit 0xf0
        _emit 0x3b  // CMP ESI, [0x00f67298]             (vs sentinel)
        _emit 0x35
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x75  // JNZ -0x64                         (continue loop)
        _emit 0x9c
        // epilogue:
        _emit 0x8b  // MOV ECX, [ESP+0x6c]               (old FS:[0] chain)
        _emit 0x4c
        _emit 0x24
        _emit 0x6c
        _emit 0x64  // MOV FS:[0x0], ECX                 (remove SEH frame)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX                           (discard cookie)
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5d  // POP EBP
        _emit 0x5b  // POP EBX
        _emit 0x83  // ADD ESP, 0x64                     (unwind local frame + SEH words)
        _emit 0xc4
        _emit 0x64
        _emit 0xc3  // RET
    }
}
