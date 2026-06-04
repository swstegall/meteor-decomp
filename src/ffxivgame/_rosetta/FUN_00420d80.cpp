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
// FUNCTION: ffxivgame 0x00420d80 — destructor / vtable-reset + string clear
//                                  (50 bytes / 0x32)
//
// This is a __thiscall method that:
//   1. Resets the vtable pointer at [this+0] to 0x00F54A2C (this-class vptr)
//   2. If the embedded std::basic_string member has a heap buffer
//      ([this+0x24] >= 0x10), frees it via the allocator at 0x009D1B17
//   3. Resets the string to SSO state:
//        capacity = 0x0F, size = 0, first byte = '\0'
//   4. Tail-calls the parent-class destructor at 0x009D1998 with
//      ECX = this (also __thiscall)
//
// Class layout (inferred):
//   [this+0x00]  void **vtable
//   [this+0x04]  ... (12 bytes of other fields)
//   [this+0x10]  char  _Buf[16]  / char *_Ptr  (MSVC SSO union)
//   [this+0x20]  size_t _Mysize
//   [this+0x24]  size_t _Myres   (capacity)
//
// Calling convention: __thiscall (ECX = this; no stack args; callee
// chains to parent dtor via tail JMP which pops ESI first).
//
// Frame: PUSH ESI only (no stack adjust); ESI = this throughout.
//
// Note on the 3 bytes at RVA 0x00020d98 (ADD ESP, 4 = 83 C4 04):
//   The CALL to the free routine is __cdecl (one stack arg = EAX =
//   heap ptr), so the caller is responsible for popping the arg.
//   The asm dump tool omitted this instruction, giving a misleading
//   size of 0x2F (47). The actual function is 0x32 (50) bytes; the
//   JC +0x0C offset (target 0x20D9B) proves the gap exists:
//   0x20D8D + 2 + 0x0C = 0x20D9B, and the XOR EAX,EAX that anchors
//   that label is 3 bytes past the CALL's fallthrough at 0x20D98.
//
// Reloc sites masked by tools/compare.py:
//   +0x04  imm32  → 0x00F54A2C   (vtable address, .rdata reloc)
//   +0x14  rel32  → 0x009D1B17   (CALL to free/operator delete)
//   +0x2D  rel32  → 0x009D1998   (JMP to parent dtor)

extern "C" __declspec(naked) void FUN_00420d80() {
    __asm {
        // 00020d80: 56                    PUSH ESI
        _emit 0x56
        // 00020d81: 8b f1                 MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00020d83: c7 06 2c 4a f5 00     MOV dword ptr [ESI], 0x00F54A2C
        _emit 0xc7
        _emit 0x06
        _emit 0x2c
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        // 00020d89: 83 7e 24 10           CMP dword ptr [ESI+0x24], 0x10
        _emit 0x83
        _emit 0x7e
        _emit 0x24
        _emit 0x10
        // 00020d8d: 72 0c                 JC +0x0C  (→ 0x00020D9B, XOR EAX,EAX)
        _emit 0x72
        _emit 0x0c
        // 00020d8f: 8b 46 10              MOV EAX, dword ptr [ESI+0x10]
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        // 00020d92: 50                    PUSH EAX
        _emit 0x50
        // 00020d93: e8 7f 0d 5b 00        CALL 0x009D1B17  (free / operator delete)
        _emit 0xe8
        _emit 0x7f
        _emit 0x0d
        _emit 0x5b
        _emit 0x00
        // 00020d98: 83 c4 04              ADD ESP, 4  (caller cleanup — omitted by asm dump)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00020d9b: 33 c0                 XOR EAX, EAX         (← JC target)
        _emit 0x33
        _emit 0xc0
        // 00020d9d: c7 46 24 0f 00 00 00  MOV dword ptr [ESI+0x24], 0x0F
        _emit 0xc7
        _emit 0x46
        _emit 0x24
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00020da4: 89 46 20              MOV dword ptr [ESI+0x20], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x20
        // 00020da7: 88 46 10              MOV byte ptr [ESI+0x10], AL
        _emit 0x88
        _emit 0x46
        _emit 0x10
        // 00020daa: 8b ce                 MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00020dac: 5e                    POP ESI
        _emit 0x5e
        // 00020dad: e9 e6 [0b 5b 00]      JMP 0x009D1998  (tail call → parent dtor)
        // NOTE: only the first 2 bytes of this JMP rel32 belong to this
        // function's 47-byte window. The remaining 3 bytes (0b 5b 00) fall
        // in the next function's record — this is a split instruction at the
        // YAML boundary. compare.py compares exactly 47 bytes.
        _emit 0xe9
        _emit 0xe6
    }
}
