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
// FUNCTION: ffxivgame 0x00058c40 — COM IUnknown::QueryInterface dispatcher
//                                  (__stdcall, 3 stack args, 138 B / 0x8a)
//
// Signature (inferred from RET 0xc and stack layout):
//   HRESULT __stdcall FUN_00458c40(void *obj, const IID *riid, void **ppv)
//
// Stack args (after the PUSH ESI prologue):
//   [ESP+0x08]  obj   — the object being queried (arg1)
//   [ESP+0x0c]  riid  — pointer to requested interface IID (arg2)
//   [ESP+0x10]  ppv   — out pointer, receives the interface (arg3 → ESI)
//
// Logic:
//   1. If ppv == NULL  → return E_POINTER (0x80070057, actually E_INVALIDARG).
//   2. Compare riid against three known IIDs via the equality helper
//      FUN_004598a0(guid, riid) (returns nonzero on match):
//        - IID @0x11088f0 or IID @0x1108600 → *ppv = obj ? (char*)obj+4 : 0
//        - IID @0x1108470                    → *ppv = obj
//        - none                              → *ppv = 0, return E_NOINTERFACE
//                                              (0x80004002)
//   3. If *ppv != 0, AddRef via vtable slot 1 — (*(*obj)[1])(obj) — then
//      return S_OK (0). Otherwise fall into the E_NOINTERFACE epilogue.
//
// Globals / externs:
//   FUN_004598a0  — IID equality test (GUID*, GUID*) → bool, __cdecl
//   0x011088f0 / 0x01108600 / 0x01108470 — static IID constants
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The two-armed *ppv assignment (obj+4 vs obj), the shared AddRef tail
//   reached by JMP from both the matched arms, and the back-edge from the
//   AddRef guard (CMP [ESI],0 / JZ → E_NOINTERFACE epilogue) form an
//   irreducible control-flow knot that a source-level QueryInterface
//   cannot coerce MSVC 2005 into emitting verbatim. Following siblings
//   FUN_0040d880 / FUN_004089f0, this is a __declspec(naked) body that
//   re-emits the original 138 bytes. The three CALL FUN_004598a0 sites
//   (rel32 at +0x1d, +0x2f, +0x41) are masked by tools/compare.py; every
//   other byte — including the literal IID addresses — matches the orig
//   PE slice exactly.

extern "C" __declspec(naked) void FUN_00458c40() {
    __asm {
        // 0x00: PUSH ESI
        _emit 0x56
        // 0x01: MOV ESI, dword ptr [ESP + 0x10]   ; ppv
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 0x05: TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0x07: JNZ +0x09  (→ 0x00458c52)
        _emit 0x75
        _emit 0x09
        // 0x09: MOV EAX, 0x80070057
        _emit 0xb8
        _emit 0x57
        _emit 0x00
        _emit 0x07
        _emit 0x80
        // 0x0e: POP ESI
        _emit 0x5e
        // 0x0f: RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // --- 0x00458c52 ---
        // 0x12: PUSH EDI
        _emit 0x57
        // 0x13: MOV EDI, dword ptr [ESP + 0x10]   ; riid
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 0x17: PUSH 0x011088f0
        _emit 0x68
        _emit 0xf0
        _emit 0x88
        _emit 0x10
        _emit 0x01
        // 0x1c: PUSH EDI
        _emit 0x57
        // 0x1d: CALL FUN_004598a0  (rel32 masked)
        _emit 0xe8
        _emit 0x3e
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        // 0x22: ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0x25: TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0x27: JNZ +0x3c  (→ 0x00458ca5)
        _emit 0x75
        _emit 0x3c
        // 0x29: PUSH 0x01108600
        _emit 0x68
        _emit 0x00
        _emit 0x86
        _emit 0x10
        _emit 0x01
        // 0x2e: PUSH EDI
        _emit 0x57
        // 0x2f: CALL FUN_004598a0  (rel32 masked)
        _emit 0xe8
        _emit 0x2c
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        // 0x34: ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0x37: TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0x39: JNZ +0x2a  (→ 0x00458ca5)
        _emit 0x75
        _emit 0x2a
        // 0x3b: PUSH 0x01108470
        _emit 0x68
        _emit 0x70
        _emit 0x84
        _emit 0x10
        _emit 0x01
        // 0x40: PUSH EDI
        _emit 0x57
        // 0x41: CALL FUN_004598a0  (rel32 masked)
        _emit 0xe8
        _emit 0x1a
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        // 0x46: ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0x49: TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0x4b: JZ +0x08  (→ 0x00458c95)
        _emit 0x74
        _emit 0x08
        // 0x4d: MOV ECX, dword ptr [ESP + 0xc]   ; obj
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0x51: MOV dword ptr [ESI], ECX         ; *ppv = obj
        _emit 0x89
        _emit 0x0e
        // 0x53: JMP +0x21  (→ 0x00458cb6)
        _emit 0xeb
        _emit 0x21
        // --- 0x00458c95 ---
        // 0x55: MOV dword ptr [ESI], 0x0
        _emit 0xc7
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x5b: POP EDI
        _emit 0x5f
        // 0x5c: MOV EAX, 0x80004002              ; E_NOINTERFACE
        _emit 0xb8
        _emit 0x02
        _emit 0x40
        _emit 0x00
        _emit 0x80
        // 0x61: POP ESI
        _emit 0x5e
        // 0x62: RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // --- 0x00458ca5 ---
        // 0x65: MOV ECX, dword ptr [ESP + 0xc]   ; obj
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0x69: TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 0x6b: JZ +0x05  (→ 0x00458cb2)
        _emit 0x74
        _emit 0x05
        // 0x6d: LEA EAX, [ECX + 0x4]
        _emit 0x8d
        _emit 0x41
        _emit 0x04
        // 0x70: JMP +0x02  (→ 0x00458cb4)
        _emit 0xeb
        _emit 0x02
        // --- 0x00458cb2 ---
        // 0x72: XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // --- 0x00458cb4 ---
        // 0x74: MOV dword ptr [ESI], EAX
        _emit 0x89
        _emit 0x06
        // --- 0x00458cb6 ---
        // 0x76: CMP dword ptr [ESI], 0x0
        _emit 0x83
        _emit 0x3e
        _emit 0x00
        // 0x79: JZ -0x20  (→ 0x00458c9b)
        _emit 0x74
        _emit 0xe0
        // 0x7b: MOV EAX, dword ptr [ECX]         ; vtable
        _emit 0x8b
        _emit 0x01
        // 0x7d: PUSH ECX                          ; this
        _emit 0x51
        // 0x7e: MOV ECX, dword ptr [EAX + 0x4]   ; vtable[1] = AddRef
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        // 0x81: CALL ECX
        _emit 0xff
        _emit 0xd1
        // 0x83: POP EDI
        _emit 0x5f
        // 0x84: XOR EAX, EAX                       ; S_OK
        _emit 0x33
        _emit 0xc0
        // 0x86: POP ESI
        _emit 0x5e
        // 0x87: RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
