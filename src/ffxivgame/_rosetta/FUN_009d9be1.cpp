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
// FUNCTION: ffxivgame 0x005d9be1 — `__get_dstbias` (FID_conflict, 52 B / 0x34)
//           MSVC CRT function: retrieve the DST bias value.
//           If `pbias` is NULL: sets errno to EINVAL (22), calls
//           `_invalid_parameter` with five NULL/zero args, and returns EINVAL.
//           If `pbias` is non-NULL: stores the DST-bias global into *pbias
//           and returns 0.
//
// Calling convention: __cdecl (caller cleans; one DWORD stack arg; `ret`).
// Stack frame: PUSH ESI only (one callee-save).
//
// Callee identities (RVA in ffxivgame.exe):
//   FUN_009d9d47  — `__errno`             (returns int*)
//   FUN_009d2290  — `__invalid_parameter` (wchar_t*, wchar_t*, wchar_t*,
//                                          unsigned, uintptr_t)
//   __dstbias     — DST bias global       (int, VA 0x01364988)
//
// Notable encoding:
//   - ESI zeroed via XOR and reused as the constant 0 for all five
//     `_invalid_parameter` pushes, avoiding five separate `PUSH 0` / 5-byte
//     MOV sequences.
//   - Return of EINVAL encoded as `PUSH 0x16; POP EAX` (3 bytes) rather
//     than `MOV EAX, 0x16` (5 bytes) — MSVC size-optimised form.
//   - `MOV EAX, [ESP+4]` hoisted before `PUSH ESI` so the load uses the
//     pre-push stack offset (+4) instead of post-push (+8).
//
// Naked __asm so the PUSH/XOR/CMP interleave, the short-form JNZ, the
// five `PUSH ESI` zero-pushes, and the `PUSH 0x16; POP EAX` return form
// are pinned to the original encoding. REL32 call targets and the
// absolute MOV [addr32] are masked by tools/compare.py.

extern "C" int * __cdecl FUN_009d9d47(void);   // __errno
extern "C" void  __cdecl FUN_009d2290(         // __invalid_parameter
    const void *, const void *, const void *,
    unsigned int, unsigned int);
extern "C" int __dstbias;                       // DST-bias global @ VA 0x01364988

extern "C" __declspec(naked) void FUN_009d9be1()
{
    __asm {
        mov     eax, dword ptr [esp + 0x4]
        push    esi
        xor     esi, esi
        cmp     eax, esi
        jnz     success
        call    FUN_009d9d47
        push    esi
        push    esi
        push    esi
        push    esi
        push    esi
        mov     dword ptr [eax], 0x16
        call    FUN_009d2290
        add     esp, 0x14
        push    0x16
        pop     eax
        pop     esi
        ret
    success:
        mov     ecx, dword ptr [__dstbias]
        mov     dword ptr [eax], ecx
        xor     eax, eax
        pop     esi
        ret
    }
}
