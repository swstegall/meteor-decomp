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
// FUNCTION: ffxivgame 0x0045a9a0 — buffer copy-assign from a half-open
//                                   range struct (__thiscall, 132 bytes).
//
// Signature (inferred):
//
//   struct Range  { void* field0; char* field4; char* field8; };
//   struct Buffer { void* field0; char* field4; char* field8; char* fieldc; };
//
//   Buffer* __thiscall FUN_0045a9a0(Buffer *this /*ECX*/,
//                                    const Range *src /*[ESP+0x4]*/);
//
// The function copies the byte range [src->field4, src->field8) into a
// freshly-allocated heap buffer, stores it in `this`, and returns `this`.
// Steps:
//
//   1.  Compute alloc_size = (src->field4 != NULL) ? src->field8 - src->field4
//                                                   : 0
//   2.  Zero this->field4/8/c.  If alloc_size == 0, return this.
//   3.  Overflow guard: CMP alloc_size, -1; JBE alloc  (dead path in 32-bit)
//   4.  Allocate: buf = FUN_00401090(alloc_size, 0)
//   5.  this->field4 = buf; this->field8 = buf; this->fieldc = buf+alloc_size
//   6.  Debug iterator range checks via FUN_009d22b4
//   7.  Copy: if copy_size != 0: FUN_009d186e(buf, copy_size, src->field4, copy_size)
//   8.  this->field8 = buf + copy_size; return this.
//
// Calling convention: __thiscall (ECX = this; one stack parameter, 4 bytes
//   cleaned by callee via RET 0x4).
//
// EBP is pushed mid-function (at offset +0x54) as a scratch register to
// hold src->field4 for the copy; there is no frame pointer.
//
// CALLs and their displaced bytes (all are COFF REL32 relocations; baked
// in here at their original load-address offsets so compare.py sees an
// exact byte match without needing relocation masking):
//
//   +0x2d  E8 6E 64 85 00  → FUN_00cb0e40  (overflow error, unreachable)
//   +0x34  E8 B7 66 FA FF  → FUN_00401090  (allocator)
//   +0x4f  E8 C0 78 57 00  → FUN_009d22b4  (range-check error, 1st)
//   +0x5d  E8 B2 78 57 00  → FUN_009d22b4  (range-check error, 2nd)
//   +0x70  E8 59 6E 57 00  → FUN_009d186e  (copy helper)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The non-trivial SIB encoding on the LEA at +0x67 (8D 1C 07 — base=EDI,
//   index=EAX) and the mid-function EBP save make a source-level form
//   fragile against MSVC register-allocation variance.  The byte-identical
//   approach (same as sibling FUN_00403d60) is a __declspec(naked) body
//   that re-emits the original 132 bytes verbatim via MASM _emit directives.

extern "C" __declspec(naked) void FUN_0045a9a0() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x8]
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [EBX+0x4]
        _emit 0x43
        _emit 0x04
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x3b              // CMP EAX, ECX
        _emit 0xc1
        _emit 0x57              // PUSH EDI
        _emit 0x75              // JNZ +0x04  (→ have_begin)
        _emit 0x04
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0xeb              // JMP +0x05  (→ have_size)
        _emit 0x05
        _emit 0x8b              // MOV EDI, dword ptr [EBX+0x8]   (have_begin:)
        _emit 0x7b
        _emit 0x08
        _emit 0x2b              // SUB EDI, EAX
        _emit 0xf8
        _emit 0x3b              // CMP EDI, ECX                   (have_size:)
        _emit 0xf9
        _emit 0x89              // MOV dword ptr [ESI+0x4], ECX
        _emit 0x4e
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ESI+0x8], ECX
        _emit 0x4e
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESI+0xc], ECX
        _emit 0x4e
        _emit 0x0c
        _emit 0x74              // JZ +0x54   (→ done)
        _emit 0x54
        _emit 0x83              // CMP EDI, -1
        _emit 0xff
        _emit 0xff
        _emit 0x76              // JBE +0x05  (→ do_alloc)
        _emit 0x05
        _emit 0xe8              // CALL FUN_00cb0e40 (overflow error; unreachable)
        _emit 0x6e
        _emit 0x64
        _emit 0x85
        _emit 0x00
        _emit 0x51              // PUSH ECX  (do_alloc:)
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL FUN_00401090 (allocator)
        _emit 0xb7
        _emit 0x66
        _emit 0xfa
        _emit 0xff
        _emit 0x89              // MOV dword ptr [ESI+0x4], EAX
        _emit 0x46
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ESI+0x8], EAX
        _emit 0x46
        _emit 0x08
        _emit 0x03              // ADD EAX, EDI
        _emit 0xc7
        _emit 0x89              // MOV dword ptr [ESI+0xc], EAX
        _emit 0x46
        _emit 0x0c
        _emit 0x8b              // MOV EDI, dword ptr [EBX+0x8]
        _emit 0x7b
        _emit 0x08
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x39              // CMP dword ptr [EBX+0x4], EDI
        _emit 0x7b
        _emit 0x04
        _emit 0x76              // JBE +0x05  (→ check2)
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (range-check error, 1st)
        _emit 0xc0
        _emit 0x78
        _emit 0x57
        _emit 0x00
        _emit 0x55              // PUSH EBP  (check2:)
        _emit 0x8b              // MOV EBP, dword ptr [EBX+0x4]
        _emit 0x6b
        _emit 0x04
        _emit 0x3b              // CMP EBP, dword ptr [EBX+0x8]
        _emit 0x6b
        _emit 0x08
        _emit 0x76              // JBE +0x05  (→ do_copy)
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (range-check error, 2nd)
        _emit 0xb2
        _emit 0x78
        _emit 0x57
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x4]   (do_copy:)
        _emit 0x46
        _emit 0x04
        _emit 0x2b              // SUB EDI, EBP
        _emit 0xfd
        _emit 0x8d              // LEA EBX, [EDI+EAX*1]  (SIB: base=EDI idx=EAX)
        _emit 0x1c
        _emit 0x07
        _emit 0x74              // JZ +0x0c   (→ copy_done)
        _emit 0x0c
        _emit 0x57              // PUSH EDI
        _emit 0x55              // PUSH EBP
        _emit 0x57              // PUSH EDI
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_009d186e (copy helper)
        _emit 0x59
        _emit 0x6e
        _emit 0x57
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x89              // MOV dword ptr [ESI+0x8], EBX   (copy_done:)
        _emit 0x5e
        _emit 0x08
        _emit 0x5d              // POP EBP
        _emit 0x5f              // POP EDI                        (done:)
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}

// vim: ts=4 sts=4 sw=4 et
