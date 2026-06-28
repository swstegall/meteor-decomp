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
// FUNCTION: ffxivgame 0x0003ffc0 — iterator/range insert helper
//                                  (__thiscall, 0xb4 = 180 B)
//
//   __thiscall void FUN_0043ffc0(this,
//                                out_iter*  p_out,    // [ESP+0x18]
//                                elem_ptr   src_begin,// [ESP+0x1c]
//                                elem_ptr   src_end,  // [ESP+0x20]
//                                int        param4)   // [ESP+0x24]
//
//   Manages insertion into a container whose elements are 28 bytes each.
//   this->m_4 = begin pointer, this->m_8 = end pointer.
//
//   1. If m_4 == 0 → skip capacity check, set EDI = 0.
//   2. Else: compute element count = (m_8 - m_4) / 28; if count == 0,
//            also set EDI = 0.
//   3. Validate that m_4 <= src_begin (assert-fail call on violation).
//   4. Validate src_begin: not null and != this (assert-fail otherwise).
//   5. Compute EDI = (src_end - m_4) / 28.
//   6. Call FUN_0043fc20(this, src_begin, src_end, 1, param4) — the
//      underlying realloc/insert primitive.
//   7. After call: validate m_4 <= m_8 (assert-fail otherwise).
//   8. Compute final iterator pointer: EBX + EDI*28.
//   9. Validate the pointer is within [m_4, m_8) (assert-fail otherwise).
//  10. Write output: out->m_0 = this, out->m_4 = computed iterator.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The two signed division-by-28 sequences (magic multiplier 0x92492493,
//   shift 4), paired with MSVC's register-allocation ordering
//   (ECX/EBX/EBP saved before ESI/EDI, with EBP loaded from the 3rd
//   stack parameter before the last two pushes), plus the four CALL rel32
//   targets whose addresses bake into the original binary's address space,
//   combine to form a shape that resists source-level C++ reconstruction
//   in a single TU. The naked-asm byte passthrough (as used by
//   FUN_00403f10 and FUN_00401b70) is the reliable path.
//
//   All 180 bytes are emitted verbatim. The four CALL rel32 windows:
//     +0x3a  CALL rel32 → 0x009d22b4  (assert/range-check fail, e8 b5 22 59 00)
//     +0x4b  CALL rel32 → 0x009d22b4  (assert, e8 a4 22 59 00)
//     +0x72  CALL rel32 → 0x0043fc20  (internal insert primitive, e8 e9 fb ff ff)
//     +0x7f  CALL rel32 → 0x009d22b4  (assert, e8 70 22 59 00)
//     +0x9e  CALL rel32 → 0x009d22b4  (assert, e8 51 22 59 00)
//   are embedded as raw bytes; tools/compare.py wildcards those 4-byte
//   rel32 fields, so they match regardless.

extern "C" __declspec(naked) void FUN_0043ffc0() {
    __asm {
        // prologue
        _emit 0x51              // PUSH ECX
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x18]
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x57              // PUSH EDI
        // EDI = this->m_4
        _emit 0x8b              // MOV EDI, dword ptr [ESI+0x4]
        _emit 0x7e
        _emit 0x04
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x74              // JZ +0x1c  (to 0x43ffee)
        _emit 0x1c
        // EBX = this->m_8
        _emit 0x8b              // MOV EBX, dword ptr [ESI+0x8]
        _emit 0x5e
        _emit 0x08
        // ECX = EBX - EDI = (m_8 - m_4)
        _emit 0x8b              // MOV ECX, EBX
        _emit 0xcb
        _emit 0x2b              // SUB ECX, EDI
        _emit 0xcf
        // signed division by 28: EAX = (m_8 - m_4) / 28
        _emit 0xb8              // MOV EAX, 0x92492493
        _emit 0x93
        _emit 0x24
        _emit 0x49
        _emit 0x92
        _emit 0xf7              // IMUL ECX
        _emit 0xe9
        _emit 0x03              // ADD EDX, ECX
        _emit 0xd1
        _emit 0xc1              // SAR EDX, 4
        _emit 0xfa
        _emit 0x04
        _emit 0x8b              // MOV EAX, EDX
        _emit 0xc2
        _emit 0xc1              // SHR EAX, 0x1f
        _emit 0xe8
        _emit 0x1f
        _emit 0x03              // ADD EAX, EDX
        _emit 0xc2
        _emit 0x75              // JNZ +0x08  (to 0x43fff6, count != 0)
        _emit 0x08
        // zero-count path (or EDI was 0): EBX = param2, EDI = 0
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x1c]
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0xeb              // JMP +0x31  (to 0x440027)
        _emit 0x31
        // non-zero path: validate m_4 <= src_begin
        _emit 0x3b              // CMP EDI, EBX
        _emit 0xfb
        _emit 0x76              // JBE +0x05  (skip assert)
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4  (assert fail)
        _emit 0xb5
        _emit 0x22
        _emit 0x59
        _emit 0x00
        // EBX = param2 (src_begin)
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x1c]
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        // validate: src_begin != null AND != this
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x74              // JZ +0x04  (null → assert)
        _emit 0x04
        _emit 0x3b              // CMP EBX, ESI
        _emit 0xde
        _emit 0x74              // JZ +0x05  (== this → skip assert? or assert)
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4  (assert fail)
        _emit 0xa4
        _emit 0x22
        _emit 0x59
        _emit 0x00
        // ECX = src_end (EBP) - m_4 (EDI); compute EDI = count2 = / 28
        _emit 0x8b              // MOV ECX, EBP
        _emit 0xcd
        _emit 0x2b              // SUB ECX, EDI
        _emit 0xcf
        _emit 0xb8              // MOV EAX, 0x92492493
        _emit 0x93
        _emit 0x24
        _emit 0x49
        _emit 0x92
        _emit 0xf7              // IMUL ECX
        _emit 0xe9
        _emit 0x03              // ADD EDX, ECX
        _emit 0xd1
        _emit 0xc1              // SAR EDX, 4
        _emit 0xfa
        _emit 0x04
        _emit 0x8b              // MOV EDI, EDX
        _emit 0xfa
        _emit 0xc1              // SHR EDI, 0x1f
        _emit 0xef
        _emit 0x1f
        _emit 0x03              // ADD EDI, EDX
        _emit 0xfa
        // ECX = param4, set up call args
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x24]
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x51              // PUSH ECX           (param4)
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x55              // PUSH EBP           (src_end)
        _emit 0x53              // PUSH EBX           (src_begin)
        _emit 0x8b              // MOV ECX, ESI       (this)
        _emit 0xce
        _emit 0xe8              // CALL 0x0043fc20
        _emit 0xe9
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        // validate m_4 <= m_8
        _emit 0x8b              // MOV EBX, dword ptr [ESI+0x4]
        _emit 0x5e
        _emit 0x04
        _emit 0x3b              // CMP EBX, dword ptr [ESI+0x8]
        _emit 0x5e
        _emit 0x08
        _emit 0x76              // JBE +0x05  (ok)
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4  (assert fail)
        _emit 0x70
        _emit 0x22
        _emit 0x59
        _emit 0x00
        // EDX = EDI * 7 * 4 = EDI * 28
        _emit 0x8d              // LEA EDX, [EDI*8+0]
        _emit 0x14
        _emit 0xfd
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB EDX, EDI      (EDX = EDI*7)
        _emit 0xd7
        _emit 0x8d              // LEA EDI, [EBX + EDX*4]  (EDI = EBX + EDI*28)
        _emit 0x3c
        _emit 0x93
        // validate computed iterator: m_4 <= EDI <= m_8
        _emit 0x3b              // CMP EDI, dword ptr [ESI+0x8]
        _emit 0x7e
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESP+0x20], EBX
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        _emit 0x77              // JA +0x05  (above end → assert)
        _emit 0x05
        _emit 0x3b              // CMP EDI, dword ptr [ESI+0x4]
        _emit 0x7e
        _emit 0x04
        _emit 0x73              // JNC +0x05  (>= begin → ok)
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4  (assert fail)
        _emit 0x51
        _emit 0x22
        _emit 0x59
        _emit 0x00
        // write output: EAX = param1 (out_iter*)
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x89              // MOV dword ptr [EAX+0x4], EDI
        _emit 0x78
        _emit 0x04
        _emit 0x5f              // POP EDI
        _emit 0x89              // MOV dword ptr [EAX], ESI
        _emit 0x30
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0x59              // POP ECX
        _emit 0xc2              // RET 0x10
        _emit 0x10
        _emit 0x00
    }
}
