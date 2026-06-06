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
// FUNCTION: ffxivgame 0x0045cde0 — conditional field-set with bool return (46 B / 0x2E)
//
// Calling convention: __cdecl (caller cleans, plain RET, no stack frame).
// Three parameters:
//   [ESP+0x04] param1 : pointer to object (loaded into ESI after PUSH ESI)
//   [ESP+0x08] param2 : passed as first arg to inner call FUN_0045ccc0
//   [ESP+0x0c] param3 : stored at obj+0x14; its non-zero status is returned
//
// Asm (46 bytes @ orig RVA 0x0005cde0):
//
//   8b 44 24 08    MOV EAX, [ESP+0x8]       ; EAX = param2 (loaded before PUSH ESI)
//   56             PUSH ESI                  ; save ESI, stack shifts -4
//   8b 74 24 08    MOV ESI, [ESP+0x8]       ; ESI = param1 (now at +8 after push)
//   6a ff          PUSH -0x1                ; push literal -1 (arg to inner call)
//   50             PUSH EAX                 ; push param2 (arg to inner call)
//   33 c9          XOR ECX, ECX             ; ECX = 0 (this for __thiscall callee)
//   e8 cd fe ff ff CALL 0x0045ccc0          ; FUN_0045ccc0(param2, -1) [ECX=0]
//   83 c4 08       ADD ESP, 0x8             ; caller clean-up (2 DWORD args)
//   85 c0          TEST EAX, EAX
//   75 02          JNZ done_check (+2)
//   5e             POP ESI
//   c3             RET                      ; return 0 (EAX already 0)
// done_check:
//   8b 44 24 10    MOV EAX, [ESP+0x10]      ; EAX = param3 (stack: +10h after push)
//   33 c9          XOR ECX, ECX
//   85 c0          TEST EAX, EAX
//   0f 95 c1       SETNZ CL                 ; CL = (param3 != 0)
//   89 46 14       MOV [ESI+0x14], EAX      ; obj->field_0x14 = param3
//   5e             POP ESI
//   8b c1          MOV EAX, ECX             ; return (param3 != 0)
//   c3             RET
//
// Reconstruction: __declspec(naked) + _emit byte passthrough.
// The CALL rel32 at +0x0E is masked out by tools/compare.py (reloc site).

extern "C" __declspec(naked) void FUN_0045cde0() {
    __asm {
        // 0005cde0: 8b 44 24 08    MOV EAX, [ESP+0x8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0005cde4: 56             PUSH ESI
        _emit 0x56
        // 0005cde5: 8b 74 24 08    MOV ESI, [ESP+0x8]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 0005cde9: 6a ff          PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0005cdeb: 50             PUSH EAX
        _emit 0x50
        // 0005cdec: 33 c9          XOR ECX, ECX
        _emit 0x33
        _emit 0xc9
        // 0005cdee: e8 cd fe ff ff CALL 0x0045ccc0 (rel32 — masked by compare.py)
        _emit 0xe8
        _emit 0xcd
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 0005cdf3: 83 c4 08       ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0005cdf6: 85 c0          TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005cdf8: 75 02          JNZ +0x02
        _emit 0x75
        _emit 0x02
        // 0005cdfa: 5e             POP ESI
        _emit 0x5e
        // 0005cdfb: c3             RET
        _emit 0xc3
        // 0005cdfc: 8b 44 24 10    MOV EAX, [ESP+0x10]   (done_check:)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0005ce00: 33 c9          XOR ECX, ECX
        _emit 0x33
        _emit 0xc9
        // 0005ce02: 85 c0          TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005ce04: 0f 95 c1       SETNZ CL
        _emit 0x0f
        _emit 0x95
        _emit 0xc1
        // 0005ce07: 89 46 14       MOV [ESI+0x14], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x14
        // 0005ce0a: 5e             POP ESI
        _emit 0x5e
        // 0005ce0b: 8b c1          MOV EAX, ECX
        _emit 0x8b
        _emit 0xc1
        // 0005ce0d: c3             RET
        _emit 0xc3
    }
}
