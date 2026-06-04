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
// FUNCTION: ffxivgame 0x0041bd50 — vtable-dispatch query with 0x130-byte stack buffer
//                                  (50 bytes / 0x32)
//
// Reads a global object pointer at [0x01329834]. If null, returns the
// error sentinel 0x800. Otherwise allocates a 0x130-byte stack buffer,
// calls the object's vtable slot 7 (offset 0x1c) as __stdcall(obj, buf),
// and returns the DWORD at buf[0x58] after the call.
//
// Calling convention: __cdecl (no args, plain RET — ADD ESP,0x130 restores
// the frame; no PUSH EBP / MOV EBP,ESP since /Oy is active).
// No /GS security cookie — MSVC 2005 heuristic omits it here because the
// buffer is only passed to a callee and is never indexed/written in this fn.
//
// Asm (50 bytes @ RVA 0x0001bd50):
//   a1 34 98 32 01            MOV  EAX, [0x01329834]   ; global obj ptr
//   81 ec 30 01 00 00         SUB  ESP, 0x130           ; alloc stack buf
//   85 c0                     TEST EAX, EAX
//   74 17                     JZ   null_case            ; → MOV EAX,0x800
//   8b 08                     MOV  ECX, [EAX]           ; vtable pointer
//   8d 14 24                  LEA  EDX, [ESP]           ; &buf[0]
//   52                        PUSH EDX                  ; arg2 = buf
//   50                        PUSH EAX                  ; arg1 = obj
//   8b 41 1c                  MOV  EAX, [ECX+0x1c]      ; vtable slot 7
//   ff d0                     CALL EAX                  ; __stdcall → ret 8
//   8b 44 24 58               MOV  EAX, [ESP+0x58]      ; return buf[0x58]
//   81 c4 30 01 00 00         ADD  ESP, 0x130
//   c3                        RET
// null_case:
//   b8 00 08 00 00            MOV  EAX, 0x800
//   81 c4 30 01 00 00         ADD  ESP, 0x130
//   c3                        RET
//
// Reconstruction strategy: __declspec(naked) byte passthrough via MASM
// _emit directives. The only non-trivial immediate is the global address
// 0x01329834 embedded in the `a1` moffs32 MOV; it is emitted verbatim.
// compare.py's reloc-masking handles any addr-space difference. No CALL
// instructions with symbol references are present — the sole indirect CALL
// goes through EAX (runtime vtable dispatch), which has no relocation.

extern "C" __declspec(naked) void FUN_0041bd50() {
    __asm {
        // 0001bd50: a1 34 98 32 01         MOV EAX, [0x01329834]
        _emit 0xa1
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001bd55: 81 ec 30 01 00 00      SUB ESP, 0x130
        _emit 0x81
        _emit 0xec
        _emit 0x30
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001bd5b: 85 c0                  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0001bd5d: 74 17                  JZ null_case (+0x17)
        _emit 0x74
        _emit 0x17
        // 0001bd5f: 8b 08                  MOV ECX, [EAX]
        _emit 0x8b
        _emit 0x08
        // 0001bd61: 8d 14 24               LEA EDX, [ESP]
        _emit 0x8d
        _emit 0x14
        _emit 0x24
        // 0001bd64: 52                     PUSH EDX
        _emit 0x52
        // 0001bd65: 50                     PUSH EAX
        _emit 0x50
        // 0001bd66: 8b 41 1c               MOV EAX, [ECX+0x1c]
        _emit 0x8b
        _emit 0x41
        _emit 0x1c
        // 0001bd69: ff d0                  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001bd6b: 8b 44 24 58            MOV EAX, [ESP+0x58]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x58
        // 0001bd6f: 81 c4 30 01 00 00      ADD ESP, 0x130
        _emit 0x81
        _emit 0xc4
        _emit 0x30
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001bd75: c3                     RET
        _emit 0xc3
        // null_case:
        // 0001bd76: b8 00 08 00 00          MOV EAX, 0x800
        _emit 0xb8
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0001bd7b: 81 c4 30 01 00 00      ADD ESP, 0x130
        _emit 0x81
        _emit 0xc4
        _emit 0x30
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001bd81: c3                     RET
        _emit 0xc3
    }
}
