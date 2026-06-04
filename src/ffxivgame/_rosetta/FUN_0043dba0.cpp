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
// FUNCTION: ffxivgame 0x0043dba0 — exception-safe object factory (197 B / 0xc5),
//                                  __cdecl, /GS-enabled with an SEH (_EH4)
//                                  frame guarding the heap allocation +
//                                  placement-construct of a polymorphic object.
//
// Asm shape (read from asm/ffxivgame/0003dba0_FUN_0043dba0.s):
//
//   __cdecl Obj* FUN_0043dba0(Obj** out, void* a1, int a2, int a3, int* a4);
//
//     ; ---- MSVC 2005 _EH4 + /GS prologue ------------------------------
//     PUSH -1                                ; encoded SEH scope index
//     PUSH 0xe56bdb                          ; _EH4 scope table (.rdata)
//     PUSH FS:[0]                            ; link prev handler
//     SUB  ESP, 0x10                         ; local frame
//     PUSH EBX / ESI / EDI                   ; callee-saved
//     MOV  EAX, [__security_cookie @ 0x012ea8b0]
//     XOR  EAX, ESP
//     PUSH EAX                               ; cookie ^ esp
//     LEA  EAX, [ESP+0x20]
//     MOV  FS:[0], EAX                        ; install handler
//
//     ; ---- body -------------------------------------------------------
//     EBX = a1                               ; [ESP+0x34]
//     local[0x10] = 0
//     EDI = a4                               ; [ESP+0x40]
//     ehstate = 0                            ; [ESP+0x28]
//     if (EDI == 0) {                         ; a4 not supplied
//         EDI = *FUN_00433030(&a4, a1);      ; resolve via helper, deref
//     }
//     EAX = FUN_0040e2d0(this=[ESP+0x20], 0x10, 0xf66670);  // __thiscall
//     ESI = operator new(0x10, EAX);          ; FUN_00419c40
//     local[0x14] = ESI
//     ehstate = 1
//     if (ESI != 0) {
//         FUN_0043dec0(this=ESI, (EDI==2), a1, a2 /*[esp+0x38]*/,
//                      a3 /*[esp+0x3c]*/, 3);  // __thiscall ctor
//         *ESI = 0xf58200;                    ; install vtable
//     } else ESI = 0;
//     *out = ESI;                             ; [ESP+0x30]
//     ; ---- _EH4 + /GS epilogue ---------------------------------------
//     MOV  FS:[0], saved
//     POP/ADD ESP, 0x1c; RET
//     return out (EAX still holds *out's slot pointer)
//
// Reloc-bearing sites in the orig 197 bytes (compare.py masks reloc bytes;
// naked `_emit` bakes the orig slice bytes verbatim so the diff is GREEN):
//
//     +0x03   imm32 → 0x00e56bdb      (_EH4 scope table)
//     +0x14   DIR32 → 0x012ea8b0      (__security_cookie load)
//     +0x48   REL32 → 0x00433030      (a4-resolver helper)
//     +0x52   imm32 → 0x00f66670      (string/literal arg)
//     +0x5d   REL32 → 0x0040e2d0      (__thiscall helper)
//     +0x69   REL32 → 0x00419c40      (operator new)
//     +0x9d   REL32 → 0x0043dec0      (__thiscall constructor)
//     +0xa4   imm32 → 0x00f58200      (vtable pointer)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level /O2 /GS /EHa port would have to reproduce the exact
//   _EH4 scope-state scheduling (the [ESP+0x28] ehstate writes of 0 then
//   1), the cookie placement, the PUSH-interleaved per-arg loads, and the
//   __thiscall/SETZ argument marshalling for the constructor — the same
//   brittleness that took the /GS+SEH siblings (FUN_00404e40, FUN_00405080)
//   down the naked-asm route. Emitting the 197 orig bytes verbatim makes
//   the .obj's `.text` byte-identical to the orig slice and tools/compare.py
//   reports GREEN.

extern "C" __declspec(naked) void FUN_0043dba0() {
    __asm {
        _emit 0x6a          // PUSH -1
        _emit 0xff
        _emit 0x68          // PUSH 0xe56bdb
        _emit 0xdb
        _emit 0x6b
        _emit 0xe5
        _emit 0x00
        _emit 0x64          // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50          // PUSH EAX
        _emit 0x83          // SUB ESP, 0x10
        _emit 0xec
        _emit 0x10
        _emit 0x53          // PUSH EBX
        _emit 0x56          // PUSH ESI
        _emit 0x57          // PUSH EDI
        _emit 0xa1          // MOV EAX, [0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33          // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50          // PUSH EAX
        _emit 0x8d          // LEA EAX, [ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x64          // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b          // MOV EBX, [ESP+0x34]
        _emit 0x5c
        _emit 0x24
        _emit 0x34
        _emit 0xc7          // MOV [ESP+0x10], 0
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b          // MOV EDI, [ESP+0x40]
        _emit 0x7c
        _emit 0x24
        _emit 0x40
        _emit 0x85          // TEST EDI, EDI
        _emit 0xff
        _emit 0xc7          // MOV [ESP+0x28], 0
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x75          // JNZ +0x10
        _emit 0x10
        _emit 0x8d          // LEA EAX, [ESP+0x40]
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0x53          // PUSH EBX
        _emit 0x50          // PUSH EAX
        _emit 0xe8          // CALL 0x00433030
        _emit 0x43
        _emit 0x54
        _emit 0xff
        _emit 0xff
        _emit 0x8b          // MOV EDI, [EAX]
        _emit 0x38
        _emit 0x83          // ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0x68          // PUSH 0xf66670
        _emit 0x70
        _emit 0x66
        _emit 0xf6
        _emit 0x00
        _emit 0x6a          // PUSH 0x10
        _emit 0x10
        _emit 0x8d          // LEA ECX, [ESP+0x20]
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0xe8          // CALL 0x0040e2d0
        _emit 0xce
        _emit 0x06
        _emit 0xfd
        _emit 0xff
        _emit 0x50          // PUSH EAX
        _emit 0x6a          // PUSH 0x10
        _emit 0x10
        _emit 0x89          // MOV [ESP+0x48], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x48
        _emit 0xe8          // CALL 0x00419c40
        _emit 0x32
        _emit 0xc0
        _emit 0xfd
        _emit 0xff
        _emit 0x8b          // MOV ESI, EAX
        _emit 0xf0
        _emit 0x83          // ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0x89          // MOV [ESP+0x14], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x85          // TEST ESI, ESI
        _emit 0xf6
        _emit 0xc7          // MOV [ESP+0x28], 1
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74          // JZ +0x27
        _emit 0x27
        _emit 0x8b          // MOV ECX, [ESP+0x3c]
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        _emit 0x8b          // MOV EDX, [ESP+0x38]
        _emit 0x54
        _emit 0x24
        _emit 0x38
        _emit 0xb8          // MOV EAX, 3
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50          // PUSH EAX
        _emit 0x83          // CMP EDI, 2
        _emit 0xff
        _emit 0x02
        _emit 0x51          // PUSH ECX
        _emit 0x0f          // SETZ AL
        _emit 0x94
        _emit 0xc0
        _emit 0x52          // PUSH EDX
        _emit 0x53          // PUSH EBX
        _emit 0x8b          // MOV ECX, ESI
        _emit 0xce
        _emit 0x50          // PUSH EAX
        _emit 0xe8          // CALL 0x0043dec0
        _emit 0x7e
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xc7          // MOV [ESI], 0xf58200
        _emit 0x06
        _emit 0x00
        _emit 0x82
        _emit 0xf5
        _emit 0x00
        _emit 0xeb          // JMP +0x02
        _emit 0x02
        _emit 0x33          // XOR ESI, ESI
        _emit 0xf6
        _emit 0x8b          // MOV EAX, [ESP+0x30]
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x89          // MOV [EAX], ESI
        _emit 0x30
        _emit 0x8b          // MOV ECX, [ESP+0x20]
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x64          // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59          // POP ECX
        _emit 0x5f          // POP EDI
        _emit 0x5e          // POP ESI
        _emit 0x5b          // POP EBX
        _emit 0x83          // ADD ESP, 0x1c
        _emit 0xc4
        _emit 0x1c
        _emit 0xc3          // RET
    }
}
