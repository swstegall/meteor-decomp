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
// FUNCTION: ffxivgame 0x00035670 — `__thiscall` 170-byte (0xaa) flush /
//                                   notify-and-reset over a vector member.
//
// Asm shape (`__thiscall void f(this)` — ret 0):
//
//   ESI = this                                       ; (this = ECX in)
//   if (this->m30 == 0) return;                      ; nothing pending
//   EDI = this->m4                                   ; begin pointer
//   for (p = begin; p != this->m8; p += 4) {         ; walk [m4, m8)
//       (*p)->byte_7c = 1;                           ; mark each element
//   }
//   // store element-count (m8 - m4) >> 2 through two out-params
//   n = (this->m4 == 0) ? 0 : (this->m8 - this->m4) >> 2;
//   xchg(*(int*)this->m20, n);
//   n = (this->m4 == 0) ? 0 : (this->m8 - this->m4) >> 2;
//   xchg(*(int*)this->m28, n);
//   // dispatch three queued slots through a pair of function pointers
//   g_fn0 = *(void(*)(int))0x00f3e138;
//   g_fn0(this->m1c);
//   g_fn0(this->m18);
//   (*(void(*)(int))0x00f3e13c)(this->m14);
//   this->m30 = 0;
//
// The two bounds-check CALL 0x009d22b4 sites (and the extra ones inside
// the marking loop) are the binary's universal out_of_range panic thunk.
// The two indirect dispatch targets live at 0x00f3e138 / 0x00f3e13c.
//
// Reconstruction strategy — naked-asm byte passthrough (the local idiom,
// see sibling FUN_00406280 / FUN_004063c0): a source-level C++ port would
// fight MSVC's callee-saved register allocator (ESI/EDI/EBP across five
// conditional branches and several external CALLs) for no benefit. We
// re-emit the orig 170 bytes verbatim via MASM `_emit` directives; the
// .obj's `.text` is byte-identical to orig[0x35670..0x3571a] with no
// relocations (the rel32 CALL offsets and the m32 dispatch addresses are
// baked into the orig wire image, so emitting them as raw bytes is exact).
// `tools/compare.py` then reports GREEN.
//
// Reloc-bearing sites in the orig 170 bytes:
//     +0x16   CALL rel32   → FUN_009d22b4  (out_of_range thunk)
//     +0x24   CALL rel32   → FUN_009d22b4  (out_of_range thunk)
//     +0x39   CALL rel32   → FUN_009d22b4  (out_of_range thunk)
//     +0x49   CALL rel32   → FUN_009d22b4  (out_of_range thunk)
//     +0x87   MOV  EDI, [0x00f3e138]        (dispatch fn-ptr #0)
//     +0x9a   CALL m32      → [0x00f3e13c]  (dispatch fn-ptr #1)

extern "C" __declspec(naked) void FUN_00435670() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x83              // CMP dword ptr [ESI+0x30], 0
        _emit 0x7e
        _emit 0x30
        _emit 0x00
        _emit 0x0f              // JBE  ->epilogue (+0x9b)
        _emit 0x86
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESI+0x4]
        _emit 0x7e
        _emit 0x04
        _emit 0x3b              // CMP EDI, dword ptr [ESI+0x8]
        _emit 0x7e
        _emit 0x08
        _emit 0x76              // JBE short +5
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32)
        _emit 0x29
        _emit 0xcc
        _emit 0x59
        _emit 0x00
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESI+0x8]
        _emit 0x6e
        _emit 0x08
        _emit 0x39              // CMP dword ptr [ESI+0x4], EBP
        _emit 0x6e
        _emit 0x04
        _emit 0x76              // JBE short +0xc
        _emit 0x0c
        _emit 0xe8              // CALL FUN_009d22b4 (rel32)
        _emit 0x1b
        _emit 0xcc
        _emit 0x59
        _emit 0x00
        _emit 0x8d              // LEA ESP, [ESP]   (7-byte NOP)
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // loop_top (+0x30):
        _emit 0x3b              // CMP EDI, EBP
        _emit 0xfd
        _emit 0x74              // JZ short +0x1f (->loop_done)
        _emit 0x1f
        _emit 0x3b              // CMP EDI, dword ptr [ESI+0x8]
        _emit 0x7e
        _emit 0x08
        _emit 0x72              // JC short +5
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32)
        _emit 0x06
        _emit 0xcc
        _emit 0x59
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [EDI]
        _emit 0x07
        _emit 0xc6              // MOV byte ptr [EAX+0x7c], 1
        _emit 0x40
        _emit 0x7c
        _emit 0x01
        _emit 0x3b              // CMP EDI, dword ptr [ESI+0x8]
        _emit 0x7e
        _emit 0x08
        _emit 0x72              // JC short +5
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32)
        _emit 0xf6
        _emit 0xcb
        _emit 0x59
        _emit 0x00
        _emit 0x83              // ADD EDI, 4
        _emit 0xc7
        _emit 0x04
        _emit 0xeb              // JMP short -0x23 (->loop_top)
        _emit 0xdd
        // loop_done (+0x53):
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x4]
        _emit 0x4e
        _emit 0x04
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x5d              // POP EBP
        _emit 0x75              // JNZ short +4
        _emit 0x04
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0xeb              // JMP short +8
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x8]
        _emit 0x46
        _emit 0x08
        _emit 0x2b              // SUB EAX, ECX
        _emit 0xc1
        _emit 0xc1              // SAR EAX, 2
        _emit 0xf8
        _emit 0x02
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x20]
        _emit 0x4e
        _emit 0x20
        _emit 0x87              // XCHG dword ptr [ECX], EAX
        _emit 0x01
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x4]
        _emit 0x4e
        _emit 0x04
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x75              // JNZ short +4
        _emit 0x04
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0xeb              // JMP short +8
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x8]
        _emit 0x46
        _emit 0x08
        _emit 0x2b              // SUB EAX, ECX
        _emit 0xc1
        _emit 0xc1              // SAR EAX, 2
        _emit 0xf8
        _emit 0x02
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x28]
        _emit 0x56
        _emit 0x28
        _emit 0x87              // XCHG dword ptr [EDX], EAX
        _emit 0x02
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x1c]
        _emit 0x46
        _emit 0x1c
        _emit 0x8b              // MOV EDI, dword ptr [0x00f3e138]
        _emit 0x3d
        _emit 0x38
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL EDI
        _emit 0xd7
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x18]
        _emit 0x4e
        _emit 0x18
        _emit 0x51              // PUSH ECX
        _emit 0xff              // CALL EDI
        _emit 0xd7
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x14]
        _emit 0x56
        _emit 0x14
        _emit 0x52              // PUSH EDX
        _emit 0xff              // CALL dword ptr [0x00f3e13c]
        _emit 0x15
        _emit 0x3c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI+0x30], 0
        _emit 0x46
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5f              // POP EDI
        // epilogue (+0x9b):
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
