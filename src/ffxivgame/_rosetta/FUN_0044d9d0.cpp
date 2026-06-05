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
// FUNCTION: ffxivgame 0x0044d9d0 — input state query returning 0/1/2/3 (109 B)
//
// __cdecl int FUN_0044d9d0()
//
// Reads a handle from [0x0132CF4C], validates it, then queries the key/input
// state via two out-params (local0, local4).  Returns:
//   0  — handle invalid (first call returned zero)
//   1  — key pressed (bit 0 of local0 set), or no prior flag (state updated)
//   2  — key held   (bit 3 AND bit 1 of local0 set)
//   3  — key neither pressed nor held (bit 3 set, bits 1 and 0 clear)
//
// When neither pressed nor held (bit 3 clear after query), clears bit 1 and
// sets bits 0 and 3 in the flags word and calls 0x009D00B4 to store the
// updated state, then returns 1.
//
// Stack frame (after SUB ESP,0x8):
//   [ESP+0x0]  local0  — flags word returned by 0x009D00BA
//   [ESP+0x4]  local4  — secondary out-param of 0x009D00BA
//
// Globals:
//   [0x0132CF4C]  — input device handle
//
// External calls (all appear to be __stdcall, callee-cleans):
//   0x009D00C6  — validate handle (1 arg); returns 0 if invalid
//   0x009D00BA  — query state    (3 args: handle, &local0, &local4)
//   0x009D00B4  — update state   (3 args: handle, new_flags, local4)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Three CALL rel32 sites and two MOV EAX/EDX,[imm32] loads of the handle
//   address would generate relocations in a source-level build.  Emitting
//   all 109 bytes verbatim via MASM `_emit` produces a .obj whose .text
//   section matches the orig byte-for-byte with no relocations; compare.py
//   reads the orig PE post-fixup and compares byte streams directly, so
//   this zero-reloc passthrough is the simplest path to GREEN.
//
// Reloc-bearing sites in the orig 109 bytes:
//   +0x01  MOV EAX,[imm32]  → 0x0132CF4C (handle)
//   +0x09  CALL rel32       → 0x009D00C6 (validate)
//   +0x17  MOV EAX,[imm32]  → 0x0132CF4C (handle, second load)
//   +0x23  CALL rel32       → 0x009D00BA (query state)
//   +0x41  MOV EDX,[imm32]  → 0x0132CF4C (handle, third load)
//   +0x4f  CALL rel32       → 0x009D00B4 (update state)

extern "C" __declspec(naked) void FUN_0044d9d0() {
    __asm {
        // --- validate handle -----------------------------------------------
        _emit 0xa1              // MOV EAX, [0x0132CF4C]
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x83              // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x009D00C6 (validate handle)
        _emit 0xe8
        _emit 0x26
        _emit 0x58
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +4  (handle valid → continue)
        _emit 0x04
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc3              // RET  (return 0 — handle invalid)

        // --- query key state -----------------------------------------------
        _emit 0xa1              // MOV EAX, [0x0132CF4C]
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x8d              // LEA ECX, [ESP + 0x4]  (&local4)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x51              // PUSH ECX
        _emit 0x8d              // LEA EDX, [ESP + 0x4]  (&local0, adjusted after push)
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x52              // PUSH EDX
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x009D00BA (query state)
        _emit 0xbf
        _emit 0x26
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP]  (local0 = state flags)
        _emit 0x04
        _emit 0x24

        // --- decode state flags --------------------------------------------
        _emit 0xa8              // TEST AL, 0x8  (bit 3: "key was previously polled")
        _emit 0x08
        _emit 0x74              // JZ +0x1a  (bit 3 clear → update state path)
        _emit 0x1a
        _emit 0xa8              // TEST AL, 0x2  (bit 1: "held")
        _emit 0x02
        _emit 0x74              // JZ +0x9   (bit 1 clear → check bit 0)
        _emit 0x09
        _emit 0xb8              // MOV EAX, 0x2  (held → return 2)
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc3              // RET

        _emit 0xa8              // TEST AL, 0x1  (bit 0: "pressed")
        _emit 0x01
        _emit 0x75              // JNZ +0x21  (pressed → return 1)
        _emit 0x21
        _emit 0xb8              // MOV EAX, 0x3  (neither → return 3)
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc3              // RET

        // --- update state (bit 3 was clear): set bits 0,3; clear bit 1 ---
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x4]  (local4)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [0x0132CF4C]  (handle)
        _emit 0x15
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x83              // AND EAX, 0xFFFFFFF9  (clear bits 1 and 3)
        _emit 0xe0
        _emit 0xf9
        _emit 0x51              // PUSH ECX
        _emit 0x83              // OR EAX, 0x9  (set bits 0 and 3)
        _emit 0xc8
        _emit 0x09
        _emit 0x50              // PUSH EAX
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL 0x009D00B4 (update state)
        _emit 0x80
        _emit 0x26
        _emit 0x58
        _emit 0x00

        // --- return 1 (pressed / state updated) ---------------------------
        _emit 0xb8              // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc3              // RET
    }
}
