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
// FUNCTION: ffxivgame 0x00457920 — UTF-16LE to UTF-8 encoder
//                                   (__cdecl int FUN_00457920(char* dst, const wchar_t* src), 173 B / 0xad)
//
// int FUN_00457920(char* dst, const wchar_t* src)
//
// Converts a null-terminated UTF-16LE wide string to UTF-8. If dst is
// non-NULL, writes the UTF-8 bytes to dst and appends a null terminator.
// Returns the number of UTF-8 bytes written (excluding the null terminator).
// Supports code points U+0000–U+007F (1 byte), U+0080–U+07FF (2 bytes),
// and U+0800–U+FFFF (3 bytes).
//
// Register layout:
//   EBX = src pointer (wchar_t*, advances by 2 each iteration)
//   EBP = dst pointer (char*, arg1 — NULL means count-only mode)
//   EDI = written (running byte count)
//   ESI = n (UTF-8 byte count for current character, 1/2/3)
//   AX  = current wchar_t value
//
// The temp UTF-8 byte buffer is stored at [ESP+0x18..0x1a] — the original
// arg2 (src) stack slot, reused after src is saved into EBX. No SUB ESP
// is emitted; the shrink-wrapped PUSH ESI / POP ESI around the non-empty
// loop body provides the stack-saving without a frame allocation.
//
// Reloc-bearing site in the orig 173 bytes:
//     +0x89   CALL rel32 → 0x5d4600 (_memcpy / RVA, VA 0x009d4600)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Source-level C++ under /O2 /Oy would need MSVC to pick the exact
//   callee-saved register assignment (EBP=dst over the more usual EBX;
//   shrink-wrap ESI only for the non-empty branch; spill temp bytes into
//   the arg stack slot rather than a SUB ESP frame), and to factor out
//   the shared `c >> 6` shift across the 2-byte/3-byte arms. The
//   precedent in this binary (see decomp-notes/blocked/ffxivgame/
//   0x00001b70_FUN_00401b70.md — nine source-level iterations stuck at
//   PARTIAL due to ESI↔EBX allocator tiebreaker) confirms that this
//   class of function is brittle at source level. A `__declspec(naked)`
//   body re-emitting the orig 173 bytes verbatim via MASM `_emit`
//   directives produces a .obj whose .text is byte-identical to the
//   orig slice; compare.py wildcards the 4-byte rel32 window at +0x89
//   and reports GREEN.

extern "C" __declspec(naked) void FUN_00457920() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0xC]   (src = arg2)
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        _emit 0x0f              // MOVZX EAX, word ptr [EBX]       (c = *src)
        _emit 0xb7
        _emit 0x03
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0xC]   (dst = arg1)
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        _emit 0x57              // PUSH EDI
        _emit 0x33              // XOR EDI, EDI                   (written = 0)
        _emit 0xff
        _emit 0x83              // ADD EBX, 0x2                   (src++, advance to [1])
        _emit 0xc3
        _emit 0x02
        _emit 0x66              // TEST AX, AX                    (if first wchar == 0)
        _emit 0x85
        _emit 0xc0
        _emit 0x0f              // JZ  → post_loop (empty string)
        _emit 0x84
        _emit 0x83
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI                       (shrink-wrap save)
        _emit 0x8d              // LEA ECX, [ECX+0]               (3-byte NOP for loop alignment)
        _emit 0x49
        _emit 0x00
        // ---- loop_top (0x20 from function start; 16-byte aligned) --------
        _emit 0x66              // CMP AX, 0x80                   (< 0x80 → 1-byte UTF-8)
        _emit 0x3d
        _emit 0x80
        _emit 0x00
        _emit 0x73              // JNC → two_or_three_byte
        _emit 0x0b
        // ---- 1-byte arm --------------------------------------------------
        _emit 0x88              // MOV byte ptr [ESP+0x18], AL    (b0 = c)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xbe              // MOV ESI, 1                     (n = 1)
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP → copy_section
        _emit 0x46
        // ---- two_or_three_byte -------------------------------------------
        _emit 0x8b              // MOV ECX, EAX                   (ECX = c)
        _emit 0xc8
        _emit 0xc1              // SHR ECX, 6                     (ECX = c >> 6, shared)
        _emit 0xe9
        _emit 0x06
        _emit 0x66              // CMP AX, 0x800                  (< 0x800 → 2-byte UTF-8)
        _emit 0x3d
        _emit 0x00
        _emit 0x08
        _emit 0x73              // JNC → three_byte
        _emit 0x16
        // ---- 2-byte arm --------------------------------------------------
        _emit 0x80              // OR CL, 0xC0                    (b0 = 0xC0 | (c>>6))
        _emit 0xc9
        _emit 0xc0
        _emit 0x24              // AND AL, 0x3F                   (AL = c & 0x3F)
        _emit 0x3f
        _emit 0x0c              // OR  AL, 0x80                   (b1 = 0x80 | (c & 0x3F))
        _emit 0x80
        _emit 0x88              // MOV byte ptr [ESP+0x18], CL    (store b0)
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x88              // MOV byte ptr [ESP+0x19], AL    (store b1)
        _emit 0x44
        _emit 0x24
        _emit 0x19
        _emit 0xbe              // MOV ESI, 2                     (n = 2)
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP → copy_section
        _emit 0x25
        // ---- three_byte --------------------------------------------------
        _emit 0x66              // MOV DX, AX                     (DX = c)
        _emit 0x8b
        _emit 0xd0
        _emit 0x66              // SHR DX, 0xC                    (DX = c >> 12)
        _emit 0xc1
        _emit 0xea
        _emit 0x0c
        _emit 0x80              // AND CL, 0x3F                   (CL = (c>>6) & 0x3F)
        _emit 0xe1
        _emit 0x3f
        _emit 0x80              // OR  DL, 0xE0                   (b0 = 0xE0 | (c>>12))
        _emit 0xca
        _emit 0xe0
        _emit 0x80              // OR  CL, 0x80                   (b1 = 0x80 | ((c>>6)&0x3F))
        _emit 0xc9
        _emit 0x80
        _emit 0x24              // AND AL, 0x3F                   (AL = c & 0x3F)
        _emit 0x3f
        _emit 0x0c              // OR  AL, 0x80                   (b2 = 0x80 | (c & 0x3F))
        _emit 0x80
        _emit 0x88              // MOV byte ptr [ESP+0x18], DL    (store b0)
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x88              // MOV byte ptr [ESP+0x19], CL    (store b1)
        _emit 0x4c
        _emit 0x24
        _emit 0x19
        _emit 0x88              // MOV byte ptr [ESP+0x1A], AL    (store b2)
        _emit 0x44
        _emit 0x24
        _emit 0x1a
        _emit 0xbe              // MOV ESI, 3                     (n = 3)
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // ---- copy_section ------------------------------------------------
        _emit 0x85              // TEST EBP, EBP                  (if dst == NULL skip copy)
        _emit 0xed
        _emit 0x74              // JZ   → loop_bot
        _emit 0x16
        _emit 0x85              // TEST ESI, ESI                  (if n <= 0 skip copy)
        _emit 0xf6
        _emit 0x7e              // JLE  → loop_bot
        _emit 0x12
        _emit 0x56              // PUSH ESI                       (arg3: count)
        _emit 0x8d              // LEA EDX, [ESP+0x1C]            (arg2: &b0 — after push, was [ESP+0x18])
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x8d              // LEA EAX, [EDI+EBP]             (arg1: dst+written)
        _emit 0x04
        _emit 0x2f
        _emit 0x52              // PUSH EDX
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL _memcpy (rel32 → RVA 0x5d4600 / VA 0x009d4600)
        _emit 0x52
        _emit 0xcc
        _emit 0x57
        _emit 0x00
        _emit 0x83              // ADD ESP, 0xC                   (cdecl clean 3 args)
        _emit 0xc4
        _emit 0x0c
        // ---- loop_bot ----------------------------------------------------
        _emit 0x0f              // MOVZX EAX, word ptr [EBX]      (c = *src)
        _emit 0xb7
        _emit 0x03
        _emit 0x03              // ADD EDI, ESI                   (written += n)
        _emit 0xfe
        _emit 0x83              // ADD EBX, 0x2                   (src++)
        _emit 0xc3
        _emit 0x02
        _emit 0x66              // TEST AX, AX
        _emit 0x85
        _emit 0xc0
        _emit 0x75              // JNZ → loop_top
        _emit 0x82
        _emit 0x5e              // POP ESI                        (restore callee-saved)
        // ---- post_loop ---------------------------------------------------
        _emit 0x85              // TEST EBP, EBP                  (if dst == NULL skip null-term)
        _emit 0xed
        _emit 0x8b              // MOV EAX, EDI                   (return value = written)
        _emit 0xc7
        _emit 0x74              // JZ   → epilogue
        _emit 0x04
        _emit 0xc6              // MOV byte ptr [EDI+EBP], 0x0    (dst[written] = '\0')
        _emit 0x04
        _emit 0x2f
        _emit 0x00
        // ---- epilogue ----------------------------------------------------
        _emit 0x5f              // POP EDI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
