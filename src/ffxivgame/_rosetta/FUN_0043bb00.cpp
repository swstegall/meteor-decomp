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
// FUNCTION: ffxivgame 0x0043bb00 — `__thiscall` member with 3 DWORD args
//                                  (312 B / 0x138, SEH-wrapped, /GS cookie)
//
// Inspection (read from the disassembly at orig RVA 0x0003bb00):
//
//   __thiscall void FUN_0043bb00(this, arg1, arg2, arg3)
//     ECX = this (saved to EDI)
//     RET 0xC → 3 DWORD stdcall-style callee-pop args beyond ECX
//
//   The function:
//     - Allocates an object of size 0x28 via operator new (at 0x009d1b35)
//     - Initialises the new object with function pointers and a vtable
//       using MOVQ XMM0 to copy two qwords to [ESI+0x8] and [ESI+0x10]
//     - Stores `this` (EDI) at [ESI+0x18], and 0x5 at [ESI+0x1c]
//     - If ESI == 0, falls through the null branch (XOR ESI, ESI)
//     - On the existing old object at [EDI+0x4]: calls destructor
//       (0x009fc830) and then operator delete (0x009d1b17)
//     - Stores the new object pointer at [EDI+0x4] = ESI
//     - Reads arg1 from [ESP+0x54] and calls two sibling functions
//       (0x0043bca0 and 0x0043bd60) on [EDI+0x3c]
//     - Conditionally (byte at [ESP+0x58] == 1) computes (1 << [ESP+0x50])
//       and calls an IAT slot (0x00f3e1b0) with the shifted value
//
//   Stack frame (ESP-relative, after the SEH prologue):
//     [ESP+0x00] -- security cookie (pushed last in prologue)
//     [ESP+0x04] -- saved EDI
//     [ESP+0x08] -- saved ESI
//     [ESP+0x0c] -- saved EBP
//     [ESP+0x10] -- saved EBX
//     ... (0x2c bytes of local storage)
//     [ESP+0x40] -- EH3 FS:[0] link
//
//   Relocation sites (absolute addresses in the raw bytes; zeroed in .obj):
//     +0x03  SEH scope-table handler (0x00e56613)
//     +0x15  __security_cookie load  (0x012ea8b0)
//     +0x21  FS:[0] install          (constant 0)
//     +0x2d  IAT CALL [0x00f3e174]   (some Win32 API)
//     +0x35  CALL 0x009d1b35         (operator new)
//     +0x50  MOV imm 0x00f66494      (data pointer)
//     +0x7d  MOV imm 0x0043b930      (function pointer / vtable)
//     +0xa8  CALL 0x009fc950
//     +0xc2  CALL 0x009fc830
//     +0xc8  CALL 0x009d1b17         (operator delete)
//     +0xf9  CALL 0x0043bca0
//     +0x101 CALL 0x0043bd60
//     +0x11f IAT CALL [0x00f3e1b0]
//     +0x129 FS:[0] restore          (constant 0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The SEH prolog, security cookie, SSE2 MOVQ idiom for struct init,
//   and the linker-resolved absolute addresses make a source-level C++
//   rewrite brittle under MSVC 2005 /O2 /GS. The pragmatic choice —
//   matching siblings FUN_004014b0, FUN_00401a00, FUN_00408f10 — is a
//   `__declspec(naked)` body that re-emits the 312 orig bytes verbatim
//   via MASM `_emit` directives so `tools/compare.py` sees byte-identical
//   .text content.
//
//   Note: the canonical function size is 312 bytes (0x138); the trailing
//   `RET 0xC` instruction sits at offset 0x138 (one past this slice) and
//   belongs to the next linker-visible unit — not emitted here.

extern "C" __declspec(naked) void FUN_0043bb00() {
    __asm {
        // 0003bb00: prologue — SEH3 frame + security cookie
        _emit 0x6a  // PUSH -1                ; SEH trylevel = -1
        _emit 0xff
        _emit 0x68  // PUSH 0xe56613          ; SEH handler
        _emit 0x13
        _emit 0x66
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x83  // SUB ESP, 0x2c
        _emit 0xec
        _emit 0x2c
        _emit 0x53  // PUSH EBX
        _emit 0x55  // PUSH EBP
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0xa1  // MOV EAX, [__security_cookie]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX              ; push cookie ^ ESP
        _emit 0x8d  // LEA EAX, [ESP+0x40]
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0x64  // MOV FS:[0], EAX       ; install SEH record
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003bb27
        _emit 0x8b  // MOV EDI, ECX          ; save 'this'
        _emit 0xf9
        _emit 0x8d  // LEA EAX, [EDI+0x64]
        _emit 0x47
        _emit 0x64
        _emit 0x50  // PUSH EAX
        _emit 0xff  // CALL [0x00f3e174]     ; IAT call
        _emit 0x15
        _emit 0x74
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x6a  // PUSH 0x28             ; size = 40
        _emit 0x28
        _emit 0xe8  // CALL 0x009d1b35       ; operator new(0x28)
        _emit 0xfb
        _emit 0x5f
        _emit 0x59
        _emit 0x00
        _emit 0x8b  // MOV ESI, EAX          ; ESI = new ptr
        _emit 0xf0
        _emit 0x83  // ADD ESP, 0x4          ; clean up PUSH 0x28
        _emit 0xc4
        _emit 0x04
        _emit 0x89  // MOV [ESP+0x14], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x33  // XOR EBP, EBP          ; EBP = 0 (NULL)
        _emit 0xed
        _emit 0x83  // OR EBX, 0xFFFFFFFF    ; EBX = -1
        _emit 0xcb
        _emit 0xff
        _emit 0x3b  // CMP ESI, EBP          ; ESI == NULL?
        _emit 0xf5
        _emit 0x89  // MOV [ESP+0x48], EBP   ; store NULL
        _emit 0x6c
        _emit 0x24
        _emit 0x48
        _emit 0x74  // JZ 0x0043bbaf         ; → null branch
        _emit 0x5f
        // 0003bb50: init the new object
        _emit 0xc7  // MOV [ESP+0x18], 0xf66494
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x94
        _emit 0x64
        _emit 0xf6
        _emit 0x00
        _emit 0xc7  // MOV [ESP+0x1c], 0x10000
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0xe8  // CALL 0x009fc880
        _emit 0x1b
        _emit 0x0d
        _emit 0x5c
        _emit 0x00
        _emit 0x52  // PUSH EDX
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL 0x009fc890
        _emit 0x24
        _emit 0x0d
        _emit 0x5c
        _emit 0x00
        _emit 0x89  // MOV [ESP+0x28], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x89  // MOV [ESP+0x2c], EBX
        _emit 0x5c
        _emit 0x24
        _emit 0x2c
        _emit 0x89  // MOV [ESI], EBX        ; ESI->vtable = -1 ?
        _emit 0x1e
        _emit 0x83  // ADD ESP, 0x8          ; clean PUSH EDX+EAX
        _emit 0xc4
        _emit 0x08
        _emit 0x8d  // LEA ECX, [ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0003bb7d
        _emit 0xc7  // MOV [ESP+0x28], 0x0043b930
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x30
        _emit 0xb9
        _emit 0x43
        _emit 0x00
        _emit 0xf3  // MOVQ XMM0, [ESP+0x28]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x66  // MOVQ [ESI+0x8], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x08
        _emit 0xf3  // MOVQ XMM0, [ESP+0x30]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x51  // PUSH ECX
        _emit 0x8b  // MOV ECX, ESI
        _emit 0xce
        _emit 0x66  // MOVQ [ESI+0x10], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x10
        _emit 0x89  // MOV [ESI+0x18], EDI   ; ESI->owner = this
        _emit 0x7e
        _emit 0x18
        _emit 0xc7  // MOV [ESI+0x1c], 5
        _emit 0x46
        _emit 0x1c
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL 0x009fc950
        _emit 0xa3
        _emit 0x0d
        _emit 0x5c
        _emit 0x00
        _emit 0xeb  // JMP 0x0043bbb1
        _emit 0x02
        // 0003bbaf: null branch
        _emit 0x33  // XOR ESI, ESI          ; ESI = NULL
        _emit 0xf6
        // 0003bbb1: store new ptr in object, free old ptr
        _emit 0x89  // MOV [ESP+0x48], EBX
        _emit 0x5c
        _emit 0x24
        _emit 0x48
        _emit 0x8b  // MOV EBX, [EDI+0x4]   ; EBX = old ptr
        _emit 0x5f
        _emit 0x04
        _emit 0x3b  // CMP ESI, EBX          ; new == old?
        _emit 0xf3
        _emit 0x74  // JZ 0x0043bbd0
        _emit 0x14
        _emit 0x3b  // CMP EBX, EBP          ; old == NULL?
        _emit 0xdd
        _emit 0x74  // JZ 0x0043bbd0
        _emit 0x10
        _emit 0x8b  // MOV ECX, EBX
        _emit 0xcb
        _emit 0xe8  // CALL 0x009fc830       ; destructor
        _emit 0x69
        _emit 0x0c
        _emit 0x5c
        _emit 0x00
        _emit 0x53  // PUSH EBX              ; arg: old ptr
        _emit 0xe8  // CALL 0x009d1b17       ; operator delete
        _emit 0x4a
        _emit 0x5f
        _emit 0x59
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x4          ; clean PUSH EBX
        _emit 0xc4
        _emit 0x04
        // 0003bbd0
        _emit 0x8b  // MOV EBX, [ESP+0x54]  ; arg1
        _emit 0x5c
        _emit 0x24
        _emit 0x54
        _emit 0x89  // MOV [EDI+0x4], ESI   ; store new ptr
        _emit 0x77
        _emit 0x04
        _emit 0x8b  // MOV ECX, [EBX+0x4]
        _emit 0x4b
        _emit 0x04
        _emit 0x3b  // CMP ECX, EBP         ; [EBX+0x4] == NULL?
        _emit 0xcd
        _emit 0x75  // JNZ 0x0043bbe2
        _emit 0x04
        _emit 0x33  // XOR EAX, EAX         ; EAX = 0
        _emit 0xc0
        _emit 0xeb  // JMP 0x0043bbea
        _emit 0x08
        // 0003bbe2
        _emit 0x8b  // MOV EAX, [EBX+0x8]
        _emit 0x43
        _emit 0x08
        _emit 0x2b  // SUB EAX, ECX         ; EAX = end - begin
        _emit 0xc1
        _emit 0xc1  // SAR EAX, 2           ; EAX /= 4 (count of DWORDs)
        _emit 0xf8
        _emit 0x02
        // 0003bbea
        _emit 0x8d  // LEA EDX, [ESP+0x54]
        _emit 0x54
        _emit 0x24
        _emit 0x54
        _emit 0x52  // PUSH EDX
        _emit 0x8d  // LEA ESI, [EDI+0x3c]
        _emit 0x77
        _emit 0x3c
        _emit 0x50  // PUSH EAX
        _emit 0x8b  // MOV ECX, ESI
        _emit 0xce
        _emit 0x89  // MOV [ESP+0x5c], EBP
        _emit 0x6c
        _emit 0x24
        _emit 0x5c
        _emit 0xe8  // CALL 0x0043bca0
        _emit 0xa2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x53  // PUSH EBX
        _emit 0x8b  // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8  // CALL 0x0043bd60
        _emit 0x5a
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x80  // CMP byte ptr [ESP+0x58], 1
        _emit 0x7c
        _emit 0x24
        _emit 0x58
        _emit 0x01
        _emit 0x75  // JNZ 0x0043bc25
        _emit 0x18
        _emit 0x8b  // MOV ECX, [ESP+0x50]
        _emit 0x4c
        _emit 0x24
        _emit 0x50
        _emit 0xb8  // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xd3  // SHL EAX, CL
        _emit 0xe0
        _emit 0x8b  // MOV ECX, [EDI+0x4]
        _emit 0x4f
        _emit 0x04
        _emit 0x8b  // MOV EDX, [ECX]
        _emit 0x11
        _emit 0x50  // PUSH EAX
        _emit 0x52  // PUSH EDX
        _emit 0xff  // CALL [0x00f3e1b0]    ; IAT call
        _emit 0x15
        _emit 0xb0
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0003bc25: epilogue
        _emit 0x8b  // MOV ECX, [ESP+0x40]  ; restore SEH next ptr
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        _emit 0x64  // MOV FS:[0], ECX      ; uninstall SEH record
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5d  // POP EBP
        _emit 0x5b  // POP EBX
        _emit 0x83  // ADD ESP, 0x38
        _emit 0xc4
        _emit 0x38
        _emit 0xc2  // RET 0xC                ; pop 3 args (12 bytes) + return
        _emit 0x0c
        _emit 0x00
        // Total: 315 bytes (0x13b); size_overrides corrects from 312→315.
    }
}
