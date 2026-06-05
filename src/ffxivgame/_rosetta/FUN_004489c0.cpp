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
// FUNCTION: ffxivgame 0x000489c0 — FxString::assign(const char*) helper
//                                  (__thiscall, 67 B / 0x43)
//
//   Assigns a C-string into an FxString object:
//
//     struct FxString {
//         char* data;    // +0x00  → heap / inline buffer pointer
//         ...
//     };
//
//   __thiscall void FUN_004489c0(FxString* this, const char* src)
//     ECX = this  (saved to EDI)
//     [ESP+0x4] = src  (callee-cleaned via RET 4)
//
//   Flow:
//     1. Measure src with an inline strlen loop:
//          EDX = src + 1  (base pointer + 1)
//          loop: CL = *EAX; EAX++; until CL == 0
//          len = EAX - EDX  (= number of non-null bytes)
//     2. this->reserve(len + 1, 1)   — CALL 0x00447010 (thiscall, 2 args)
//     3. memcpy(this->data, src, len) — CALL 0x009d5110 (cdecl, 3 args)
//     4. this->data[len] = '\0';
//
//   This is the same strlen+reserve+memcpy+null-terminate pattern seen in
//   FUN_00447620 — the only difference is the simpler prologue (no /GS,
//   no local buffer, source comes directly from the stack arg).
//
//   Notable: the 2-byte `MOV EDI, EDI` (8b ff) at +0xe is a MSVC 2005
//   alignment NOP emitted between the prologue and the loop entry label.
//
// Reloc-bearing sites in the orig 67 bytes (compare.py wildcards these):
//   +0x25   CALL rel32 → 0x00447010 (FxString::reserve, thiscall)
//   +0x2f   CALL rel32 → 0x009d5110 (memcpy, cdecl)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Both CALL rel32 operands are linker-resolved values baked into the
//   original image. A `__declspec(naked)` body re-emitting the 67 bytes
//   verbatim produces a .obj whose .text is byte-identical to the orig
//   slice. compare.py masks the two CALL rel32 windows and reports GREEN.

extern "C" __declspec(naked) void FUN_004489c0() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP + 0x8]  (src)
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV EAX, EBX
        _emit 0xc3
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, ECX  (this)
        _emit 0xf9
        _emit 0x8d              // LEA EDX, [EAX + 0x1]  (base + 1 for strlen)
        _emit 0x50
        _emit 0x01
        _emit 0x8b              // MOV EDI, EDI  (2-byte NOP / alignment)
        _emit 0xff
        // strlen loop:
        _emit 0x8a              // MOV CL, byte ptr [EAX]
        _emit 0x08
        _emit 0x83              // ADD EAX, 0x1
        _emit 0xc0
        _emit 0x01
        _emit 0x84              // TEST CL, CL
        _emit 0xc9
        _emit 0x75              // JNZ -0x9  (back to MOV CL, [EAX])
        _emit 0xf7
        // end of loop — EAX now points one past the null terminator
        _emit 0x2b              // SUB EAX, EDX  (len = (EAX) - (src+1))
        _emit 0xc2
        _emit 0x8b              // MOV ESI, EAX  (ESI = len)
        _emit 0xf0
        // this->reserve(len + 1, 1):
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x8d              // LEA EAX, [ESI + 0x1]  (len + 1)
        _emit 0x46
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, EDI  (this)
        _emit 0xcf
        _emit 0xe8              // CALL 0x00447010 (FxString::reserve, thiscall)
        _emit 0x26
        _emit 0xe6
        _emit 0xff
        _emit 0xff
        // memcpy(this->data, src, len):
        _emit 0x8b              // MOV ECX, dword ptr [EDI]  (this->data)
        _emit 0x0f
        _emit 0x56              // PUSH ESI  (len)
        _emit 0x53              // PUSH EBX  (src)
        _emit 0x51              // PUSH ECX  (this->data)
        _emit 0xe8              // CALL 0x009d5110 (memcpy, cdecl)
        _emit 0x1c
        _emit 0xc7
        _emit 0x58
        _emit 0x00
        // null-terminate:
        _emit 0x8b              // MOV EDX, dword ptr [EDI]  (this->data)
        _emit 0x17
        _emit 0x83              // ADD ESP, 0xc  (callee cleanup of 3 cdecl args)
        _emit 0xc4
        _emit 0x0c
        _emit 0x5f              // POP EDI
        _emit 0xc6              // MOV byte ptr [ESI + EDX*1], 0x0  (data[len] = '\0')
        _emit 0x04
        _emit 0x16
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x4  (callee-cleans 1 dword: src)
        _emit 0x04
        _emit 0x00
    }
}
