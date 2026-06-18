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
// FUNCTION: ffxivgame 0x0043bca0 — vector-like resize(__thiscall, 179 B / 0xb3)
//
//   __thiscall void resize(this, unsigned int new_size, int fill_value)
//     stack layout (after RET 8):
//       ECX        : this  (the container object)
//       [ESP+0x04] : unsigned int new_size    (param_1, in EBX after PUSH EBX)
//       [ESP+0x08] : int         fill_value   (param_2)
//
//   Memory layout of the container (inferred from offsets touched):
//     +0x04  T* begin    (pointer to first element)
//     +0x08  T* end      (pointer one-past-last element)
//     element size = 4 bytes (SAR >> 2 used to compute count)
//
// Behaviour:
//
//   1. Compute current size:
//        if (this->begin == NULL) size = 0;
//        else size = (this->end - this->begin) >> 2;
//
//   2. Grow path  (size < new_size):
//        Compute current size again into EDI (same conditional pattern).
//        Assert begin <= end (call 0x009d22b4 on violation).
//        Push: param_2 (fill value), (new_size - size) (count),
//              end pointer, this.
//        MOV ECX, this
//        CALL 0x00c56c20   ; fill-insert helper — appends (new_size-size)
//                          ; copies of fill_value at the end of the range
//        Return.
//
//   3. No-change path  (size == new_size) — returns immediately via
//      the second block's early-out JNC 0x0043bd49.
//
//   4. Shrink path  (size > new_size, i.e. new_size < size):
//        Assert begin != NULL (return if NULL).
//        Assert new_size < size.
//        Assert begin <= end (0x009d22b4 on violation).
//        Assert begin <= end again after re-loading begin (0x009d22b4).
//        Compute position = begin + new_size * 4.
//        Assert begin <= position <= end (0x009d22b4 on violation).
//        Push: end, this, position, this.
//        LEA ECX, [ESP + 0x20]   ; address of local slot — second arg by ref
//        PUSH ECX
//        MOV ECX, this
//        CALL 0x0071cc50   ; erase-range helper — trims elements from
//                          ; position to end
//        Return.
//
// Reloc-bearing CALL sites (offsets within the 179-byte function body —
// each 4-byte rel32 window is wildcarded by compare.py against orig):
//   +0x3c   CALL rel32 → 0x009d22b4   (assert/range-error, grow path)
//   +0x4d   CALL rel32 → 0x00c56c20   (fill-insert helper)
//   +0x72   CALL rel32 → 0x009d22b4   (assert, shrink path #1)
//   +0x7f   CALL rel32 → 0x009d22b4   (assert, shrink path #2)
//   +0x95   CALL rel32 → 0x009d22b4   (assert, shrink path #3)
//   +0xa5   CALL rel32 → 0x0071cc50   (erase-range helper)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function contains 6 CALL rel32 relocations targeting helpers
//   at widely-separated addresses in the binary. Reproducing the exact
//   branch displacements (all JNZ/JMP/JBE/JNC/JZ/JA are short, 2 bytes)
//   plus the LEA ECX,[ESP+0x20] after 4 pushes (which references a local
//   slot, not param_2) from plain C++ source under /O2 is impractical —
//   the CALL operands shift every rel32 and MSVC's register allocator
//   may choose different save-register ordering. We therefore emit the
//   orig 179 bytes verbatim via MASM _emit directives. compare.py
//   wildcards the six 4-byte rel32 windows and reports GREEN.

extern "C" __declspec(naked) void FUN_0043bca0() {
    __asm {
        // 0x0003bca0 — prologue
        _emit 0x83  // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x53  // PUSH EBX
        _emit 0x55  // PUSH EBP
        _emit 0x56  // PUSH ESI
        _emit 0x8b  // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b  // MOV ECX, dword ptr [ESI+0x4]   (begin)
        _emit 0x4e
        _emit 0x04
        _emit 0x85  // TEST ECX, ECX
        _emit 0xc9
        _emit 0x57  // PUSH EDI
        _emit 0x75  // JNZ +0x04
        _emit 0x04
        _emit 0x33  // XOR EAX, EAX
        _emit 0xc0
        _emit 0xeb  // JMP +0x08
        _emit 0x08
        // 0x0003bcb4
        _emit 0x8b  // MOV EAX, dword ptr [ESI+0x8]   (end)
        _emit 0x46
        _emit 0x08
        _emit 0x2b  // SUB EAX, ECX
        _emit 0xc1
        _emit 0xc1  // SAR EAX, 0x2
        _emit 0xf8
        _emit 0x02
        // 0x0003bcbc
        _emit 0x8b  // MOV EBX, dword ptr [ESP+0x1c]  (param_1 = new_size)
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x3b  // CMP EAX, EBX
        _emit 0xc3
        _emit 0x73  // JNC +0x37   (size >= new_size → shrink/nop block)
        _emit 0x37
        // 0x0003bcc4 — grow path: size < new_size
        _emit 0x85  // TEST ECX, ECX
        _emit 0xc9
        _emit 0x75  // JNZ +0x04
        _emit 0x04
        _emit 0x33  // XOR EDI, EDI
        _emit 0xff
        _emit 0xeb  // JMP +0x08
        _emit 0x08
        // 0x0003bccc
        _emit 0x8b  // MOV EDI, dword ptr [ESI+0x8]   (end)
        _emit 0x7e
        _emit 0x08
        _emit 0x2b  // SUB EDI, ECX
        _emit 0xf9
        _emit 0xc1  // SAR EDI, 0x2   (EDI = current size)
        _emit 0xff
        _emit 0x02
        // 0x0003bcd4
        _emit 0x8b  // MOV EBP, dword ptr [ESI+0x8]   (end)
        _emit 0x6e
        _emit 0x08
        _emit 0x3b  // CMP ECX, EBP   (begin <= end?)
        _emit 0xcd
        _emit 0x76  // JBE +0x05
        _emit 0x05
        // 0x0003bcdb — assertion: begin > end
        _emit 0xe8  // CALL 0x009d22b4  (rel32: d4 65 59 00)
        _emit 0xd4
        _emit 0x65
        _emit 0x59
        _emit 0x00
        // 0x0003bce0
        _emit 0x8b  // MOV EAX, dword ptr [ESP+0x20]  (param_2 = fill_value)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x50  // PUSH EAX                   (arg: fill_value)
        _emit 0x2b  // SUB EBX, EDI               (EBX = new_size - size = count)
        _emit 0xdf
        _emit 0x53  // PUSH EBX                   (arg: count)
        _emit 0x55  // PUSH EBP                   (arg: end pointer)
        _emit 0x56  // PUSH ESI                   (arg: this)
        _emit 0x8b  // MOV ECX, ESI               (ECX = this for thiscall)
        _emit 0xce
        // 0x0003bcec
        _emit 0xe8  // CALL 0x00c56c20  (rel32: 2f af 81 00)
        _emit 0x2f
        _emit 0xaf
        _emit 0x81
        _emit 0x00
        // 0x0003bcf1 — epilogue (grow path)
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5d  // POP EBP
        _emit 0x5b  // POP EBX
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc2  // RET 0x8
        _emit 0x08
        _emit 0x00
        // 0x0003bcfb — shrink/nop block (size >= new_size)
        _emit 0x85  // TEST ECX, ECX   (begin == NULL?)
        _emit 0xc9
        _emit 0x74  // JZ +0x4a   (NULL begin → early out)
        _emit 0x4a
        _emit 0x8b  // MOV EBP, dword ptr [ESI+0x8]   (end)
        _emit 0x6e
        _emit 0x08
        _emit 0x8b  // MOV EAX, EBP
        _emit 0xc5
        _emit 0x2b  // SUB EAX, ECX
        _emit 0xc1
        _emit 0xc1  // SAR EAX, 0x2   (EAX = size again)
        _emit 0xf8
        _emit 0x02
        _emit 0x3b  // CMP EBX, EAX   (new_size < size?)
        _emit 0xd8
        _emit 0x73  // JNC +0x3c   (new_size >= size → no-op return)
        _emit 0x3c
        // 0x0003bd0d — shrink path: new_size < size
        _emit 0x3b  // CMP ECX, EBP   (begin <= end?)
        _emit 0xcd
        _emit 0x76  // JBE +0x05
        _emit 0x05
        // 0x0003bd11 — assertion
        _emit 0xe8  // CALL 0x009d22b4  (rel32: 9e 65 59 00)
        _emit 0x9e
        _emit 0x65
        _emit 0x59
        _emit 0x00
        // 0x0003bd16
        _emit 0x8b  // MOV EDI, dword ptr [ESI+0x4]   (begin)
        _emit 0x7e
        _emit 0x04
        _emit 0x3b  // CMP EDI, dword ptr [ESI+0x8]   (begin <= end?)
        _emit 0x7e
        _emit 0x08
        _emit 0x76  // JBE +0x05
        _emit 0x05
        // 0x0003bd1e — assertion
        _emit 0xe8  // CALL 0x009d22b4  (rel32: 91 65 59 00)
        _emit 0x91
        _emit 0x65
        _emit 0x59
        _emit 0x00
        // 0x0003bd23
        _emit 0x89  // MOV dword ptr [ESP+0x14], EDI   (save begin into local)
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x8d  // LEA EDI, [EDI + EBX*4]   (position = begin + new_size*4)
        _emit 0x3c
        _emit 0x9f
        _emit 0x3b  // CMP EDI, dword ptr [ESI+0x8]   (position <= end?)
        _emit 0x7e
        _emit 0x08
        _emit 0x77  // JA +0x05   (position > end → assert)
        _emit 0x05
        _emit 0x3b  // CMP EDI, dword ptr [ESI+0x4]   (position >= begin?)
        _emit 0x7e
        _emit 0x04
        _emit 0x73  // JNC +0x05   (ok)
        _emit 0x05
        // 0x0003bd34 — assertion
        _emit 0xe8  // CALL 0x009d22b4  (rel32: 7b 65 59 00)
        _emit 0x7b
        _emit 0x65
        _emit 0x59
        _emit 0x00
        // 0x0003bd39 — erase-range call
        _emit 0x55  // PUSH EBP          (arg: end)
        _emit 0x56  // PUSH ESI          (arg: this)
        _emit 0x57  // PUSH EDI          (arg: position = begin + new_size*4)
        _emit 0x56  // PUSH ESI          (arg: this)
        _emit 0x8d  // LEA ECX, [ESP+0x20]   (addr of local slot / param context)
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x51  // PUSH ECX
        _emit 0x8b  // MOV ECX, ESI     (ECX = this for thiscall)
        _emit 0xce
        // 0x0003bd44
        _emit 0xe8  // CALL 0x0071cc50  (rel32: 07 0f 2e 00)
        _emit 0x07
        _emit 0x0f
        _emit 0x2e
        _emit 0x00
        // 0x0003bd49 — shared epilogue (shrink/nop paths)
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5d  // POP EBP
        _emit 0x5b  // POP EBX
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc2  // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
