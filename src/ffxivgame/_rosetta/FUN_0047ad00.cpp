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
// FUNCTION: ffxivgame 0x0047ad00 — _EC_POINT_set_to_infinity (87 bytes)
//
// OpenSSL EC_POINT_set_to_infinity: validates that the group's method has a
// point_set_to_infinity function pointer and that the point belongs to the
// same group, then dispatches to the method's implementation.
//
// EC_GROUP layout (relevant fields):
//   +0x00  EC_METHOD *meth       (pointer to method table)
//   ...
// EC_METHOD layout (relevant offset):
//   +0x30  copy                  (EC_POINT_copy function pointer)
//   +0x34  point_set_to_infinity (function pointer for this call)
//
// EC_POINT layout (relevant field):
//   +0x00  EC_METHOD *meth       (must match group->meth)
//
// Signature:
//   int __cdecl _EC_POINT_set_to_infinity(EC_GROUP *group, EC_POINT *point)
//
// Calling convention: __cdecl (ret with no imm16; two DWORD args on stack).
//   After PUSH ESI: [ESP+0x4]=ret, [ESP+0x8]=group, [ESP+0xC]=point
//
// Register allocation:
//   ESI = group   (arg1, loaded from [ESP+0x8])
//   EAX = group->meth
//   ECX = group->meth->point_set_to_infinity  (function pointer)
//   EDX = point   (arg2, loaded from [ESP+0xC] at the second check)
//
// Control flow:
//   1. Load group->meth (EAX) and group->meth->point_set_to_infinity (ECX).
//   2. If ECX == NULL: ERR_put_error(0x10, 0x7f, 0x42, file, 0x31d), return 0.
//   3. Load EDX = point, compare group->meth (EAX) with point->meth ([EDX]).
//   4. If not equal: ERR_put_error(0x10, 0x7f, 0x65, file, 0x322), return 0.
//   5. Call ECX(ESI, EDX) = point_set_to_infinity(group, point), return result.
//
// Reloc: two CALL FUN_0045c940 (ERR_put_error) at +0x1e and +0x42 are
// REL32 relocations masked by tools/compare.py.

extern "C" void FUN_0045c940();   // ERR_put_error(lib, func, reason, file, line)

extern "C" __declspec(naked) void FUN_0047ad00() {
    __asm {
        push    esi
        mov     esi, dword ptr [esp + 0x8]      ; group (arg1)
        mov     eax, dword ptr [esi]             ; group->meth
        mov     ecx, dword ptr [eax + 0x34]      ; ->point_set_to_infinity
        test    ecx, ecx
        jnz     meth_ok
        push    0x31d                            ; line number
        push    0xf7b4f8                         ; file string
        push    0x42                             ; reason
        push    0x7f                             ; func
        push    0x10                             ; lib
        call    FUN_0045c940
        add     esp, 0x14
        xor     eax, eax
        pop     esi
        ret
    meth_ok:
        mov     edx, dword ptr [esp + 0xc]       ; point (arg2)
        cmp     eax, dword ptr [edx]             ; group->meth == point->meth?
        jz      same_meth
        push    0x322                            ; line number
        push    0xf7b4f8                         ; file string
        push    0x65                             ; reason
        push    0x7f                             ; func
        push    0x10                             ; lib
        call    FUN_0045c940
        add     esp, 0x14
        xor     eax, eax
        pop     esi
        ret
    same_meth:
        push    edx                              ; point
        push    esi                              ; group
        call    ecx                              ; ->point_set_to_infinity(group, point)
        add     esp, 0x8
        pop     esi
        ret
    }
}
