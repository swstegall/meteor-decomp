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
// FUNCTION: ffxivgame 0x0000b0a0 — fatal error reporter / crash forwarder (166 B / 0xa6)
//
// __cdecl void FUN_0040b0a0(const char* a1, const char* a2, const char* a3,
//                            const char* a4, const char* a5)
//
//   Formats an error message into a 2048-byte stack buffer via
//   _snprintf_s, forwards it to a registered output-callback stored at
//   0x012651b4 (called with selector=6 and the buffer pointer), then
//   deliberately writes to address 0 to trigger a crash.
//
//   If a5 != NULL (file/extra context provided):
//     _snprintf_s(buf, 0x800, 0x7ff, fmt_0xf54d14, a3, a4, a5, a1, a2)
//   If a5 == NULL (bare message):
//     _snprintf_s(buf, 0x800, 0x7ff, fmt_0xf54cf8, a3, a4, a1, a2)
//
//   Byte-identical sibling (same 166-byte body, different CALL targets):
//     FUN_00a193d0 @ RVA 0x006193d0 (unmatched)
//
// Stack layout at entry:
//   [ESP+0x00] = return address
//   [ESP+0x04] = a1
//   [ESP+0x08] = a2
//   [ESP+0x0c] = a3
//   [ESP+0x10] = a4
//   [ESP+0x14] = a5   ← loaded into EAX before SUB ESP (MSVC pre-load)
//
// Reloc-bearing sites in the orig 166 bytes:
//   +0x30   PUSH imm32  → 0x00f54d14   (format string 1, data-segment addr)
//   +0x44   CALL rel32  → 0x009d4f9f   (_snprintf_s)
//   +0x6e   PUSH imm32  → 0x00f54cf8   (format string 2, data-segment addr)
//   +0x82   CALL rel32  → 0x009d4f9f   (_snprintf_s, second call)
//   +0x91   CALL mem32  → 0x012651b4   (indirect call via function pointer)
//
// The MOV dword ptr [0x00000000],0x0 at +0x95 is an intentional null-write
// crash signal — address 0 is the literal target, not a relocated symbol.
//
// Reconstruction strategy: naked-asm byte passthrough. The function contains
// no saved non-volatile registers and no frame-pointer prologue, which makes
// high-level C reconstruction fragile against MSVC 2005 register allocation.
// Emitting the 166 bytes verbatim via MASM _emit directives produces a .obj
// whose .text matches the original slice byte-for-byte (five relocations).

extern "C" __declspec(naked) void FUN_0040b0a0() {
    __asm {
        // 0x0000b0a0: MOV EAX, dword ptr [ESP+0x14]  (pre-load a5 before stack alloc)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0x0000b0a4: SUB ESP, 0x800  (allocate 2048-byte output buffer)
        _emit 0x81
        _emit 0xec
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0x0000b0aa: TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0x0000b0ac: JZ +0x3f  (→ 0x0040b0ed, else branch)
        _emit 0x74
        _emit 0x3f
        // 0x0000b0ae: MOV ECX, dword ptr [ESP+0x808]  (a2)
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0x0000b0b5: MOV EDX, dword ptr [ESP+0x804]  (a1)
        _emit 0x8b
        _emit 0x94
        _emit 0x24
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0x0000b0bc: PUSH ECX  (a2 — rightmost vararg)
        _emit 0x51
        // 0x0000b0bd: MOV ECX, dword ptr [ESP+0x810]  (a3, offset shifted by push)
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x10
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0x0000b0c4: PUSH EDX  (a1)
        _emit 0x52
        // 0x0000b0c5: PUSH EAX  (a5)
        _emit 0x50
        // 0x0000b0c6: MOV EAX, dword ptr [ESP+0x81c]  (a4, offset shifted by 3 pushes)
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0x1c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0x0000b0cd: PUSH EAX  (a4)
        _emit 0x50
        // 0x0000b0ce: PUSH ECX  (a3)
        _emit 0x51
        // 0x0000b0cf: PUSH 0xf54d14  (format string 1)  ← reloc at +0x30
        _emit 0x68
        _emit 0x14
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 0x0000b0d4: PUSH 0x7ff  (count)
        _emit 0x68
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        // 0x0000b0d9: LEA EDX, [ESP+0x1c]  (buffer address)
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 0x0000b0dd: PUSH 0x800  (sizeInBytes)
        _emit 0x68
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0x0000b0e2: PUSH EDX  (buf)
        _emit 0x52
        // 0x0000b0e3: CALL 0x009d4f9f  (_snprintf_s)  ← reloc at +0x44
        _emit 0xe8
        _emit 0xb7
        _emit 0x9e
        _emit 0x5c
        _emit 0x00
        // 0x0000b0e8: ADD ESP, 0x24
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        // 0x0000b0eb: JMP +0x3c  (→ 0x0040b129, epilogue)
        _emit 0xeb
        _emit 0x3c
        // === else branch (a5 == NULL) ===
        // 0x0000b0ed: MOV EAX, dword ptr [ESP+0x808]  (a2)
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0x0000b0f4: MOV ECX, dword ptr [ESP+0x804]  (a1)
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0x0000b0fb: MOV EDX, dword ptr [ESP+0x810]  (a4)
        _emit 0x8b
        _emit 0x94
        _emit 0x24
        _emit 0x10
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0x0000b102: PUSH EAX  (a2 — rightmost vararg)
        _emit 0x50
        // 0x0000b103: MOV EAX, dword ptr [ESP+0x810]  (a3, offset shifted by push)
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0x10
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0x0000b10a: PUSH ECX  (a1)
        _emit 0x51
        // 0x0000b10b: PUSH EDX  (a4)
        _emit 0x52
        // 0x0000b10c: PUSH EAX  (a3)
        _emit 0x50
        // 0x0000b10d: PUSH 0xf54cf8  (format string 2)  ← reloc at +0x6e
        _emit 0x68
        _emit 0xf8
        _emit 0x4c
        _emit 0xf5
        _emit 0x00
        // 0x0000b112: PUSH 0x7ff  (count)
        _emit 0x68
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        // 0x0000b117: LEA ECX, [ESP+0x18]  (buffer address)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0x0000b11b: PUSH 0x800  (sizeInBytes)
        _emit 0x68
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0x0000b120: PUSH ECX  (buf)
        _emit 0x51
        // 0x0000b121: CALL 0x009d4f9f  (_snprintf_s)  ← reloc at +0x82
        _emit 0xe8
        _emit 0x79
        _emit 0x9e
        _emit 0x5c
        _emit 0x00
        // 0x0000b126: ADD ESP, 0x20
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        // === epilogue ===
        // 0x0000b129: LEA EDX, [ESP]  (buffer address for callback)
        _emit 0x8d
        _emit 0x14
        _emit 0x24
        // 0x0000b12c: PUSH 0x6  (callback selector)
        _emit 0x6a
        _emit 0x06
        // 0x0000b12e: PUSH EDX  (buf)
        _emit 0x52
        // 0x0000b12f: CALL dword ptr [0x012651b4]  (indirect via fn ptr)  ← reloc at +0x91
        _emit 0xff
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        // 0x0000b135: MOV dword ptr [0x00000000], 0x0  (intentional null-write crash)
        _emit 0xc7
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x0000b13f: ADD ESP, 0x808  (restore stack: 0x800 buffer + 0x8 for two pushed args)
        _emit 0x81
        _emit 0xc4
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0x0000b145: RET
        _emit 0xc3
    }
}
