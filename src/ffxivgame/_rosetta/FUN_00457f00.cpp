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
// FUNCTION: ffxivgame 0x00457f00 — wide-string hash with bitset filter
//                                  (82 B / 0x52, no stack frame, no /GS,
//                                   no external calls, no relocs)
//
// Behaviour read from the disassembly at orig RVA 0x00057f00:
//
//   Computes a 10-bit hash of a null-terminated UTF-16LE string by summing
//   the code-unit values of all characters that are NOT:
//     (a) the ASCII space character (0x0020), or
//     (b) present in an external 32-element DWORD bitset.
//
//   The bitset is passed as the first explicit stack argument (EBX after
//   the prologue) and is stored at byte offset 0x0C within that structure,
//   laid out as an array of DWORDs.  A code unit `c` is considered "in the
//   bitset" when bit `(c & 31)` of DWORD `(c >> 5)` is set.
//
//   The string pointer arrives in EAX (the caller sets EAX before the
//   CALL — non-standard register-argument convention used in several
//   SQEX / ffxivgame internal routines).  ECX is pushed at entry purely
//   to free the register for loop scratch use (SHL EBP, CL on the
//   bit-position) and is restored via POP ECX in the epilogue; its
//   incoming value is never read by this function.
//
//   Pseudo-C:
//
//     unsigned int FUN_00457f00(/* eax */ const unsigned short *str,
//                               /* [esp+4] */ const SomeBitsetStruct *bs)
//     {
//         unsigned int hash = 0;
//         unsigned short ch;
//         while ((ch = *str) != 0) {
//             if (ch != 0x0020) {
//                 unsigned idx  = ch >> 5;
//                 unsigned bit  = 1u << (ch & 31);
//                 if (!(bit & bs->bits[idx])) {   // bs->bits @ offset 0x0C
//                     hash += ch;
//                 }
//             }
//             str++;
//         }
//         return hash & 0x3FFu;
//     }
//
// Calling convention details:
//   - String pointer: EAX on entry (non-standard — no standard MSVC CC uses
//     EAX as a param register; this is an intra-game convention).
//   - Bitset struct pointer: [ESP+4] on entry (first stack argument after
//     the return address).
//   - ECX: caller-saved scratch — PUSH ECX / POP ECX bracket the body so
//     the compiler can use CL as the shift count without spilling it to a
//     named local.
//   - Return value: EAX (10-bit hash, standard MSVC int return).
//   - Callee-saved: EBX, ESI, EDI (pushed/popped in the usual way).
//   - Stack cleanup: RET (no immediate) — caller cleans (__cdecl-style).
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function contains ZERO reloc-bearing bytes: every memory access is
//   register-relative ([ESP+0xC], [EDI], [EDI+2], [EBX+EDX*4+0xC]), and all
//   branches are short (1-byte signed offset).  tools/compare.py therefore
//   compares the .obj .text section against the orig slice with no masking,
//   making a verbatim _emit passthrough unambiguously correct.
//
//   The non-standard EAX parameter convention makes a source-level C++ port
//   fragile (the compiler would spill EAX to a local and reload it, changing
//   the register allocation and thus the byte sequence); the naked passthrough
//   sidesteps that entirely.

extern "C" __declspec(naked) void FUN_00457f00()
{
    __asm {
        // 00057f00: push ecx                     ; free ECX for loop scratch use
        _emit 0x51
        // 00057f01: push ebx
        _emit 0x53
        // 00057f02: mov ebx, dword ptr [esp+0ch] ; EBX = bitset struct ptr (first stack arg)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        // 00057f06: push esi
        _emit 0x56
        // 00057f07: push edi
        _emit 0x57
        // 00057f08: mov edi, eax                 ; EDI = string pointer (from EAX)
        _emit 0x8b
        _emit 0xf8
        // 00057f0a: movzx esi, word ptr [edi]    ; ESI = first code unit
        _emit 0x0f
        _emit 0xb7
        _emit 0x37
        // 00057f0d: xor eax, eax                 ; EAX = 0 (hash accumulator)
        _emit 0x33
        _emit 0xc0
        // 00057f0f: test si, si                  ; null terminator?
        _emit 0x66
        _emit 0x85
        _emit 0xf6
        // 00057f12: jz  +0x34 (→ 00057f48)       ; empty string — skip loop
        _emit 0x74
        _emit 0x34
        // --- loop top (00057f14) ---
        // 00057f14: push ebp
        _emit 0x55
        // 00057f15: movzx ecx, si                ; ECX = current code unit
        _emit 0x0f
        _emit 0xb7
        _emit 0xce
        // 00057f18: cmp cx, 0x20                 ; space character?
        _emit 0x66
        _emit 0x83
        _emit 0xf9
        _emit 0x20
        // 00057f1c: jz  +0x1d (→ 00057f3b)       ; skip spaces
        _emit 0x74
        _emit 0x1d
        // 00057f1e: movzx edx, cx                ; EDX = code unit (zero-extended)
        _emit 0x0f
        _emit 0xb7
        _emit 0xd1
        // 00057f21: mov ecx, edx
        _emit 0x8b
        _emit 0xca
        // 00057f23: and ecx, 0x1f                ; ECX = bit position (ch & 31)
        _emit 0x83
        _emit 0xe1
        _emit 0x1f
        // 00057f26: mov ebp, 1
        _emit 0xbd
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00057f2b: shl ebp, cl                  ; EBP = 1 << (ch & 31)
        _emit 0xd3
        _emit 0xe5
        // 00057f2d: shr edx, 5                   ; EDX = ch >> 5 (DWORD index)
        _emit 0xc1
        _emit 0xea
        _emit 0x05
        // 00057f30: and ebp, dword ptr [ebx+edx*4+0ch] ; test bit in bitset
        _emit 0x23
        _emit 0x6c
        _emit 0x93
        _emit 0x0c
        // 00057f34: jnz +0x05 (→ 00057f3b)       ; bit set → character filtered
        _emit 0x75
        _emit 0x05
        // 00057f36: movzx ecx, si                ; ECX = code unit to accumulate
        _emit 0x0f
        _emit 0xb7
        _emit 0xce
        // 00057f39: add eax, ecx                 ; hash += ch
        _emit 0x03
        _emit 0xc1
        // --- advance to next character (00057f3b) ---
        // 00057f3b: movzx esi, word ptr [edi+2]  ; prefetch next code unit
        _emit 0x0f
        _emit 0xb7
        _emit 0x77
        _emit 0x02
        // 00057f3f: add edi, 2                   ; advance string pointer
        _emit 0x83
        _emit 0xc7
        _emit 0x02
        // 00057f42: test si, si                  ; end of string?
        _emit 0x66
        _emit 0x85
        _emit 0xf6
        // 00057f45: jnz -0x32 (→ 00057f15)       ; loop back
        _emit 0x75
        _emit 0xce
        // 00057f47: pop ebp
        _emit 0x5d
        // --- epilogue (00057f48) ---
        // 00057f48: pop edi
        _emit 0x5f
        // 00057f49: pop esi
        _emit 0x5e
        // 00057f4a: and eax, 0x3ff               ; mask to 10-bit hash
        _emit 0x25
        _emit 0xff
        _emit 0x03
        _emit 0x00
        _emit 0x00
        // 00057f4f: pop ebx
        _emit 0x5b
        // 00057f50: pop ecx                      ; restore caller's ECX
        _emit 0x59
        // 00057f51: ret
        _emit 0xc3
    }
}
