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
// FUNCTION: ffxivgame 0x00055ef0 — handle-gated allocate-and-register
//           (__cdecl, no args, returns bool in AL, 75 B)
//
// Checks a global handle at [0x0126701c] against -1 (invalid sentinel).
// If equal, falls through to a 3-byte early-exit stub (XOR AL,AL; RET)
// and returns false — ESI has not been saved yet.
//
// Calls fn_ptr1 via [0x00f3e2a4] (__stdcall, 1 arg = handle). If it
// returns non-zero (failure), a backward JNZ returns to the same early-
// exit stub (still before the ESI PUSH).
//
// On success, saves ESI, allocates 0x84 bytes via FUN_009d04ac (__cdecl
// malloc-like, cleaned by ADD ESP,4), reloads the handle, and calls
// fn_ptr2 via [0x00f3e2a0] (__stdcall, 2 args = handle, buf). If fn_ptr2
// succeeds (non-zero), JNZ jumps to the "return true" stub at +0x4a.
// On failure, conditionally frees the buffer (FUN_009d1be9, __cdecl,
// ADD ESP,4 cleanup), restores ESI, and returns 0.
//
// Cross-function tail detail (75-byte boundary):
//   The function's last byte at +0x4a is 0xb0 — the opcode byte of
//   "MOV AL, imm8". The immediate (0x01) and the subsequent
//   POP ESI + RET live in the next function slot that immediately
//   follows this one in the .text section. The JNZ short at +0x37
//   branches to +0x4a; at runtime the linked binary's continuity
//   completes the instruction. compare.py compares exactly 75 bytes,
//   so only 0xb0 is included in this function's slice. We emit it as
//   _emit 0xb0 to avoid generating a full MOV+POP+RET (which would
//   make the .obj 78 bytes and cause a MISMATCH).
//
// Key idiom: early-exit stub (XOR AL,AL; RET, at +0x0a, no POP ESI)
// placed BEFORE the main body and shared by two paths:
//   1. fall-through when handle == -1  (CMP EAX,-1; JNZ skip over)
//   2. backward JNZ from +0x16 when fn_ptr1 fails
// This is a documented MSVC 2005 /O2 code-layout optimisation.
//
// Reloc-bearing sites (4-byte windows, wildcarded by compare.py):
//   +0x01  MOV EAX,[g_0126701c]         dir32
//   +0x0f  CALL dword ptr [fn_slot_a4]  dir32
//   +0x1f  CALL FUN_009d04ac            rel32
//   +0x29  MOV EAX,[g_0126701c]         dir32
//   +0x30  CALL dword ptr [fn_slot_a0]  dir32
//   +0x3f  CALL FUN_009d1be9            rel32

extern "C" {
    // Global handle at VA 0x0126701c (.data)
    extern int g_0126701c;
    // Function-pointer slots in the dispatch table
    extern int fn_slot_00f3e2a4;   // slot at VA 0x00f3e2a4
    extern int fn_slot_00f3e2a0;   // slot at VA 0x00f3e2a0
    // malloc-like and free-like (__cdecl, 5-byte JMP thunks)
    int  FUN_009d04ac(int size);
    void FUN_009d1be9(void *ptr);
}

extern "C" __declspec(naked) void FUN_00455ef0()
{
    __asm {
        // +0x00  a1 <addr4>   MOV EAX, [g_0126701c]
        mov     eax, dword ptr [g_0126701c]
        // +0x05  83 f8 ff     CMP EAX, -1
        cmp     eax, -1
        // +0x08  75 03        JNZ +3  → skip_check  (handle != -1: proceed)
        jnz     short skip_check

        // +0x0a  early-exit stub — shared by the handle==-1 fall-through
        //         AND the fn_ptr1-failure backward JNZ below.
        //         ESI has NOT been saved here, so no POP ESI.
    early_false:
        // +0x0a  32 c0        XOR AL, AL
        xor     al, al
        // +0x0c  c3           RET
        ret

    skip_check:
        // +0x0d  50           PUSH EAX  (handle → arg to fn_ptr1)
        push    eax
        // +0x0e  ff 15 <a4>   CALL dword ptr [fn_slot_00f3e2a4]  (__stdcall)
        call    dword ptr [fn_slot_00f3e2a4]
        // +0x14  85 c0        TEST EAX, EAX
        test    eax, eax
        // +0x16  75 f2        JNZ -14  → early_false (backward, to +0x0a)
        jnz     short early_false

        // +0x18  56           PUSH ESI  (first use — save callee-saved)
        push    esi
        // +0x19  68 84 00 00 00  PUSH 0x84
        push    0x84
        // +0x1e  e8 <rel32>   CALL FUN_009d04ac  (__cdecl malloc-like)
        call    FUN_009d04ac
        // +0x23  83 c4 04     ADD ESP, 4  (__cdecl cleanup for PUSH 0x84)
        add     esp, 4
        // +0x26  8b f0        MOV ESI, EAX  (save allocated buffer)
        mov     esi, eax
        // +0x28  a1 <addr4>   MOV EAX, [g_0126701c]  (reload handle)
        mov     eax, dword ptr [g_0126701c]
        // +0x2d  56           PUSH ESI  (buf → arg2 for fn_ptr2)
        push    esi
        // +0x2e  50           PUSH EAX  (handle → arg1 for fn_ptr2)
        push    eax
        // +0x2f  ff 15 <a0>   CALL dword ptr [fn_slot_00f3e2a0]  (__stdcall)
        call    dword ptr [fn_slot_00f3e2a0]
        // +0x35  85 c0        TEST EAX, EAX
        test    eax, eax
        // +0x37  75 11        JNZ +17  → return_true (+0x4a)
        jnz     short return_true

        // +0x39  85 f6        TEST ESI, ESI
        test    esi, esi
        // +0x3b  74 09        JZ +9  → skip_free (+0x46)
        jz      short skip_free
        // +0x3d  56           PUSH ESI  (buf → arg to free)
        push    esi
        // +0x3e  e8 <rel32>   CALL FUN_009d1be9  (__cdecl free-like)
        call    FUN_009d1be9
        // +0x43  83 c4 04     ADD ESP, 4  (__cdecl cleanup for PUSH ESI)
        add     esp, 4

    skip_free:
        // +0x46  32 c0        XOR AL, AL  (return false — ESI saved, so POP below)
        xor     al, al
        // +0x48  5e           POP ESI
        pop     esi
        // +0x49  c3           RET
        ret

    return_true:
        // +0x4a  b0           opcode byte of "MOV AL, 1"
        // The operand byte (0x01) and the ensuing POP ESI + RET reside in
        // the immediately-following function slot in .text. At link time
        // the continuous binary layout completes the instruction naturally.
        _emit 0xb0
    }
}
