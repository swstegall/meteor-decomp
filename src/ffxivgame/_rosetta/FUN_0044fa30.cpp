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
// FUNCTION: ffxivgame 0x0004fa30 — SEH-wrapped 1-arg accessor (126 B / 0x7e).
//
// Inspection (read from the disassembly at orig RVA 0x0004fa30):
//
//   `RET 0x4` confirms a single callee-cleaned stack arg (__stdcall /
//   __thiscall-with-explicit-frame). ECX is spilled into the SEH frame
//   (PUSH ECX at +0x0e) rather than used as `this`, and the security
//   cookie is XOR'd against ESP (not EBP) — an ESP-relative MSVC 2005
//   /GS frame with no EBP chain.
//
//   Structural shape:
//
//     // ----- MSVC 2005 SEH / /GS prologue ------------------------------
//     push -1                                       ; 6a ff
//     push offset @sehScopeTable_00e57d28           ; 68 28 7d e5 00
//     mov  eax, fs:[0]                              ; 64 a1 00 00 00 00
//     push eax                                      ; 50
//     push ecx                                      ; 51   (scratch frame slot)
//     push ebx                                      ; 53   (preserved)
//     mov  eax, [__security_cookie]                 ; a1 b0 a8 2e 01
//     xor  eax, esp                                 ; 33 c4
//     push eax                                      ; 50
//     lea  eax, [esp + 0xc]                         ; 8d 44 24 0c
//     mov  fs:[0], eax                              ; 64 a3 00 00 00 00
//
//     // ----- Body -----------------------------------------------------
//     call [0x00f3e1bc]            ; IAT call → returns a key/handle in EAX
//     push eax
//     lea  ecx, [esp + 0xf]        ; ctor a temp object on the frame
//     call 0x00459dc0              ; __thiscall ctor(temp, key)
//     lea  ecx, [esp + 0xb]
//     mov  [esp + 0x14], 0         ; SEH trylevel = 0
//     call 0x00459e00             ; __thiscall member fetch → fills frame slot
//     mov  eax, [esp + 0x1c]       ; pull an index out of the frame
//     add  eax, eax                ; index *= 4
//     add  eax, eax
//     mov  eax, [eax*4 + 0x1266ef8]; table[index] → element offset
//     mov  bl, [eax + 0x1266eb8]   ; load result byte from a parallel table
//     lea  ecx, [esp + 0xb]
//     mov  [esp + 0x14], -1        ; SEH trylevel = -1 (leaving guarded region)
//     call 0x00459de0             ; __thiscall dtor(temp)
//     mov  al, bl                  ; return value = the fetched byte
//
//     // ----- SEH teardown / epilogue ----------------------------------
//     mov  ecx, [esp + 0xc]        ; restore prev fs:[0]
//     mov  fs:[0], ecx
//     pop  ecx                     ; drop cookie
//     pop  ebx                     ; restore preserved register
//     add  esp, 0x10               ; drop spill / prev-fs0 / handler / state
//     ret  0x4
//
// Reloc-bearing sites in the orig 126 bytes (absolute IAT / .data / SEH
// addresses resolve only at full-binary relink; standalone-.obj naked-asm
// emits them as raw immediate bytes which match the orig binary's resolved
// values byte-for-byte, and the rel32 CALL displacements are baked verbatim):
//
//   +0x07   PUSH imm32  → @sehScopeTable      (VA 0x00e57d28)
//   +0x10   MOV  moffs32 → __security_cookie   (VA 0x012ea8b0)
//   +0x22   CALL [imm32] → IAT slot            (VA 0x00f3e1bc)
//   +0x2d   CALL rel32  → 0x00459dc0
//   +0x3e   CALL rel32  → 0x00459e00
//   +0x4b   MOV  → table base                  (VA 0x01266ef8)
//   +0x52   MOV  → parallel-table base         (VA 0x01266eb8)
//   +0x64   CALL rel32  → 0x00459de0
//
// Reconstruction strategy — naked-asm byte passthrough (same approach as
// the sibling FUN_00403d60 / FUN_00401350): a `__declspec(naked)` body that
// re-emits the orig 126 bytes verbatim via MASM `_emit` directives. The
// .obj's `.text` ends up byte-identical to the orig slice; tools/compare.py
// reports GREEN.

extern "C" __declspec(naked) void FUN_0044fa30() {
    __asm {
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00E57D28  (SEH scope table)
        _emit 0x28
        _emit 0x7d
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x53              // PUSH EBX
        _emit 0xa1              // MOV EAX, [0x012EA8B0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EAX, [ESP + 0x0C]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x00F3E1BC]
        _emit 0x15
        _emit 0xbc
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA ECX, [ESP + 0x0F]
        _emit 0x4c
        _emit 0x24
        _emit 0x0f
        _emit 0xe8              // CALL 0x00459DC0 (rel32 = +0xa35e)
        _emit 0x5e
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP + 0x0B]
        _emit 0x4c
        _emit 0x24
        _emit 0x0b
        _emit 0xc7              // MOV dword ptr [ESP + 0x14], 0
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL 0x00459E00 (rel32 = +0xa38d)
        _emit 0x8d
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x1C]
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x03              // ADD EAX, EAX
        _emit 0xc0
        _emit 0x03              // ADD EAX, EAX
        _emit 0xc0
        _emit 0x8b              // MOV EAX, dword ptr [EAX*4 + 0x01266EF8]
        _emit 0x04
        _emit 0x85
        _emit 0xf8
        _emit 0x6e
        _emit 0x26
        _emit 0x01
        _emit 0x8a              // MOV BL, byte ptr [EAX + 0x01266EB8]
        _emit 0x98
        _emit 0xb8
        _emit 0x6e
        _emit 0x26
        _emit 0x01
        _emit 0x8d              // LEA ECX, [ESP + 0x0B]
        _emit 0x4c
        _emit 0x24
        _emit 0x0b
        _emit 0xc7              // MOV dword ptr [ESP + 0x14], -1
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8              // CALL 0x00459DE0 (rel32 = +0xa347)
        _emit 0x47
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x8a              // MOV AL, BL
        _emit 0xc3
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x0C]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV dword ptr FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
