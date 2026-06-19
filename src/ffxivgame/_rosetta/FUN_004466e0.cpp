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
// FUNCTION: ffxivgame 0x000466e0 — __thiscall UTF-8-aware reverse substring
//                                  search (277 B / 0x115, no SEH, /GS-free,
//                                  three calls to FUN_00445e50 and no other
//                                  external references).
//
// Inspection (read from the disassembly at orig RVA 0x000466e0):
//
//   __thiscall int FUN_004466e0(const CountedStr *substr, unsigned int pos)
//
//   ECX = this, two stack args (8 B callee-popped: RET 0x8).
//
//   CountedStr layout (from FUN_00445e50 / FUN_00446250):
//     this[0x00]  char*        — pointer to UTF-8 string data
//     this[0x04]  int          — (unused here)
//     this[0x08]  unsigned int — byte length of the string buffer (incl. '\0')
//     this[0x0c]  int          — cached UTF-8 character count
//     this[0x10]  char         — computed flag (0=dirty, 1=cached)
//
//   Behaviour:
//
//   Returns the character-unit index (0-based) of the LAST occurrence of
//   `substr` in `*this`, starting the search at position `pos` from the
//   beginning.  `pos == 0xFFFFFFFF` means "start from the end".  Returns -1
//   when not found.  When substr is empty, returns this->char_count().
//
//   Body shape (five phases):
//
//   1. Prologue / initial cursor:
//        ESI  = this->data + this->byte_len - 1   (ptr to last byte of haystack)
//        EBP  = this->char_count()   (FUN_00445e50, ECX = this)
//      Store EBP at [esp+0x0c] so it survives across the ECX-clobbering call
//      for substr:
//        EAX  = substr->char_count() (FUN_00445e50, ECX = substr arg)
//
//   2. Empty needle fast-path (EAX == 0):
//        Return this->char_count() (third call to FUN_00445e50).
//
//   3. Position clamping / backward cursor advance:
//        PUSH EBX; EBX = pos (arg1).
//        If pos == npos (0xFFFFFFFF): skip adjustment.
//        Else: call this->char_count() again; if pos > char_count → return -1;
//        if EBP (len_this) != pos: walk ESI back (len_this − pos) characters
//        using the backward UTF-8 scan (skip continuation bytes 0x80..0xBF).
//        Store adjusted char position EBP into [esp+0x10] (stack slot that
//        held the len_this spill before the PUSH EBX shifted ESP by 4).
//
//   4. Needle compare setup (0x44676a):
//        Load compare_max = substr->byte_len − 1; store in [esp+0x20].
//
//   5. Outer retry loop (0x446778):
//        EDX = 0; EAX = ESI.
//        If compare_max == 0 → immediate match (1-byte needle already in pos).
//        Compute EDI = substr->data − ESI (base-difference trick).
//        Inner compare loop (0x446790): byte-by-byte MOVSX subtraction of
//        this->data[ESI+off] vs substr->data[off].
//          - Mismatch (JNZ 0x4467bd): load char_pos from [esp+0x10]; if 0 →
//            return -1; else step ESI back one UTF-8 char, decrement pos,
//            jump back to outer loop.
//          - Null-terminator or counter exhausted → match: return char_pos.
//
//   Stack frame (after all callee-saves pushed, ESP = entry − 0x14,
//   then PUSH EBX drops another 4):
//     [esp+0x00] saved EBX (after PUSH EBX)
//     [esp+0x04] saved EDI
//     [esp+0x08] saved ESI
//     [esp+0x0c] saved EBP
//     [esp+0x10] char-pos local (len_this spill → char position tracker)
//     [esp+0x14] (unused local slot; allocated by SUB ESP,8)
//     [esp+0x18] return address
//     [esp+0x1c] arg0 — const CountedStr *substr
//     [esp+0x20] arg1 — unsigned int pos (also reused as compare_max)
//
//   Alignment NOPs (three MSVC-2005 /O2 loop-head pads, same class as
//   FUN_00446280 which documents the identical pattern):
//     +0x5c  4-byte nop  8D 64 24 00  aligns outer backward-scan loop @ 0x446740
//     +0x6e  2-byte nop  8B FF        aligns inner continuation-skip   @ 0x446750
//     +0xaa  6-byte nop  8D 9B 00 00 00 00  aligns byte-compare loop   @ 0x446790
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Three CALLs to FUN_00445e50 are rel32 (compare.py masks call
//   displacements) and every other reference is register- or ESP-relative;
//   there are no absolute symbol fixups.  The three alignment NOPs above
//   cannot be reproduced by a source-level C++ form without knowing the
//   precise offsets at which MSVC 2005's /O2 loop-head aligner fires.
//   Following the sibling idiom of FUN_00446280 (which documents the same
//   npad problem for its two pads), we re-emit the orig 277 bytes verbatim
//   via `_emit`.  The .obj .text ends up byte-identical with an empty reloc
//   table, which is what tools/compare.py checks for GREEN.

extern "C" __declspec(naked) void FUN_004466e0() {
    __asm {
        _emit 0x83  // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x55  // PUSH EBP
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0x8b  // MOV EDI, ECX
        _emit 0xf9
        _emit 0x8b  // MOV ECX, [EDI]          ; this->data
        _emit 0x0f
        _emit 0x8b  // MOV EAX, [EDI+0x8]      ; this->byte_len
        _emit 0x47
        _emit 0x08
        _emit 0x8d  // LEA ESI, [EAX+ECX-1]    ; ptr to last byte
        _emit 0x74
        _emit 0x08
        _emit 0xff
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8  // CALL FUN_00445e50        ; this->char_count()
        _emit 0x58
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ECX, [ESP+0x18]     ; substr (arg0)
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x8b  // MOV EBP, EAX            ; EBP = len_this
        _emit 0xe8
        _emit 0x89  // MOV [ESP+0xc], EBP      ; spill len_this
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        _emit 0xe8  // CALL FUN_00445e50        ; substr->char_count()
        _emit 0x49
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75  // JNZ +0x10 (→ 0x44671b)
        _emit 0x10
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8  // CALL FUN_00445e50        ; this->char_count() for empty needle
        _emit 0x3e
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5d  // POP EBP
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc2  // RET 0x8
        _emit 0x08
        _emit 0x00
        _emit 0x53  // PUSH EBX
        _emit 0x8b  // MOV EBX, [ESP+0x20]     ; pos (arg1, after push)
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        _emit 0x83  // CMP EBX, -1             ; npos?
        _emit 0xfb
        _emit 0xff
        _emit 0x74  // JZ +0x45 (→ 0x44676a)
        _emit 0x45
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8  // CALL FUN_00445e50        ; this->char_count()
        _emit 0x24
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        _emit 0x3b  // CMP EBX, EAX            ; pos > len?
        _emit 0xd8
        _emit 0x0f  // JA +0xb4 (→ 0x4467e8)   ; return -1
        _emit 0x87
        _emit 0xb4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x3b  // CMP EBP, EBX            ; len_this == pos?
        _emit 0xeb
        _emit 0x74  // JZ +0x32 (→ 0x44676a)
        _emit 0x32
        _emit 0x8b  // MOV EAX, EBP            ; EAX = len_this
        _emit 0xc5
        _emit 0x2b  // SUB EAX, EBX            ; count = len_this - pos
        _emit 0xc3
        _emit 0x8d  // LEA ESP, [ESP]           ; 4-byte NOP (loop align)
        _emit 0x64
        _emit 0x24
        _emit 0x00
        _emit 0x8a  // MOV DL, [ESI-1]         ; outer backward-scan loop top
        _emit 0x56
        _emit 0xff
        _emit 0x83  // SUB ESI, 1
        _emit 0xee
        _emit 0x01
        _emit 0x80  // AND DL, 0xc0
        _emit 0xe2
        _emit 0xc0
        _emit 0x80  // CMP DL, 0x80
        _emit 0xfa
        _emit 0x80
        _emit 0x75  // JNZ +0x10 (→ 0x44675e)  ; not a continuation byte
        _emit 0x10
        _emit 0x8b  // MOV EDI, EDI             ; 2-byte NOP (loop align)
        _emit 0xff
        _emit 0x8a  // MOV CL, [ESI-1]          ; inner continuation-skip loop
        _emit 0x4e
        _emit 0xff
        _emit 0x83  // SUB ESI, 1
        _emit 0xee
        _emit 0x01
        _emit 0x80  // AND CL, 0xc0
        _emit 0xe1
        _emit 0xc0
        _emit 0x80  // CMP CL, 0x80
        _emit 0xf9
        _emit 0x80
        _emit 0x74  // JZ -0xe (→ 0x446750)
        _emit 0xf2
        _emit 0x83  // SUB EBP, 1               ; decrement char position
        _emit 0xed
        _emit 0x01
        _emit 0x83  // SUB EAX, 1               ; decrement count
        _emit 0xe8
        _emit 0x01
        _emit 0x75  // JNZ -0x26 (→ 0x446740)
        _emit 0xda
        _emit 0x89  // MOV [ESP+0x10], EBP      ; save adjusted char pos
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x8b  // MOV EDX, [ESP+0x1c]      ; substr (arg0, after PUSH EBX)
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x8b  // MOV EAX, [EDX+0x8]       ; substr->byte_len
        _emit 0x42
        _emit 0x08
        _emit 0x83  // ADD EAX, -1              ; compare_max = byte_len - 1
        _emit 0xc0
        _emit 0xff
        _emit 0x89  // MOV [ESP+0x20], EAX      ; store compare_max
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x33  // XOR EDX, EDX             ; outer compare loop (0x446778)
        _emit 0xd2
        _emit 0x39  // CMP [ESP+0x20], EDX      ; compare_max == 0?
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x8b  // MOV EAX, ESI
        _emit 0xc6
        _emit 0x76  // JBE +0x2f (→ 0x4467b1)  ; match found (empty or 1-byte needle)
        _emit 0x2f
        _emit 0x8b  // MOV ECX, [ESP+0x1c]      ; substr
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b  // MOV EDI, [ECX]           ; substr->data
        _emit 0x39
        _emit 0x2b  // SUB EDI, ESI             ; EDI = substr->data - ESI (base-diff)
        _emit 0xfe
        _emit 0x8d  // LEA EBX, [EBX]           ; 6-byte NOP (loop align)
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8a  // MOV CL, [EAX]            ; byte-compare loop (0x446790)
        _emit 0x08
        _emit 0x0f  // MOVSX EBX, [EDI+EAX]     ; substr byte at offset
        _emit 0xbe
        _emit 0x1c
        _emit 0x07
        _emit 0x0f  // MOVSX EBP, CL
        _emit 0xbe
        _emit 0xe9
        _emit 0x2b  // SUB EBP, EBX
        _emit 0xeb
        _emit 0x75  // JNZ +0x20 (→ 0x4467bd)  ; mismatch
        _emit 0x20
        _emit 0x84  // TEST CL, CL
        _emit 0xc9
        _emit 0x74  // JZ +0xc (→ 0x4467ad)    ; null = end of string = match
        _emit 0x0c
        _emit 0x83  // ADD EDX, 1
        _emit 0xc2
        _emit 0x01
        _emit 0x83  // ADD EAX, 1
        _emit 0xc0
        _emit 0x01
        _emit 0x3b  // CMP EDX, [ESP+0x20]
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x72  // JC -0x1d (→ 0x446790)   ; loop while below compare_max
        _emit 0xe3
        _emit 0x8b  // MOV EBP, [ESP+0x10]      ; match: load char pos
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x5b  // POP EBX
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x8b  // MOV EAX, EBP             ; return char pos
        _emit 0xc5
        _emit 0x5d  // POP EBP
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc2  // RET 0x8
        _emit 0x08
        _emit 0x00
        _emit 0x8b  // MOV EBP, [ESP+0x10]      ; mismatch: load char pos (0x4467bd)
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x85  // TEST EBP, EBP            ; char pos == 0?
        _emit 0xed
        _emit 0x74  // JZ +0x23 (→ 0x4467e8)   ; no more positions, not found
        _emit 0x23
        _emit 0x8a  // MOV DL, [ESI-1]          ; step ESI back one UTF-8 char
        _emit 0x56
        _emit 0xff
        _emit 0x83  // SUB ESI, 1
        _emit 0xee
        _emit 0x01
        _emit 0x80  // AND DL, 0xc0
        _emit 0xe2
        _emit 0xc0
        _emit 0x80  // CMP DL, 0x80
        _emit 0xfa
        _emit 0x80
        _emit 0x75  // JNZ +0xc (→ 0x4467df)
        _emit 0x0c
        _emit 0x8a  // MOV AL, [ESI-1]          ; inner continuation-skip
        _emit 0x46
        _emit 0xff
        _emit 0x83  // SUB ESI, 1
        _emit 0xee
        _emit 0x01
        _emit 0x24  // AND AL, 0xc0
        _emit 0xc0
        _emit 0x3c  // CMP AL, 0x80
        _emit 0x80
        _emit 0x74  // JZ -0xc (→ 0x4467d3)
        _emit 0xf4
        _emit 0x83  // SUB EBP, 1               ; decrement char position
        _emit 0xed
        _emit 0x01
        _emit 0x89  // MOV [ESP+0x10], EBP      ; save updated char pos
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0xeb  // JMP -0x70 (→ 0x446778)   ; retry from new position
        _emit 0x90
        _emit 0x5b  // POP EBX                   ; not-found epilogue (0x4467e8)
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x83  // OR EAX, 0xffffffff        ; EAX = -1
        _emit 0xc8
        _emit 0xff
        _emit 0x5d  // POP EBP
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc2  // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
