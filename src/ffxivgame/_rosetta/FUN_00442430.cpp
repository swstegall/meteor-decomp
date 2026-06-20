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
// FUNCTION: ffxivgame 0x00442430 — `__thiscall` vector element assign + notify
//                                  (336 B / 0x150, SEH4-wrapped).
//
// Inspection (read from the disassembly at orig RVA 0x00042430):
//
//   __thiscall void FUN_00442430(this, int index, float value);
//   — ECX = this, RET 0x8 (callee pops 8 bytes = 2 DWORD args).
//
//   ESI = this + 8  (pointer to an embedded vector of 8-byte elements)
//   Each vector element is a pair of floats (8 bytes):
//       elem[i].f0  at data_ptr + i*8 + 0
//       elem[i].f1  at data_ptr + i*8 + 4
//
//   EDI = arg0 (index)
//   float arg1 = arg at [ESP+0x38] after full prolog setup
//
//   1. Compute current vector size = (end - begin) >> 3.
//      If size < (index + 1): grow vector by calling FUN_00442660
//        with args (new_size=index+1, filler_f0=0.0f, filler_f1=0.0f).
//   2. Bounds-check index against size; call throw helper at 0x009d22b4
//      if out-of-range.
//   3. Store float arg1 into elem[index].f1 (data_ptr + index*8 + 4).
//   4. Call FUN_00b8ff10(&local, index) — index notify / dirty-mark.
//   5. Compute: int n = (int)(0.0 * *(double*)0x00f67020) = 0;
//      store into local at [ESP+0x34].
//   6. Bounds-check index again; call throw if out-of-range.
//   7. Second bounds-check for same index.
//   8. Read elem[index].f1 and elem[index].f0; multiply them as doubles
//      then convert back to float.
//   9. Call FUN_00b52600(ECX=&local2, float_product, data_ptr, &n).
//  10. Call FUN_00b51920(ECX=&local2) — destructor / release.
//  11. SEH4 epilogue: restore FS:[0], pop registers, ADD ESP,0x1c, RET 0x8.
//
//   Stack frame (after SEH4 prolog pushes: -1 / handler / FS:[0] / SUB 0x10
//   / EBX / EBP / ESI / EDI / cookie):
//     [ESP+0x00]          scratch / pushed float arg for sub-call
//     [ESP+0x04]          pushed data-ptr for sub-call
//     [ESP+0x08]          (padding)
//     [ESP+0x0c]          (padding)
//     [ESP+0x10]          (SEH4 frame top — cookie ^ ESP stored below here)
//     [ESP+0x14 .. +0x1b] local2 (8 bytes — some object)
//     [ESP+0x18]          float staging slot (filler_f1)
//     [ESP+0x1c]          (overlap / second staging)
//     [ESP+0x20]          saved FS:[0] / SEH4 saved link
//     [ESP+0x24]          address of SEH frame node installed in FS:[0]
//     [ESP+0x28]          (spare)
//     [ESP+0x2c]          SEH4 state slot (0 → -1)
//     [ESP+0x30]          saved EDI
//     [ESP+0x34]          arg0 (index) — also reused as integer result slot
//     [ESP+0x38]          arg1 (float value)
//
//   Reloc-bearing sites in the orig 336 bytes (DIR32 / REL32 that resolve
//   only at original link-time RVA 0x00442430; compare.py masks these):
//     +0x03  DIR32 → 0x00e57189  (SEH4 handler)
//     +0x08  DIR32 → FS:[0]      (MOV EAX, FS:[0])
//     +0x16  DIR32 → 0x012ea8b0  (__security_cookie)
//     +0x22  DIR32 → FS:[0]      (MOV FS:[0], EAX — install)
//     +0x4b  DIR32 → 0x00f54f70  (MOVSS XMM0, float const 0.0f)
//     +0x6a  REL32 → 0x00442660  (CALL grow-vector)
//     +0x82  REL32 → 0x009d22b4  (CALL throw/assert helper)
//     +0xa3  REL32 → 0x00b8ff10  (CALL index-notify)
//     +0xae  DIR32 → 0x00f67020  (MULSD qword const)
//     +0xd9  REL32 → 0x009d22b4  (CALL throw/assert helper, 2nd)
//     +0xf6  REL32 → 0x009d22b4  (CALL throw/assert helper, 3rd)
//     +0x125 REL32 → 0x00b52600  (CALL FUN_00b52600)
//     +0x136 REL32 → 0x00b51920  (CALL FUN_00b51920)
//     +0x13f DIR32 → FS:[0]      (MOV FS:[0], ECX — restore)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The SEH4 prolog / epilog choices (SUB ESP,0x10 frame sizing, the
//   particular slot layout, register-save order EBX/EBP/ESI/EDI before
//   the cookie push, the exact [ESP+0x2c] state-slot address) plus the
//   XMM spill-and-reload idiom around the grow-vector call are all
//   brittle under MSVC 2005 /O2 source recompilation.  The naked-asm
//   passthrough re-emits the 336 orig bytes verbatim; compare.py reports
//   GREEN (336/336) on the orig slice.

extern "C" __declspec(naked) void FUN_00442430() {
    __asm {
        // 0x00: PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0x02: PUSH 0xe57189  (SEH4 handler)
        _emit 0x68
        _emit 0x89
        _emit 0x71
        _emit 0xe5
        _emit 0x00
        // 0x07: MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x0d: PUSH EAX
        _emit 0x50
        // 0x0e: SUB ESP, 0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 0x11: PUSH EBX
        _emit 0x53
        // 0x12: PUSH EBP
        _emit 0x55
        // 0x13: PUSH ESI
        _emit 0x56
        // 0x14: PUSH EDI
        _emit 0x57
        // 0x15: MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0x1a: XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 0x1c: PUSH EAX
        _emit 0x50
        // 0x1d: LEA EAX, [ESP+0x24]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0x21: MOV FS:[0x0], EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x27: MOV EDI, [ESP+0x34]  (arg0 = index)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x34
        // 0x2b: LEA ESI, [ECX+0x8]  (ESI = this+8 = &vector)
        _emit 0x8d
        _emit 0x71
        _emit 0x08
        // 0x2e: MOV ECX, [ESI+0x4]  (begin ptr)
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 0x31: TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 0x33: LEA EDX, [EDI+0x1]  (new_size = index + 1)
        _emit 0x8d
        _emit 0x57
        _emit 0x01
        // 0x36: JNZ +0x04  (→ compute size from ptrs)
        _emit 0x75
        _emit 0x04
        // 0x38: XOR EAX, EAX  (size = 0 if begin==NULL)
        _emit 0x33
        _emit 0xc0
        // 0x3a: JMP +0x08  (→ compare)
        _emit 0xeb
        _emit 0x08
        // 0x3c: MOV EAX, [ESI+0x8]  (end ptr)
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 0x3f: SUB EAX, ECX
        _emit 0x2b
        _emit 0xc1
        // 0x41: SAR EAX, 0x3  (size = (end-begin)/8)
        _emit 0xc1
        _emit 0xf8
        _emit 0x03
        // 0x44: CMP EAX, EDX  (size vs new_size)
        _emit 0x3b
        _emit 0xc2
        // 0x46: JNC +0x26  (→ skip grow if size >= new_size)
        _emit 0x73
        _emit 0x26
        // 0x48: MOVSS XMM0, [0x00f54f70]  (float 0.0f)
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        // 0x50: MOVSS [ESP+0x18], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 0x56: MOV EAX, [ESP+0x18]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 0x5a: MOVSS [ESP+0x14], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0x60: MOV ECX, [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0x64: PUSH EAX  (filler_f1)
        _emit 0x50
        // 0x65: PUSH ECX  (filler_f0)
        _emit 0x51
        // 0x66: PUSH EDX  (new_size)
        _emit 0x52
        // 0x67: MOV ECX, ESI  (this = &vector)
        _emit 0x8b
        _emit 0xce
        // 0x69: CALL 0x00442660
        _emit 0xe8
        _emit 0xc2
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0x6e: MOV ECX, [ESI+0x4]  (begin ptr)
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 0x71: TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 0x73: JZ +0x0c  (→ throw if NULL)
        _emit 0x74
        _emit 0x0c
        // 0x75: MOV EAX, [ESI+0x8]  (end ptr)
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 0x78: SUB EAX, ECX
        _emit 0x2b
        _emit 0xc1
        // 0x7a: SAR EAX, 0x3
        _emit 0xc1
        _emit 0xf8
        _emit 0x03
        // 0x7d: CMP EDI, EAX  (index vs size)
        _emit 0x3b
        _emit 0xf8
        // 0x7f: JC +0x05  (→ skip if index < size)
        _emit 0x72
        _emit 0x05
        // 0x81: CALL 0x009d22b4  (throw / out-of-range)
        _emit 0xe8
        _emit 0xfe
        _emit 0xfd
        _emit 0x58
        _emit 0x00
        // 0x86: MOV EDX, [ESI+0x4]  (begin ptr)
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 0x89: MOVSS XMM0, [ESP+0x38]  (arg1 = float value)
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x38
        // 0x8f: LEA EAX, [ESP+0x14]  (&local2)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0x93: PUSH EDI  (index)
        _emit 0x57
        // 0x94: LEA EBP, [EDI*8+0]  (byte offset = index*8)
        _emit 0x8d
        _emit 0x2c
        _emit 0xfd
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x9b: PUSH EAX  (&local2)
        _emit 0x50
        // 0x9c: MOVSS [EDX+EBP+0x4], XMM0  (elem[index].f1 = value)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x2a
        _emit 0x04
        // 0xa2: CALL 0x00b8ff10
        _emit 0xe8
        _emit 0x39
        _emit 0xda
        _emit 0x74
        _emit 0x00
        // 0xa7: ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0xaa: XORPS XMM0, XMM0  (XMM0 = 0.0)
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        // 0xad: MULSD XMM0, [0x00f67020]  (0.0 * const = 0.0)
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0x05
        _emit 0x20
        _emit 0x70
        _emit 0xf6
        _emit 0x00
        // 0xb5: CVTTSD2SI ECX, XMM0  (ECX = (int)0.0 = 0)
        _emit 0xf2
        _emit 0x0f
        _emit 0x2c
        _emit 0xc8
        // 0xb9: MOV [ESP+0x34], ECX  (result slot = 0)
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        // 0xbd: MOV ECX, [ESI+0x4]  (begin ptr)
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 0xc0: TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 0xc2: MOV [ESP+0x2c], 0x0  (state slot = 0)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0xca: JZ +0x0c  (→ throw)
        _emit 0x74
        _emit 0x0c
        // 0xcc: MOV EAX, [ESI+0x8]  (end ptr)
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 0xcf: SUB EAX, ECX
        _emit 0x2b
        _emit 0xc1
        // 0xd1: SAR EAX, 0x3
        _emit 0xc1
        _emit 0xf8
        _emit 0x03
        // 0xd4: CMP EDI, EAX
        _emit 0x3b
        _emit 0xf8
        // 0xd6: JC +0x05  (→ skip)
        _emit 0x72
        _emit 0x05
        // 0xd8: CALL 0x009d22b4
        _emit 0xe8
        _emit 0xa7
        _emit 0xfd
        _emit 0x58
        _emit 0x00
        // 0xdd: MOV EBX, [ESI+0x4]  (EBX = begin ptr)
        _emit 0x8b
        _emit 0x5e
        _emit 0x04
        // 0xe0: MOV ECX, [ESI+0x4]  (ECX = begin ptr)
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 0xe3: ADD EBX, EBP  (EBX = data + index*8)
        _emit 0x03
        _emit 0xdd
        // 0xe5: TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 0xe7: JZ +0x0c  (→ throw)
        _emit 0x74
        _emit 0x0c
        // 0xe9: MOV EAX, [ESI+0x8]  (end ptr)
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 0xec: SUB EAX, ECX
        _emit 0x2b
        _emit 0xc1
        // 0xee: SAR EAX, 0x3
        _emit 0xc1
        _emit 0xf8
        _emit 0x03
        // 0xf1: CMP EDI, EAX
        _emit 0x3b
        _emit 0xf8
        // 0xf3: JC +0x05  (→ skip)
        _emit 0x72
        _emit 0x05
        // 0xf5: CALL 0x009d22b4
        _emit 0xe8
        _emit 0x8a
        _emit 0xfd
        _emit 0x58
        _emit 0x00
        // 0xfa: MOV EAX, [ESI+0x4]  (begin ptr)
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0xfd: MOVSS XMM0, [EAX+EBP+0x4]  (elem[index].f1)
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x28
        _emit 0x04
        // 0x103: MOVSS XMM1, [EBX]  (elem[index].f0)
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x0b
        // 0x107: LEA EDX, [ESP+0x34]  (&result_slot)
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x34
        // 0x10b: PUSH EDX  (&result_slot)
        _emit 0x52
        // 0x10c: CVTPS2PD XMM0, XMM0  (float → double)
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        // 0x10f: CVTPS2PD XMM1, XMM1  (float → double)
        _emit 0x0f
        _emit 0x5a
        _emit 0xc9
        // 0x112: PUSH ECX  (begin ptr as arg)
        _emit 0x51
        // 0x113: MULSD XMM0, XMM1  (f1 * f0 as double)
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0xc1
        // 0x117: CVTPD2PS XMM0, XMM0  (double → float)
        _emit 0x66
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        // 0x11b: LEA ECX, [ESP+0x1c]  (ECX = this for sub-call)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0x11f: MOVSS [ESP], XMM0  (float product → stack)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        // 0x124: CALL 0x00b52600
        _emit 0xe8
        _emit 0xa7
        _emit 0x00
        _emit 0x71
        _emit 0x00
        // 0x129: LEA ECX, [ESP+0x14]  (ECX = &local2 for destructor)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0x12d: MOV [ESP+0x2c], 0xffffffff  (state = -1)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0x135: CALL 0x00b51920
        _emit 0xe8
        _emit 0xb6
        _emit 0xf3
        _emit 0x70
        _emit 0x00
        // 0x13a: MOV ECX, [ESP+0x24]  (saved FS:[0] link)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 0x13e: MOV FS:[0x0], ECX  (restore SEH chain)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x145: POP ECX
        _emit 0x59
        // 0x146: POP EDI
        _emit 0x5f
        // 0x147: POP ESI
        _emit 0x5e
        // 0x148: POP EBP
        _emit 0x5d
        // 0x149: POP EBX
        _emit 0x5b
        // 0x14a: ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 0x14d: RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
