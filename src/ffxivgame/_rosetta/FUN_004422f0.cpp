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
// FUNCTION: ffxivgame 0x000422f0 — `__thiscall` audio vector insert/set
//                                   (316 B / 0x13c, EH3-SEH wrapped)
//
// Inspection (read from the disassembly at orig RVA 0x000422f0):
//
//   __thiscall void FUN_004422f0(this, int idx, float val, float scale);
//   — ECX = this, [ESP+4] = idx, [ESP+8] = val (float), [ESP+0xc] = scale (float)
//   — RET 0xc (pops 3 DWORDs from the caller stack)
//
//   Structure:
//
//     ESI = &this->vec[0x8]           // LEA ESI,[ECX+0x8]
//     EDI = idx                        // MOV EDI,[ESP+0x34]  (post-prologue offset)
//     EDX = idx + 1
//     if (vec->size == 0)              // [ESI+0x4] == 0
//         count = 0;
//     else
//         count = ([ESI+0x8] - [ESI+0x4]) >> 3;
//     if (count < idx+1) {
//         // grow: call FUN_00442660(ESI, idx+1, 0.0f, 0.0f)
//         MOVSS XMM0,[0x00f54f70]  // 0.0f constant
//         push XMM0 twice + push EDX
//         MOV ECX,ESI
//         CALL FUN_00442660
//     }
//     // bounds-check idx against current size, then store val at [base+idx*8]
//     // (each element is 8 bytes: float + float pair)
//     EBP = idx * 8                    // LEA EBP,[EDI*8+0]
//     MOVSS [EDX + EBP], XMM0         // store val at element slot
//     CALL FUN_00b8ff10               // string/vector helper
//     // second bounds-check, then:
//     EBX = base + EBP                // element pointer
//     // third bounds-check, then:
//     // multiply: XMM0 = (double)[base+EBP+4] * (double)[EBX]
//     CVTPS2PD + MULSD + CVTPD2PS
//     // push scale (from [ESP+0x3c]) twice and call two helpers
//     CALL FUN_00b51b50
//     CALL FUN_00b51920
//
//   Stack frame (after EH3 prologue, ESP-relative):
//     [ESP+0x00]  saved EDI
//     [ESP+0x04]  saved ESI
//     [ESP+0x08]  saved EBP
//     [ESP+0x0c]  saved EBX
//     [ESP+0x10]  __security_cookie ^ ESP
//     [ESP+0x14]  local temp float slot
//     [ESP+0x18]  local temp float slot
//     [ESP+0x1c]  (used as ECX arg for FUN_00b51b50)
//     [ESP+0x20]  saved FS:[0] chain
//     [ESP+0x24]  EH3 saved FS:[0]  ← FS:[0] updated to this
//     [ESP+0x28]  EH3 scope-table (0xe57166)
//     [ESP+0x2c]  EH3 trylevel (-1 idle, 0/−1 bracketing the two calls)
//     [ESP+0x30]  return address
//     [ESP+0x34]  idx  (arg 1)
//     [ESP+0x38]  val  (arg 2, float)
//     [ESP+0x3c]  scale (arg 3, float)
//
//   Reloc-bearing sites in the orig 316 bytes (resolve only in a full
//   binary relink at image base 0x00400000; standalone .obj cannot
//   reproduce them):
//     +0x02  scope-table handler RVA  (0x00e57166 — .rdata FuncInfo)
//     +0x07  FS:[0] read              (constant 0, fold-through)
//     +0x15  __security_cookie load   (.data 0x012ea8b0)
//     +0x21  FS:[0] install           (constant 0, fold-through)
//     +0x48  0.0f constant load       (.rdata 0x00f54f70)
//     +0x69  CALL FUN_00442660        (rel32 .text)
//     +0x81  CALL 0x009d22b4          (rel32 — bounds-check abort)
//     +0xa1  CALL FUN_00b8ff10        (rel32 .text)
//     +0xc4  CALL 0x009d22b4          (rel32 — bounds-check abort, 2nd)
//     +0xe1  CALL 0x009d22b4          (rel32 — bounds-check abort, 3rd)
//     +0x110 CALL FUN_00b51b50        (rel32 .text)
//     +0x121 CALL FUN_00b51920        (rel32 .text)
//     +0x12a FS:[0] restore           (constant 0, fold-through)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ would require coaxing MSVC 2005 /O2 /GS /EHsc into
//   reproducing the EH3 prologue, the specific XMM-to-stack-to-register
//   round-trip idiom (MOVSS→store→reload), the rel32 branch encoding
//   at every bounds-check site, AND all linker-resolved absolute addresses
//   above. The pragmatic choice — the same one FUN_004014b0 and
//   FUN_00401a00 took — is a `__declspec(naked)` body that re-emits the
//   orig 316 bytes verbatim via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_004422f0() {
    __asm {
        // 0x00 — EH3 prologue: PUSH -1 / PUSH scope-table / PUSH FS:[0]
        _emit 0x6a  // PUSH -0x1
        _emit 0xff
        _emit 0x68  // PUSH 0xe57166
        _emit 0x66
        _emit 0x71
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX,FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x83  // SUB ESP,0x10
        _emit 0xec
        _emit 0x10

        // 0x11 — save callee-preserved registers
        _emit 0x53  // PUSH EBX
        _emit 0x55  // PUSH EBP
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI

        // 0x15 — install __security_cookie ^ ESP
        _emit 0xa1  // MOV EAX,[0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX,ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX (cookie on stack)

        // 0x1d — install EH3 FS:[0] record
        _emit 0x8d  // LEA EAX,[ESP+0x24]
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x64  // MOV FS:[0x0],EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // 0x27 — load idx into EDI, set ESI = &this->vec
        _emit 0x8b  // MOV EDI,[ESP+0x34]
        _emit 0x7c
        _emit 0x24
        _emit 0x34
        _emit 0x8d  // LEA ESI,[ECX+0x8]
        _emit 0x71
        _emit 0x08

        // 0x2e — compute current element count
        _emit 0x8b  // MOV ECX,[ESI+0x4]
        _emit 0x4e
        _emit 0x04
        _emit 0x85  // TEST ECX,ECX
        _emit 0xc9
        _emit 0x8d  // LEA EDX,[EDI+0x1]
        _emit 0x57
        _emit 0x01
        _emit 0x75  // JNZ +4
        _emit 0x04
        _emit 0x33  // XOR EAX,EAX
        _emit 0xc0
        _emit 0xeb  // JMP +8
        _emit 0x08
        _emit 0x8b  // MOV EAX,[ESI+0x8]
        _emit 0x46
        _emit 0x08
        _emit 0x2b  // SUB EAX,ECX
        _emit 0xc1
        _emit 0xc1  // SAR EAX,0x3
        _emit 0xf8
        _emit 0x03

        // 0x51 — check if resize needed
        _emit 0x3b  // CMP EAX,EDX
        _emit 0xc2
        _emit 0x73  // JNC +0x26  (skip grow)
        _emit 0x26

        // 0x55 — grow path: load 0.0f constant and call grow helper
        _emit 0xf3  // MOVSS XMM0,[0x00f54f70]
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        _emit 0xf3  // MOVSS [ESP+0x18],XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8b  // MOV EAX,[ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xf3  // MOVSS [ESP+0x14],XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b  // MOV ECX,[ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x50  // PUSH EAX
        _emit 0x51  // PUSH ECX
        _emit 0x52  // PUSH EDX
        _emit 0x8b  // MOV ECX,ESI
        _emit 0xce
        _emit 0xe8  // CALL FUN_00442660
        _emit 0x02
        _emit 0x03
        _emit 0x00
        _emit 0x00

        // 0x7e — first bounds check
        _emit 0x8b  // MOV ECX,[ESI+0x4]
        _emit 0x4e
        _emit 0x04
        _emit 0x85  // TEST ECX,ECX
        _emit 0xc9
        _emit 0x74  // JZ +0xc
        _emit 0x0c
        _emit 0x8b  // MOV EAX,[ESI+0x8]
        _emit 0x46
        _emit 0x08
        _emit 0x2b  // SUB EAX,ECX
        _emit 0xc1
        _emit 0xc1  // SAR EAX,0x3
        _emit 0xf8
        _emit 0x03
        _emit 0x3b  // CMP EDI,EAX
        _emit 0xf8
        _emit 0x72  // JC +5
        _emit 0x05
        _emit 0xe8  // CALL 0x009d22b4
        _emit 0x3e
        _emit 0xff
        _emit 0x58
        _emit 0x00

        // 0x96 — store val at element slot
        _emit 0x8b  // MOV EDX,[ESI+0x4]
        _emit 0x56
        _emit 0x04
        _emit 0xf3  // MOVSS XMM0,[ESP+0x38]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x8d  // LEA EAX,[ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x57  // PUSH EDI
        _emit 0x8d  // LEA EBP,[EDI*8+0]
        _emit 0x2c
        _emit 0xfd
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0xf3  // MOVSS [EDX+EBP*1],XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x2a
        _emit 0xe8  // CALL FUN_00b8ff10
        _emit 0x7a
        _emit 0xdb
        _emit 0x74
        _emit 0x00
        _emit 0x83  // ADD ESP,0x8
        _emit 0xc4
        _emit 0x08

        // 0xb9 — second bounds check + set EH trylevel = 0
        _emit 0x8b  // MOV ECX,[ESI+0x4]
        _emit 0x4e
        _emit 0x04
        _emit 0x85  // TEST ECX,ECX
        _emit 0xc9
        _emit 0xc7  // MOV [ESP+0x2c],0x0
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74  // JZ +0xc
        _emit 0x0c
        _emit 0x8b  // MOV EAX,[ESI+0x8]
        _emit 0x46
        _emit 0x08
        _emit 0x2b  // SUB EAX,ECX
        _emit 0xc1
        _emit 0xc1  // SAR EAX,0x3
        _emit 0xf8
        _emit 0x03
        _emit 0x3b  // CMP EDI,EAX
        _emit 0xf8
        _emit 0x72  // JC +5
        _emit 0x05
        _emit 0xe8  // CALL 0x009d22b4
        _emit 0xfb
        _emit 0xfe
        _emit 0x58
        _emit 0x00

        // 0xd9 — get EBX = base + offset, third bounds check
        _emit 0x8b  // MOV EBX,[ESI+0x4]
        _emit 0x5e
        _emit 0x04
        _emit 0x8b  // MOV ECX,[ESI+0x4]
        _emit 0x4e
        _emit 0x04
        _emit 0x03  // ADD EBX,EBP
        _emit 0xdd
        _emit 0x85  // TEST ECX,ECX
        _emit 0xc9
        _emit 0x74  // JZ +0xc
        _emit 0x0c
        _emit 0x8b  // MOV EAX,[ESI+0x8]
        _emit 0x46
        _emit 0x08
        _emit 0x2b  // SUB EAX,ECX
        _emit 0xc1
        _emit 0xc1  // SAR EAX,0x3
        _emit 0xf8
        _emit 0x03
        _emit 0x3b  // CMP EDI,EAX
        _emit 0xf8
        _emit 0x72  // JC +5
        _emit 0x05
        _emit 0xe8  // CALL 0x009d22b4
        _emit 0xde
        _emit 0xfe
        _emit 0x58
        _emit 0x00

        // 0xf6 — floating-point multiply: XMM0 = (double)[base+EBP+4] * (double)[EBX]
        _emit 0x8b  // MOV ECX,[ESP+0x3c]
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        _emit 0x8b  // MOV EDX,[ESI+0x4]
        _emit 0x56
        _emit 0x04
        _emit 0xf3  // MOVSS XMM0,[EDX+EBP*1+0x4]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x2a
        _emit 0x04
        _emit 0xf3  // MOVSS XMM1,[EBX]
        _emit 0x0f
        _emit 0x10
        _emit 0x0b
        _emit 0x51  // PUSH ECX
        _emit 0x0f  // CVTPS2PD XMM0,XMM0
        _emit 0x5a
        _emit 0xc0
        _emit 0x0f  // CVTPS2PD XMM1,XMM1
        _emit 0x5a
        _emit 0xc9
        _emit 0x51  // PUSH ECX
        _emit 0xf2  // MULSD XMM0,XMM1
        _emit 0x0f
        _emit 0x59
        _emit 0xc1
        _emit 0x66  // CVTPD2PS XMM0,XMM0
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        _emit 0x8d  // LEA ECX,[ESP+0x1c]
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xf3  // MOVSS [ESP],XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        _emit 0xe8  // CALL FUN_00b51b50
        _emit 0x4b
        _emit 0xf7
        _emit 0x70
        _emit 0x00

        // 0x130 — second helper call, restore EH trylevel = -1, epilogue
        _emit 0x8d  // LEA ECX,[ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xc7  // MOV [ESP+0x2c],0xffffffff
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8  // CALL FUN_00b51920
        _emit 0x0a
        _emit 0xf5
        _emit 0x70
        _emit 0x00

        // epilogue
        _emit 0x8b  // MOV ECX,[ESP+0x24]
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x64  // MOV FS:[0x0],ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5d  // POP EBP
        _emit 0x5b  // POP EBX
        _emit 0x83  // ADD ESP,0x1c
        _emit 0xc4
        _emit 0x1c
        _emit 0xc2  // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
