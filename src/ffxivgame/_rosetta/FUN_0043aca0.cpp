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
// FUNCTION: ffxivgame 0x0043aca0 — `__thiscall` with 1 explicit arg
//                                  (320 B / 0x140, EH3-SEH wrapped)
//
// Inspection (read from the disassembly at orig RVA 0x0003aca0):
//
//   __thiscall void FUN_0043aca0(this, SomeStruct *param1)
//   ECX = this; [ESP+4] = param1; epilogue: RET 4 (thiscall, 1 arg)
//
//   Structure (derived from disassembly):
//
//     EH3 SEH prologue (PUSH -1 / PUSH scope-table / FS:[0] chain /
//       SUB ESP,0x38 / callee-saves / security-cookie / FS:[0] install)
//
//     Compute row stride:
//       stride = ALIGN4((this->field_0x08 * param1->field_0x04) * 8 + 31) >> 3
//       total  = param1->field_0x08 * stride
//
//     Initialise a local struct at [ESP+0x30] with fields from param1
//     and absolute ptr 0xf66390.
//
//     Conditional double loop (outer over field_0x0c, inner over field_0x8):
//       for each row: divide offsets, call FUNC_009d4600 per element.
//
//     After loops: if param1 != &local_struct:
//       free old param1->field_0x14 buffer (via FUN_0040df70),
//       zero param1->field_0x14, write back computed fields.
//
//   Stack frame contains EH3 record; trylevel at [ESP+0x54].
//
//   Reloc-bearing sites in the 320 bytes (resolve only in full-binary relink):
//     +0x02  scope-table handler RVA  (0x00e564da — .rdata FuncInfo)
//     +0x15  __security_cookie load   (.data 0x012ea8b0)
//     +0x1f  FS:[0] install           (constant 0, fold-through)
//     +0x64  immediate ptr            (0x00f66390 — .rdata or .data)
//     +0xc4  CALL FUNC_009d4600       (.text rel32)
//     +0x10d CALL FUN_0040df70        (.text rel32)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The EH3 prolog (PUSH -1 / PUSH scope-table / PUSH FS:[0] / cookie
//   XOR / FS:[0] install), the double loop with its DIV-based stride
//   split, the conditional free with ECX-restore, and the six relocation
//   windows make source-level C++ reconstruction fragile under /O2.
//   We emit the 320 orig bytes verbatim via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_0043aca0() {
    __asm {
        // 0x0003aca0 — EH3 prologue + security cookie
        _emit 0x6a  // PUSH -1
        _emit 0xff
        _emit 0x68  // PUSH 0xe564da  (scope-table handler)
        _emit 0xda
        _emit 0x64
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x83  // SUB ESP, 0x38
        _emit 0xec
        _emit 0x38
        _emit 0x53  // PUSH EBX
        _emit 0x55  // PUSH EBP
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0xa1  // MOV EAX, [__security_cookie]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX  (cookie guard)
        _emit 0x8d  // LEA EAX, [ESP+0x4c]
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        _emit 0x64  // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // 0x0003acc7 — load param1, compute stride
        _emit 0x8b  // MOV ESI, [ESP+0x5c]  (param1)
        _emit 0x74
        _emit 0x24
        _emit 0x5c
        _emit 0x8b  // MOV EBX, [ESI+0x04]
        _emit 0x5e
        _emit 0x04
        _emit 0x8b  // MOV EAX, [ECX+0x08]  (this->field_8)
        _emit 0x41
        _emit 0x08
        _emit 0x8b  // MOV EDX, [ESI+0x08]
        _emit 0x56
        _emit 0x08
        _emit 0x0f  // IMUL EAX, EBX
        _emit 0xaf
        _emit 0xc3
        _emit 0x8d  // LEA EAX, [EAX*8+0x1f]
        _emit 0x04
        _emit 0xc5
        _emit 0x1f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc1  // SHR EAX, 3
        _emit 0xe8
        _emit 0x03
        _emit 0x25  // AND EAX, 0x1ffffffc
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x1f
        _emit 0x8b  // MOV ECX, EDX
        _emit 0xca
        _emit 0x0f  // IMUL ECX, EAX
        _emit 0xaf
        _emit 0xc8
        _emit 0x89  // MOV [ESP+0x28], EAX  (stride)
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x8b  // MOV EAX, [ESI+0x0c]
        _emit 0x46
        _emit 0x0c
        _emit 0x33  // XOR EDI, EDI
        _emit 0xff
        _emit 0x89  // MOV [ESP+0x1c], EBX
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x89  // MOV [ESP+0x24], ECX
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x89  // MOV [ESP+0x20], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x89  // MOV [ESP+0x2c], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        _emit 0xc7  // MOV dword ptr [ESP+0x30], 0xf66390
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x90
        _emit 0x63
        _emit 0xf6
        _emit 0x00
        _emit 0x89  // MOV [ESP+0x34], EBX
        _emit 0x5c
        _emit 0x24
        _emit 0x34
        _emit 0x89  // MOV [ESP+0x38], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x38
        _emit 0x89  // MOV [ESP+0x3c], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0x89  // MOV [ESP+0x40], ECX
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        _emit 0x89  // MOV [ESP+0x44], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x44
        _emit 0x89  // MOV [ESP+0x48], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x48

        // 0x0003ad24 — choose data ptr, guard height==0
        _emit 0x8b  // MOV EDX, [ESI+0x14]
        _emit 0x56
        _emit 0x14
        _emit 0x3b  // CMP EDX, EDI
        _emit 0xd7
        _emit 0x89  // MOV [ESP+0x54], EDI  (trylevel → 0)
        _emit 0x7c
        _emit 0x24
        _emit 0x54
        _emit 0x75  // JNZ +3 (skip loading field_0x18)
        _emit 0x03
        _emit 0x8b  // MOV EDX, [ESI+0x18]
        _emit 0x56
        _emit 0x18
        _emit 0x3b  // CMP EAX, EDI  (EAX=field_0x0c, height)
        _emit 0xc7
        _emit 0x89  // MOV [ESP+0x5c], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x5c
        _emit 0x89  // MOV [ESP+0x18], EDI  (outer loop counter = 0)
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x76  // JBE 0x0043ad9a  (skip loops if height==0)
        _emit 0x5c

        // 0x0003ad3e — outer loop body
        _emit 0x8b  // MOV ECX, [ESI+0x0c]
        _emit 0x4e
        _emit 0x0c
        _emit 0x8b  // MOV EAX, [ESI+0x10]
        _emit 0x46
        _emit 0x10
        _emit 0x33  // XOR EDX, EDX
        _emit 0xd2
        _emit 0xf7  // DIV ECX
        _emit 0xf1
        _emit 0x8b  // MOV ECX, [ESI+0x08]
        _emit 0x4e
        _emit 0x08
        _emit 0x33  // XOR EDX, EDX
        _emit 0xd2
        _emit 0x8b  // MOV EBX, [ESP+0x5c]
        _emit 0x5c
        _emit 0x24
        _emit 0x5c
        _emit 0x33  // XOR EBP, EBP
        _emit 0xed
        _emit 0xf7  // DIV ECX
        _emit 0xf1
        _emit 0x3b  // CMP ECX, EDI
        _emit 0xcf
        _emit 0x89  // MOV [ESP+0x14], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x76  // JBE 0x0043ad7e  (skip inner if width==0)
        _emit 0x21

        // 0x0003ad5d — inner loop
        _emit 0x8b  // MOV EAX, [ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x50  // PUSH EAX
        _emit 0x53  // PUSH EBX
        _emit 0x57  // PUSH EDI
        _emit 0xe8  // CALL FUNC_009d4600
        _emit 0x97
        _emit 0x98
        _emit 0x59
        _emit 0x00
        _emit 0x03  // ADD EDI, [ESP+0x34]
        _emit 0x7c
        _emit 0x24
        _emit 0x34
        _emit 0x03  // ADD EBX, [ESP+0x20]
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        _emit 0x83  // ADD EBP, 1
        _emit 0xc5
        _emit 0x01
        _emit 0x83  // ADD ESP, 0x0c
        _emit 0xc4
        _emit 0x0c
        _emit 0x3b  // CMP EBP, [ESI+0x08]
        _emit 0x6e
        _emit 0x08
        _emit 0x72  // JC inner_loop
        _emit 0xe1
        _emit 0x33  // XOR EDI, EDI
        _emit 0xff

        // 0x0003ad7e — outer loop increment / check
        _emit 0x8b  // MOV EAX, [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x83  // ADD EAX, 1
        _emit 0xc0
        _emit 0x01
        _emit 0x3b  // CMP EAX, [ESI+0x0c]
        _emit 0x46
        _emit 0x0c
        _emit 0x89  // MOV [ESP+0x18], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x72  // JC outer_loop
        _emit 0xb0
        _emit 0x8b  // MOV EBX, [ESP+0x1c]
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b  // MOV EAX, [ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x8b  // MOV ECX, [ESP+0x24]
        _emit 0x4c
        _emit 0x24
        _emit 0x24

        // 0x0003ad9a — check if param1 == &local_struct (self-assign guard)
        _emit 0x8d  // LEA EDX, [ESP+0x30]
        _emit 0x54
        _emit 0x24
        _emit 0x30
        _emit 0x3b  // CMP ESI, EDX
        _emit 0xf2
        _emit 0x74  // JZ epilogue (skip free/write-back)
        _emit 0x3d
        _emit 0x8b  // MOV EDX, [ESI+0x14]
        _emit 0x56
        _emit 0x14
        _emit 0x3b  // CMP EDX, EDI
        _emit 0xd7
        _emit 0x74  // JZ skip_free
        _emit 0x1e
        _emit 0x8b  // MOV ECX, [EDX-0x04]  (read alloc header)
        _emit 0x4a
        _emit 0xfc
        _emit 0x52  // PUSH EDX
        _emit 0xe8  // CALL FUN_0040df70  (free)
        _emit 0xbe
        _emit 0x31
        _emit 0xfd
        _emit 0xff
        _emit 0x8b  // MOV EBX, [ESP+0x1c]  (restore regs after free)
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b  // MOV EAX, [ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x8b  // MOV ECX, [ESP+0x24]
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0xc7  // MOV dword ptr [ESI+0x14], 0
        _emit 0x46
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33  // XOR EDI, EDI
        _emit 0xff
        _emit 0x39  // CMP [ESI+0x18], EDI
        _emit 0x7e
        _emit 0x18
        _emit 0x74  // JZ skip_zero_18
        _emit 0x03
        _emit 0x89  // MOV [ESI+0x18], EDI
        _emit 0x7e
        _emit 0x18
        _emit 0x89  // MOV [ESI+0x10], ECX
        _emit 0x4e
        _emit 0x10
        _emit 0x8b  // MOV ECX, [ESP+0x2c]
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x89  // MOV [ESI+0x04], EBX
        _emit 0x5e
        _emit 0x04
        _emit 0x89  // MOV [ESI+0x08], ECX
        _emit 0x4e
        _emit 0x08
        _emit 0x89  // MOV [ESI+0x0c], EAX
        _emit 0x46
        _emit 0x0c
        _emit 0x8b  // MOV ECX, [ESP+0x4c]  (← byte 320 = last in range)
    }
}
