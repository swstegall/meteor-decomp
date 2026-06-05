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
// FUNCTION: ffxivgame 0x0045bef0 — int_thread_release (53 bytes)
//
// Releases an OpenSSL-managed integer "thread" handle stored at *pHandle.
// The callee at RVA 0x66000 is _CRYPTO_add_lock (OpenSSL reference-count
// decrement: CRYPTO_add_lock(pointer, amount=-1, type=1, file, line)).
// The fixed refcount lives at VA 0x132e794; file=0xf6802c; line=0x1d5=469.
//
// Logic:
//   - Return immediately if pHandle is NULL or *pHandle == 0.
//   - Call CRYPTO_add_lock with the above fixed args (decrements refcount).
//   - If the return value <= 0 (refcount exhausted), zero out *pHandle.
//
// Calling convention : __cdecl (one stack arg, caller cleans).
// Frame             : PUSH ESI only (no SUB ESP).
// Branch shape      : two early-exit JZ tests; one JG skip over the clear.
//
// Reconstruction: __declspec(naked) to pin the exact branch encodings,
// push-immediate sizes, and register usage.  The CALL rel32 to
// CRYPTO_add_lock (byte +0x21) is masked by tools/compare.py.

extern "C" int CRYPTO_add_lock(int *, int, int, const char *, int);

extern "C" __declspec(naked) void FUN_0045bef0() {
    __asm {
        push    esi
        mov     esi, dword ptr [esp + 8]
        test    esi, esi
        jz      done
        cmp     dword ptr [esi], 0
        jz      done
        push    0x1d5
        push    0xf6802c
        push    1
        push    -1
        push    0x132e794
        call    CRYPTO_add_lock
        add     esp, 0x14
        test    eax, eax
        jg      done
        mov     dword ptr [esi], 0
    done:
        pop     esi
        ret
    }
}
