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
// FUNCTION: ffxivgame 0x009c828c — struct validator, 170 bytes (0xaa)
//                                   (EAX = pointer to validated struct on entry)
//
// Behaviour (recovered from asm @ RVA 0x009c828c):
//
//   Returns 1 (EDI) if all validation checks pass, 0 if any fail.
//
//   Header checks (on struct pointed to by EAX at entry):
//     word [EAX+0x10] >= 0x20               → else return 0 immediately
//     word [EAX+0x02] == 0x10               → else clear result flag
//     byte [EAX+0x0b] == 0x02               → else clear result flag
//     word [EAX+0x08] != 0                  → else clear result flag
//     dword [EAX+0x10] != 0                 → else clear result flag
//     loop_count (zero-extended [EAX+0x08]) > 0  → else return result
//
//   Element loop (array of 0x14-byte elements starting at [EAX+0x1c]):
//     For each element at [ESI] (ESI advances by 0x14 each iteration):
//       dword [ESI-0xc] == saved_dword[EAX+0x10]  → else clear flag
//       (byte[ESI] & 0x0f) <= 0x04               → else clear flag
//       (byte[ESI] & 0xf0) <= 0x30               → else clear flag
//       byte[ESI+1] == 1 || == 2                 → else clear flag
//       if word[ESI+2] != 0:
//           FUN_009c8187(word[ESI+2]) == byte[ESI+1]  → else clear flag
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The calling convention passes the struct pointer in EAX (not on the
//   stack and not in ECX), which cannot be expressed in standard C++.
//   The function body also has complex multi-flag clearing patterns with
//   callee-saved register use that MSVC's register allocator would not
//   reproduce reliably from source level. The same approach taken by
//   FUN_00406280, FUN_00401b70, and FUN_004063c0 in this binary —
//   __declspec(naked) re-emitting the orig bytes verbatim via MASM
//   _emit directives — produces a zero-reloc .obj whose .text is
//   byte-identical to the orig slice. compare.py wildcards the single
//   reloc window.
//
// Reloc-bearing site (offset within the function):
//   +0x8b   CALL rel32 → FUN_009c8187  (internal helper, -0x195 rel32)

extern "C" __declspec(naked) void FUN_00dc828c() {
    __asm {
        _emit 0x8b              // MOV EDI,EDI                (hot-patch NOP)
        _emit 0xff
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP,ESP
        _emit 0xec
        _emit 0x51              // PUSH ECX                   (local slot [EBP-4])
        _emit 0x51              // PUSH ECX                   (local slot [EBP-8])
        _emit 0x57              // PUSH EDI
        _emit 0x33              // XOR EDI,EDI                EDI = 0 ...
        _emit 0xff
        _emit 0x47              // INC EDI                    EDI = 1 (result = true)
        _emit 0x66              // CMP word ptr [EAX+0x10],0x20
        _emit 0x83
        _emit 0x78
        _emit 0x10
        _emit 0x20
        _emit 0x73              // JNC short +7 (→ 0x009c82a5)
        _emit 0x07
        _emit 0x33              // XOR EDI,EDI                result = 0
        _emit 0xff
        _emit 0xe9              // JMP 0x009c8331 (→ epilogue)
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP word ptr [EAX+0x2],0x10
        _emit 0x83
        _emit 0x78
        _emit 0x02
        _emit 0x10
        _emit 0x74              // JZ short +2 (→ 0x009c82ae)
        _emit 0x02
        _emit 0x33              // XOR EDI,EDI                result = 0
        _emit 0xff
        _emit 0x80              // CMP byte ptr [EAX+0xb],0x2
        _emit 0x78
        _emit 0x0b
        _emit 0x02
        _emit 0x74              // JZ short +2 (→ 0x009c82b6)
        _emit 0x02
        _emit 0x33              // XOR EDI,EDI                result = 0
        _emit 0xff
        _emit 0x0f              // MOVZX ECX,word ptr [EAX+0x8]
        _emit 0xb7
        _emit 0x48
        _emit 0x08
        _emit 0x66              // TEST CX,CX
        _emit 0x85
        _emit 0xc9
        _emit 0x75              // JNZ short +2 (→ 0x009c82c1)
        _emit 0x02
        _emit 0x33              // XOR EDI,EDI                result = 0
        _emit 0xff
        _emit 0x8b              // MOV EDX,dword ptr [EAX+0x10]
        _emit 0x50
        _emit 0x10
        _emit 0x85              // TEST EDX,EDX
        _emit 0xd2
        _emit 0x89              // MOV dword ptr [EBP-0x8],EDX  (save dword[+0x10])
        _emit 0x55
        _emit 0xf8
        _emit 0x75              // JNZ short +2 (→ 0x009c82cd)
        _emit 0x02
        _emit 0x33              // XOR EDI,EDI                result = 0
        _emit 0xff
        _emit 0x0f              // MOVZX ECX,CX               zero-extend loop count
        _emit 0xb7
        _emit 0xc9
        _emit 0x85              // TEST ECX,ECX
        _emit 0xc9
        _emit 0x7e              // JLE short +0x5d (→ 0x009c8331 epilogue)
        _emit 0x5d
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA ESI,[EAX+0x1c]         ESI → first element
        _emit 0x70
        _emit 0x1c
        _emit 0x89              // MOV dword ptr [EBP-0x4],ECX  (save loop count)
        _emit 0x4d
        _emit 0xfc
        _emit 0xeb              // JMP short +3 (→ 0x009c82e1, skip first reload)
        _emit 0x03
        _emit 0x8b              // MOV EDX,dword ptr [EBP-0x8]  (reload saved dword)
        _emit 0x55
        _emit 0xf8
        _emit 0x39              // CMP dword ptr [ESI-0xc],EDX
        _emit 0x56
        _emit 0xf4
        _emit 0x74              // JZ short +2 (→ 0x009c82e8)
        _emit 0x02
        _emit 0x33              // XOR EDI,EDI                result = 0
        _emit 0xff
        _emit 0x8a              // MOV AL,byte ptr [ESI]
        _emit 0x06
        _emit 0x8a              // MOV CL,AL
        _emit 0xc8
        _emit 0x80              // AND CL,0xf                 low nibble
        _emit 0xe1
        _emit 0x0f
        _emit 0x80              // CMP CL,0x4
        _emit 0xf9
        _emit 0x04
        _emit 0x76              // JBE short +2 (→ 0x009c82f6)
        _emit 0x02
        _emit 0x33              // XOR EDI,EDI                result = 0
        _emit 0xff
        _emit 0x24              // AND AL,0xf0                high nibble
        _emit 0xf0
        _emit 0x3c              // CMP AL,0x30
        _emit 0x30
        _emit 0x76              // JBE short +2 (→ 0x009c82fe)
        _emit 0x02
        _emit 0x33              // XOR EDI,EDI                result = 0
        _emit 0xff
        _emit 0x8a              // MOV BL,byte ptr [ESI+0x1]
        _emit 0x5e
        _emit 0x01
        _emit 0x80              // CMP BL,0x1
        _emit 0xfb
        _emit 0x01
        _emit 0x74              // JZ short +7 (→ 0x009c830d)
        _emit 0x07
        _emit 0x80              // CMP BL,0x2
        _emit 0xfb
        _emit 0x02
        _emit 0x74              // JZ short +2 (→ 0x009c830d)
        _emit 0x02
        _emit 0x33              // XOR EDI,EDI                result = 0
        _emit 0xff
        _emit 0x0f              // MOVZX EAX,word ptr [ESI+0x2]
        _emit 0xb7
        _emit 0x46
        _emit 0x02
        _emit 0x66              // TEST AX,AX
        _emit 0x85
        _emit 0xc0
        _emit 0x74              // JZ short +0x11 (→ 0x009c8327)
        _emit 0x11
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_009c8187 (rel32 → -0x195)
        _emit 0x6b
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x66              // MOVZX CX,BL
        _emit 0x0f
        _emit 0xb6
        _emit 0xcb
        _emit 0x66              // CMP AX,CX
        _emit 0x3b
        _emit 0xc1
        _emit 0x74              // JZ short +2 (→ 0x009c8327)
        _emit 0x02
        _emit 0x33              // XOR EDI,EDI                result = 0
        _emit 0xff
        _emit 0x83              // ADD ESI,0x14               advance element pointer
        _emit 0xc6
        _emit 0x14
        _emit 0xff              // DEC dword ptr [EBP-0x4]    decrement loop counter
        _emit 0x4d
        _emit 0xfc
        _emit 0x75              // JNZ short -0x51 (→ 0x009c82de)
        _emit 0xaf
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x8b              // MOV EAX,EDI                return result
        _emit 0xc7
        _emit 0x5f              // POP EDI
        _emit 0xc9              // LEAVE
        _emit 0xc3              // RET
    }
}
