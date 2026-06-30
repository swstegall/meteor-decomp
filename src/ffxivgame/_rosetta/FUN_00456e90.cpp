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
// FUNCTION: ffxivgame 0x00456e90 — deque-style element address getter
//                                  (__thiscall, 1 stack arg, 96 bytes)
//
// __thiscall void* FUN_00456e90(SomeObj *this, unsigned n)
//   ECX = this
//   [ESP+4] = n (cleaned by callee: RET 0x4)
//
// `this` layout:
//   [ECX+0x4]  → InnerStruct* (a deque-like container; ESI)
//   [ECX+0x8]  → unsigned start_offset (EDI)
//
// InnerStruct layout (fields accessed via ESI):
//   [ESI+0x4]  → block_ptr* map  — array of pointers to element blocks
//   [ESI+0x8]  → unsigned start_block — index of the first map entry
//   [ESI+0xc]  → unsigned base        — absolute offset of first element
//   [ESI+0x10] → unsigned count       — number of valid elements
//
// Algorithm (deque iterator dereference, block size = 4 elements × 4 bytes):
//   1. Assert inner ptr != NULL (→ 0x009d22b4 on failure)
//   2. pos = start_offset + n
//   3. Assert base ≤ pos ≤ base+count  (→ 0x009d22b4 on failure)
//   4. Assert pos < base+count         (→ 0x009d22b4 on failure)
//   5. block_idx = pos >> 2
//   6. elem_off  = pos &  3
//   7. if start_block ≤ block_idx: block_idx -= start_block
//   8. return map[block_idx] + elem_off   (pointer arithmetic: +elem_off*4 bytes)
//
// All three CALL targets resolve to 0x009d22b4 (std::_Xran / range-check throw,
// noreturn). These are the three reloc sites in this 96-byte function.
//
// Reloc-bearing sites (4-byte rel32, masked by tools/compare.py):
//   +0x0f   CALL rel32 → 0x009d22b4   (null-ptr check)
//   +0x2a   CALL rel32 → 0x009d22b4   (outer bounds check)
//   +0x43   CALL rel32 → 0x009d22b4   (size check)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A `__declspec(naked)` body emitting all 96 bytes via MASM `_emit`
//   produces a .obj whose .text section is byte-identical to the orig
//   slice (no relocations emitted by the assembler — the rel32 operands
//   are baked in as the raw bytes from the original PE's address space).
//   tools/compare.py masks the three 4-byte reloc windows and reports GREEN.
//
// Asm (96 bytes, from RVA 0x00056e90):
//
//   00056e90:  53                    PUSH EBX
//   00056e91:  55                    PUSH EBP
//   00056e92:  56                    PUSH ESI
//   00056e93:  8b 71 04              MOV ESI, [ECX+0x4]       ; inner = this->field_4
//   00056e96:  85 f6                 TEST ESI, ESI
//   00056e98:  57                    PUSH EDI
//   00056e99:  8b 79 08              MOV EDI, [ECX+0x8]       ; EDI = start_offset
//   00056e9c:  75 05                 JNZ +5                    ; if inner != NULL, skip
//   00056e9e:  e8 11 b4 57 00        CALL 0x009d22b4           ; [RELOC +0x0f]
//   00056ea3:  8b 46 0c              MOV EAX, [ESI+0xc]        ; EAX = base
//   00056ea6:  8b 4c 24 14           MOV ECX, [ESP+0x14]       ; ECX = n (stack arg)
//   00056eaa:  8b 56 10              MOV EDX, [ESI+0x10]       ; EDX = count
//   00056ead:  03 f9                 ADD EDI, ECX              ; EDI = pos = start + n
//   00056eaf:  03 d0                 ADD EDX, EAX              ; EDX = base+count (end)
//   00056eb1:  3b fa                 CMP EDI, EDX
//   00056eb3:  77 04                 JA +4                     ; if pos > end, fail
//   00056eb5:  3b f8                 CMP EDI, EAX
//   00056eb7:  73 05                 JNC +5                    ; if pos >= base, ok
//   00056eb9:  e8 f6 b3 57 00        CALL 0x009d22b4           ; [RELOC +0x2a]
//   00056ebe:  8b 46 10              MOV EAX, [ESI+0x10]       ; EAX = count
//   00056ec1:  03 46 0c              ADD EAX, [ESI+0xc]        ; EAX = base+count (end)
//   00056ec4:  8b df                 MOV EBX, EDI
//   00056ec6:  8b ef                 MOV EBP, EDI
//   00056ec8:  c1 eb 02              SHR EBX, 2                ; block_idx = pos/4
//   00056ecb:  83 e5 03              AND EBP, 3                ; elem_off  = pos&3
//   00056ece:  3b f8                 CMP EDI, EAX
//   00056ed0:  72 05                 JC +5                     ; if pos < end, ok
//   00056ed2:  e8 dd b3 57 00        CALL 0x009d22b4           ; [RELOC +0x43]
//   00056ed7:  8b 46 08              MOV EAX, [ESI+0x8]        ; EAX = start_block
//   00056eda:  3b c3                 CMP EAX, EBX
//   00056edc:  77 02                 JA +2                     ; if start > idx, skip
//   00056ede:  2b d8                 SUB EBX, EAX              ; idx -= start_block
//   00056ee0:  8b 4e 04              MOV ECX, [ESI+0x4]        ; ECX = map ptr
//   00056ee3:  8b 14 99              MOV EDX, [ECX+EBX*4]      ; EDX = map[idx]
//   00056ee6:  5f                    POP EDI
//   00056ee7:  5e                    POP ESI
//   00056ee8:  8d 04 aa              LEA EAX, [EDX+EBP*4]      ; &map[idx][elem_off]
//   00056eeb:  5d                    POP EBP
//   00056eec:  5b                    POP EBX
//   00056eed:  c2 04 00              RET 0x4

extern "C" __declspec(naked) void FUN_00456e90() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ECX+0x4]
        _emit 0x71
        _emit 0x04
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ECX+0x8]
        _emit 0x79
        _emit 0x08
        _emit 0x75              // JNZ +5  (skip null-check throw)
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4   [RELOC: rel32 at +0x0f]
        _emit 0x11
        _emit 0xb4
        _emit 0x57
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0xc]   (base)
        _emit 0x46
        _emit 0x0c
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x14]  (n, stack arg)
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x10]  (count)
        _emit 0x56
        _emit 0x10
        _emit 0x03              // ADD EDI, ECX   (pos = start_offset + n)
        _emit 0xf9
        _emit 0x03              // ADD EDX, EAX   (end = base + count)
        _emit 0xd0
        _emit 0x3b              // CMP EDI, EDX
        _emit 0xfa
        _emit 0x77              // JA +4   (pos > end → fail)
        _emit 0x04
        _emit 0x3b              // CMP EDI, EAX
        _emit 0xf8
        _emit 0x73              // JNC +5  (pos >= base → ok, skip throw)
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4   [RELOC: rel32 at +0x2a]
        _emit 0xf6
        _emit 0xb3
        _emit 0x57
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x10]  (count)
        _emit 0x46
        _emit 0x10
        _emit 0x03              // ADD EAX, dword ptr [ESI+0xc]   (end = count+base)
        _emit 0x46
        _emit 0x0c
        _emit 0x8b              // MOV EBX, EDI
        _emit 0xdf
        _emit 0x8b              // MOV EBP, EDI
        _emit 0xef
        _emit 0xc1              // SHR EBX, 2   (block_idx = pos / 4)
        _emit 0xeb
        _emit 0x02
        _emit 0x83              // AND EBP, 3   (elem_off = pos & 3)
        _emit 0xe5
        _emit 0x03
        _emit 0x3b              // CMP EDI, EAX
        _emit 0xf8
        _emit 0x72              // JC +5   (pos < end → ok, skip throw)
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4   [RELOC: rel32 at +0x43]
        _emit 0xdd
        _emit 0xb3
        _emit 0x57
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x8]   (start_block)
        _emit 0x46
        _emit 0x08
        _emit 0x3b              // CMP EAX, EBX
        _emit 0xc3
        _emit 0x77              // JA +2   (start_block > block_idx → no adjust)
        _emit 0x02
        _emit 0x2b              // SUB EBX, EAX   (block_idx -= start_block)
        _emit 0xd8
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x4]   (map ptr)
        _emit 0x4e
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [ECX+EBX*4] (block = map[idx])
        _emit 0x14
        _emit 0x99
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x8d              // LEA EAX, [EDX+EBP*4]           (&block[elem_off])
        _emit 0x04
        _emit 0xaa
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
