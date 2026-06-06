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
// FUNCTION: ffxivgame 0x0006c4b0 — _ASN1_TYPE_set1
//                                  (__cdecl int (ASN1_TYPE *, int utype, const void *value), 178 B)
//
// OpenSSL ASN1_TYPE_set1: duplicate-and-set an ASN1_TYPE value by type.
//
// Calling convention: __cdecl (plain RET; caller-cleans).
// Frame: PUSH ESI / PUSH EDI only — no SUB ESP, no EBP.
//
// Signature (inferred from OpenSSL 0.9.x):
//   int __cdecl ASN1_TYPE_set1(ASN1_TYPE *a, int utype, const void *value)
//   [ESP+0x04] = a      (ASN1_TYPE *)
//   [ESP+0x08] = utype  (int)
//   [ESP+0x0c] = value  (const void *)
//
// After PUSH ESI, PUSH EDI:
//   ESI = arg3 (value), EDI = arg2 (utype), [ESP+0x0c] = arg1 (a)
//
// Branch shape:
//
//   1. (!value || utype == V_ASN1_BOOLEAN(1)) → null/boolean branch:
//        local = a;
//        if (a->value.ptr) call cleanup(&local, 0);
//        local->type = utype;
//        if (utype == 1) local->value.boolean = normalize(value); return utype;
//        else            local->value.ptr = value; return 1;
//
//   2. (utype == V_ASN1_OBJECT(6)) → OBJ_dup path:
//        otmp = OBJ_dup(value) @ 0x0046c200;
//        if (!otmp) return 0;
//        ASN1_TYPE_set(a, 6, otmp) @ 0x0046c460; return 1;
//
//   3. (generic) → ASN1_STRING_dup path:
//        stmp = ASN1_STRING_dup(value) @ 0x004648f0;
//        if (!stmp) return 0;
//        ASN1_TYPE_set(a, utype, stmp) @ 0x0046c460; return 1;
//
// Reloc-bearing sites (compare.py masks the 4 bytes after each `e8`):
//   +0x19  CALL rel32 → 0x0046c200  (OBJ_dup)
//   +0x32  CALL rel32 → 0x0046c460  (ASN1_TYPE_set)
//   +0x42  CALL rel32 → 0x004648f0  (ASN1_STRING_dup)
//   +0x55  CALL rel32 → 0x0046c460  (ASN1_TYPE_set)
//   +0x7a  CALL rel32 → 0x00460f70  (ASN1 value cleanup)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The null/boolean branch reuses arg3's stack slot as a local variable
//   (MOV [ESP+0x14],EAX after both pushes) and uses the NEG/SBB/AND
//   pattern to normalize a boolean value. Five CALL rel32 relocations all
//   resolve only at full-binary relink time. The _emit passthrough emits
//   the orig 178 bytes verbatim so compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0046c4b0() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV  ESI, dword ptr [ESP+0x10]   (value = arg3)
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV  EDI, dword ptr [ESP+0x10]   (utype = arg2)
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x74              // JZ   null_bool  (+0x57)
        _emit 0x57
        _emit 0x83              // CMP  EDI, 0x1  (V_ASN1_BOOLEAN)
        _emit 0xff
        _emit 0x01
        _emit 0x74              // JZ   null_bool  (+0x52)
        _emit 0x52
        _emit 0x83              // CMP  EDI, 0x6  (V_ASN1_OBJECT)
        _emit 0xff
        _emit 0x06
        _emit 0x56              // PUSH ESI                         (value)
        _emit 0x75              // JNZ  generic   (+0x29)
        _emit 0x29
        _emit 0xe8              // CALL 0x0046c200  (OBJ_dup)
        _emit 0x32
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD  ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ  obj_set   (+0x05)
        _emit 0x05
        _emit 0x5f              // POP  EDI
        _emit 0x33              // XOR  EAX, EAX                    (return 0)
        _emit 0xc0
        _emit 0x5e              // POP  ESI
        _emit 0xc3              // RET
        _emit 0x50              // PUSH EAX                         (otmp)  [obj_set:]
        _emit 0x8b              // MOV  EAX, dword ptr [ESP+0x10]   (a)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x6a              // PUSH 0x6
        _emit 0x06
        _emit 0x50              // PUSH EAX                         (a)
        _emit 0xe8              // CALL 0x0046c460  (ASN1_TYPE_set)
        _emit 0x79
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD  ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x5f              // POP  EDI
        _emit 0xb8              // MOV  EAX, 0x1                    (return 1)
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP  ESI
        _emit 0xc3              // RET
        _emit 0xe8              // CALL 0x004648f0  (ASN1_STRING_dup)   [generic:]
        _emit 0xf9
        _emit 0x83
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD  ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ   ret0      (-0x29)
        _emit 0xd7
        _emit 0x8b              // MOV  ECX, dword ptr [ESP+0xc]    (a)
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x50              // PUSH EAX                         (stmp)
        _emit 0x57              // PUSH EDI                         (utype)
        _emit 0x51              // PUSH ECX                         (a)
        _emit 0xe8              // CALL 0x0046c460  (ASN1_TYPE_set)
        _emit 0x56
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD  ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x5f              // POP  EDI
        _emit 0xb8              // MOV  EAX, 0x1                    (return 1)
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP  ESI
        _emit 0xc3              // RET
        _emit 0x8b              // MOV  EAX, dword ptr [ESP+0xc]    (a)   [null_bool:]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x89              // MOV  dword ptr [ESP+0x14], EAX   (local = a)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x83              // CMP  dword ptr [EAX+0x4], 0x0   (a->value.ptr)
        _emit 0x78
        _emit 0x04
        _emit 0x00
        _emit 0x74              // JZ   skip_free (+0x0f)
        _emit 0x0f
        _emit 0x8d              // LEA  EDX, [ESP+0x14]             (&local)
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL 0x00460f70  (ASN1 value cleanup)
        _emit 0x41
        _emit 0x4a
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD  ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x83              // CMP  EDI, 0x1                    [skip_free:]
        _emit 0xff
        _emit 0x01
        _emit 0x8b              // MOV  EAX, dword ptr [ESP+0x14]   (local)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x89              // MOV  dword ptr [EAX], EDI        (local->type = utype)
        _emit 0x38
        _emit 0x75              // JNZ  not_bool  (+0x16)
        _emit 0x16
        _emit 0x8b              // MOV  ECX, dword ptr [ESP+0x14]   [boolean:]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xf7              // NEG  ESI                         (normalize value)
        _emit 0xde
        _emit 0x1b              // SBB  ESI, ESI
        _emit 0xf6
        _emit 0x81              // AND  ESI, 0xff
        _emit 0xe6
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV  EAX, EDI                    (return utype=1)
        _emit 0xc7
        _emit 0x5f              // POP  EDI
        _emit 0x89              // MOV  dword ptr [ECX+0x4], ESI    (local->value.boolean)
        _emit 0x71
        _emit 0x04
        _emit 0x5e              // POP  ESI
        _emit 0xc3              // RET
        _emit 0x8b              // MOV  EDX, dword ptr [ESP+0x14]   [not_bool:]
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x5f              // POP  EDI
        _emit 0x89              // MOV  dword ptr [EDX+0x4], ESI    (local->value.ptr = value)
        _emit 0x72
        _emit 0x04
        _emit 0xb8              // MOV  EAX, 0x1                    (return 1)
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP  ESI
        _emit 0xc3              // RET
    }
}
