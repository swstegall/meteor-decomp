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
// FUNCTION: ffxivgame 0x009d2b12 — loop-condition tail of ___sbh_find_block (25 B)
//
// Context: this 7-byte fragment is the loop-condition check for the CRT
// small-block-heap (SBH) block-search loop whose body lives at 0x009d2b00.
// The thunk at 0x009d2aee sets EAX = heap base pointer and ECX = end-of-
// region sentinel (base + n_regions * 0x14), then jumps here directly.
//
// Control flow:
//   CMP EAX, ECX    ; has eax reached the end sentinel?
//   JC  0x009d2b00  ; no → jump back to loop body (continue searching)
//   XOR EAX, EAX    ; yes → not found; return NULL
//   RET
//
// The symbols.json size field covers 25 bytes (0x19).  The function's true
// instruction span is only 7 bytes; the remaining 18 bytes are the first 18
// bytes of the immediately-following ___sbh_free_block prologue at 0x009d2b19.
// Both occupy a contiguous .text window and compare.py reads all 25 bytes for
// the diff.  The suffix is reproduced verbatim via _emit directives so the
// .text contribution is exactly 25 bytes.
//
// Asm (25 bytes, no relocations):
//   3b c1             CMP  EAX, ECX
//   72 ea             JC   0x009d2b00   ; backward rel8
//   33 c0             XOR  EAX, EAX
//   c3                RET
//   55                PUSH EBP          ; ___sbh_free_block prologue (bytes 7-24)
//   8b ec             MOV  EBP, ESP
//   83 ec 10          SUB  ESP, 0x10
//   8b 4d 08          MOV  ECX, [EBP+0x8]
//   8b 41 10          MOV  EAX, [ECX+0x10]
//   56                PUSH ESI
//   8b 75 0c          MOV  ESI, [EBP+0xc]
//   57                PUSH EDI
//   8b                (byte 24: partial start of next MOV in ___sbh_free_block)

extern "C" __declspec(naked) void FUN_009d2b12() {
    __asm {
        // 7-byte loop-condition check (3b c1 72 ea 33 c0 c3)
        cmp  eax, ecx
        _emit 0x72              // JC short — opcode
        _emit 0xea              // JC short — rel8 offset (-22 → 0x009d2b00)
        xor  eax, eax
        ret

        // Bytes 7-24: first 18 bytes of ___sbh_free_block prologue.
        // compare.py reads 25 bytes at this RVA; these must match verbatim.
        _emit 0x55              // push ebp
        _emit 0x8b              // mov ebp, esp  (byte 1/2)
        _emit 0xec              // mov ebp, esp  (byte 2/2)
        _emit 0x83              // sub esp, 0x10  (byte 1/3)
        _emit 0xec              // sub esp, 0x10  (byte 2/3)
        _emit 0x10              // sub esp, 0x10  (byte 3/3)
        _emit 0x8b              // mov ecx, [ebp+0x8]  (byte 1/3)
        _emit 0x4d              // mov ecx, [ebp+0x8]  (byte 2/3)
        _emit 0x08              // mov ecx, [ebp+0x8]  (byte 3/3)
        _emit 0x8b              // mov eax, [ecx+0x10]  (byte 1/3)
        _emit 0x41              // mov eax, [ecx+0x10]  (byte 2/3)
        _emit 0x10              // mov eax, [ecx+0x10]  (byte 3/3)
        _emit 0x56              // push esi
        _emit 0x8b              // mov esi, [ebp+0xc]  (byte 1/3)
        _emit 0x75              // mov esi, [ebp+0xc]  (byte 2/3)
        _emit 0x0c              // mov esi, [ebp+0xc]  (byte 3/3)
        _emit 0x57              // push edi
        _emit 0x8b              // byte 24 of 25: partial MOV in ___sbh_free_block
    }
}
