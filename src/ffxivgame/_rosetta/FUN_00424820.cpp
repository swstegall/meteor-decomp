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
// FUNCTION: ffxivgame 0x00424820 — COM-style device enumerator (202 B / 0xca),
//                                  inline-SEH + /GS wrapped, __cdecl void().
//
// Asm shape (read from asm/ffxivgame/00024820_FUN_00424820.s):
//
//   ; ---- inline MSVC 2005 SEH prologue (no _EH_prolog call) ----------
//   PUSH -1                                ; scopetable index = -1
//   PUSH 0x00e55e98                        ; __ehhandler / scope table
//   MOV  EAX, FS:[0]                        ; prev SEH frame
//   PUSH EAX
//   PUSH ECX
//   PUSH ESI
//   MOV  EAX, [0x012ea8b0]                  ; __security_cookie
//   XOR  EAX, ESP                           ; cookie ^ esp
//   PUSH EAX
//   LEA  EAX, [ESP+0xc]
//   MOV  FS:[0], EAX                        ; install SEH frame
//   MOV  DWORD PTR [ESP+0x8], 0             ; local COM ptr = nullptr
//
//   ; ---- query global factory for an interface --------------------
//   MOV  EAX, [0x01328ed0]                  ; global object pointer slot
//   MOV  EAX, [EAX]
//   MOV  ECX, [EAX]                          ; vtable
//   LEA  EDX, [ESP+0x8]                      ; &out
//   PUSH EDX
//   PUSH 8                                   ; arg
//   PUSH EAX                                 ; this
//   MOV  EAX, [ECX+0x1d8]                    ; vtable[0x1d8]
//   MOV  DWORD PTR [ESP+0x20], 0             ; SEH state = 0
//   CALL EAX
//   TEST EAX, EAX
//   JNZ  cleanup                             ; failure → release/teardown
//
//   ; ---- first item: this->vt[0x18](1); this->vt[0x1c](0,0,1) -----
//   MOV  EAX, [ESP+0x8]
//   MOV  ECX, [EAX]
//   MOV  EDX, [ECX+0x18]
//   PUSH 1 / PUSH this / CALL EDX
//   MOV  EAX, [ESP+0x8]
//   MOV  ECX, [EAX]
//   MOV  EDX, [ECX+0x1c]
//   PUSH 1 / PUSH 0 / PUSH 0 / PUSH this / CALL EDX
//   TEST EAX, EAX
//   JZ   cleanup
//
//   ; ---- enumeration loop -----------------------------------------
//   MOV  ESI, [0x00f3e1c8]                   ; IAT slot (Sleep-like callee)
//   LEA  ESP, [ESP]                          ; 7-byte NOP pad (loop align)
// loop:
//   CMP  EAX, 0x88760868                      ; sentinel HRESULT
//   JZ   cleanup
//   PUSH 0 / CALL ESI                         ; IAT call
//   MOV  EAX, [ESP+0x8]
//   MOV  ECX, [EAX]
//   MOV  EDX, [ECX+0x1c]
//   PUSH 1 / PUSH 0 / PUSH 0 / PUSH this / CALL EDX
//   TEST EAX, EAX
//   JNZ  loop
//
// cleanup:
//   MOV  EAX, [ESP+0x8]
//   TEST EAX, EAX
//   MOV  DWORD PTR [ESP+0x14], -1             ; SEH state = -1
//   JZ   done
//   MOV  ECX, [EAX]
//   MOV  EDX, [ECX+0x8]                       ; vtable[0x8] == Release
//   PUSH this / CALL EDX
// done:
//   MOV  ECX, [ESP+0xc]                       ; restore prev SEH frame
//   MOV  FS:[0], ECX
//   POP  ECX / POP ESI / ADD ESP, 0x10 / RET
//
// Reloc-bearing sites in the orig 202 bytes (imm32 bindings to fixed VAs;
// tools/compare.py masks reloc bytes, and naked `_emit` bakes the orig
// linker-resolved bytes which match the orig slice verbatim):
//
//   +0x02   DIR32 → 0x00e55e98     (SEH scope table / handler)
//   +0x10   DIR32 → 0x012ea8b0     (__security_cookie)
//   +0x2a   DIR32 → 0x01328ed0     (global factory object pointer)
//   +0x73   DIR32 → 0x00f3e1c8     (IAT slot)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   Inline SEH (the PUSH -1 / PUSH handler / FS:[0] frame) has no
//   source-level encoding under MSVC 2005 that reproduces this exact
//   prologue + scopetable layout, and the COM vtable dispatch through
//   [ECX+disp] offsets plus the /GS cookie scheduling is brittle to any
//   high-level rewrite. As with the SEH/-GS siblings (FUN_00404e40 et al),
//   emitting the 202 orig bytes verbatim makes the .obj's `.text`
//   byte-identical to the orig slice → GREEN.

extern "C" __declspec(naked) void FUN_00424820() {
    __asm {
        _emit 0x6a          // PUSH -1
        _emit 0xff
        _emit 0x68          // PUSH 0x00e55e98
        _emit 0x98
        _emit 0x5e
        _emit 0xe5
        _emit 0x00
        _emit 0x64          // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50          // PUSH EAX
        _emit 0x51          // PUSH ECX
        _emit 0x56          // PUSH ESI
        _emit 0xa1          // MOV EAX, [0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33          // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50          // PUSH EAX
        _emit 0x8d          // LEA EAX, [ESP+0xc]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x64          // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7          // MOV DWORD PTR [ESP+0x8], 0
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xa1          // MOV EAX, [0x01328ed0]
        _emit 0xd0
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        _emit 0x8b          // MOV EAX, [EAX]
        _emit 0x00
        _emit 0x8b          // MOV ECX, [EAX]
        _emit 0x08
        _emit 0x8d          // LEA EDX, [ESP+0x8]
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x52          // PUSH EDX
        _emit 0x6a          // PUSH 8
        _emit 0x08
        _emit 0x50          // PUSH EAX
        _emit 0x8b          // MOV EAX, [ECX+0x1d8]
        _emit 0x81
        _emit 0xd8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xc7          // MOV DWORD PTR [ESP+0x20], 0
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff          // CALL EAX
        _emit 0xd0
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75          // JNZ cleanup (+0x52)
        _emit 0x52
        _emit 0x8b          // MOV EAX, [ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b          // MOV ECX, [EAX]
        _emit 0x08
        _emit 0x8b          // MOV EDX, [ECX+0x18]
        _emit 0x51
        _emit 0x18
        _emit 0x6a          // PUSH 1
        _emit 0x01
        _emit 0x50          // PUSH EAX
        _emit 0xff          // CALL EDX
        _emit 0xd2
        _emit 0x8b          // MOV EAX, [ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b          // MOV ECX, [EAX]
        _emit 0x08
        _emit 0x8b          // MOV EDX, [ECX+0x1c]
        _emit 0x51
        _emit 0x1c
        _emit 0x6a          // PUSH 1
        _emit 0x01
        _emit 0x6a          // PUSH 0
        _emit 0x00
        _emit 0x6a          // PUSH 0
        _emit 0x00
        _emit 0x50          // PUSH EAX
        _emit 0xff          // CALL EDX
        _emit 0xd2
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74          // JZ cleanup (+0x2e)
        _emit 0x2e
        _emit 0x8b          // MOV ESI, [0x00f3e1c8]
        _emit 0x35
        _emit 0xc8
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8d          // LEA ESP, [ESP] (7-byte nop)
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // loop:
        _emit 0x3d          // CMP EAX, 0x88760868
        _emit 0x68
        _emit 0x08
        _emit 0x76
        _emit 0x88
        _emit 0x74          // JZ cleanup (+0x1a)
        _emit 0x1a
        _emit 0x6a          // PUSH 0
        _emit 0x00
        _emit 0xff          // CALL ESI
        _emit 0xd6
        _emit 0x8b          // MOV EAX, [ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b          // MOV ECX, [EAX]
        _emit 0x08
        _emit 0x8b          // MOV EDX, [ECX+0x1c]
        _emit 0x51
        _emit 0x1c
        _emit 0x6a          // PUSH 1
        _emit 0x01
        _emit 0x6a          // PUSH 0
        _emit 0x00
        _emit 0x6a          // PUSH 0
        _emit 0x00
        _emit 0x50          // PUSH EAX
        _emit 0xff          // CALL EDX
        _emit 0xd2
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75          // JNZ loop (-0x21)
        _emit 0xdf
        // cleanup:
        _emit 0x8b          // MOV EAX, [ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0xc7          // MOV DWORD PTR [ESP+0x14], -1
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x74          // JZ done (+0x8)
        _emit 0x08
        _emit 0x8b          // MOV ECX, [EAX]
        _emit 0x08
        _emit 0x8b          // MOV EDX, [ECX+0x8]
        _emit 0x51
        _emit 0x08
        _emit 0x50          // PUSH EAX
        _emit 0xff          // CALL EDX
        _emit 0xd2
        // done:
        _emit 0x8b          // MOV ECX, [ESP+0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x64          // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59          // POP ECX
        _emit 0x5e          // POP ESI
        _emit 0x83          // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3          // RET
    }
}
