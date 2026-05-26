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
// FUNCTION: ffxivgame 0x0000d880 — object slot setter with allocation (136 B / 0x88)
//
// __thiscall void FUN_0040d880(this=ECX, void* new_ptr, unsigned short count)
//
//   Sets [this+0x00] = new_ptr. If count == 0, clears [this+0x28] and returns.
//   Otherwise allocates a new buffer via FUN_0040e110 (size = count * 0x1002)
//   using a format-string object built by FUN_0040e230, then if allocation
//   succeeds calls FUN_0040dd90 to initialise [this+0x04] with the buffer.
//
//   On entry, if [this] != NULL and [this+0x28] != NULL, the old value at
//   [this+0x28] is first freed/released via FUN_0040df70.
//
// Stack layout at entry (after the callee SUB ESP,8 and push of 4 registers):
//   [ESP+0x1c] = new_ptr  (1st stack arg)
//   [ESP+0x20] = count    (2nd stack arg, only low 16 bits used)
//
// RET 8 — __thiscall, 2 stack arguments (8 bytes).
//
// Reloc-bearing sites in the orig 136 bytes:
//   +0x19  CALL rel32 → 0x0040df70 (old-buffer release)
//   +0x3d  MOV  mem32 → 0x012652f8 (global loader/context ptr)
//   +0x42  PUSH imm32 → 0x00f55864 (format string / descriptor addr)
//   +0x4f  CALL rel32 → 0x0040e230 (format-obj constructor)
//   +0x60  CALL rel32 → 0x0040e110 (allocator)
//   +0x7c  CALL rel32 → 0x0040dd90 (buffer initialiser)
//
// Reconstruction strategy: naked-asm byte passthrough. Emit the orig 136
// bytes verbatim via MASM _emit directives for a byte-identical .text section.

extern "C" __declspec(naked) void FUN_0040d880() {
    __asm {
        // 0x0000d880: SUB ESP, 0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0x0000d883: PUSH EBX
        _emit 0x53
        // 0x0000d884: PUSH EBP
        _emit 0x55
        // 0x0000d885: PUSH ESI
        _emit 0x56
        // 0x0000d886: MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 0x0000d888: MOV ECX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x0e
        // 0x0000d88a: PUSH EDI
        _emit 0x57
        // 0x0000d88b: XOR EDI, EDI
        _emit 0x33
        _emit 0xff
        // 0x0000d88d: CMP ECX, EDI
        _emit 0x3b
        _emit 0xcf
        // 0x0000d88f: JZ +0x10 (→ 0x0040d8a1)
        _emit 0x74
        _emit 0x10
        // 0x0000d891: MOV EAX, dword ptr [ESI+0x28]
        _emit 0x8b
        _emit 0x46
        _emit 0x28
        // 0x0000d894: CMP EAX, EDI
        _emit 0x3b
        _emit 0xc7
        // 0x0000d896: JZ +0x09 (→ 0x0040d8a1)
        _emit 0x74
        _emit 0x09
        // 0x0000d898: PUSH EAX
        _emit 0x50
        // 0x0000d899: CALL 0x0040df70  ← reloc at +0x19
        _emit 0xe8
        _emit 0xd2
        _emit 0x06
        _emit 0x00
        _emit 0x00
        // 0x0000d89e: MOV dword ptr [ESI+0x28], EDI  (zero out slot after release)
        _emit 0x89
        _emit 0x7e
        _emit 0x28
        // 0x0000d8a1: MOV EBX, dword ptr [ESP+0x20]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        // 0x0000d8a5: CMP BX, DI
        _emit 0x66
        _emit 0x3b
        _emit 0xdf
        // 0x0000d8a8: MOV EBP, dword ptr [ESP+0x1c]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        // 0x0000d8ac: MOV dword ptr [ESI], EBP
        _emit 0x89
        _emit 0x2e
        // 0x0000d8ae: JNZ +0x0d (→ 0x0040d8bd)
        _emit 0x75
        _emit 0x0d
        // 0x0000d8b0: MOV dword ptr [ESI+0x28], EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x28
        // 0x0000d8b3: POP EDI
        _emit 0x5f
        // 0x0000d8b4: POP ESI
        _emit 0x5e
        // 0x0000d8b5: POP EBP
        _emit 0x5d
        // 0x0000d8b6: POP EBX
        _emit 0x5b
        // 0x0000d8b7: ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0x0000d8ba: RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // 0x0000d8bd: MOV EAX, dword ptr [0x012652f8]  ← reloc at +0x3d
        _emit 0xa1
        _emit 0xf8
        _emit 0x52
        _emit 0x26
        _emit 0x01
        // 0x0000d8c2: PUSH 0x00f55864  ← reloc at +0x42
        _emit 0x68
        _emit 0x64
        _emit 0x58
        _emit 0xf5
        _emit 0x00
        // 0x0000d8c7: PUSH EAX
        _emit 0x50
        // 0x0000d8c8: LEA ECX, [ESP+0x18]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0x0000d8cc: MOVZX EDI, BX
        _emit 0x0f
        _emit 0xb7
        _emit 0xfb
        // 0x0000d8cf: CALL 0x0040e230  ← reloc at +0x4f
        _emit 0xe8
        _emit 0x5c
        _emit 0x09
        _emit 0x00
        _emit 0x00
        // 0x0000d8d4: MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 0x0000d8d6: IMUL ECX, ECX, 0x1002
        _emit 0x69
        _emit 0xc9
        _emit 0x02
        _emit 0x10
        _emit 0x00
        _emit 0x00
        // 0x0000d8dc: PUSH EAX
        _emit 0x50
        // 0x0000d8dd: PUSH ECX
        _emit 0x51
        // 0x0000d8de: MOV ECX, EBP
        _emit 0x8b
        _emit 0xcd
        // 0x0000d8e0: CALL 0x0040e110  ← reloc at +0x60
        _emit 0xe8
        _emit 0x2b
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0x0000d8e5: TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0x0000d8e7: MOV dword ptr [ESI+0x28], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x28
        // 0x0000d8ea: JZ +0x15 (→ 0x0040d901)
        _emit 0x74
        _emit 0x15
        // 0x0000d8ec: PUSH EBX
        _emit 0x53
        // 0x0000d8ed: SHL EDI, 0xc
        _emit 0xc1
        _emit 0xe7
        _emit 0x0c
        // 0x0000d8f0: PUSH 0x1000
        _emit 0x68
        _emit 0x00
        _emit 0x10
        _emit 0x00
        _emit 0x00
        // 0x0000d8f5: ADD EDI, EAX
        _emit 0x03
        _emit 0xf8
        // 0x0000d8f7: PUSH EDI
        _emit 0x57
        // 0x0000d8f8: PUSH EAX
        _emit 0x50
        // 0x0000d8f9: LEA ECX, [ESI+0x4]
        _emit 0x8d
        _emit 0x4e
        _emit 0x04
        // 0x0000d8fc: CALL 0x0040dd90  ← reloc at +0x7c
        _emit 0xe8
        _emit 0x8f
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0x0000d901: POP EDI
        _emit 0x5f
        // 0x0000d902: POP ESI
        _emit 0x5e
        // 0x0000d903: POP EBP
        _emit 0x5d
        // 0x0000d904: POP EBX
        _emit 0x5b
        // 0x0000d905: ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0x0000d908: RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
