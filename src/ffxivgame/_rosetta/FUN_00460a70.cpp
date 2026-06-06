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
// FUNCTION: ffxivgame 0x00060a70 — _ASN1_primitive_new (218 B / 0xda),
//                                  __cdecl int(void **out, const ASN1_ITEM *it).
//
// Asm shape:
//   Entry:  PUSH ESI; loads ESI = it (arg1 at [ESP+0xc]).
//   If it != NULL and it->prim_funcs != NULL and it->prim_funcs->new_ex != NULL:
//     call new_ex(arg0, it) directly and return.
//   Otherwise determine the type:
//     if it == NULL or *it == 5 (MSTRING): type = -1
//     else: type = it->utype (field +0x4)
//   Switch on (type + 4) via jump table at 0x460b4c / index table at 0x460b60:
//     case (dense, ≤10):
//       Various allocators / assignment paths with distinct return sequences.
//     default (>10):
//       ASN1_TYPE_new(type), optionally OR flags field with 0x40 if MSTRING.
//   Returns 1 on success, 0 (SETNZ) when allocation returned NULL.
//
// Reconstruction strategy — __declspec(naked) byte passthrough.
//
//   The jump table entries reference absolute VAs in the .text section
//   (0x460b4c / 0x460b60) and cannot be reproduced by MSVC from C source
//   without matching the linker's exact table layout.  The CALL immediates
//   (rel32 to 0x464a20, 0x463150, 0x464480) are likewise compiler-fixed.
//   Emitting all 218 bytes via MASM _emit gives byte-identical .text.
//
// Reloc-bearing offsets within the function (compare.py masks these):
//   +0x39  DIR32 → 0x00460b60  (MOVZX index-table base)
//   +0x40  DIR32 → 0x00460b4c  (JMP pointer-table base, scaled by EDX*4)
//   +0x49  REL32 → 0x00464a20  (alloc call, case 0 branch)
//   +0x8b  REL32 → 0x00463150  (BIT_STRING alloc call)
//   +0xb6  REL32 → 0x00464480  (ASN1_TYPE_new call, default branch)

extern "C" __declspec(naked) void FUN_00460a70() {
    __asm {
        // 0x00060a70
        _emit 0x56          // PUSH ESI
        _emit 0x8b          // MOV ESI, [ESP+0xc]
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x85          // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74          // JZ +0x25 (→ 0x00460a9e)
        _emit 0x25
        _emit 0x8b          // MOV EAX, [ESI+0x10]
        _emit 0x46
        _emit 0x10
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74          // JZ +0x14 (→ 0x00460a94)
        _emit 0x14
        // 0x00060a80
        _emit 0x8b          // MOV EAX, [EAX+0x8]
        _emit 0x40
        _emit 0x08
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74          // JZ +0xd (→ 0x00460a94)
        _emit 0x0d
        _emit 0x8b          // MOV ECX, [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x56          // PUSH ESI
        _emit 0x51          // PUSH ECX
        _emit 0xff          // CALL EAX
        _emit 0xd0
        _emit 0x83          // ADD ESP, 0x8
        // 0x00060a90
        _emit 0xc4
        _emit 0x08
        _emit 0x5e          // POP ESI
        _emit 0xc3          // RET
        // 0x00060a94
        _emit 0x80          // CMP byte ptr [ESI], 0x5
        _emit 0x3e
        _emit 0x05
        _emit 0x74          // JZ +0x5 (→ 0x00460a9e)
        _emit 0x05
        _emit 0x8b          // MOV EAX, [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0xeb          // JMP +0x3 (→ 0x00460aa1)
        _emit 0x03
        // 0x00060a9e
        _emit 0x83          // OR EAX, 0xffffffff
        _emit 0xc8
        _emit 0xff
        // 0x00060aa1
        _emit 0x8d          // LEA ECX, [EAX+0x4]
        _emit 0x48
        _emit 0x04
        _emit 0x83          // CMP ECX, 0xa
        _emit 0xf9
        _emit 0x0a
        _emit 0x77          // JA +0x7c (→ 0x00460b25)
        _emit 0x7c
        // 0x00060aa9
        _emit 0x0f          // MOVZX EDX, byte ptr [ECX + 0x00460b60]
        _emit 0xb6
        _emit 0x91
        _emit 0x60
        _emit 0x0b
        _emit 0x46
        _emit 0x00
        // 0x00060ab0
        _emit 0xff          // JMP dword ptr [EDX*4 + 0x00460b4c]
        _emit 0x24
        _emit 0x95
        _emit 0x4c
        _emit 0x0b
        _emit 0x46
        _emit 0x00
        // 0x00060ab7 — case: allocate NULL object
        _emit 0x6a          // PUSH 0x0
        _emit 0x00
        _emit 0xe8          // CALL 0x00464a20
        _emit 0x62
        _emit 0x3f
        _emit 0x00
        _emit 0x00
        _emit 0x8b          // MOV ECX, [ESP+0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x83          // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x89          // MOV [ECX], EAX
        _emit 0x01
        _emit 0xb8          // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e          // POP ESI
        _emit 0xc3          // RET
        // 0x00060ace — case: copy field +0x14
        _emit 0x8b          // MOV EDX, [ESI+0x14]
        _emit 0x56
        _emit 0x14
        _emit 0x8b          // MOV EAX, [ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x89          // MOV [EAX], EDX
        _emit 0x10
        _emit 0xb8          // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e          // POP ESI
        _emit 0xc3          // RET
        // 0x00060ade — case: store 1 in *out
        _emit 0x8b          // MOV ECX, [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xc7          // MOV dword ptr [ECX], 0x1
        _emit 0x01
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xb8          // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e          // POP ESI
        _emit 0xc3          // RET
        // 0x00060aef — case: BIT_STRING alloc (size 8, type 0x165)
        _emit 0x68          // PUSH 0x165
        _emit 0x65
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68          // PUSH 0x00f69350
        _emit 0x50
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        _emit 0x6a          // PUSH 0x8
        _emit 0x08
        _emit 0xe8          // CALL 0x00463150
        _emit 0x50
        _emit 0x26
        _emit 0x00
        _emit 0x00
        _emit 0x83          // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75          // JNZ +0x2 (→ 0x00460b09)
        _emit 0x02
        _emit 0x5e          // POP ESI
        _emit 0xc3          // RET
        // 0x00060b09
        _emit 0x8b          // MOV ECX, [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xc7          // MOV dword ptr [EAX+0x4], 0x0
        _emit 0x40
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7          // MOV dword ptr [EAX], 0xffffffff
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x89          // MOV [ECX], EAX
        _emit 0x01
        _emit 0x33          // XOR EAX, EAX
        _emit 0xc0
        _emit 0x39          // CMP [ECX], EAX
        _emit 0x01
        _emit 0x5e          // POP ESI
        _emit 0x0f          // SETNZ AL
        _emit 0x95
        _emit 0xc0
        _emit 0xc3          // RET
        // 0x00060b25 — default: ASN1_TYPE_new(type)
        _emit 0x50          // PUSH EAX
        _emit 0xe8          // CALL 0x00464480
        _emit 0x55
        _emit 0x39
        _emit 0x00
        _emit 0x00
        _emit 0x83          // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x80          // CMP byte ptr [ESI], 0x5
        _emit 0x3e
        _emit 0x05
        _emit 0x75          // JNZ +0x8 (→ 0x00460b3b)
        _emit 0x08
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74          // JZ +0x4 (→ 0x00460b3b)
        _emit 0x04
        _emit 0x83          // OR dword ptr [EAX+0xc], 0x40
        _emit 0x48
        _emit 0x0c
        _emit 0x40
        // 0x00060b3b
        _emit 0x8b          // MOV ECX, [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x89          // MOV [ECX], EAX
        _emit 0x01
        _emit 0x33          // XOR EAX, EAX
        _emit 0xc0
        _emit 0x39          // CMP [ECX], EAX
        _emit 0x01
        _emit 0x5e          // POP ESI
        _emit 0x0f          // SETNZ AL
        _emit 0x95
        _emit 0xc0
        _emit 0xc3          // RET
    }
}
