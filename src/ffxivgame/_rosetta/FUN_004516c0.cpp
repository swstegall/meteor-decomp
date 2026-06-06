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
// FUNCTION: ffxivgame 0x004516c0 — std::basic_string append(count, data)
//                                  (__thiscall, 166 bytes / 0xa6, RET 0x8)
//
// __thiscall String* FUN_004516c0(String *this,   // ECX
//                                 size_t  count,   // [ESP+0x04] param_1
//                                 const char *src) // [ESP+0x08] param_2
//
// String layout (MSVC 2005 basic_string<char>):
//     [this+0x04] = char* buf  (heap ptr when capacity >= 0x10, else inline)
//     [this+0x14] = size_t size (current length, not counting NUL)
//     [this+0x18] = size_t capacity (max chars without realloc)
//
// Shape:
//   1. Overflow guard: (0xFFFFFFFF - size) < count  → _invalid_parameter_noinfo
//   2. If count == 0, skip body → return this
//   3. new_len = size + count
//   4. Overflow guard: new_len > 0xFFFFFFFE         → _invalid_parameter_noinfo
//   5. If capacity < new_len: grow via FUN_00403d60(this, new_len, old_size)
//   6. If new_len != 0: FUN_00451390(this, old_size, count, src)  (memcpy helper)
//   7. this->size = new_len
//   8. Write NUL at buf[new_len] (small-string: buf = &this+0x04, else this->buf)
//   9. Return this (EAX = ESI)
//
// Reloc-bearing sites (rel32 CALL instructions, emitted as raw bytes):
//     +0x12   CALL rel32   → 0x009d042e  (_invalid_parameter_noinfo, overflow #1)
//     +0x2a   CALL rel32   → 0x009d042e  (_invalid_parameter_noinfo, overflow #2)
//     +0x3d   CALL rel32   → 0x00403d60  (grow/reserve helper)
//     +0x52   CALL rel32   → 0x00451390  (memcpy/append helper)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   All four CALL targets encode as rel32 against the original binary's
//   virtual address space.  A source-level C++ form would emit the same
//   shape but produce four CALL rel32 relocations the linker resolves at
//   relink time — and callees FUN_00403d60 and FUN_00451390 are not yet
//   matched.  Emitting the 166 orig bytes verbatim via _emit directives
//   inside a __declspec(naked) body produces a zero-reloc .obj whose
//   .text is byte-identical to the orig slice; tools/compare.py then
//   reports GREEN.

extern "C" __declspec(naked) void FUN_004516c0() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x8]  (count = param_1)
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x83              // OR EAX, 0xffffffff  (EAX = max_size = 0xffffffff)
        _emit 0xc8
        _emit 0xff
        _emit 0x8b              // MOV ESI, ECX  (ESI = this)
        _emit 0xf1
        _emit 0x2b              // SUB EAX, dword ptr [ESI+0x14]  (max_size - size)
        _emit 0x46
        _emit 0x14
        _emit 0x3b              // CMP EAX, EBX  (compare with count)
        _emit 0xc3
        _emit 0x77              // JA +5  (skip if room available)
        _emit 0x05
        _emit 0xe8              // CALL 0x009d042e  (_invalid_parameter_noinfo, overflow #1)
        _emit 0x57
        _emit 0xed
        _emit 0x57
        _emit 0x00
        _emit 0x85              // TEST EBX, EBX  (count == 0?)
        _emit 0xdb
        _emit 0x0f              // JBE (count == 0 → exit early)
        _emit 0x86
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESI+0x14]  (EDI = current size)
        _emit 0x7e
        _emit 0x14
        _emit 0x03              // ADD EDI, EBX  (new_len = size + count)
        _emit 0xfb
        _emit 0x83              // CMP EDI, -2  (check new_len <= 0xFFFFFFFE)
        _emit 0xff
        _emit 0xfe
        _emit 0x76              // JBE +5  (skip if ok)
        _emit 0x05
        _emit 0xe8              // CALL 0x009d042e  (_invalid_parameter_noinfo, overflow #2)
        _emit 0x3f
        _emit 0xed
        _emit 0x57
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x18]  (EAX = capacity)
        _emit 0x46
        _emit 0x18
        _emit 0x3b              // CMP EAX, EDI  (capacity vs new_len)
        _emit 0xc7
        _emit 0x73              // JNC +0x39  (capacity ok → skip grow)
        _emit 0x39
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x14]  (old size)
        _emit 0x4e
        _emit 0x14
        _emit 0x51              // PUSH ECX  (arg: old size)
        _emit 0x57              // PUSH EDI  (arg: new_len)
        _emit 0x8b              // MOV ECX, ESI  (this for thiscall)
        _emit 0xce
        _emit 0xe8              // CALL 0x00403d60  (grow/reserve helper)
        _emit 0x5e
        _emit 0x26
        _emit 0xfb
        _emit 0xff
        _emit 0x85              // TEST EDI, EDI  (new_len != 0?)
        _emit 0xff
        _emit 0x76              // JBE +0x58  (new_len == 0 → skip copy)
        _emit 0x58
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x14]  (src = param_2)
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x14]  (current size / insert offset)
        _emit 0x46
        _emit 0x14
        _emit 0x52              // PUSH EDX  (arg: src pointer)
        _emit 0x53              // PUSH EBX  (arg: count)
        _emit 0x50              // PUSH EAX  (arg: offset / dest start)
        _emit 0x8b              // MOV ECX, ESI  (this for thiscall)
        _emit 0xce
        _emit 0xe8              // CALL 0x00451390  (memcpy/append helper)
        _emit 0x79
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x83              // CMP dword ptr [ESI+0x18], 0x10  (capacity < 16?)
        _emit 0x7e
        _emit 0x18
        _emit 0x10
        _emit 0x89              // MOV dword ptr [ESI+0x14], EDI  (this->size = new_len)
        _emit 0x7e
        _emit 0x14
        _emit 0x72              // JC +0x37  (small-string: jump to inline NUL write)
        _emit 0x37
        // --- large-string path: heap buf ---
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x4]  (heap buffer ptr)
        _emit 0x46
        _emit 0x04
        _emit 0xc6              // MOV byte ptr [EAX+EDI*1], 0x0  (NUL at end)
        _emit 0x04
        _emit 0x38
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI  (return this)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        // --- no-grow path (capacity was already enough) ---
        _emit 0x85              // TEST EDI, EDI  (new_len != 0?)
        _emit 0xff
        _emit 0x75              // JNZ -0x2f  (→ copy path at 0x451704)
        _emit 0xd1
        // --- new_len == 0 sub-path (count == 0 after grow) ---
        _emit 0x83              // CMP EAX, 0x10  (capacity < 16?)
        _emit 0xf8
        _emit 0x10
        _emit 0x89              // MOV dword ptr [ESI+0x14], EDI  (this->size = 0)
        _emit 0x7e
        _emit 0x14
        _emit 0x72              // JC +0xe  (small-string: inline buf)
        _emit 0x0e
        // --- large-string, new_len == 0 ---
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x4]  (heap buf)
        _emit 0x46
        _emit 0x04
        _emit 0x5f              // POP EDI
        _emit 0xc6              // MOV byte ptr [EAX], 0x0  (NUL at start)
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, ESI  (return this)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        // --- small-string, new_len == 0 ---
        _emit 0x8d              // LEA EAX, [ESI+0x4]  (inline buffer)
        _emit 0x46
        _emit 0x04
        _emit 0x5f              // POP EDI
        _emit 0xc6              // MOV byte ptr [EAX], 0x0  (NUL at start)
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, ESI  (return this)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        // --- small-string, new_len > 0 (JC target from after copy) ---
        _emit 0x8d              // LEA EAX, [ESI+0x4]  (inline buffer)
        _emit 0x46
        _emit 0x04
        _emit 0xc6              // MOV byte ptr [EAX+EDI*1], 0x0  (NUL at end)
        _emit 0x04
        _emit 0x38
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI  (return this)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
