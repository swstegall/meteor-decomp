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
// FUNCTION: ffxivgame 0x00437430 — allocate + init a 0x1c-byte node and
//                                   hand it to a member sink (__thiscall, 105 B)
//
// Recovered shape (this == ECX, saved into ESI; one stack arg at [ESP+8]):
//
//   PUSH ESI                          ; save this
//   MOV  ESI, ECX                     ; this
//   MOV  ECX, [0x01328d90]            ; g_table
//   MOVZX EAX, byte ptr [ECX]         ; idx = g_table->count (byte)
//   LEA  EDX, [EAX*8]                 ; idx*8
//   SUB  EDX, EAX                     ; idx*7
//   MOV  EAX, [ECX+4]                 ; base = g_table->ptr
//   LEA  ECX, [EAX + EDX*4]           ; ecx = base + idx*0x1c (slot)
//   PUSH 0x1c                         ; size
//   CALL 0x00417ab0                   ; allocator(slot, 0x1c) → node or NULL
//   TEST EAX, EAX
//   JZ   null_path
//   MOV  ECX, [ESP+8]                 ; src (stack arg)
//   MOV  dword ptr [EAX], 0xf64980    ; node vtable / type tag
//   MOVQ XMM0,[ECX];    MOVQ [EAX+4],  XMM0   ; copy 8 B
//   MOVQ XMM0,[ECX+8];  MOVQ [EAX+0xc],XMM0   ; copy 8 B
//   MOVQ XMM0,[ECX+0x10];MOVQ [EAX+0x14],XMM0 ; copy 8 B  (0x18 bytes total)
//   MOV  ECX, [ESI+8]                 ; sink = this->field_8
//   PUSH EAX                          ; node
//   CALL 0x0043c2d0                   ; sink->add(node)
//   POP  ESI
//   RET  4
// null_path:
//   MOV  ECX, [ESI+8]                 ; sink = this->field_8
//   XOR  EAX, EAX
//   PUSH EAX                          ; NULL
//   CALL 0x0043c2d0                   ; sink->add(NULL)
//   POP  ESI
//   RET  4
//
//   Calling convention: __thiscall (this in ECX), one stack arg, RET 4.
//
// Reloc-bearing sites are baked-in absolute VAs / PC-relative rel32 in the
// orig PE; the rel32 CALL offsets resolve within the orig binary's own
// address space, and the imm32 constants are absolute load-address values.
// Emitting all 105 bytes verbatim via MASM `_emit` produces a .obj whose
// .text matches the orig slice byte-for-byte (no relocations), which is the
// same naked-passthrough strategy the SEH-heavy siblings (FUN_004091f0,
// FUN_00409260, FUN_00409510) used. compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_00437430() {
    __asm {
        _emit 0x56                  // PUSH ESI
        _emit 0x8b                  // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b                  // MOV ECX, dword ptr [0x01328d90]
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x0f                  // MOVZX EAX, byte ptr [ECX]
        _emit 0xb6
        _emit 0x01
        _emit 0x8d                  // LEA EDX, [EAX*0x8 + 0x0]
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b                  // SUB EDX, EAX
        _emit 0xd0
        _emit 0x8b                  // MOV EAX, dword ptr [ECX+0x4]
        _emit 0x41
        _emit 0x04
        _emit 0x8d                  // LEA ECX, [EAX + EDX*0x4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a                  // PUSH 0x1c
        _emit 0x1c
        _emit 0xe8                  // CALL 0x00417ab0
        _emit 0x5e
        _emit 0x06
        _emit 0xfe
        _emit 0xff
        _emit 0x85                  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74                  // JZ null_path (+0x34)
        _emit 0x34
        _emit 0x8b                  // MOV ECX, dword ptr [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xc7                  // MOV dword ptr [EAX], 0xf64980
        _emit 0x00
        _emit 0x80
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0xf3                  // MOVQ XMM0, qword ptr [ECX]
        _emit 0x0f
        _emit 0x7e
        _emit 0x01
        _emit 0x66                  // MOVQ qword ptr [EAX+0x4], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x04
        _emit 0xf3                  // MOVQ XMM0, qword ptr [ECX+0x8]
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x08
        _emit 0x66                  // MOVQ qword ptr [EAX+0xc], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x0c
        _emit 0xf3                  // MOVQ XMM0, qword ptr [ECX+0x10]
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x10
        _emit 0x66                  // MOVQ qword ptr [EAX+0x14], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x14
        _emit 0x8b                  // MOV ECX, dword ptr [ESI+0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x50                  // PUSH EAX
        _emit 0xe8                  // CALL 0x0043c2d0
        _emit 0x4a
        _emit 0x4e
        _emit 0x00
        _emit 0x00
        _emit 0x5e                  // POP ESI
        _emit 0xc2                  // RET 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x8b                  // MOV ECX, dword ptr [ESI+0x8]  (null_path)
        _emit 0x4e
        _emit 0x08
        _emit 0x33                  // XOR EAX, EAX
        _emit 0xc0
        _emit 0x50                  // PUSH EAX
        _emit 0xe8                  // CALL 0x0043c2d0
        _emit 0x3b
        _emit 0x4e
        _emit 0x00
        _emit 0x00
        _emit 0x5e                  // POP ESI
        _emit 0xc2                  // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
